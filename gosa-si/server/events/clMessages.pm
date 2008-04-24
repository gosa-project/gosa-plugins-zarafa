package clMessages;
use Exporter;
@ISA = qw(Exporter);
my @events = (
    "confirm_usr_msg",
    "PROGRESS",
    "FAIREBOOT",
    "TASKSKIP",
    "TASKBEGIN",
    "TASKEND",
    "TASKERROR",
    "HOOK",
    "GOTOACTIVATION",
    "LOGIN",
    "LOGOUT",
    "CURRENTLY_LOGGED_IN",
    "save_fai_log",
    );
@EXPORT = @events;

use strict;
use warnings;
use Data::Dumper;
use GOSA::GosaSupportDaemon;
use MIME::Base64;


BEGIN {}

END {}

### Start ######################################################################

my $ldap_uri;
my $ldap_base;
my $ldap_admin_dn;
my $ldap_admin_password;

my %cfg_defaults = (
"server" => {
   "ldap-uri" => [\$ldap_uri, ""],
   "ldap-base" => [\$ldap_base, ""],
   "ldap-admin-dn" => [\$ldap_admin_dn, ""],
   "ldap-admin-password" => [\$ldap_admin_password, ""],
   },
);
&read_configfile($main::cfg_file, %cfg_defaults);


sub get_events {
    return \@events;
}


sub confirm_usr_msg {
    my ($msg, $msg_hash, $session_id) = @_;
    my $message = @{$msg_hash->{'message'}}[0];
    my $subject = @{$msg_hash->{'subject'}}[0];
    my $usr = @{$msg_hash->{'usr'}}[0];

    # set update for this message
    my $sql = "UPDATE $main::messaging_tn SET flag='s' WHERE (message='$message' AND subject='$subject' AND message_to='$usr')"; 
    &main::daemon_log("$session_id DEBUG: $sql", 7);
    my $res = $main::messaging_db->exec_statement($sql); 


    return;
}



sub read_configfile {
    my ($cfg_file, %cfg_defaults) = @_;
    my $cfg;

    if( defined( $cfg_file) && ( (-s $cfg_file) > 0 )) {
        if( -r $cfg_file ) {
            $cfg = Config::IniFiles->new( -file => $cfg_file );
        } else {
            &main::daemon_log("ERROR: clMessages.pm couldn't read config file!", 1);
        }
    } else {
        $cfg = Config::IniFiles->new() ;
    }
    foreach my $section (keys %cfg_defaults) {
        foreach my $param (keys %{$cfg_defaults{ $section }}) {
            my $pinfo = $cfg_defaults{ $section }{ $param };
            ${@$pinfo[0]} = $cfg->val( $section, $param, @$pinfo[1] );
        }
    }
}


sub save_fai_log {
    my ($msg, $msg_hash, $session_id) = @_;
    my $header = @{$msg_hash->{'header'}}[0];
    my $source = @{$msg_hash->{'source'}}[0];
    my $macaddress = @{$msg_hash->{'macaddress'}}[0];
    my $all_logs = @{$msg_hash->{$header}}[0];

    # if there is nothing to log
    if( ref($all_logs) eq "HASH" ) { return; }
        
    my $client_fai_log_dir = $main::client_fai_log_dir;
    if (not -d $client_fai_log_dir) {
        mkdir($client_fai_log_dir, 0755)
    }

    $client_fai_log_dir = File::Spec->catfile( $client_fai_log_dir, $macaddress );
    if (not -d $client_fai_log_dir) {
        mkdir($client_fai_log_dir, 0755)
    }

    my $time = &get_time;
    $time = substr($time, 0, 8)."_".substr($time, 8, 6);
    $client_fai_log_dir = File::Spec->catfile( $client_fai_log_dir, "install_$time" );
    mkdir($client_fai_log_dir, 0755);

    my @all_logs = split(/log_file:/, $all_logs); 
    foreach my $log (@all_logs) {
        if (length $log == 0) { next; };
        my ($log_file, $log_string) = split(":", $log);
        my $client_fai_log_file = File::Spec->catfile( $client_fai_log_dir, $log_file);

	open(my $LOG_FILE, ">$client_fai_log_file"); 
        print $LOG_FILE &decode_base64($log_string);
        close($LOG_FILE);

    }
    return;
}


sub LOGIN {
    my ($msg, $msg_hash, $session_id) = @_;
    my $header = @{$msg_hash->{'header'}}[0];
    my $source = @{$msg_hash->{'source'}}[0];
    my $login = @{$msg_hash->{$header}}[0];

    my %add_hash = ( table=>$main::login_users_tn, 
        primkey=> ['client', 'user'],
        client=>$source,
        user=>$login,
        timestamp=>&get_time,
        ); 
    my ($res, $error_str) = $main::login_users_db->add_dbentry( \%add_hash );
    if ($res != 0)  {
        &main::daemon_log("$session_id ERROR: cannot add entry to known_clients: $error_str");
        return;
    }

    return;   
}


sub LOGOUT {
    my ($msg, $msg_hash, $session_id) = @_;
    my $header = @{$msg_hash->{'header'}}[0];
    my $source = @{$msg_hash->{'source'}}[0];
    my $login = @{$msg_hash->{$header}}[0];
    
    my $sql_statement = "DELETE FROM $main::login_users_tn WHERE (client='$source' AND user='$login')"; 
    my $res =  $main::login_users_db->del_dbentry($sql_statement);
    &main::daemon_log("$session_id INFO: delete user '$login' at client '$source' from login_user_db", 5); 
    
    return;
}


sub CURRENTLY_LOGGED_IN {
    my ($msg, $msg_hash, $session_id) = @_;
    my ($sql_statement, $db_res);
    my $header = @{$msg_hash->{'header'}}[0];
    my $source = @{$msg_hash->{'source'}}[0];
    my $login = @{$msg_hash->{$header}}[0];

    if(ref $login eq "HASH") { 
        &main::daemon_log("$session_id INFO: no logged in users reported from host '$source'", 5); 
        return;     
    }
    
    # fetch all user currently assigned to the client at login_users_db
    my %currently_logged_in_user = (); 
    $sql_statement = "SELECT * FROM $main::login_users_tn WHERE client='$source'"; 
    $db_res = $main::login_users_db->select_dbentry($sql_statement);
    while( my($hit_id, $hit) = each(%{$db_res}) ) {
        $currently_logged_in_user{$hit->{'user'}} = 1;
    }
    &main::daemon_log("$session_id DEBUG: logged in users from login_user_db: ".join(", ", keys(%currently_logged_in_user)), 7); 

    # update all reported users in login_user_db
    my @logged_in_user = split(/\s+/, $login);
    &main::daemon_log("$session_id DEBUG: logged in users reported from client: ".join(", ", @logged_in_user), 7); 
    foreach my $user (@logged_in_user) {
        my %add_hash = ( table=>$main::login_users_tn, 
                primkey=> ['client', 'user'],
                client=>$source,
                user=>$user,
                timestamp=>&get_time,
                ); 
        my ($res, $error_str) = $main::login_users_db->add_dbentry( \%add_hash );
        if ($res != 0)  {
            &main::daemon_log("$session_id ERROR: cannot add entry to known_clients: $error_str");
            return;
        }

        delete $currently_logged_in_user{$user};
    }

    # if there is still a user in %currently_logged_in_user 
    # although he is not reported by client 
    # then delete it from $login_user_db
    foreach my $obsolete_user (keys(%currently_logged_in_user)) {
        &main::daemon_log("$session_id WARNING: user '$obsolete_user' is currently not logged ".
                "in at client '$source' but still found at login_user_db", 3); 
        my $sql_statement = "DELETE FROM $main::login_users_tn WHERE client='$source' AND user='$obsolete_user'"; 
        my $res =  $main::login_users_db->del_dbentry($sql_statement);
        &main::daemon_log("$session_id WARNING: delete user '$obsolete_user' at client '$source' from login_user_db", 3); 
    }

    return;
}


sub GOTOACTIVATION {
    my ($msg, $msg_hash, $session_id) = @_;
    my $header = @{$msg_hash->{'header'}}[0];
    my $source = @{$msg_hash->{'target'}}[0];
    my $macaddress = @{$msg_hash->{'macaddress'}}[0];

    # test whether content is an empty hash or a string which is required
    my $content = @{$msg_hash->{$header}}[0];
    if(ref($content) eq "HASH") { $content = ""; }

    # clean up header
    $header =~ s/CLMSG_//g;

    my $sql_statement = "UPDATE $main::job_queue_tn ".
            "SET status='processing', progress='goto-activation' ".
            "WHERE status='processing' AND macaddress LIKE '$macaddress'"; 
    &main::daemon_log("$session_id DEBUG: $sql_statement", 7);         
    my $res = $main::job_db->update_dbentry($sql_statement);
    &main::daemon_log("$session_id INFO: $header at '$macaddress'", 5); 
    return; 
}


sub PROGRESS {
    my ($msg, $msg_hash, $session_id) = @_;
    my $header = @{$msg_hash->{'header'}}[0];
    my $source = @{$msg_hash->{'target'}}[0];
    my $macaddress = @{$msg_hash->{'macaddress'}}[0];

    # test whether content is an empty hash or a string which is required
    my $content = @{$msg_hash->{$header}}[0];
    if(ref($content) eq "HASH") { $content = ""; }

    # clean up header
    $header =~ s/CLMSG_//g;

    my $sql_statement = "UPDATE $main::job_queue_tn ".
        "SET progress='$content' ".
        "WHERE status='processing' AND macaddress LIKE '$macaddress'";
    &main::daemon_log("$session_id DEBUG: $sql_statement", 7);         
    my $res = $main::job_db->update_dbentry($sql_statement);
    &main::daemon_log("$session_id INFO: $header at '$macaddress' - $content%", 5); 

    return;
}


sub FAIREBOOT {
    my ($msg, $msg_hash, $session_id) = @_;
    my $header = @{$msg_hash->{'header'}}[0];
    my $source = @{$msg_hash->{'target'}}[0];
    my $macaddress = @{$msg_hash->{'macaddress'}}[0];

    # test whether content is an empty hash or a string which is required
	my $content = @{$msg_hash->{$header}}[0];
    if(ref($content) eq "HASH") { $content = ""; }

    # clean up header
    $header =~ s/CLMSG_//g;

    my $sql_statement = "UPDATE $main::job_queue_tn ".
            "SET status='processing', result='$header "."$content' ".
            "WHERE status='processing' AND macaddress LIKE '$macaddress'"; 
    &main::daemon_log("$session_id DEBUG: $sql_statement", 7);         
    my $res = $main::job_db->update_dbentry($sql_statement);
    &main::daemon_log("$session_id INFO: $header at '$macaddress' - '$content'", 5); 

    return; 
}


sub TASKSKIP {
    my ($msg, $msg_hash, $session_id) = @_;
    my $header = @{$msg_hash->{'header'}}[0];
    my $source = @{$msg_hash->{'target'}}[0];
    my $macaddress = @{$msg_hash->{'macaddress'}}[0];

    # test whether content is an empty hash or a string which is required
	my $content = @{$msg_hash->{$header}}[0];
    if(ref($content) eq "HASH") { $content = ""; }

    # clean up header
    $header =~ s/CLMSG_//g;

    my $sql_statement = "UPDATE $main::job_queue_tn ".
            "SET status='processing', result='$header "."$content' ".
            "WHERE status='processing' AND macaddress LIKE '$macaddress'"; 
    &main::daemon_log("$session_id DEBUG: $sql_statement", 7);         
    my $res = $main::job_db->update_dbentry($sql_statement);
    &main::daemon_log("$session_id INFO: $header at '$macaddress' - '$content'", 5); 

    return; 
}


sub TASKBEGIN {
    my ($msg, $msg_hash, $session_id) = @_;
    my $header = @{$msg_hash->{'header'}}[0];
    my $source = @{$msg_hash->{'target'}}[0];
    my $macaddress = @{$msg_hash->{'macaddress'}}[0];

    # test whether content is an empty hash or a string which is required
	my $content = @{$msg_hash->{$header}}[0];
    if(ref($content) eq "HASH") { $content = ""; }

    # clean up header
    $header =~ s/CLMSG_//g;

    # TASKBEGIN eq finish or faiend 
    if (($content eq 'finish') || ($content eq 'faiend')){
        my $sql_statement = "UPDATE $main::job_queue_tn ".
            "SET status='done', result='$header "."$content' ".
            "WHERE status='processing' AND macaddress LIKE '$macaddress'"; 
        &main::daemon_log("$session_id DEBUG: $sql_statement", 7);         
        my $res = $main::job_db->update_dbentry($sql_statement);
        &main::daemon_log("$session_id INFO: $header at '$macaddress' - '$content'", 5); 
        
        # set fai_state to localboot
        &main::change_fai_state('localboot', \@{$msg_hash->{'macaddress'}}, $session_id);

	# other TASKBEGIN msgs
    } else {
		# select processing jobs for host
		my $sql_statement = "SELECT * FROM $main::job_queue_tn WHERE status='processing' AND macaddress LIKE '$macaddress'"; 
		&main::daemon_log("$session_id DEBUG: $sql_statement", 7);
		my $res = $main::job_db->select_dbentry($sql_statement);

		# there is exactly one job entry in queue for this host
		if (keys(%$res) == 1) {
			&main::daemon_log("$session_id DEBUG: there is already one processing job in queue for host '$macaddress', run an update for this entry", 7);
			my $sql_statement = "UPDATE $main::job_queue_tn SET result='$header $content' WHERE status='processing' AND macaddress LIKE '$macaddress'";
			my $err = $main::job_db->update_dbentry($sql_statement);
			if (not defined  $err) {
				&main::daemon_log("$session_id ERROR: cannot update job_db entry: ".Dumper($err), 1);
			}
			
		# there is no entry or more than one enties
		} else {
			# in case of more than one running jobs in queue, delete all jobs
			if (keys(%$res) > 1) {
				&main::daemon_log("$session_id DEBUG: there are more than one processing job in queue for host '$macaddress', ".
								"delete entries", 7); 

				my $sql_statement = "DELETE FROM $main::job_queue_tn WHERE status='processing' AND macaddress LIKE '$macaddress'";
				my ($err) = $main::job_db->del_dbentry($sql_statement);
				if (not defined $err) {
					&main::daemon_log("$session_id ERROR: can not delete multiple processing queue entries for host '$macaddress': ".Dumper($err), 1); 
				}
			}
		
			# in case of no and more than one running jobs in queue, add one single job

# TODO
			# resolve plain name for host $macaddress
			&main::daemon_log("$session_id DEBUG: add job to queue for host '$macaddress'", 7); 
			my $func_dic = {table=>$main::job_queue_tn,
					primkey=>[],
					timestamp=>&get_time,
					status=>'processing',
					result=>"$header $content",
					progress=>'none',
					headertag=>'trigger_action_reinstall',
					targettag=>$source,
					xmlmessage=>'none',
					macaddress=>$macaddress,
					plainname=>'none',
			};
			my ($err, $error_str) = $main::job_db->add_dbentry($func_dic);
			if ($err != 0)  {
					&main::daemon_log("$session_id ERROR: cannot add entry to job_db: $error_str", 1);
			}

		}

# -----------------------> Update hier
#  <CLMSG_TASKBEGIN>finish</CLMSG_TASKBEGIN>
#  <header>CLMSG_TASKBEGIN</header>
# macaddress auslesen, Client im LDAP lokalisieren
# FAIstate auf "localboot" setzen, wenn FAIstate "install" oder "softupdate" war
    }

    return; 
}


sub TASKEND {
    my ($msg, $msg_hash, $session_id) = @_;
    my $header = @{$msg_hash->{'header'}}[0];
    my $source = @{$msg_hash->{'target'}}[0];
    my $macaddress = @{$msg_hash->{'macaddress'}}[0];

    # test whether content is an empty hash or a string which is required
    my $content = @{$msg_hash->{$header}}[0];
    if(ref($content) eq "HASH") { $content = ""; }

    # clean up header
    $header =~ s/CLMSG_//g;

	if ($content eq "savelog 0") {
		&main::daemon_log("$session_id DEBUG: got savelog from host '$source' - jub done", 7);
		my $sql_statement = "DELETE FROM $main::job_queue_tn WHERE status='processing' AND macaddress LIKE '$macaddress'"; 
		&main::daemon_log("$session_id DEBUG: $sql_statement", 7);
		my $res = $main::job_db->del_dbentry($sql_statement);

	} else {

			my $sql_statement = "UPDATE $main::job_queue_tn ".
					"SET status='processing', result='$header "."$content' ".
					"WHERE status='processing' AND macaddress LIKE '$macaddress'"; 
			&main::daemon_log("$session_id DEBUG: $sql_statement", 7);         
			my $res = $main::job_db->update_dbentry($sql_statement);
			&main::daemon_log("$session_id INFO: $header at '$macaddress' - '$content'", 5); 

	}
# -----------------------> Update hier
#  <CLMSG_TASKBEGIN>finish</CLMSG_TASKBEGIN>
#  <header>CLMSG_TASKBEGIN</header>
# macaddress auslesen, Client im LDAP lokalisieren
# FAIstate auf "error" setzen

    return; 
}


sub TASKERROR {
    my ($msg, $msg_hash, $session_id) = @_;
    my $header = @{$msg_hash->{'header'}}[0];
    my $source = @{$msg_hash->{'target'}}[0];
    my $macaddress = @{$msg_hash->{'macaddress'}}[0];

    # clean up header
    $header =~ s/CLMSG_//g;

    # test whether content is an empty hash or a string which is required
    my $content = @{$msg_hash->{$header}}[0];
    if(ref($content) eq "HASH") { $content = ""; } 

	# set fai_state to localboot
	&main::change_fai_state('error', \@{$msg_hash->{'macaddress'}}, $session_id);
		
    my $sql_statement = "UPDATE $main::job_queue_tn ".
            "SET status='processing', result='$header "."$content' ".
            "WHERE status='processing' AND macaddress LIKE '$macaddress'"; 
    &main::daemon_log("$session_id DEBUG: $sql_statement", 7);         
    my $res = $main::job_db->update_dbentry($sql_statement);
    &main::daemon_log("$session_id INFO: $header at '$macaddress' - '$content'", 5); 

# -----------------------> Update hier
#  <CLMSG_TASKBEGIN>finish</CLMSG_TASKBEGIN>
#  <header>CLMSG_TASKBEGIN</header>
# macaddress auslesen, Client im LDAP lokalisieren
# FAIstate auf "error" setzen

    return; 
}


sub HOOK {
    my ($msg, $msg_hash, $session_id) = @_;
    my $header = @{$msg_hash->{'header'}}[0];
    my $source = @{$msg_hash->{'target'}}[0];
    my $macaddress = @{$msg_hash->{'macaddress'}}[0];

    # clean up header
    $header =~ s/CLMSG_//g;

    # test whether content is an empty hash or a string which is required
	my $content = @{$msg_hash->{$header}}[0];
    if(ref($content) eq "HASH") { $content = ""; }

    my $sql_statement = "UPDATE $main::job_queue_tn ".
            "SET status='processing', result='$header "."$content' ".
            "WHERE status='processing' AND macaddress LIKE '$macaddress'"; 
    &main::daemon_log("$session_id DEBUG: $sql_statement", 7);         
    my $res = $main::job_db->update_dbentry($sql_statement);
    &main::daemon_log("$session_id INFO: $header at '$macaddress' - '$content'", 5); 

    return;
}


1;
