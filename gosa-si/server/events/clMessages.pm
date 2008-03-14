package clMessages;
use Exporter;
@ISA = qw(Exporter);
my @events = (
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


sub read_configfile {
    my ($cfg_file, %cfg_defaults) = @_;
    my $cfg;

    if( defined( $cfg_file) && ( length($cfg_file) > 0 )) {
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
    $client_fai_log_dir = File::Spec->catfile( $client_fai_log_dir, "install-$time" );
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

# TODO umstellen wie bei LOGIN
sub LOGOUT {
    my ($msg, $msg_hash, $session_id) = @_;
    my $header = @{$msg_hash->{'header'}}[0];
    my $source = @{$msg_hash->{'source'}}[0];
    my $login = @{$msg_hash->{$header}}[0];

    my $sql_statement = "SELECT * FROM known_clients WHERE hostname='$source'";
    my $res = $main::known_clients_db->select_dbentry($sql_statement);
    if( 1 != keys(%$res) ) {
        &main::daemon_log("DEBUG: clMessages.pm: LOGOUT: no or more hits found in known_clients_db for host '$source'");
        return;
    }

    my $act_login = $res->{'1'}->{'login'};
    $act_login =~ s/$login,?//gi;

    if( $act_login eq "" ){ $act_login = "nobody"; }

    $sql_statement = "UPDATE known_clients SET login='$act_login' WHERE hostname='$source'";
    $res = $main::known_clients_db->update_dbentry($sql_statement);
    
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
# TODO eval ändern auf if(ref($content) eq "HASH") dann ... else, dann...
#	my $content = @{$msg_hash->{$header}}[0];
#	$content = "$content";
#	if(ref($content) eq "HASH") { $content = ""; }
#		
    eval{ if( 0 == keys(%$content) ) { $content = ""; } };
    if( $@ ) { $content = "$content"; }

    # clean up header
    $header =~ s/CLMSG_//g;

    my $sql_statement = "UPDATE $main::job_queue_tn ".
            "SET status='processing', result='$header"."$content' ".
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
    my $content;
    my $cont = @{$msg_hash->{$header}}[0];
# TODO eval ändern auf if(ref($content) eq "HASH") dann ... else, dann...
    eval{ if( 0 == keys(%$cont) ) { $content = ""; } };
    if( $@ ) { $content = "$cont"; }

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
# TODO eval ändern auf if(ref($content) eq "HASH") dann ... else, dann...
    eval{ if( 0 == keys(%$content) ) { $content = ""; } };
    if( $@ ) { $content = "$content"; }

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
# TODO eval ändern auf if(ref($content) eq "HASH") dann ... else, dann...
    eval{ if( 0 == keys(%$content) ) { $content = ""; } };
    if( $@ ) { $content = "$content"; }

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
    my $content = @{$msg_hash->{$header}}[0];

    # test whether content is an empty hash or a string which is required
# TODO eval ändern auf if(ref($content) eq "HASH") dann ... else, dann...
    eval{ if( 0 == keys(%$content) ) { $content = ""; } };
    if( $@ ) { $content = "$content"; }

    # clean up header
    $header =~ s/CLMSG_//g;

    # check if installation finished
    if (($content eq 'finish') || ($content eq 'faiend')){
        my $sql_statement = "UPDATE $main::job_queue_tn ".
            "SET status='done', result='$header "."$content' ".
            "WHERE status='processing' AND macaddress LIKE '$macaddress'"; 
        &main::daemon_log("$session_id DEBUG: $sql_statement", 7);         
        my $res = $main::job_db->update_dbentry($sql_statement);
        &main::daemon_log("$session_id INFO: $header at '$macaddress' - '$content'", 5); 
        
        # set fai_state to localboot
        &main::change_fai_state('localboot', \@{$msg_hash->{'macaddress'}}, $session_id);

    } else {
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
# TODO eval ändern auf if(ref($content) eq "HASH") dann ... else, dann...
    eval{ if( 0 == keys(%$content) ) { $content = ""; } };
    if( $@ ) { $content = "$content"; }

    # clean up header
    $header =~ s/CLMSG_//g;

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


sub TASKERROR {
    my ($msg, $msg_hash, $session_id) = @_;
    my $header = @{$msg_hash->{'header'}}[0];
    my $source = @{$msg_hash->{'target'}}[0];
    my $macaddress = @{$msg_hash->{'macaddress'}}[0];

    # clean up header
    $header =~ s/CLMSG_//g;

    # test whether content is an empty hash or a string which is required
    my $content = @{$msg_hash->{$header}}[0];
# TODO eval ändern auf if(ref($content) eq "HASH") dann ... else, dann...
    eval{ if( 0 == keys(%$content) ) { $content = ""; } };
    if( $@ ) { $content = "$content"; }

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

# TODO eval ändern auf if(ref($content) eq "HASH") dann ... else, dann...
    eval{ if( 0 == keys(%$content) ) { $content = ""; } };
    if( $@ ) { $content = "$content"; }

    my $sql_statement = "UPDATE $main::job_queue_tn ".
            "SET status='processing', result='$header "."$content' ".
            "WHERE status='processing' AND macaddress LIKE '$macaddress'"; 
    &main::daemon_log("$session_id DEBUG: $sql_statement", 7);         
    my $res = $main::job_db->update_dbentry($sql_statement);
    &main::daemon_log("$session_id INFO: $header at '$macaddress' - '$content'", 5); 

    return;
}


1;
