package gosaTriggered;
use Exporter;
@ISA = qw(Exporter);
my @events = (
    "get_events", 
    "get_login_usr_for_client",
    "get_client_for_login_usr",
    "gen_smb_hash",
    "trigger_reload_ldap_config",
    "ping",
    "network_completition",
    "set_activated_for_installation",
    "new_key_for_client",
    "detect_hardware",
    "get_login_usr",
    "get_login_client",
    "trigger_action_localboot",
    "trigger_action_faireboot",
    "trigger_action_reboot",
    "trigger_action_activate",
    "trigger_action_lock",
    "trigger_action_halt",
    "trigger_action_update", 
    "trigger_action_reinstall",
    "trigger_action_memcheck", 
    "trigger_action_sysinfo",
    "trigger_action_instant_update",
    "trigger_action_rescan",
    "trigger_action_wake",
    "recreate_fai_server_db",
    "send_user_msg", 
    );
@EXPORT = @events;

use strict;
use warnings;
use GOSA::GosaSupportDaemon;
use Data::Dumper;
use Crypt::SmbHash;
use Net::ARP;
use Net::Ping;
use Socket;
use utf8;

BEGIN {}

END {}

### Start ######################################################################

#&main::read_configfile($main::cfg_file, %cfg_defaults);

sub get_events {
    return \@events;
}

sub send_user_msg {
    my ($msg, $msg_hash, $session_id) = @_ ;
    my @out_msg_l;
    my @user_list;
    my @group_list;

    my $header = @{$msg_hash->{'header'}}[0];
    my $source = @{$msg_hash->{'source'}}[0];
    my $target = @{$msg_hash->{'target'}}[0];
    my $message = @{$msg_hash->{'message'}}[0];
    if( exists $msg_hash->{'user'} ) { @user_list = @{$msg_hash->{'user'}}; }
    if( exists $msg_hash->{'group'} ) { @group_list = @{$msg_hash->{'group'}}; }

    # error handling
    if( not @user_list && not @group_list ) {
        &main::daemon_log("WARNING: no user-tag or a group-tag specified in 'send_user_msg'", 3); 
        return ("<xml><header>$header</header><source>GOSA</source><target>GOSA</target>".
                "<error_string>no user-tag or a group-tag specified in 'send_user_msg'</error_string></xml>");
    }
    if( not defined $message ) {
        &main::daemon_log("WARNING: no message-tag specified in 'send_user_msg'", 3); 
        return ("<xml><header>$header</header><source>GOSA</source><target>GOSA</target>".
                "<error_string>no message-tag specified in 'send_user_msg'</error_string></xml>");

    }

    # resolve groups to users
    if( @group_list ) {
        # build ldap connection
        &main::refresh_ldap_handle();
        if( not defined $main::ldap_handle ) {
            &main::daemon_log("ERROR: cannot connect to ldap", 1);
            return ();
        } 
        foreach my $group (@group_list) {
            # Perform search
            my $mesg = $main::ldap_handle->search( 
                    base => $main::ldap_base,
                    scope => 'sub',
                    attrs => ['memberUid'],
                    filter => "(&(objectClass=posixGroup)(cn=$group)(memberUid=*))");
            if($mesg->code) {
                &main::daemon_log($mesg->error, 1);
                return ();
            }
            my $entry= $mesg->entry(0);
            my @users= $entry->get_value("memberUid");
            foreach my $user (@users) { push(@user_list, $user); }
        }
    }

    # drop multiple users in @user_list
    my %seen = ();
    foreach my $user (@user_list) {
        $seen{$user}++;
    }
    @user_list = keys %seen;

    # build xml messages sended to client where user is logged in
    foreach my $user (@user_list) {
        my $sql_statement = "SELECT * FROM $main::login_users_tn WHERE user='$user'"; 
        my $db_res = $main::login_users_db->select_dbentry($sql_statement);

        if(0 == keys(%{$db_res})) {

        } else {
            while( my($hit, $content) = each %{$db_res} ) {
                my $out_hash = &create_xml_hash('send_user_msg', $main::server_address, $content->{'client'});
                &add_content2xml_hash($out_hash, 'message', $message);
                &add_content2xml_hash($out_hash, 'user', $user);
                if( exists $msg_hash->{'jobdb_id'} ) { 
                    &add_content2xml_hash($out_hash, 'jobdb_id', @{$msg_hash->{'jobdb_id'}}[0]); 
                }
                my $out_msg = &create_xml_string($out_hash);
                push(@out_msg_l, $out_msg);
            }
        }
    }

    return @out_msg_l;
}


sub recreate_fai_server_db {
    my ($msg, $msg_hash, $session_id) = @_ ;
    my $out_msg;

    $main::fai_server_db->create_table("new_fai_server", \@main::fai_server_col_names);
    &main::create_fai_server_db("new_fai_server");
    $main::fai_server_db->move_table("new_fai_server", $main::fai_server_tn);

    my @out_msg_l = ( $out_msg );
    return @out_msg_l;
}


sub get_login_usr_for_client {
    my ($msg, $msg_hash, $session_id) = @_ ;
    my $header = @{$msg_hash->{'header'}}[0];
    my $source = @{$msg_hash->{'source'}}[0];
    my $target = @{$msg_hash->{'target'}}[0];
    my $client = @{$msg_hash->{'client'}}[0];

    $header =~ s/^gosa_//;

    my $sql_statement = "SELECT * FROM known_clients WHERE hostname='$client' OR macaddress='$client'";
    my $res = $main::known_clients_db->select_dbentry($sql_statement);

    my $out_msg = "<xml><header>$header</header><source>$target</source><target>$source</target>";
    $out_msg .= &db_res2xml($res);
    $out_msg .= "</xml>";

    my @out_msg_l = ( $out_msg );
    return @out_msg_l;
}


sub get_client_for_login_usr {
    my ($msg, $msg_hash, $session_id) = @_ ;
    my $header = @{$msg_hash->{'header'}}[0];
    my $source = @{$msg_hash->{'source'}}[0];
    my $target = @{$msg_hash->{'target'}}[0];
    my $usr = @{$msg_hash->{'usr'}}[0];
    $header =~ s/^gosa_//;

    my $sql_statement = "SELECT * FROM known_clients WHERE login LIKE '%$usr%'";
    my $res = $main::known_clients_db->select_dbentry($sql_statement);

    my $out_msg = "<xml><header>$header</header><source>$target</source><target>$source</target>";
    $out_msg .= &db_res2xml($res);
    $out_msg .= "</xml>";
    my @out_msg_l = ( $out_msg );
    return @out_msg_l;

}


sub ping {
    my ($msg, $msg_hash, $session_id) = @_ ;
    my $out_msg = $msg;
    $out_msg =~ s/<header>gosa_/<header>/;
    $out_msg =~ s/<\/xml>/<session_id>$session_id<\/session_id><\/xml>/; 

    my @out_msg_l = ( $out_msg );
    return @out_msg_l;
}

sub gen_smb_hash {
     my ($msg, $msg_hash, $session_id) = @_ ;
     my $source = @{$msg_hash->{source}}[0];
     my $target = @{$msg_hash->{target}}[0];
     my $password = @{$msg_hash->{password}}[0];

     my %data= ('hash' => join(q[:], ntlmgen $password));
     my $out_msg = &build_msg("gen_smb_hash", $target, 'GOSA', \%data );
     return ( $out_msg );
}


sub network_completition {
     my ($msg, $msg_hash, $session_id) = @_ ;
     my $source = @{$msg_hash->{source}}[0];
     my $target = @{$msg_hash->{target}}[0];
     my $name = @{$msg_hash->{hostname}}[0];

     # Can we resolv the name?
     my %data;
     if (inet_aton($name)){
	     my $address = inet_ntoa(inet_aton($name));
	     my $p = Net::Ping->new('tcp');
	     my $mac= "";
	     if ($p->ping($address, 1)){
	       $mac = Net::ARP::arp_lookup("", $address);
	     }

	     %data= ('ip' => $address, 'mac' => $mac);
     } else {
	     %data= ('ip' => '', 'mac' => '');
     }

     my $out_msg = &build_msg("network_completition", $target, 'GOSA', \%data );
    
     return ( $out_msg );
}


sub detect_hardware {
    my ($msg, $msg_hash) = @_ ;
    # just forward msg to client, but dont forget to split off 'gosa_' in header
    my $source = @{$msg_hash->{source}}[0];
    my $target = @{$msg_hash->{target}}[0];

    my $out_hash = &create_xml_hash("detect_hardware", $source, $target);
    if( exists $msg_hash->{'jobdb_id'} ) { 
        &add_content2xml_hash($out_hash, 'jobdb_id', @{$msg_hash->{'jobdb_id'}}[0]); 
    }
    my $out_msg = &create_xml_string($out_hash);

    my @out_msg_l = ( $out_msg );
    return @out_msg_l;

}


sub trigger_reload_ldap_config {
    my ($msg, $msg_hash) = @_ ;
    my $target = @{$msg_hash->{target}}[0];

    my $out_hash = &create_xml_hash("reload_ldap_config", $main::server_address, $main::server_address, $target);
    if( exists $msg_hash->{'jobdb_id'} ) { 
        &add_content2xml_hash($out_hash, 'jobdb_id', @{$msg_hash->{'jobdb_id'}}[0]); 
    }
    my $out_msg = &create_xml_string($out_hash);
    my @out_msg_l;
    push(@out_msg_l, $out_msg);
    return @out_msg_l;
}


sub set_activated_for_installation {
    my ($msg, $msg_hash) = @_;

    my $header = @{$msg_hash->{header}}[0];
    my $source = @{$msg_hash->{source}}[0];
    my $target = @{$msg_hash->{target}}[0];

    my $out_hash = &create_xml_hash("set_activated_for_installation", $source, $target);
    if( exists $msg_hash->{'jobdb_id'} ) { 
        &add_content2xml_hash($out_hash, 'jobdb_id', @{$msg_hash->{'jobdb_id'}}[0]); 
    }
    my $out_msg = &create_xml_string($out_hash);
    my @out_msg_l = ( $out_msg );
    return @out_msg_l;
}


sub trigger_action_faireboot {
    my ($msg, $msg_hash) = @_;
    my $macaddress = @{$msg_hash->{target}}[0];
    my $source = @{$msg_hash->{source}}[0];

    my @out_msg_l;
    $msg =~ s/<header>gosa_trigger_action_faireboot<\/header>/<header>trigger_action_faireboot<\/header>/;
    push(@out_msg_l, $msg);

    &main::change_goto_state('locked', \@{$msg_hash->{target}});
    &main::change_fai_state('localboot', \@{$msg_hash->{target}});

    # delete all jobs from jobqueue which correspond to fai
    my $sql_statement = "DELETE FROM $main::job_queue_tn WHERE (macaddress='$macaddress' AND ".
        "status='processing')";
    $main::job_db->del_dbentry($sql_statement ); 
                                             
    return @out_msg_l;
}


sub trigger_action_lock {
    my ($msg, $msg_hash) = @_;
    my $macaddress = @{$msg_hash->{target}}[0];
    my $source = @{$msg_hash->{source}}[0];

    &main::change_goto_state('locked', \@{$msg_hash->{target}});
                                             
    my @out_msg_l;
    return @out_msg_l;
}


sub trigger_action_activate {
    my ($msg, $msg_hash) = @_;
    my $macaddress = @{$msg_hash->{target}}[0];
    my $source = @{$msg_hash->{source}}[0];

    &main::change_goto_state('active', \@{$msg_hash->{target}});
                                             
    my $out_hash = &create_xml_hash("set_activated_for_installation", $source, $macaddress);
    if( exists $msg_hash->{'jobdb_id'} ) { 
        &add_content2xml_hash($out_hash, 'jobdb_id', @{$msg_hash->{'jobdb_id'}}[0]); 
    }
    my $out_msg = &create_xml_string($out_hash);

    return ( $out_msg );
}


sub trigger_action_localboot {
    my ($msg, $msg_hash) = @_;
    $msg =~ s/<header>gosa_trigger_action_localboot<\/header>/<header>trigger_action_localboot<\/header>/;

    &main::change_fai_state('localboot', \@{$msg_hash->{target}});

    my @out_msg_l = ($msg);  
    return @out_msg_l;
}


sub trigger_action_halt {
    my ($msg, $msg_hash) = @_;
    $msg =~ s/<header>gosa_trigger_action_halt<\/header>/<header>trigger_action_halt<\/header>/;

    &main::change_fai_state('halt', \@{$msg_hash->{target}});

    my @out_msg_l = ($msg);  
    return @out_msg_l;
}


sub trigger_action_reboot {
    my ($msg, $msg_hash) = @_;
    $msg =~ s/<header>gosa_trigger_action_reboot<\/header>/<header>trigger_action_reboot<\/header>/;

    &main::change_fai_state('reboot', \@{$msg_hash->{target}});

    my @out_msg_l = ($msg);  
    return @out_msg_l;
}


sub trigger_action_memcheck {
    my ($msg, $msg_hash) = @_ ;
    $msg =~ s/<header>gosa_trigger_action_memcheck<\/header>/<header>trigger_action_memcheck<\/header>/;

    &main::change_fai_state('memcheck', \@{$msg_hash->{target}});

    my @out_msg_l = ($msg);  
    return @out_msg_l;
}


sub trigger_action_reinstall {
    my ($msg, $msg_hash) = @_;
    $msg =~ s/<header>gosa_trigger_action_reinstall<\/header>/<header>trigger_action_reinstall<\/header>/;

    &main::change_fai_state('reinstall', \@{$msg_hash->{target}});

    my %data = ( 'macAddress'  => \@{$msg_hash->{target}} );
    my $wake_msg = &build_msg("trigger_wake", "GOSA", "KNOWN_SERVER", \%data);
    my @out_msg_l = ($wake_msg, $msg);  
    return @out_msg_l;
}


sub trigger_action_update {
    my ($msg, $msg_hash) = @_;
    $msg =~ s/<header>gosa_trigger_action_update<\/header>/<header>trigger_action_update<\/header>/;

    &main::change_fai_state('update', \@{$msg_hash->{target}});

    my %data = ( 'macAddress'  => \@{$msg_hash->{target}} );
    my $wake_msg = &build_msg("trigger_wake", "GOSA", "KNOWN_SERVER", \%data);
    my @out_msg_l = ($wake_msg, $msg);  
    return @out_msg_l;
}


sub trigger_action_instant_update {
    my ($msg, $msg_hash) = @_;
    $msg =~ s/<header>gosa_trigger_action_instant_update<\/header>/<header>trigger_action_instant_update<\/header>/;

    &main::change_fai_state('update', \@{$msg_hash->{target}});

    my %data = ( 'macAddress'  => \@{$msg_hash->{target}} );
    my $wake_msg = &build_msg("trigger_wake", "GOSA", "KNOWN_SERVER", \%data);
    my @out_msg_l = ($wake_msg, $msg);  
    return @out_msg_l;
}


sub trigger_action_sysinfo {
    my ($msg, $msg_hash) = @_;
    $msg =~ s/<header>gosa_trigger_action_sysinfo<\/header>/<header>trigger_action_sysinfo<\/header>/;

    &main::change_fai_state('sysinfo', \@{$msg_hash->{target}});

    my @out_msg_l = ($msg);  
    return @out_msg_l;
}


sub new_key_for_client {
    my ($msg, $msg_hash) = @_;
    $msg =~ s/<header>gosa_new_key_for_client<\/header>/<header>new_key<\/header>/;
    my @out_msg_l = ($msg);  
    return @out_msg_l;
}


sub trigger_action_rescan {
    my ($msg, $msg_hash) = @_;
    $msg =~ s/<header>gosa_trigger_action_rescan<\/header>/<header>trigger_action_rescan<\/header>/;
    my @out_msg_l = ($msg);  
    return @out_msg_l;
}


sub trigger_action_wake {
    my ($msg, $msg_hash) = @_;
    my %data = ( 'macAddress'  => \@{$msg_hash->{target}} );
    my $out_msg = &build_msg("trigger_wake", "GOSA", "KNOWN_SERVER", \%data);
    my @out_msg_l = ($out_msg);  
    return @out_msg_l;
}


1;
