package server_server_com;
use Exporter;
@ISA = qw(Exporter);
my @events = (
    'new_server',
    'confirm_new_server',
    'new_foreign_client',
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
    my @clients = exists $msg_hash->{'client'} ? @{$msg_hash->{'client'}} : qw();

    # sanity check
    if (ref $key eq 'HASH') {
        &main::daemon_log("$session_id ERROR: 'new_server'-message from host '$source' contains no key!", 1);
        return;
    }

    # add foreign server to known_server_db
    my $func_dic = {table=>$main::known_server_tn,
        primkey=>['hostname'],
        hostname => $source,
        status => "new_server",
        hostkey => $key,
        timestamp=>&get_time(),
    };
    my $res = $main::known_server_db->add_dbentry($func_dic);
    if (not $res == 0) {
        &main::daemon_log("$session_id ERROR: server_server_com.pm: cannot add server to known_server_db: $res", 1);
    } else {
        &main::daemon_log("$session_id INFO: server_server_com.pm: server '$source' successfully added to known_server_db", 5);
    }

    # delete all entries at foreign_clients_db coresponding to this server
    my $del_sql = "DELETE FROM $main::foreign_clients_tn WHERE regserver='$source' ";
    my $del_res = $main::foreign_clients_db->exec_statement($del_sql);

    # add clients of foreign server to known_foreign_clients_db
    my @sql_list;
    foreach my $client (@clients) {
        my @client_details = split(/,/, $client);

        # workaround to avoid double entries in foreign_clients_db
        my $del_sql = "DELETE FROM $main::foreign_clients_tn WHERE hostname='".$client_details[0]."'";
        push(@sql_list, $del_sql);

        my $sql = "INSERT INTO $main::foreign_clients_tn VALUES ("
            ."'".$client_details[0]."',"   # hostname
            ."'".$client_details[1]."',"   # macaddress
            ."'".$source."',"              # regserver
            ."'".&get_time()."')";         # timestamp
        push(@sql_list, $sql);
    }
    if (@sql_list) {
		my $len = @sql_list;
		$len /= 2;
        &main::daemon_log("$session_id DEBUG: Inserting ".$len." entries to foreign_clients_db", 8);
        my $res = $main::foreign_clients_db->exec_statementlist(\@sql_list);
    }
            
    # build confirm_new_server message
    my %data = ( key=>$key );
    my $out_msg = &build_msg('confirm_new_server', $main::server_address, $source, \%data);
    my $error =  &main::send_msg_to_target($out_msg, $source, $main::ServerPackages_key, 'confirm_new_server', $session_id); 
    

}


sub confirm_new_server {
    my ($msg, $msg_hash, $session_id) = @_ ;
    my $header = @{$msg_hash->{'header'}}[0];
    my $source = @{$msg_hash->{'source'}}[0];
    my $key = @{$msg_hash->{'key'}}[0];

    my $sql = "UPDATE $main::known_server_tn SET status='$header', hostkey='$key' WHERE hostname='$source'"; 
    my $res = $main::known_server_db->update_dbentry($sql);


}


sub new_foreign_client {
    my ($msg, $msg_hash, $session_id) = @_ ;
    my $header = @{$msg_hash->{'header'}}[0];
    my $source = @{$msg_hash->{'source'}}[0];
    my $hostname = @{$msg_hash->{'client'}}[0];
    my $macaddress = @{$msg_hash->{'macaddress'}}[0];

    my $func_dic = { table => $main::foreign_clients_tn,
        primkey => ['hostname'],
        hostname =>   $hostname,
        macaddress => $macaddress,
        regserver =>  $source,
        timestamp =>  &get_time(),
    };
    my $res = $main::foreign_clients_db->add_dbentry($func_dic);
    if (not $res == 0) {
        &main::daemon_log("$session_id ERROR: server_server_com.pm: cannot add server to foreign_clients_db: $res", 1);
    } else {
        &main::daemon_log("$session_id INFO: server_server_com.pm: client '$hostname' successfully added to foreign_clients_db", 5);
    }

    return;
}


1;
