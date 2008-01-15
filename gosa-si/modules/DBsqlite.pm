package GOSA::DBsqlite;


use strict;
use warnings;
use DBI;
use Data::Dumper;

my $col_names = {};
my $checking=0;

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

sub lock_exists {
    while($checking) {
        sleep 1;
    }
    $checking=1;
    my $self=shift;
    my $funcname=shift;
    my $lock = $self->{db_lock};
    my $result=(-f $lock);
    if($result) {
        print STDERR "(".((defined $funcname)?$funcname:"").") Lock (PID ".$$.") $lock gefunden\n";
        sleep 2;
    }
    $checking=0;
    return $result;
}

sub create_lock {
    my $self=shift;
    my $funcname=shift;
    print STDERR "(".((defined $funcname)?$funcname:"").") Erzeuge Lock (PID ".$$.") ".($self->{db_lock})."\n";
    open($self->{db_lock_handle},'>',$self->{db_lock});
}

sub remove_lock {
    my $self=shift;
    my $funcname=shift;
    print STDERR "(".((defined $funcname)?$funcname:"").") Entferne Lock (PID ".$$.") ".$self->{db_lock}."\n";
    close($self->{db_lock_handle});
    unlink($self->{db_lock});
}

sub create_table {
    my $self = shift;
    my $table_name = shift;
    my $col_names_ref = shift;
    $col_names->{ $table_name } = $col_names_ref;
    my $col_names_string = join(', ', @{$col_names_ref});
    while(&lock_exists($self,'create_table')) {
        print STDERR "Lock in create_table\n";
    }
    &create_lock($self,'create_table');
    my $sql_statement = "CREATE TABLE IF NOT EXISTS $table_name ( $col_names_string )"; 
    $self->{dbh}->do($sql_statement);
    &remove_lock($self,'create_table');
    return 0;
}
 


sub add_dbentry {

    my $self = shift;
    my $arg = shift;

    while(&lock_exists($self,'add_dbentry')) {
        print STDERR "Lock in add_dbentry\n";
    }
    &create_lock($self,'add_dbentry');
    # if dbh not specified, return errorflag 1
    my $table = $arg->{table};
    if (not defined $table) { 
        &remove_lock($self,'add_dbentry');
        return 1; 
    }

    # specify primary key in table
    if (not exists $arg->{primkey}) {
        &remove_lock($self,'add_dbentry');
        return 2;
    }
    my $primkey = $arg->{primkey};

    # if primkey is id, fetch max id from table and give new job id=  max(id)+1
    if ($primkey eq 'id') {
        my $id;
        my $sql_statement = "SELECT MAX(id) FROM $table";
        my $max_id = @{ @{ $self->{dbh}->selectall_arrayref($sql_statement) }[0] }[0];
        if( defined $max_id) {
            $id = $max_id + 1; 
        } else {
            $id = 1;
        }
        $arg->{id} = $id;
    }

    # check wether value to primary key is specified
    if ( not exists $arg->{ $primkey } ) {
        &remove_lock($self,'add_dbentry');
        return 3;
    }
     
    # if timestamp is not provided, add timestamp   
    if( not exists $arg->{timestamp} ) {
        $arg->{timestamp} = &get_time;
    }

    # check wether primkey is unique in table, otherwise return errorflag 3
    my $sql_statement = "SELECT * FROM $table WHERE $primkey='$arg->{$primkey}'";
    my $res = @{ $self->{dbh}->selectall_arrayref($sql_statement) };
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

        my $sql_statement = "BEGIN TRANSACTION; INSERT INTO $table VALUES ('".join("', '", @add_list)."'); COMMIT;";
        my $db_res = $self->{dbh}->do($sql_statement);
        if( $db_res != 1 ) {
            &remove_lock($self,'add_dbentry');
            return 1;
        } else { 
            &remove_lock($self,'add_dbentry');
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
            &remove_lock($self,'add_dbentry');
            return 1;
        } else { 
            &remove_lock($self,'add_dbentry');
            return 0;
        }

    }
    &remove_lock($self,'add_dbentry');
}


# error-flags
# 1 no table ($table) defined
# 2 no restriction parameter ($restric_pram) defined
# 3 no restriction value ($restric_val) defined
# 4 column name not known in table
# 5 no column names to change specified
sub update_dbentry {
    my $self = shift;
    my $arg = shift;

    while(&lock_exists($self,'update_dbentry')) {
        print STDERR "Lock in update_dbentry\n";
    }
    &create_lock($self,'update_dbentry');
    # check completeness of function parameter
    # extract table statement from arg hash
    my $table = $arg->{table};
    if (not defined $table) {
        &remove_lock($self,'update_dbentry');
        return 1;
    } else {
        delete $arg->{table};
    }
    
    # extract where parameter from arg hash
    my $where_statement = "";
    if( exists $arg->{where} ) {
        my $where_hash = @{ $arg->{where} }[0];
        if( 0 < keys %{ $where_hash } ) {
            my @where_list;    
            while( my ($rest_pram, $rest_val) = each %{ $where_hash } ) {
                my $statement;
                if( $rest_pram eq 'timestamp' ) {
                    $statement = "$rest_pram<'@{ $rest_val }[0]'";
                } else {
                    $statement = "$rest_pram='@{ $rest_val }[0]'";
                }
                push( @where_list, $statement );
            }
            $where_statement .= "WHERE ".join('AND ', @where_list);
        }
    }

    # extract update parameter from arg hash
    my $update_hash = @{ $arg->{update} }[0];
    my $update_statement = "";
    if( 0 < keys %{ $update_hash } ) {
        my @update_list;    
        while( my ($rest_pram, $rest_val) = each %{ $update_hash } ) {
            my $statement = "$rest_pram='@{ $rest_val }[0]'";
            push( @update_list, $statement );
        }
        $update_statement .= join(', ', @update_list);
    }

    my $sql_statement = "BEGIN TRANSACTION; UPDATE $table SET $update_statement $where_statement; COMMIT;";
    my $db_answer = $self->{dbh}->do($sql_statement);
    &remove_lock($self,'update_dbentry');
    return $db_answer;
}  


sub del_dbentry {
    my $self = shift;
    my $arg = shift;

    while(&lock_exists($self,'del_dbentry')) {
        print STDERR "Lock in del_dbentry\n";
    }
    &create_lock($self,'del_dbentry');
    # check completeness of function parameter
    # extract table statement from arg hash
    my $table = $arg->{table};
    if (not defined $table) {
        &remove_lock($self,'del_dbentry');
        return 1;
    } else {
        delete $arg->{table};
    }

    # collect select statements
    my @del_list;
    while (my ($pram, $val) = each %{$arg}) {
        if ( $pram eq 'timestamp' ) {
            push(@del_list, "$pram < '$val'");
        } else {
            push(@del_list, "$pram = '$val'");
        }
    }

    my $where_statement;
    if( not @del_list ) {
        $where_statement = "";
    } else {
        $where_statement = "WHERE ".join(' AND ', @del_list);
    }

    my $sql_statement = "BEGIN TRANSACTION; DELETE FROM $table $where_statement; COMMIT;";
    my $db_res = $self->{dbh}->do($sql_statement);
 
    &remove_lock($self,'del_dbentry');
    return $db_res;
}


sub get_table_columns {
    my $self = shift;
    my $table = shift;

    my @column_names = @{$col_names->{$table}};
    return \@column_names;

}

sub select_dbentry {
    my $self = shift;
    my $arg = shift;

    while(&lock_exists($self,'select_dbentry')) {
        print STDERR "Lock in select_dbentry\n";
    }
    &create_lock($self,'select_dbentry');
    
    # check completeness of function parameter
    # extract table statement from arg hash
    my $table = $arg->{table};
    if (not defined $table) {
        &remove_lock($self,'select_dbentry');
        return 1;
    } else {
        delete $arg->{table};
    }

    # collect select statements
    my @select_list;
    my $sql_statement;
    while (my ($pram, $val) = each %{$arg}) {
        if ( $pram eq 'timestamp' ) {
            push(@select_list, "$pram < '$val'");
        } else {
            push(@select_list, "$pram = '$val'");
        }
    }

    if (@select_list == 0) {
        $sql_statement = "SELECT ROWID, * FROM '$table'";
    } else {
        $sql_statement = "SELECT ROWID, * FROM '$table' WHERE ".join(' AND ', @select_list);
    }

    # query db
    my $query_answer = $self->{dbh}->selectall_arrayref($sql_statement);

    # fetch column list of db and create a hash with column_name->column_value of the select query
    my $column_list = &get_table_columns($self, $table);    
    my $list_len = @{ $column_list } ;
    my $answer = {};
    my $hit_counter = 0;

    
    foreach my $hit ( @{ $query_answer }) {
        $hit_counter++;
        $answer->{ $hit_counter }->{ 'ROWID' } = shift @{ $hit };
        for ( my $i = 0; $i < $list_len; $i++) {
            $answer->{ $hit_counter }->{ @{ $column_list }[$i] } = @{ $hit }[$i];
        }
    }

    &remove_lock($self,'select_dbentry');
    return $answer;  
}


sub show_table {
    my $self = shift;
    my $table_name = shift;
    while(&lock_exists($self,'show_table')) {
        print STDERR "Lock in show_table\n";
    }
    &create_lock($self,'show_table');
    my @res = @{$self->{dbh}->selectall_arrayref( "SELECT * FROM $table_name")};
    my @answer;
    foreach my $hit (@res) {
        push(@answer, "hit: ".join(', ', @{$hit}));
    }
    &remove_lock($self,'show_table');
    return join("\n", @answer);
}


sub exec_statement {
    my $self = shift;
    my $sql_statement = shift;
    while(&lock_exists($self,'exec_statement')) {
        print STDERR "Lock in exec_statement\n";
    }
    &create_lock($self,'exec_statement');
    my @res = @{$self->{dbh}->selectall_arrayref($sql_statement)};
    &remove_locK;
    return \@res;
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
