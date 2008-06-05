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
use Time::HiRes qw( usleep);


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
            
    # fetch all registered clients
    my $client_sql = "SELECT * FROM $main::known_clients_tn"; 
    my $client_res = $main::known_clients_db->exec_statement($client_sql);


    # add already connected clients to registration message 
    my $myhash = &create_xml_hash('confirm_new_server', $main::server_address, $source);
    &add_content2xml_hash($myhash, 'key', $key);
    map(&add_content2xml_hash($myhash, 'client', @{$_}[0].",".@{$_}[4]), @$client_res);

    # build registration message and send it
    my $out_msg = &create_xml_string($myhash);


    # build confirm_new_server message
    #my %data = ( key=>$key );
    #my $out_msg = &build_msg('confirm_new_server', $main::server_address, $source, \%data);
    my $error =  &main::send_msg_to_target($out_msg, $source, $main::ServerPackages_key, 'confirm_new_server', $session_id); 
    
    return;
}


sub confirm_new_server {
    my ($msg, $msg_hash, $session_id) = @_ ;
    my $header = @{$msg_hash->{'header'}}[0];
    my $source = @{$msg_hash->{'source'}}[0];
    my $key = @{$msg_hash->{'key'}}[0];
    my @clients = exists $msg_hash->{'client'} ? @{$msg_hash->{'client'}} : qw();

    my $sql = "UPDATE $main::known_server_tn SET status='$header', hostkey='$key' WHERE hostname='$source'"; 
    my $res = $main::known_server_db->update_dbentry($sql);

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


    return;
}


sub new_foreign_client {
    my ($msg, $msg_hash, $session_id) = @_ ;
    my $header = @{$msg_hash->{'header'}}[0];
    my $source = @{$msg_hash->{'source'}}[0];
    my $hostname = @{$msg_hash->{'client'}}[0];
    my $macaddress = @{$msg_hash->{'macaddress'}}[0];
	# if new client is known in known_clients_db
	my $check_sql = "SELECT * FROM $main::known_clients_tn WHERE (macaddress LIKE '$macaddress')"; 
	my $check_res = $main::known_clients_db->select_dbentry($check_sql);

	if( (keys(%$check_res) == 1) ) {
			my $host_key = $check_res->{1}->{'hostkey'};

			# check if new client is still alive
			my $client_hash = &create_xml_hash("ping", $main::server_address, $hostname);
			&add_content2xml_hash($client_hash, 'session_id', $session_id);
			my $client_msg = &create_xml_string($client_hash);
			my $error = &main::send_msg_to_target($client_msg, $hostname, $host_key, 'ping', $session_id);
			my $message_id;
			my $i = 0;
			while (1) {
					$i++;
					my $sql = "SELECT * FROM $main::incoming_tn WHERE headertag='answer_$session_id'";
					my $res = $main::incoming_db->exec_statement($sql);
					if (ref @$res[0] eq "ARRAY") {
							$message_id = @{@$res[0]}[0];
							last;
					}

					# do not run into a endless loop
					if ($i > 50) { last; }
					usleep(100000);
			}

			# client is alive
			# -> new_foreign_client will be ignored
			if (defined $message_id) {
				&main::daemon_log("$session_id ERROR: At new_foreign_clients: host '$hostname' is reported as a new foreign client, ".
								"but the host is still registered at this server. So, the new_foreign_client-msg will be ignored: $msg", 1);
			}
	}

	
	# new client is not found in known_clients_db or
	# new client is dead -> new_client-msg from foreign server is valid
	# -> client will be deleted from known_clients_db 
	# -> inserted to foreign_clients_db
	
	my $del_sql = "DELETE FROM $main::known_clients_tn WHERE (hostname='$hostname')";
	my $del_res = $main::known_clients_db->exec_statement($del_sql);
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
