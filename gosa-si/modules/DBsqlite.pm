package GOSA::DBsqlite;


use strict;
use warnings;
use DBI;
use Data::Dumper;
use threads;
use Time::HiRes qw(usleep);

my $col_names = {};

sub new {
    my $class = shift;
    my $db_name = shift;

    my $lock='/tmp/gosa_si_lock';
    my $_lock = $db_name;
    $_lock =~ tr/\//_/;
    $lock.=$_lock;
    my $self = {dbh=>undef,db_name=>undef,db_lock=>undef,db_lock_handle=>undef};
    my $dbh = DBI->connect("dbi:SQLite:dbname=$db_name");
    $self->{dbh} = $dbh;
    $self->{db_name} = $db_name;
    $self->{db_lock} = $lock;
    bless($self,$class);

    return($self);
}

sub lock_exists : locked {
    my $self=shift;
    my $funcname=shift;
    my $lock = $self->{db_lock};
    my $result=(-f $lock);
    if($result) {
        #print STDERR "(".((defined $funcname)?$funcname:"").") Lock (PID ".$$.") $lock gefunden\n";
        usleep 100;
    }
    return $result;
}

sub create_lock : locked {
    my $self=shift;
    my $funcname=shift;
    #print STDERR "(".((defined $funcname)?$funcname:"").") Erzeuge Lock (PID ".$$.") ".($self->{db_lock})."\n";

    my $lock = $self->{db_lock};
    while( -f $lock ) {
        #print STDERR "(".((defined $funcname)?$funcname:"").") Lock (PID ".$$.") $lock gefunden\n";
        sleep 1;
    }

    open($self->{db_lock_handle},'>',$self->{db_lock});
}

sub remove_lock : locked {
    my $self=shift;
    my $funcname=shift;
    #print STDERR "(".((defined $funcname)?$funcname:"").") Entferne Lock (PID ".$$.") ".$self->{db_lock}."\n";
    close($self->{db_lock_handle});
    unlink($self->{db_lock});
}

sub create_table {
    my $self = shift;
    my $table_name = shift;
    my $col_names_ref = shift;
    $col_names->{ $table_name } = $col_names_ref;
    my $col_names_string = join(', ', @{$col_names_ref});
    my $sql_statement = "CREATE TABLE IF NOT EXISTS $table_name ( $col_names_string )"; 
    &create_lock($self,'create_table');
    $self->{dbh}->do($sql_statement);
    &remove_lock($self,'create_table');
    return 0;
}
 


sub add_dbentry {
    my $self = shift;
    my $arg = shift;

    # if dbh not specified, return errorflag 1
    my $table = $arg->{table};
    if( not defined $table ) { 
        return 1 ; 
    }

    # specify primary key in table
    if (not exists $arg->{primkey}) {
        return 2;
    }
    my $primkey = $arg->{primkey};

    # if primkey is id, fetch max id from table and give new job id=  max(id)+1
    if ($primkey eq 'id') {
        my $id;
        my $sql_statement = "SELECT MAX(CAST(id AS INTEGER)) FROM $table";
        &create_lock($self,'add_dbentry');
        my $max_id = @{ @{ $self->{dbh}->selectall_arrayref($sql_statement) }[0] }[0];
        &remove_lock($self,'add_dbentry');
        if( defined $max_id) {
            $id = $max_id + 1; 
        } else {
            $id = 1;
        }
        $arg->{id} = $id;
    }

    # check wether value to primary key is specified
    if ( not exists $arg->{ $primkey } ) {
        return 3;
    }
     
    # if timestamp is not provided, add timestamp   
    if( not exists $arg->{timestamp} ) {
        $arg->{timestamp} = &get_time;
    }

    # check wether primkey is unique in table, otherwise return errorflag
    my $sql_statement = "SELECT * FROM $table WHERE $primkey='$arg->{$primkey}'";
    &create_lock($self,'add_dbentry');
    my $res = @{ $self->{dbh}->selectall_arrayref($sql_statement) };
    &remove_lock($self,'add_dbentry');
    if ($res == 0) {
        # fetch column names of table
        my $col_names = &get_table_columns("",$table);

        # assign values to column name variables
        my @add_list;
        foreach my $col_name (@{$col_names}) {
        # use function parameter for column values
            if (exists $arg->{$col_name}) {
                push(@add_list, $arg->{$col_name});
            }
        }    

        my $sql_statement = "INSERT INTO $table VALUES ('".join("', '", @add_list)."')";
        print STDERR $sql_statement;
        &create_lock($self,'add_dbentry');
        my $db_res = $self->{dbh}->do($sql_statement);
        &remove_lock($self,'add_dbentry');
        if( $db_res != 1 ) {
            return 4;
        } else { 
            return 0;
        }

    } else  {
        my $update_hash = { table=>$table };
        $update_hash->{where} = [ { $primkey=>[ $arg->{$primkey} ] } ];
        $update_hash->{update} = [ {} ];
        while( my ($pram, $val) = each %{$arg} ) {
            if( $pram eq 'table' ) { next; }
            if( $pram eq 'primkey' ) { next; }
            $update_hash->{update}[0]->{$pram} = [$val];
        }
        my $db_res = &update_dbentry( $self, $update_hash );
        if( $db_res != 1 ) {
            return 5;
        } else { 
            return 0;
        }

    }
}

sub update_dbentry {
    my ($self, $sql)= @_;
    my $db_answer= &exec_statement($self, $sql); 
    return $db_answer;
}


sub del_dbentry {
    my ($self, $sql)= @_;;
    my $db_res= &exec_statement($self, $sql);
    return $db_res;
}


sub get_table_columns {
    my $self = shift;
    my $table = shift;
    my @column_names;
    
    if(exists $col_names->{$table}) {
        @column_names = @{$col_names->{$table}};
    } else {
        &create_lock($self,'get_table_columns');
        my @res = @{$self->{dbh}->selectall_arrayref("pragma table_info('$table')")};
        &remove_lock($self,'get_table_columns');
        foreach my $column (@res) {
            push(@column_names, @$column[1]);
        }
    }
    return \@column_names;

}


sub select_dbentry {
    my ($self, $sql)= @_;
    my $error= 0;
    my $answer= {};
    
    my $db_answer= &exec_statement($self, $sql); 

    # fetch column list of db and create a hash with column_name->column_value of the select query
    $sql =~ /FROM ([\S]*?) /g;
    my $table = $1;
    my $column_list = &get_table_columns($self, $table);    
    my $list_len = @{ $column_list } ;
    my $hit_counter = 0;
    foreach my $hit ( @{ $db_answer }) {
        $hit_counter++;
        for ( my $i = 0; $i < $list_len; $i++) {
            $answer->{ $hit_counter }->{ @{ $column_list }[$i] } = @{ $hit }[$i];
        }
    }
    
    return $answer;  
}


sub show_table {
    my $self = shift;
    my $table_name = shift;
    &create_lock($self,'show_table');
    my @res = @{$self->{dbh}->selectall_arrayref( "SELECT * FROM $table_name")};
    &remove_lock($self,'show_table');
    my @answer;
    foreach my $hit (@res) {
        push(@answer, "hit: ".join(', ', @{$hit}));
    }
    return join("\n", @answer);
}


sub exec_statement {
    my $self = shift;
    my $sql_statement = shift;

    &create_lock($self,'exec_statement');
    my @db_answer = @{$self->{dbh}->selectall_arrayref($sql_statement)};
    &remove_lock($self, 'exec_statement');

    return \@db_answer;
}


sub get_time {
    my ($seconds, $minutes, $hours, $monthday, $month,
            $year, $weekday, $yearday, $sommertime) = localtime(time);
    $hours = $hours < 10 ? $hours = "0".$hours : $hours;
    $minutes = $minutes < 10 ? $minutes = "0".$minutes : $minutes;
    $seconds = $seconds < 10 ? $seconds = "0".$seconds : $seconds;
    $month+=1;
    $month = $month < 10 ? $month = "0".$month : $month;
    $monthday = $monthday < 10 ? $monthday = "0".$monthday : $monthday;
    $year+=1900;
    return "$year$month$monthday$hours$minutes$seconds";
}


1;
