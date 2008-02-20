package siTriggered;
use Exporter;
@ISA = qw(Exporter);
my @events = (
    "got_ping",
    );
@EXPORT = @events;

use strict;
use warnings;
use GOSA::GosaSupportDaemon;


BEGIN {}

END {}

### Start ######################################################################

#&main::read_configfile($main::cfg_file, %cfg_defaults);

sub get_events {
    return \@events;
}


sub got_ping {
    my ($msg, $msg_hash, $session_id) = @_;

    my $source = @{$msg_hash->{source}}[0];
    my $target = @{$msg_hash->{target}}[0];
    my $header = @{$msg_hash->{header}}[0];
    my $act_time = &get_time;
    my @out_msg_l;
    my $out_msg;

    $session_id = @{$msg_hash->{'session_id'}}[0];

    # check known_clients_db
    my $sql_statement = "SELECT * FROM known_clients WHERE hostname='$source'";
    my $query_res = $main::known_clients_db->select_dbentry( $sql_statement );
    if( 1 == keys %{$query_res} ) {
         my $sql_statement= "UPDATE known_clients ".
            "SET status='$header', timestamp='$act_time' ".
            "WHERE hostname='$source'";
         my $res = $main::known_clients_db->update_dbentry( $sql_statement );
    } 
    
    # check known_server_db
    $sql_statement = "SELECT * FROM known_server WHERE hostname='$source'";
    $query_res = $main::known_server_db->select_dbentry( $sql_statement );
    if( 1 == keys %{$query_res} ) {
         my $sql_statement= "UPDATE known_server ".
            "SET status='$header', timestamp='$act_time' ".
            "WHERE hostname='$source'";
         my $res = $main::known_server_db->update_dbentry( $sql_statement );
    } 

    # create out_msg
    my $out_hash = &create_xml_hash($header, $source, "GOSA");
    &add_content2xml_hash($out_hash, "session_id", $session_id);
    $out_msg = &create_xml_string($out_hash);
    push(@out_msg_l, $out_msg);
    
    return @out_msg_l;
}


1;
