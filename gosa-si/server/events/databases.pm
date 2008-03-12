package databases;
use Exporter;
@ISA = qw(Exporter);
my @events = (
    "get_events", 
    "query_jobdb",
    "count_jobdb",
    "delete_jobdb_entry",
    "clear_jobdb",
    "update_status_jobdb_entry",
    "query_packages_list",
    "count_packages_list",
    "query_fai_server",
    "count_fai_server",
    "query_fai_release",
    "count_fai_release",
    );
@EXPORT = @events;

use strict;
use warnings;
use GOSA::GosaSupportDaemon;
use Data::Dumper;
use utf8;

BEGIN {}

END {}

### Start ######################################################################

#&main::read_configfile($main::cfg_file, %cfg_defaults);

sub get_events {
    return \@events;
}

sub query_fai_releases{ return &query_db( @_ ); }
sub query_fai_server{ return &query_db( @_ ) ; }
sub query_packages_list { return &query_db( @_ ) ; }
sub query_jobdb { return &query_db( @_ ) ; }
sub query_db {
    my ($msg, $msg_hash, $session_id) = @_ ;
    my $header = @{$msg_hash->{'header'}}[0];
    my $target = @{$msg_hash->{'target'}}[0];
    my $source = @{$msg_hash->{'source'}}[0];
    my $table;
    my $db;
    if( $header =~ /query_jobdb/ ) {
        $table = $main::job_queue_tn;
        $db = $main::job_db;
    } elsif( $header =~ /query_packages_list/ ) {
        $table = $main::packages_list_tn;
        $db = $main::packages_list_db;
    } elsif( $header =~ /query_fai_server/ ) {
        $table = $main::fai_server_tn;
        $db = $main::fai_server_db;
    } elsif( $header =~ /count_fai_release/ ) {
        $table = $main::fai_release_tn;
        $db = $main::fai_server_db;
    }

   
    # prepare sql statement and execute query
    my $select= &get_select_statement($msg, $msg_hash);
    my $where= &get_where_statement($msg, $msg_hash);
    my $limit= &get_limit_statement($msg, $msg_hash);
    my $orderby= &get_orderby_statement($msg, $msg_hash);
    my $sql_statement= "SELECT $select FROM $table $where $orderby $limit";
    my $res_hash = $db->select_dbentry($sql_statement);
    
    my $out_xml = &db_res2si_msg($res_hash, $header, $target, $source);
    $out_xml =~ s/<\/xml>/<session_id>$session_id<\/session_id><\/xml>/;
    my @out_msg_l = ( $out_xml );
    return @out_msg_l;
}
    
sub count_fai_release{ return &count_db( @_ ); }    
sub count_fai_server{ return &count_db( @_ ); }
sub count_packages_list{ return &count_db( @_ ); }
sub count_jobdb{ return &count_db( @_ ); }
sub count_db {
    my ($msg, $msg_hash, $session_id) = @_;
    my $header = @{$msg_hash->{'header'}}[0];
    my $target = @{$msg_hash->{'target'}}[0];
    my $source = @{$msg_hash->{'source'}}[0];
    my $table;
    my $db;

    if( $header =~ /count_jobdb/ ) {
        $table = $main::job_queue_tn;
        $db = $main::job_db;
    } elsif( $header =~ /count_packages_list/ ) {
        $table = $main::packages_list_tn;
        $db = $main::packages_list_db;
    } elsif( $header =~ /count_fai_server/ ) {
        $table = $main::fai_server_tn;
        $db = $main::fai_server_db;
    } elsif( $header =~ /count_fai_release/ ) {
        $table = $main::fai_release_tn;
        $db = $main::fai_server_db;
    }


    # prepare sql statement and execute query
    my $res_hash = $db->select_dbentry("SELECT * FROM $table");

    my $count = keys(%{$res_hash});
    my $out_xml= "<xml><header>answer</header><source>$target</source><target>$source</target><count>$count</count><session_id>$session_id</session_id></xml>";
    my @out_msg_l = ( $out_xml );
    return @out_msg_l;
}

sub delete_jobdb_entry {
    my ($msg, $msg_hash, $session_id) = @_ ;
    my $target = @{$msg_hash->{'target'}}[0];
    my $source = @{$msg_hash->{'source'}}[0];
    
    # prepare query sql statement
    my $table= $main::job_queue_tn;
    my $where= &get_where_statement($msg, $msg_hash);
    my $sql_statement = "DELETE FROM $table $where";
    
    # execute db query
    my $db_res = $main::job_db->del_dbentry($sql_statement);

    my $res;
    if( $db_res > 0 ) { 
        $res = 0 ;
    } else {
        $res = 1;
    }

    # prepare xml answer
    my $out_xml = "<xml><header>answer</header><source>$target</source><target>$source</target><answer1>$res</answer1><session_id>$session_id</session_id></xml>";
    my @out_msg_l = ( $out_xml );
    return @out_msg_l;

}


sub clear_jobdb {
    my ($msg, $msg_hash, $session_id) = @_ ;
    my $target = @{$msg_hash->{'target'}}[0];
    my $source = @{$msg_hash->{'source'}}[0];

    my $error= 0;
    my $out_xml= "<xml><answer1>1</answer1></xml>";
 
    my $table= $main::job_queue_tn;
    
    my $sql_statement = "DELETE FROM $table";
    my $db_res = $main::job_db->del_dbentry($sql_statement);
    if( not $db_res > 0 ) { $error++; };
    
    if( $error == 0 ) {
        $out_xml = "<xml><header>answer</header><source>$target</source><target>$source</target><answer1>0</answer1><session_id>$session_id</session_id></xml>";
    }
    my @out_msg_l = ( $out_xml );
    return @out_msg_l;
}


sub update_status_jobdb_entry {
    my ($msg, $msg_hash, $session_id) = @_ ;
    my $target = @{$msg_hash->{'target'}}[0];
    my $source = @{$msg_hash->{'source'}}[0];

    my $error= 0;
    my $out_xml= "<xml><header>answer</header><source>$target</source><target>$source</target><answer1>1</answer1><session_id>$session_id</session_id></xml>";

    my @len_hash = keys %{$msg_hash};
    if( 0 == @len_hash) {  $error++; };
    
    # prepare query sql statement
    if( $error == 0) {
        my $table= $main::job_queue_tn;
        my $where= &get_where_statement($msg, $msg_hash);
        my $update= &get_update_statement($msg, $msg_hash);

        # conditions
        # no timestamp update if status eq waiting
        my $sql_statement = "SELECT * FROM $table $where AND status='processing'";
        my $res_hash = $main::job_db->select_dbentry($sql_statement);
        if( (0 != keys(%$res_hash)) && ($update =~ /timestamp/i) ) {
            $error ++;
            $out_xml = "<answer1>1</answer1><error_string>there is no timestamp update allowed while status is 'processing'</error_string>";
        }

        if( $error == 0 ) {
            my $sql_statement = "UPDATE $table $update $where";
            # execute db query
            my $db_res = $main::job_db->update_dbentry($sql_statement);

            # check success of db update
            if( not $db_res > 0 ) { $error++; };
        }
    }

    if( $error == 0) {
        $out_xml = "<answer1>0</answer1>";
    }
    
    my $out_msg = sprintf("<xml><header>answer</header><source>%s</source><target>%s</target>%s<session_id>$session_id</session_id></xml>", $target, $source, $out_xml);
    my @out_msg_l = ( $out_msg );
    return @out_msg_l;
}

1;
