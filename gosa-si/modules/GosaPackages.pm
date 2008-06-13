package GosaPackages;

use Exporter;
@ISA = ("Exporter");

use strict;
use warnings;
use GOSA::GosaSupportDaemon;
use IO::Socket::INET;
use Socket;
use XML::Simple;
use File::Spec;
use Data::Dumper;
use GOSA::DBsqlite;
use MIME::Base64;

my $event_dir = "/usr/lib/gosa-si/server/GosaPackages";
use lib "/usr/lib/gosa-si/server/GosaPackages";

BEGIN{}
END{}

my ($server_ip, $server_port, $server_passwd, $max_clients);
my ($gosa_ip, $gosa_mac_address, $gosa_port, $gosa_passwd, $network_interface);
my ($job_queue_timeout, $job_queue_file_name);

my %cfg_defaults = (
"server" => {
    "ip" => [\$server_ip, "0.0.0.0"],
    "port" => [\$server_port, "20081"],
    "key" => [\$server_passwd, ""],
    "max-clients" => [\$max_clients, 100],
    },
"GOsaPackages" => {
    "ip" => [\$gosa_ip, "0.0.0.0"],
    "port" => [\$gosa_port, "20082"],
    "key" => [\$gosa_passwd, "none"],
    "job-queue" => [\$job_queue_file_name, '/var/lib/gosa-si/jobs.db'],
    },
);
 

## START ##########################

# read configfile and import variables
&read_configfile();
$network_interface= &get_interface_for_ip($server_ip);
$gosa_mac_address= &get_mac($network_interface);

# complete addresses
if( inet_aton($server_ip) ){ $server_ip = inet_ntoa(inet_aton($server_ip)); } 
our $server_address = "$server_ip:$server_port";
if( inet_aton($gosa_ip) ){ $gosa_ip = inet_ntoa(inet_aton($gosa_ip)); }
$main::gosa_address = "$gosa_ip:$gosa_port";

# create general settings for this module
#y $gosa_cipher = &create_ciphering($gosa_passwd);
my $xml = new XML::Simple();

# import local events
my ($error, $result, $event_hash) = &import_events($event_dir);
if ($error == 0) {
    foreach my $log_line (@$result) {
        &main::daemon_log("0 DEBUG: GosaPackages - $log_line", 7);
    }
} else {
    foreach my $log_line (@$result) {
        &main::daemon_log("0 ERROR: GosaPackages - $log_line", 1);
    }
}


## FUNCTIONS #################################################################

sub get_module_info {
    my @info = ($main::gosa_address,
                $gosa_passwd,
                );
    return \@info;
}


#===  FUNCTION  ================================================================
#         NAME:  read_configfile
#   PARAMETERS:  cfg_file - string -
#      RETURNS:  nothing
#  DESCRIPTION:  read cfg_file and set variables
#===============================================================================
sub read_configfile {
    my $cfg;
    if( defined( $main::cfg_file) && ( (-s $main::cfg_file) > 0 )) {
        if( -r $main::cfg_file ) {
            $cfg = Config::IniFiles->new( -file => $main::cfg_file );
        } else {
            print STDERR "Couldn't read config file!";
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

# moved to GosaSupportDaemon: 03-06-2008: rettenbe
#===  FUNCTION  ================================================================
#         NAME:  get_interface_for_ip
#   PARAMETERS:  ip address (i.e. 192.168.0.1)
#      RETURNS:  array: list of interfaces if ip=0.0.0.0, matching interface if found, undef else
#  DESCRIPTION:  Uses proc fs (/proc/net/dev) to get list of interfaces.
#===============================================================================
#sub get_interface_for_ip {
#    my $result;
#    my $ip= shift;
#    if ($ip && length($ip) > 0) {
#        my @ifs= &get_interfaces();
#        if($ip eq "0.0.0.0") {
#            $result = "all";
#        } else {
#            foreach (@ifs) {
#                my $if=$_;
#                if(get_ip($if) eq $ip) {
#                    $result = $if;
#                }
#            }       
#        }
#    }       
#    return $result;
#}

# moved to GosaSupportDaemon: 03-06-2008: rettenbe
#===  FUNCTION  ================================================================
#         NAME:  get_interfaces 
#   PARAMETERS:  none
#      RETURNS:  (list of interfaces) 
#  DESCRIPTION:  Uses proc fs (/proc/net/dev) to get list of interfaces.
#===============================================================================
#sub get_interfaces {
#        my @result;
#        my $PROC_NET_DEV= ('/proc/net/dev');
#
#        open(PROC_NET_DEV, "<$PROC_NET_DEV")
#                or die "Could not open $PROC_NET_DEV";
#
#        my @ifs = <PROC_NET_DEV>;
#
#        close(PROC_NET_DEV);
#
#        # Eat first two line
#        shift @ifs;
#        shift @ifs;
#
#        chomp @ifs;
#        foreach my $line(@ifs) {
#                my $if= (split /:/, $line)[0];
#                $if =~ s/^\s+//;
#                push @result, $if;
#        }
#
#        return @result;
#}

#===  FUNCTION  ================================================================
#         NAME:  get_mac 
#   PARAMETERS:  interface name (i.e. eth0)
#      RETURNS:  (mac address) 
#  DESCRIPTION:  Uses ioctl to get mac address directly from system.
#===============================================================================
sub get_mac {
    my $ifreq= shift;
    my $result;
    if ($ifreq && length($ifreq) > 0) { 
        if($ifreq eq "all") {
            $result = "00:00:00:00:00:00";
        } else {
            my $SIOCGIFHWADDR= 0x8927;     # man 2 ioctl_list

                # A configured MAC Address should always override a guessed value
                if ($gosa_mac_address and length($gosa_mac_address) > 0) {
                    $result= $gosa_mac_address;
                }

            socket SOCKET, PF_INET, SOCK_DGRAM, getprotobyname('ip')
                or die "socket: $!";

            if(ioctl SOCKET, $SIOCGIFHWADDR, $ifreq) {
                my ($if, $mac)= unpack 'h36 H12', $ifreq;

                if (length($mac) > 0) {
                    $mac=~ m/^([0-9a-f][0-9a-f])([0-9a-f][0-9a-f])([0-9a-f][0-9a-f])([0-9a-f][0-9a-f])([0-9a-f][0-9a-f])([0-9a-f][0-9a-f])$/;
                    $mac= sprintf("%s:%s:%s:%s:%s:%s", $1, $2, $3, $4, $5, $6);
                    $result = $mac;
                }
            }
        }
    }
    return $result;
}

# moved to GosaSupportDaemon: 03-06-2008: rettenbe
#===  FUNCTION  ================================================================
#         NAME:  get_ip 
#   PARAMETERS:  interface name (i.e. eth0)
#      RETURNS:  (ip address) 
#  DESCRIPTION:  Uses ioctl to get ip address directly from system.
#===============================================================================
#sub get_ip {
#        my $ifreq= shift;
#        my $result= "";
#        my $SIOCGIFADDR= 0x8915;       # man 2 ioctl_list
#        my $proto= getprotobyname('ip');
#
#        socket SOCKET, PF_INET, SOCK_DGRAM, $proto
#                or die "socket: $!";
#
#        if(ioctl SOCKET, $SIOCGIFADDR, $ifreq) {
#                my ($if, $sin)    = unpack 'a16 a16', $ifreq;
#                my ($port, $addr) = sockaddr_in $sin;
#                my $ip            = inet_ntoa $addr;
#
#                if ($ip && length($ip) > 0) {
#                        $result = $ip;
#                }
#        }
#
#        return $result;
#}


#===  FUNCTION  ================================================================
#         NAME:  process_incoming_msg
#   PARAMETERS:  crypted_msg - string - incoming crypted message
#      RETURNS:  nothing
#  DESCRIPTION:  handels the proceeded distribution to the appropriated functions
#===============================================================================
sub process_incoming_msg {
    my ($msg, $msg_hash, $session_id) = @_ ;
    my $header = @{$msg_hash->{header}}[0];
    my @msg_l;
    my @out_msg_l;

    &main::daemon_log("$session_id DEBUG: GosaPackages: msg to process '$header'", 7);
    
    if ($header =~ /^job_/) {
        @msg_l = &process_job_msg($msg, $msg_hash, $session_id);
    } 
    elsif ($header =~ /^gosa_/) {
        @msg_l = &process_gosa_msg($msg, $msg_hash, $session_id);
    } 
    else {
        &main::daemon_log("$session_id ERROR: $header is not a valid GosaPackage-header, need a 'job_' or a 'gosa_' prefix", 1);
    }

    foreach my $out_msg ( @msg_l ) {
        # substitute in all outgoing msg <source>GOSA</source> of <source>$server_address</source>
        $out_msg =~ s/<source>GOSA<\/source>/<source>$server_address<\/source>/g;
        $out_msg =~ s/<\/xml>/<session_id>$session_id<\/session_id><\/xml>/;
        if (defined $out_msg){
            push(@out_msg_l, $out_msg);
        }

    }

    return \@out_msg_l;
}


sub process_gosa_msg {
    my ($msg, $msg_hash, $session_id) = @_ ;
    my $out_msg;
    my @out_msg_l = ('nohandler');
    my $sql_events;

    my $header = @{$msg_hash->{'header'}}[0];
    my $target = @{$msg_hash->{'target'}}[0];
    $header =~ s/gosa_//;

    # check local installed events
    if( exists $event_hash->{$header} ) {
        # a event exists with the header as name
        &main::daemon_log("$session_id INFO: found event '$header' at event-module '".$event_hash->{$header}."'", 5);
        no strict 'refs';
        @out_msg_l = &{$event_hash->{$header}."::$header"}( $msg, $msg_hash, $session_id );

    # check client registered events
    } else {
        $sql_events = "SELECT * FROM $main::known_clients_tn WHERE ( (macaddress LIKE '$target') OR (hostname='$target') )"; 
        my $res = $main::known_clients_db->select_dbentry( $sql_events );
        my $l = keys(%$res);
        
        # set error if no or more than 1 hits are found for sql query
        if ( $l != 1) {
            @out_msg_l = ('knownclienterror');
        
        # found exact 1 hit in db
        } else {
            my $client_events = $res->{'1'}->{'events'};

            # client is registered for this event, deliver this message to client
            if ($client_events =~ /,$header,/) {
                $msg =~ s/<header>gosa_/<header>/;
                @out_msg_l = ( $msg );

            # client is not registered for this event, set error
            } else {
                @out_msg_l = ('noeventerror');
            }
        }
    }

    # if delivery not possible raise error and return 
    if (not defined $out_msg_l[0]) {
        @out_msg_l = ();
    } elsif ($out_msg_l[0] eq 'nohandler') {
        &main::daemon_log("$session_id ERROR: GosaPackages: no event handler or core function defined for '$header'", 1);
        @out_msg_l = ();
    } elsif ($out_msg_l[0] eq 'knownclienterror') {
        &main::daemon_log("$session_id ERROR: no event handler found for '$header', check client registration events!", 1);
        &main::daemon_log("$session_id ERROR: no or more than 1 hits are found at known_clients_db with sql query: '$sql_events'", 1);
        &main::daemon_log("$session_id ERROR: processing is aborted and message will not be forwarded", 1);
        @out_msg_l = ();
    } elsif ($out_msg_l[0] eq 'noeventerror') {
        &main::daemon_log("$session_id ERROR: client '$target' is not registered for event '$header', processing is aborted", 1); 
        @out_msg_l = ();
    }

    return @out_msg_l;
}


sub process_job_msg {
    my ($msg, $msg_hash, $session_id)= @_ ;    
    my $out_msg;
    my $error = 0;

    my $header = @{$msg_hash->{'header'}}[0];
    $header =~ s/job_//;
	my $target = @{$msg_hash->{'target'}}[0];
    
    # if no timestamp is specified, use 19700101000000
    my $timestamp = "19700101000000";
    if( exists $msg_hash->{'timestamp'} ) {
        $timestamp = @{$msg_hash->{'timestamp'}}[0];
    }

    #if no macaddress is specified, raise error 
    my $macaddress;
    if( exists $msg_hash->{'macaddress'} ) {
        $macaddress = @{$msg_hash->{'macaddress'}}[0];
    } elsif (@{$msg_hash->{'target'}}[0] =~ /^([0-9a-f]{2}:[0-9a-f]{2}:[0-9a-f]{2}:[0-9a-f]{2}:[0-9a-f]{2}:[0-9a-f]{2})$/i ) {
        $macaddress = $1;
    } else {
        $error ++;
        $out_msg = "<xml>".
            "<header>answer</header>".
            "<source>$server_address</source>".
            "<target>GOSA</target>".
            "<answer1>1</answer1>".
            "<error_string>no mac address specified, neither in target-tag nor in macaddres-tag</error_string>".
            "</xml>";
    }
    
	# if mac address is already known in ldap, set plain_name to 'cn'
    my $plain_name;
	my $ldap_handle = &main::get_ldap_handle($session_id); 
	if( not defined $ldap_handle ) {
		&main::daemon_log("$session_id ERROR: cannot connect to ldap", 1);
		$plain_name = "none"; 
		
	# try to fetch a 'real name'		
	} else {
		my $mesg = $ldap_handle->search(
						base => $main::ldap_base,
						scope => 'sub',
						attrs => ['cn'],
						filter => "(macAddress=$macaddress)");
		if($mesg->code) {
			&main::daemon_log($mesg->error, 1);
			$plain_name = "none";
		} else {
			my $entry= $mesg->entry(0);
			$plain_name = $entry->get_value("cn");
		}
	}

    if( $error == 0 ) {
        # add job to job queue
        my $func_dic = {table=>$main::job_queue_tn, 
            primkey=>['macaddress', 'headertag'],
            timestamp=>$timestamp,
            status=>'waiting', 
            result=>'none',
            progress=>'none',
            headertag=>$header, 
            targettag=>$target,
            xmlmessage=>$msg,
            macaddress=>$macaddress,
			plainname=>$plain_name,
        };
        my $res = $main::job_db->add_dbentry($func_dic);
        if (not $res == 0) {
            &main::daemon_log("$session_id ERROR: GosaPackages: process_job_msg: $res", 1);
        } else {
            &main::daemon_log("$session_id INFO: GosaPackages: $header job successfully added to job queue", 5);
        }
        $out_msg = "<xml><header>answer</header><source>$server_address</source><target>GOSA</target><answer1>$res</answer1></xml>";
    }
    
    my @out_msg_l = ( $out_msg );
    return @out_msg_l;
}

# vim:ts=4:shiftwidth:expandtab
1;
