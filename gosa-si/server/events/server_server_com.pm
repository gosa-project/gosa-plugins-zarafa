package server_server_com;
use Exporter;
@ISA = qw(Exporter);
my @events = (
    'new_server',
    );
@EXPORT = @events;

use strict;
use warnings;
use Data::Dumper;
use GOSA::GosaSupportDaemon;


BEGIN {}

END {}

### Start ######################################################################

sub get_events {
    return \@events;
}


sub new_server {
    my ($msg, $msg_hash, $session_id) = @_ ;
    my $header = @{$msg_hash->{'header'}}[0];
    my $source = @{$msg_hash->{'source'}}[0];
    my $target = @{$msg_hash->{'target'}}[0];
    my $key = @{$msg_hash->{'key'}}[0];
    my $clients = @{$msg_hash->{'clients'}}[0];
    
    # sanity check
    if (ref $key eq 'HASH') {
        &main::daemon_log("$session_id ERROR: 'new_server'-message from host '$source' contains no key!", 1);
        #return;
    }

    # add foreign server to known_server_db
    my $func_dic = {table=>$main::known_server_tn,
        primkey=>['hostname'],
        hostname => $source,
        hostkey => $key,
        timestamp=>&get_time(),
    };
    my $res = $main::known_server_db->add_dbentry($func_dic);
    if (not $res == 0) {
        &main::daemon_log("$session_id ERROR: server_server_com.pm: cannot add server to known_server_db: $res", 1);
    } else {
        &main::daemon_log("$session_id INFO: server_server_com.pm: server '$source' successfully added to known_server_db", 5);
    }

    # add clients of foreign server to known_foreign_clients_db
    

    # build confirm_new_server message
    my $out_msg = &build_msg('confirm_new_server', $main::server_address, $source);
    my $error =  &main::send_msg_to_target($out_msg, $source, $key, 'confirm_new_server', 0); 
    

}

1;
