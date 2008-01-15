package GOSA::DBsqlite;


use strict;
use warnings;
use DBI;
use Data::Dumper;



sub new {
    my $object = shift;
    my $db_name = shift;

    my $obj_ref = {};
    bless($obj_ref,$object);
    my $dbh = DBI->connect("dbi:SQLite:dbname=$db_name");
    $obj_ref->{dbh} = $dbh;

    return($obj_ref);
}


sub create_table {
    my $object = shift;
    my $table_name = shift;
    my $col_names_ref = shift;
#    unshift(@{$col_names_ref}, "id INTEGER PRIMARY KEY AUTOINCREMENT");
    my $col_names_string = join(', ', @{$col_names_ref});
    my $sql_statement = "CREATE TABLE IF NOT EXISTS $table_name ( $col_names_string )"; 
    $object->{dbh}->do($sql_statement);
    return 0;
}
 


sub add_dbentry {

    my $obj = shift;
    my $arg = shift;

    # if dbh not specified, return errorflag 1
    my $table = $arg->{table};
    if (not defined $table) { 
        return 1; 
    }

    # specify primary key in table
    if (not exists $arg->{primkey}) {
        return 2;
    }
    my $primkey = $arg->{primkey};

    # if primkey is id, fetch max id from table and give new job id=  max(id)+1
    if ($primkey eq 'id') {
        my $id;
        my $sql_statement = "SELECT MAX(id) FROM $table";
        my $max_id = @{ @{ $obj->{dbh}->selectall_arrayref($sql_statement) }[0] }[0];
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

    # check wether primkey is unique in table, otherwise return errorflag 3
    my $sql_statement = "SELECT * FROM $table WHERE $primkey='$arg->{$primkey}'";
    my $res = @{ $obj->{dbh}->selectall_arrayref($sql_statement) };
    if ($res == 0) {
        # fetch column names of table
        my $col_names = $obj->get_table_columns($table);

        # assign values to column name variables
        my @add_list;
        foreach my $col_name (@{$col_names}) {
        # use function parameter for column values
            if (exists $arg->{$col_name}) {
                push(@add_list, $arg->{$col_name});
            }
        }    

        my $sql_statement = "BEGIN TRANSACTION; INSERT INTO $table VALUES ('".join("', '", @add_list)."'); COMMIT;";
        my $db_res = $obj->{dbh}->do($sql_statement);
        if( $db_res != 1 ) {
            return 1;
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
        my $db_res = &update_dbentry( $obj, $update_hash );
        if( $db_res != 1 ) {
            return 1;
        } else { 
            return 0;
        }

    }
}


# error-flags
# 1 no table ($table) defined
# 2 no restriction parameter ($restric_pram) defined
# 3 no restriction value ($restric_val) defined
# 4 column name not known in table
# 5 no column names to change specified
sub update_dbentry {
    my $obj = shift;
    my $arg = shift;

    # check completeness of function parameter
    # extract table statement from arg hash
    my $table = $arg->{table};
    if (not defined $table) {
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
    my $db_answer = $obj->{dbh}->do($sql_statement);
    return $db_answer;
}  


sub del_dbentry {
    my $obj = shift;
    my $arg = shift;

    # check completeness of function parameter
    # extract table statement from arg hash
    my $table = $arg->{table};
    if (not defined $table) {
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
    my $db_res = $obj->{dbh}->do($sql_statement);
 
    return $db_res;
}


sub get_table_columns {
    my $obj = shift;
    my $table = shift;

    my @columns;
    my @res = @{$obj->{dbh}->selectall_arrayref("pragma table_info('$table')")};
    foreach my $column (@res) {
        push(@columns, @$column[1]);
    }

    return \@columns; 
}

sub select_dbentry {
    my $obj = shift;
    my $arg = shift;

    # check completeness of function parameter
    # extract table statement from arg hash
    my $table = $arg->{table};
    if (not defined $table) {
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
    my $query_answer = $obj->{dbh}->selectall_arrayref($sql_statement);

    # fetch column list of db and create a hash with column_name->column_value of the select query
    my $column_list = &get_table_columns($obj, $table);    
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
    return $answer;  
}


sub show_table {
    my $obj = shift;
    my $table_name = shift;
    my @res = @{$obj->{dbh}->selectall_arrayref( "SELECT * FROM $table_name")};
    my @answer;
    foreach my $hit (@res) {
        push(@answer, "hit: ".join(', ', @{$hit}));
    }
    return join("\n", @answer);
}


sub exec_statement {
    my $obj = shift;
    my $sql_statement = shift;
    my @res = @{$obj->{dbh}->selectall_arrayref($sql_statement)};
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
