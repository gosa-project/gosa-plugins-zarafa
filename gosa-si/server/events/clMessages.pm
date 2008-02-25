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
    );
@EXPORT = @events;

use strict;
use warnings;
use Data::Dumper;
use GOSA::GosaSupportDaemon;



BEGIN {}

END {}

### Start ######################################################################

#&read_configfile($main::cfg_file, %cfg_defaults);


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

sub LOGIN {
    my ($msg, $msg_hash, $session_id) = @_;
    my $header = @{$msg_hash->{'header'}}[0];
    my $source = @{$msg_hash->{'source'}}[0];
    my $login = @{$msg_hash->{$header}}[0];

    my $sql_statement = "SELECT * FROM known_clients WHERE hostname='$source'";
    my $res = $main::known_clients_db->select_dbentry($sql_statement);
    if( 1 != keys(%$res) ) {
        &main::daemon_log("DEBUG: clMessages.pm: LOGIN: no or more hits found in known_clients_db for host '$source'");
        return;
    }

    my $act_login = $res->{'1'}->{'login'};
    if( $act_login eq "nobody" ) {
        $act_login = "";
    }

    $act_login =~ s/$login,?//gi;
    my @act_login = split(",", $act_login);
    unshift(@act_login, $login);
    $act_login = join(",", @act_login);

#print STDERR "source: $source\n";
#print STDERR "login: $login\n";
#print STDERR "act_login: $act_login\n";
#print STDERR "dbres: ".Dumper($res)."\n";
    $sql_statement = "UPDATE known_clients ".
                "SET login='$act_login' ".
                "WHERE hostname='$source'";
    $res = $main::known_clients_db->update_dbentry($sql_statement);
    return;   
}


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

    $sql_statement = "UPDATE known_clients ".
                "SET login='$act_login' ".
                "WHERE hostname='$source'";
    $res = $main::known_clients_db->update_dbentry($sql_statement);
    
    return;
}



# echo "GOTOACTIVATION" > /var/run/gosa-si-client.socket
sub GOTOACTIVATION {
    my ($msg, $msg_hash, $session_id) = @_;
    my $out_msg = &build_result_update_msg($msg_hash);
    my @out_msg_l = ($out_msg);  
    return @out_msg_l; 
}


# echo "PROGRESS 15" > /var/run/gosa-si-client.socket
sub PROGRESS {
    my ($msg, $msg_hash, $session_id) = @_;
    my $out_msg = &build_progress_update_msg($msg_hash);
    my @out_msg_l = ($out_msg);  
    return @out_msg_l; 
}

# echo "FAIREBOOT" > /tmp/gosa-si-client-fifo
sub FAIREBOOT {
    my ($msg, $msg_hash, $session_id) = @_;
    my $out_msg = &build_status_result_update_msg($msg_hash);
    my @out_msg_l = ($out_msg);  
    return @out_msg_l; 
}

# echo "TASKSKIP hallo welt" > /tmp/gosa-si-client-fifo
sub TASKSKIP {
    my ($msg, $msg_hash, $session_id) = @_;
    my $out_msg = &build_status_result_update_msg($msg_hash);
    my @out_msg_l = ($out_msg);  
    return @out_msg_l; 
}


# echo "TASKBEGIN hallo welt" > /tmp/gosa-si-client-fifo
sub TASKBEGIN {
    my ($msg, $msg_hash, $session_id) = @_;
    my $out_msg = &build_status_result_update_msg($msg_hash);
    my @out_msg_l = ($out_msg);  
    return @out_msg_l; 
}

# echo "TASKEND hallo welt" > /tmp/gosa-si-client-fifo
sub TASKEND {
    my ($msg, $msg_hash, $session_id) = @_;
    my $out_msg = &build_status_result_update_msg($msg_hash);
    my @out_msg_l = ($out_msg);  
    return @out_msg_l; 
}


# echo "TASKERROR hallo welt" > /tmp/gosa-si-client-fifo
sub TASKERROR {
    my ($msg, $msg_hash, $session_id) = @_;
    my $out_msg = &build_status_result_update_msg($msg_hash);
    my @out_msg_l = ($out_msg);  
    return @out_msg_l; 
}


# echo "HOOK hallo welt" > /tmp/gosa-si-client-fifo
sub HOOK {
    my ($msg, $msg_hash, $session_id) = @_;
    my $out_msg = &build_status_result_update_msg($msg_hash);
    my @out_msg_l = ($out_msg);  
    return @out_msg_l; 
}


sub build_status_result_update_msg {
    my ($msg_hash) = @_;

    my $header = @{$msg_hash->{'header'}}[0];
    my $source = @{$msg_hash->{'target'}}[0];
    my $target = @{$msg_hash->{'target'}}[0];
    my $macaddress = @{$msg_hash->{'macaddress'}}[0];

    # test whether content is an empty hash or a string which is required
    my $content = @{$msg_hash->{$header}}[0];
    eval{
        if( 0 == keys(%$content) ) {
            $content = "";
        }
    };
    if( $@ ) {
        $content = " $content";
    }

    $header =~ s/CLMSG_//g;
    my $out_msg = sprintf("<xml> ".  
        "<header>gosa_update_status_jobdb_entry</header> ".
        "<source>%s</source> ".
        "<target>%s</target>".
        "<where> ".
            "<clause> ".
                "<phrase> ".
                    "<status>processing</status> ".
                    "<macaddress>%s</macaddress> ".
                "</phrase> ".
            "</clause> ".
        "</where> ".
        "<update> ".
            "<status>processing</status> ".
            "<result>%s</result> ".
        "</update> ".
        "</xml>", $source, "JOBDB", $macaddress, $header.$content);
    return $out_msg;
}   


sub build_progress_update_msg {
    my ($msg_hash) = @_;

    my $header = @{$msg_hash->{'header'}}[0];
    my $source = @{$msg_hash->{'target'}}[0];
    my $target = @{$msg_hash->{'target'}}[0];
    my $macaddress = @{$msg_hash->{'macaddress'}}[0];

    # test whether content is an empty hash or a string which is required
    my $content = @{$msg_hash->{$header}}[0];
    eval{
        if( 0 == keys(%$content) ) {
            $content = "";
        }
    };
    if( $@ ) {
        $content = "$content";
    }

    $header =~ s/CLMSG_//g;
    my $out_msg = sprintf("<xml> ".  
        "<header>gosa_update_status_jobdb_entry</header> ".
        "<source>%s</source> ".
        "<target>%s</target>".
        "<where> ".
            "<clause> ".
                "<phrase> ".
                    "<status>processing</status> ".
                    "<macaddress>%s</macaddress> ".
                "</phrase> ".
            "</clause> ".
        "</where> ".
        "<update> ".
            "<progress>%s</progress> ".
        "</update> ".
        "</xml>", $source, "JOBDB", $macaddress, $content);
    return $out_msg;
}


sub build_result_update_msg {
    my ($msg_hash) = @_;

    my $header = @{$msg_hash->{'header'}}[0];
    my $source = @{$msg_hash->{'target'}}[0];
    my $target = @{$msg_hash->{'target'}}[0];
    my $macaddress = @{$msg_hash->{'macaddress'}}[0];

    # test whether content is an empty hash or a string which is required
    my $content = @{$msg_hash->{$header}}[0];
    eval{
        if( 0 == keys(%$content) ) {
            $content = "";
        }
    };
    if( $@ ) {
        $content = " $content";
    }

    $header =~ s/CLMSG_//g;
    my $out_msg = sprintf("<xml> ".  
        "<header>gosa_update_status_jobdb_entry</header> ".
        "<source>%s</source> ".
        "<target>%s</target>".
        "<where> ".
            "<clause> ".
                "<phrase> ".
                    "<status>processing</status> ".
                    "<macaddress>%s</macaddress> ".
                "</phrase> ".
            "</clause> ".
        "</where> ".
        "<update> ".
            "<result>%s</result> ".
        "</update> ".
        "</xml>", $source, "JOBDB", $macaddress, $header.$content);
    return $out_msg;
}


1;
