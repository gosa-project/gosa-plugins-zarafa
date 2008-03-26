package SIPackages;

use Exporter;
@ISA = ("Exporter");

# Each module has to have a function 'process_incoming_msg'. This function works as a interface to gosa-sd and receives the msg hash from gosa-sd. 'process_incoming_function checks, wether it has a function to process the incoming msg and forward the msg to it. 


use strict;
use warnings;
use GOSA::GosaSupportDaemon;
use IO::Socket::INET;
use XML::Simple;
use Data::Dumper;
use NetAddr::IP;
use Net::LDAP;
use Socket;
use Net::hostent;

my $event_dir = "/usr/lib/gosa-si/server/events";
use lib "/usr/lib/gosa-si/server/events";

BEGIN{}
END {}

my ($server_ip, $server_port, $SIPackages_key, $max_clients, $ldap_uri, $ldap_base, $ldap_admin_dn, $ldap_admin_password, $server_interface);
my ($bus_activ, $bus_key, $bus_ip, $bus_port);
my $server;
my $event_hash;
my $network_interface;
my $no_bus;
my (@ldap_cfg, @pam_cfg, @nss_cfg, $goto_admin, $goto_secret);
my $mesg;

my %cfg_defaults = (
"bus" => {
    "activ" => [\$bus_activ, "on"],
    "key" => [\$bus_key, ""],
    "ip" => [\$bus_ip, ""],
    "port" => [\$bus_port, "20080"],
    },
"server" => {
    "ip" => [\$server_ip, "0.0.0.0"],
    "mac-address" => [\$main::server_mac_address, "00:00:00:00:00"],
    "port" => [\$server_port, "20081"],
    "ldap-uri" => [\$ldap_uri, ""],
    "ldap-base" => [\$ldap_base, ""],
    "ldap-admin-dn" => [\$ldap_admin_dn, ""],
    "ldap-admin-password" => [\$ldap_admin_password, ""],
    "max-clients" => [\$max_clients, 100],
    },
"SIPackages" => {
    "key" => [\$SIPackages_key, ""],
    },
);

### START #####################################################################

# read configfile and import variables
&read_configfile();


# if server_ip is not an ip address but a name
if( inet_aton($server_ip) ){ $server_ip = inet_ntoa(inet_aton($server_ip)); } 
$network_interface= &get_interface_for_ip($server_ip);
$main::server_mac_address= &get_mac($network_interface);

&import_events();

# Unit tag can be defined in config
if((not defined($main::gosa_unit_tag)) || length($main::gosa_unit_tag) == 0) {
	# Read gosaUnitTag from LDAP
        
    my $ldap_handle = &main::get_ldap_handle(); 
    if( defined($ldap_handle) ) {
		&main::daemon_log("INFO: Searching for servers gosaUnitTag with mac address $main::server_mac_address",5);
		# Perform search for Unit Tag
		$mesg = $ldap_handle->search(
			base   => $ldap_base,
			scope  => 'sub',
			attrs  => ['gosaUnitTag'],
			filter => "(macaddress=$main::server_mac_address)"
		);

		if ($mesg->count == 1) {
			my $entry= $mesg->entry(0);
			my $unit_tag= $entry->get_value("gosaUnitTag");
			$main::ldap_server_dn= $mesg->entry(0)->dn;
			if(defined($unit_tag) && length($unit_tag) > 0) {
				&main::daemon_log("INFO: Detected gosaUnitTag $unit_tag for creating entries", 5);
				$main::gosa_unit_tag= $unit_tag;
			}
		} else {
			# Perform another search for Unit Tag
			my $hostname= `hostname -f`;
			chomp($hostname);
			&main::daemon_log("INFO: Searching for servers gosaUnitTag with hostname $hostname",5);
			$mesg = $ldap_handle->search(
				base   => $ldap_base,
				scope  => 'sub',
				attrs  => ['gosaUnitTag'],
				filter => "(&(cn=$hostname)(objectClass=goServer))"
			);
			if ($mesg->count == 1) {
				my $entry= $mesg->entry(0);
				my $unit_tag= $entry->get_value("gosaUnitTag");
			        $main::ldap_server_dn= $mesg->entry(0)->dn;
				if(defined($unit_tag) && length($unit_tag) > 0) {
					&main::daemon_log("INFO: Detected gosaUnitTag $unit_tag for creating entries", 5);
					$main::gosa_unit_tag= $unit_tag;
				}
			} else {
				# Perform another search for Unit Tag
				$hostname= `hostname -s`;
				chomp($hostname);
				&main::daemon_log("INFO: Searching for servers gosaUnitTag with hostname $hostname",5);
				$mesg = $ldap_handle->search(
					base   => $ldap_base,
					scope  => 'sub',
					attrs  => ['gosaUnitTag'],
					filter => "(&(cn=$hostname)(objectClass=goServer))"
				);
				if ($mesg->count == 1) {
					my $entry= $mesg->entry(0);
					my $unit_tag= $entry->get_value("gosaUnitTag");
			        	$main::ldap_server_dn= $mesg->entry(0)->dn;
					if(defined($unit_tag) && length($unit_tag) > 0) {
						&main::daemon_log("INFO: Detected gosaUnitTag $unit_tag for creating entries", 5);
						$main::gosa_unit_tag= $unit_tag;
					}
				} else {
					&main::daemon_log("WARNING: No gosaUnitTag detected. Not using gosaUnitTag", 3);
				}
			}
		}
	} else {
		&main::daemon_log("INFO: Using gosaUnitTag from config-file: $main::gosa_unit_tag",5);
	}
}


my $server_address = "$server_ip:$server_port";
$main::server_address = $server_address;


if( inet_aton($bus_ip) ){ $bus_ip = inet_ntoa(inet_aton($bus_ip)); } 
######################################################
# to change
if( $bus_ip eq "127.0.1.1" ) { $bus_ip = "127.0.0.1" }
######################################################
my $bus_address = "$bus_ip:$bus_port";
$main::bus_address = $bus_address;

# create general settings for this module
my $xml = new XML::Simple();

# register at bus
if ($main::no_bus > 0) {
    $bus_activ = "off"
}
if($bus_activ eq "on") {
    &register_at_bus();
}

# add myself to known_server_db
my $res = $main::known_server_db->add_dbentry( {table=>'known_server',
        primkey=>['hostname'],
        hostname=>$server_address,
        status=>'myself',
        hostkey=>$SIPackages_key,
        timestamp=>&get_time,
        } );



### functions #################################################################


sub get_module_info {
    my @info = ($server_address,
                $SIPackages_key,
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
    if( defined( $main::cfg_file) && ( length($main::cfg_file) > 0 )) {
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

    # Read non predefined sections
    my $param;
    if ($cfg->SectionExists('ldap')){
		foreach $param ($cfg->Parameters('ldap')){
			push (@ldap_cfg, "$param ".$cfg->val('ldap', $param));
		}
    }
    if ($cfg->SectionExists('pam_ldap')){
		foreach $param ($cfg->Parameters('pam_ldap')){
			push (@pam_cfg, "$param ".$cfg->val('pam_ldap', $param));
		}
    }
    if ($cfg->SectionExists('nss_ldap')){
		foreach $param ($cfg->Parameters('nss_ldap')){
			push (@nss_cfg, "$param ".$cfg->val('nss_ldap', $param));
		}
    }
    if ($cfg->SectionExists('goto')){
    	$goto_admin= $cfg->val('goto', 'terminal_admin');
    	$goto_secret= $cfg->val('goto', 'terminal_secret');
    } else {
    	$goto_admin= undef;
    	$goto_secret= undef;
    }

}

#===  FUNCTION  ================================================================
#         NAME:  get_interface_for_ip
#   PARAMETERS:  ip address (i.e. 192.168.0.1)
#      RETURNS:  array: list of interfaces if ip=0.0.0.0, matching interface if found, undef else
#  DESCRIPTION:  Uses proc fs (/proc/net/dev) to get list of interfaces.
#===============================================================================
sub get_interface_for_ip {
	my $result;
	my $ip= shift;
	if ($ip && length($ip) > 0) {
		my @ifs= &get_interfaces();
		if($ip eq "0.0.0.0") {
			$result = "all";
		} else {
			foreach (@ifs) {
				my $if=$_;
				if(&main::get_ip($if) eq $ip) {
					$result = $if;
				}
			}	
		}
	}	
	return $result;
}

#===  FUNCTION  ================================================================
#         NAME:  get_interfaces 
#   PARAMETERS:  none
#      RETURNS:  (list of interfaces) 
#  DESCRIPTION:  Uses proc fs (/proc/net/dev) to get list of interfaces.
#===============================================================================
sub get_interfaces {
	my @result;
	my $PROC_NET_DEV= ('/proc/net/dev');

	open(PROC_NET_DEV, "<$PROC_NET_DEV")
		or die "Could not open $PROC_NET_DEV";

	my @ifs = <PROC_NET_DEV>;

	close(PROC_NET_DEV);

	# Eat first two line
	shift @ifs;
	shift @ifs;

	chomp @ifs;
	foreach my $line(@ifs) {
		my $if= (split /:/, $line)[0];
		$if =~ s/^\s+//;
		push @result, $if;
	}

	return @result;
}

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
			if ($main::server_mac_address and length($main::server_mac_address) > 0) {
				$result= $main::server_mac_address;
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


#===  FUNCTION  ================================================================
#         NAME:  register_at_bus
#   PARAMETERS:  nothing
#      RETURNS:  nothing
#  DESCRIPTION:  creates an entry in known_daemons and send a 'here_i_am' msg to bus
#===============================================================================
sub register_at_bus {

    # add bus to known_server_db
    my $res = $main::known_server_db->add_dbentry( {table=>'known_server',
                                                    primkey=>['hostname'],
                                                    hostname=>$bus_address,
                                                    status=>'bus',
                                                    hostkey=>$bus_key,
                                                    timestamp=>&get_time,
                                                } );
    my $msg_hash = &create_xml_hash("here_i_am", $server_address, $bus_address);
    my $msg = &create_xml_string($msg_hash);

    &main::send_msg_to_target($msg, $bus_address, $bus_key, "here_i_am");
    return $msg;
}


sub import_events {
    if (not -e $event_dir) {
        &main::daemon_log("S ERROR: cannot find directory or directory is not readable: $event_dir", 1);   
    }
    opendir (DIR, $event_dir) or die "ERROR while loading gosa-si-events from directory $event_dir : $!\n";

    while (defined (my $event = readdir (DIR))) {
        if( $event eq "." || $event eq ".." ) { next; }  
        if( $event eq "gosaTriggered.pm" ) { next; }    # only GOsa specific events

        eval{ require $event; };
        if( $@ ) {
            &main::daemon_log("import of event module '$event' failed", 1);
            &main::daemon_log("$@", 8);
            next;
        }

        $event =~ /(\S*?).pm$/;
        my $event_module = $1;
        my $events_l = eval( $1."::get_events()") ;
        foreach my $event_name (@{$events_l}) {
            $event_hash->{$event_name} = $event_module;
        }
        my $events_string = join( ", ", @{$events_l});
        &main::daemon_log("S DEBUG: SIPackages imported events $events_string", 8);
    }
}


#===  FUNCTION  ================================================================
#         NAME:  process_incoming_msg
#   PARAMETERS:  crypted_msg - string - incoming crypted message
#      RETURNS:  nothing
#  DESCRIPTION:  handels the proceeded distribution to the appropriated functions
#===============================================================================
sub process_incoming_msg {
    my ($msg, $msg_hash, $session_id) = @_ ;
    my $error = 0;
    my $host_name;
    my $host_key;
    my @out_msg_l = ("nohandler");

    # process incoming msg
    my $header = @{$msg_hash->{header}}[0]; 
    my @target_l = @{$msg_hash->{target}};

    # skip PREFIX
    $header =~ s/^CLMSG_//;

    &main::daemon_log("$session_id DEBUG: SIPackages: msg to process: $header", 7);

    if( 0 == length @target_l){     
        &main::daemon_log("$session_id ERROR: no target specified for msg $header", 1);
        $error++;
    }

    if( 1 == length @target_l) {
        my $target = $target_l[0];
		if(&server_matches($target)) {


            if ($header eq 'new_key') {
                @out_msg_l = &new_key($msg_hash)
            } elsif ($header eq 'here_i_am') {
                @out_msg_l = &here_i_am($msg, $msg_hash, $session_id)
            } else {
                if( exists $event_hash->{$header} ) {
                    # a event exists with the header as name
                    &main::daemon_log("$session_id INFO: found event '$header' at event-module '".$event_hash->{$header}."'", 5);
                    no strict 'refs';
                    @out_msg_l = &{$event_hash->{$header}."::$header"}($msg, $msg_hash, $session_id);
                }
            }

            # if delivery not possible raise error and return 
            if( not defined $out_msg_l[0] ) {
                @out_msg_l = ();
            } elsif( $out_msg_l[0] eq 'nohandler') {
                &main::daemon_log("$session_id ERROR: SIPackages: no event handler or core function defined for '$header'", 1);
                @out_msg_l = ();
            } 

        }
		else {
			&main::daemon_log("INFO: msg is not for gosa-si-server '$server_address', deliver it to target '$target'", 5);
			push(@out_msg_l, $msg);
		}
    }

    return \@out_msg_l;
}


#===  FUNCTION  ================================================================
#         NAME:  new_passwd
#   PARAMETERS:  msg_hash - ref - hash from function create_xml_hash
#      RETURNS:  nothing
#  DESCRIPTION:  process this incoming message
#===============================================================================
sub new_key {
    my ($msg_hash) = @_;
    my @out_msg_l;
    
    my $header = @{$msg_hash->{header}}[0];
    my $source_name = @{$msg_hash->{source}}[0];
    my $source_key = @{$msg_hash->{new_key}}[0];
    my $query_res;

    # check known_clients_db
    my $sql_statement = "SELECT * FROM known_clients WHERE hostname='$source_name'";
    $query_res = $main::known_clients_db->select_dbentry( $sql_statement );
    if( 1 == keys %{$query_res} ) {
        my $act_time = &get_time;
        my $sql_statement= "UPDATE known_clients ".
            "SET hostkey='$source_key', timestamp='$act_time' ".
            "WHERE hostname='$source_name'";
        my $res = $main::known_clients_db->update_dbentry( $sql_statement );
        my $hash = &create_xml_hash("confirm_new_key", $server_address, $source_name);
        my $out_msg = &create_xml_string($hash);
        push(@out_msg_l, $out_msg);
    }

    # only do if host still not found
    if( 0 == @out_msg_l ) {
        # check known_server_db
        $sql_statement = "SELECT * FROM known_server WHERE hostname='$source_name'";
        $query_res = $main::known_server_db->select_dbentry( $sql_statement );
        if( 1 == keys %{$query_res} ) {
            my $act_time = &get_time;
            my $sql_statement= "UPDATE known_server ".
                "SET hostkey='$source_key', timestamp='$act_time' ".
                "WHERE hostname='$source_name'";
            my $res = $main::known_server_db->update_dbentry( $sql_statement );

            my $hash = &create_xml_hash("confirm_new_key", $server_address, $source_name);
            my $out_msg = &create_xml_string($hash);
            push(@out_msg_l, $out_msg);
        }
    }

    return @out_msg_l;
}


#===  FUNCTION  ================================================================
#         NAME:  here_i_am
#   PARAMETERS:  msg_hash - hash - hash from function create_xml_hash
#      RETURNS:  nothing
#  DESCRIPTION:  process this incoming message
#===============================================================================
sub here_i_am {
    my ($msg, $msg_hash, $session_id) = @_;
    my @out_msg_l;
    my $out_hash;

    my $source = @{$msg_hash->{source}}[0];
    my $mac_address = @{$msg_hash->{mac_address}}[0];
	my $gotoHardwareChecksum = @{$msg_hash->{gotoHardwareChecksum}}[0];

    # number of known clients
    my $nu_clients= $main::known_clients_db->count_dbentries('known_clients');

    # check wether client address or mac address is already known
    my $sql_statement= "SELECT * FROM known_clients WHERE hostname='$source'";
    my $db_res= $main::known_clients_db->select_dbentry( $sql_statement );
    
    if ( 1 == keys %{$db_res} ) {
        &main::daemon_log("$session_id WARNING: $source is already known as a client", 1);
        &main::daemon_log("$session_id WARNING: values for $source are being overwritten", 1);   
        $nu_clients --;
    }

    # number of actual activ clients
    my $act_nu_clients = $nu_clients;

    &main::daemon_log("$session_id INFO: number of actual activ clients: $act_nu_clients", 5);
    &main::daemon_log("$session_id INFO: number of maximal allowed clients: $max_clients", 5);

    if($max_clients <= $act_nu_clients) {
        my $out_hash = &create_xml_hash("denied", $server_address, $source);
        &add_content2xml_hash($out_hash, "denied", "I_cannot_take_any_more_clients!");
        my $passwd = @{$msg_hash->{new_passwd}}[0]; 
        &send_msg_hash2address($out_hash, $source, $passwd);
        return;
    }
    
    # new client accepted
    my $new_passwd = @{$msg_hash->{new_passwd}}[0];

    # create entry in known_clients
    my $events = @{$msg_hash->{events}}[0];
    

    # add entry to known_clients_db
    my $act_timestamp = &get_time;
    my $res = $main::known_clients_db->add_dbentry( {table=>'known_clients', 
                                                primkey=>['hostname'],
                                                hostname=>$source,
                                                events=>$events,
                                                macaddress=>$mac_address,
                                                status=>'registered',
                                                hostkey=>$new_passwd,
                                                timestamp=>$act_timestamp,
                                                } );

    if ($res != 0)  {
        &main::daemon_log("$session_id ERROR: cannot add entry to known_clients: $res");
        return;
    }
    
    # return acknowledgement to client
    $out_hash = &create_xml_hash("registered", $server_address, $source);

    # notify registered client to bus
    if( $bus_activ eq "on") {
        # fetch actual bus key
        my $sql_statement= "SELECT * FROM known_server WHERE status='bus'";
        my $query_res = $main::known_server_db->select_dbentry( $sql_statement );
        my $hostkey = $query_res->{1}->{'hostkey'};

        # send update msg to bus
        $out_hash = &create_xml_hash("new_client", $server_address, $bus_address, $source);
        &add_content2xml_hash($out_hash, "macaddress", $mac_address);
        &add_content2xml_hash($out_hash, "timestamp", $act_timestamp);
        my $new_client_out = &create_xml_string($out_hash);
        push(@out_msg_l, $new_client_out);
        &main::daemon_log("$session_id INFO: send bus msg that client '$source' has registered at server '$server_address'", 5);
    }

    # give the new client his ldap config
    # Workaround: Send within the registration response, if the client will get an ldap config later
	my $new_ldap_config_out = &new_ldap_config($source, $session_id);
	if($new_ldap_config_out && (!($new_ldap_config_out =~ /error/))) {
		&add_content2xml_hash($out_hash, "ldap_available", "true");
	} elsif($new_ldap_config_out && $new_ldap_config_out =~ /error/){
		&add_content2xml_hash($out_hash, "error", $new_ldap_config_out);

		my $sql_statement = "UPDATE $main::job_queue_tn ".
		"SET status='error', result='$new_ldap_config_out' ".
		"WHERE status='processing' AND macaddress LIKE '$mac_address'";
		my $res = $main::job_db->update_dbentry($sql_statement);
		&main::daemon_log("$session_id DEBUG: $sql_statement RESULT: $res", 7);         
	}
    my $register_out = &create_xml_string($out_hash);
    push(@out_msg_l, $register_out);

    # Really send the ldap config
    if( $new_ldap_config_out && (!($new_ldap_config_out =~ /error/))) {
            push(@out_msg_l, $new_ldap_config_out);
    }

	my $hardware_config_out = &hardware_config($msg, $msg_hash, $session_id);
	if( $hardware_config_out ) {
		push(@out_msg_l, $hardware_config_out);
	}

    return @out_msg_l;
}


#===  FUNCTION  ================================================================
#         NAME:  who_has
#   PARAMETERS:  msg_hash - hash - hash from function create_xml_hash
#      RETURNS:  nothing 
#  DESCRIPTION:  process this incoming message
#===============================================================================
sub who_has {
    my ($msg_hash) = @_ ;
    my @out_msg_l;
    
    # what is your search pattern
    my $search_pattern = @{$msg_hash->{who_has}}[0];
    my $search_element = @{$msg_hash->{$search_pattern}}[0];
    &main::daemon_log("who_has-msg looking for $search_pattern $search_element", 7);

    # scanning known_clients for search_pattern
    my @host_addresses = keys %$main::known_clients;
    my $known_clients_entries = length @host_addresses;
    my $host_address;
    foreach my $host (@host_addresses) {
        my $client_element = $main::known_clients->{$host}->{$search_pattern};
        if ($search_element eq $client_element) {
            $host_address = $host;
            last;
        }
    }
        
    # search was successful
    if (defined $host_address) {
        my $source = @{$msg_hash->{source}}[0];
        my $out_hash = &create_xml_hash("who_has_i_do", $server_address, $source, "mac_address");
        &add_content2xml_hash($out_hash, "mac_address", $search_element);
        my $out_msg = &create_xml_string($out_hash);
        push(@out_msg_l, $out_msg);
    }
    return @out_msg_l;
}


sub who_has_i_do {
    my ($msg_hash) = @_ ;
    my $header = @{$msg_hash->{header}}[0];
    my $source = @{$msg_hash->{source}}[0];
    my $search_param = @{$msg_hash->{$header}}[0];
    my $search_value = @{$msg_hash->{$search_param}}[0];
    print "\ngot msg $header:\nserver $source has client with $search_param $search_value\n";
}


#===  FUNCTION  ================================================================
#         NAME:  new_ldap_config
#   PARAMETERS:  address - string - ip address and port of a host
#      RETURNS:  gosa-si conform message
#  DESCRIPTION:  send to address the ldap configuration found for dn gotoLdapServer
#===============================================================================
sub new_ldap_config {
	my ($address, $session_id) = @_ ;

	my $sql_statement= "SELECT * FROM known_clients WHERE hostname='$address' OR macaddress LIKE '$address'";
	my $res = $main::known_clients_db->select_dbentry( $sql_statement );

	# check hit
	my $hit_counter = keys %{$res};
	if( not $hit_counter == 1 ) {
		&main::daemon_log("$session_id ERROR: more or no hit found in known_clients_db by query by '$address'", 1);
	}

    $address = $res->{1}->{hostname};
	my $macaddress = $res->{1}->{macaddress};
	my $hostkey = $res->{1}->{hostkey};

	if (not defined $macaddress) {
		&main::daemon_log("$session_id ERROR: no mac address found for client $address", 1);
		return;
	}

	# Build LDAP connection
    my $ldap_handle = &main::get_ldap_handle($session_id);
	if( not defined $ldap_handle ) {
		&main::daemon_log("$session_id ERROR: cannot connect to ldap: $ldap_uri", 1);
		return;
	} 

	# Perform search
    $mesg = $ldap_handle->search( base   => $ldap_base,
		scope  => 'sub',
		attrs => ['dn', 'gotoLdapServer', 'gosaUnitTag', 'FAIclass'],
		filter => "(&(objectClass=GOhard)(macaddress=$macaddress)(gotoLdapServer=*))");
	#$mesg->code && die $mesg->error;
	if($mesg->code) {
		&main::daemon_log("$session_id ".$mesg->error, 1);
		return;
	}

	# Sanity check
	if ($mesg->count != 1) {
		&main::daemon_log("$session_id WARNING: client with mac address $macaddress not found/unique/active - not sending ldap config".
                "\n\tbase: $ldap_base".
                "\n\tscope: sub".
                "\n\tattrs: dn, gotoLdapServer".
                "\n\tfilter: (&(objectClass=GOhard)(macaddress=$macaddress)(gotoLdapServer=*))", 1);
		return;
	}

	my $entry= $mesg->entry(0);
	my $dn= $entry->dn;
	my @servers= $entry->get_value("gotoLdapServer");
	my $unit_tag= $entry->get_value("gosaUnitTag");
	my @ldap_uris;
	my $server;
	my $base;
	my $release;

	# Fill release if available
	my $FAIclass= $entry->get_value("FAIclass");
	if (defined $FAIclass && $FAIclass =~ /^.* :([A-Za-z0-9\/.]+).*$/) {
		$release= $1;
	}

	# Do we need to look at an object class?
	if (length(@servers) < 1){
        $mesg = $ldap_handle->search( base   => $ldap_base,
			scope  => 'sub',
			attrs => ['dn', 'gotoLdapServer', 'FAIclass'],
			filter => "(&(objectClass=gosaGroupOfNames)(member=$dn))");
		#$mesg->code && die $mesg->error;
		if($mesg->code) {
			&main::daemon_log("$session_id ".$mesg->error, 1);
			return;
		}

		# Sanity check
		if ($mesg->count != 1) {
			&main::daemon_log("$session_id WARNING: no LDAP information found for client mac $macaddress", 1);
			return;
		}

		$entry= $mesg->entry(0);
		$dn= $entry->dn;
		@servers= $entry->get_value("gotoLdapServer");

		if (not defined $release){
			$FAIclass= $entry->get_value("FAIclass");
			if (defined $FAIclass && $FAIclass =~ /^.* :([A-Za-z0-9\/.]+).*$/) {
				$release= $1;
			}
		}
	}

	@servers= sort (@servers);

	foreach $server (@servers){
                # Conversation for backward compatibility
                if (not $server =~ /^\d+:[^:]+:ldap[^:]*:\/\// ) {
                    if ($server =~ /^([^:]+):([^:]+)$/ ) {
                      $server= "1:dummy:ldap://$1/$2";
                    } elsif ($server =~ /^(\d+):([^:]+):(.*)$/ ) {
                      $server= "$1:dummy:ldap://$2/$3";
                    }
                }

                $base= $server;
                $server =~ s%^[^:]+:[^:]+:(ldap.*://[^/]+)/.*$%$1%;
                $base =~ s%^[^:]+:[^:]+:ldap.*://[^/]+/(.*)$%$1%;
                push (@ldap_uris, $server);
	}

	# Assemble data package
	my %data = ( 'ldap_uri'  => \@ldap_uris, 'ldap_base' => $base,
		'ldap_cfg' => \@ldap_cfg, 'pam_cfg' => \@pam_cfg,'nss_cfg' => \@nss_cfg );
	if (defined $release){
		$data{'release'}= $release;
	}

	# Need to append GOto settings?
	if (defined $goto_admin and defined $goto_secret){
		$data{'goto_admin'}= $goto_admin;
		$data{'goto_secret'}= $goto_secret;
	}

	# Append unit tag if needed
	if (defined $unit_tag){

		# Find admin base and department name
		$mesg = $ldap_handle->search( base   => $ldap_base,
			scope  => 'sub',
			attrs => ['dn', 'ou'],
			filter => "(&(objectClass=gosaAdministrativeUnit)(gosaUnitTag=$unit_tag))");
		#$mesg->code && die $mesg->error;
		if($mesg->code) {
			&main::daemon_log($mesg->error, 1);
			return "error-unit-tag-count-0";
		}

		# Sanity check
		if ($mesg->count != 1) {
			&main::daemon_log("WARNING: cannot find administrative unit for client with tag $unit_tag", 1);
			return "error-unit-tag-count-".$mesg->count;
		}

		$entry= $mesg->entry(0);
		$data{'admin_base'}= $entry->dn;
		$data{'department'}= $entry->get_value("ou");

		# Append unit Tag
		$data{'unit_tag'}= $unit_tag;
	}

	# Send information
	return &build_msg("new_ldap_config", $server_address, $address, \%data);
}


#===  FUNCTION  ================================================================
#         NAME:  hardware_config
#   PARAMETERS:  address - string - ip address and port of a host
#      RETURNS:  
#  DESCRIPTION:  
#===============================================================================
sub hardware_config {
	my ($msg, $msg_hash, $session_id) = @_ ;
	my $address = @{$msg_hash->{source}}[0];
	my $header = @{$msg_hash->{header}}[0];
	my $gotoHardwareChecksum = @{$msg_hash->{gotoHardwareChecksum}}[0];

	my $sql_statement= "SELECT * FROM known_clients WHERE hostname='$address'";
	my $res = $main::known_clients_db->select_dbentry( $sql_statement );

	# check hit
	my $hit_counter = keys %{$res};
	if( not $hit_counter == 1 ) {
		&main::daemon_log("$session_id ERROR: more or no hit found in known_clients_db by query by '$address'", 1);
	}
	my $macaddress = $res->{1}->{macaddress};
	my $hostkey = $res->{1}->{hostkey};

	if (not defined $macaddress) {
		&main::daemon_log("$session_id ERROR: no mac address found for client $address", 1);
		return;
	}

	# Build LDAP connection
    my $ldap_handle = &main::get_ldap_handle($session_id);
	if( not defined $ldap_handle ) {
		&main::daemon_log("$session_id ERROR: cannot connect to ldap: $ldap_uri", 1);
		return;
	} 

	# Perform search
	$mesg = $ldap_handle->search(
		base   => $ldap_base,
		scope  => 'sub',
		filter => "(&(objectClass=GOhard)(|(macAddress=$macaddress)(dhcpHWaddress=ethernet $macaddress)))"
	);

	if($mesg->count() == 0) {
		&main::daemon_log("Host was not found in LDAP!", 1);

		# set status = hardware_detection at jobqueue if entry exists
		my $func_dic = {table=>$main::job_queue_tn,
				primkey=>['id'],
				timestamp=>&get_time,
				status=>'processing',
				result=>'none',
				progress=>'hardware-detection',
				headertag=>'trigger_action_reinstall',
				targettag=>$address,
				xmlmessage=>'none',
				macaddress=>$macaddress,
		};
		my $hd_res = $main::job_db->add_dbentry($func_dic);
		&main::daemon_log("$session_id INFO: add '$macaddress' to job queue as an installing job", 5);
	
	} else {
		my $entry= $mesg->entry(0);
		my $dn= $entry->dn;
		if (defined($entry->get_value("gotoHardwareChecksum"))) {
			if (! $entry->get_value("gotoHardwareChecksum") eq $gotoHardwareChecksum) {
				$entry->replace(gotoHardwareChecksum => $gotoHardwareChecksum);
				if($entry->update($ldap_handle)) {
					&main::daemon_log("$session_id INFO: Hardware changed! Detection triggered.", 5);
				}
			} else {
				# Nothing to do
				return;
			}
		} 
	} 

	# Assemble data package
	my %data = ();

	# Need to append GOto settings?
	if (defined $goto_admin and defined $goto_secret){
		$data{'goto_admin'}= $goto_admin;
		$data{'goto_secret'}= $goto_secret;
	}

	# Send information
	return &build_msg("detect_hardware", $server_address, $address, \%data);
}

sub server_matches {
	my $target = shift;
	my $target_ip = sprintf("%s", $target =~ /^([0-9\.]*?):.*$/);
	my $result = 0;

	if($server_ip eq $target_ip) {
		$result= 1;
	} elsif ($target_ip eq "0.0.0.0") {
		$result= 1;
	} elsif ($server_ip eq "0.0.0.0") {	
		if ($target_ip eq "127.0.0.1") {
			$result= 1;
		} else {
			my $PROC_NET_ROUTE= ('/proc/net/route');

			open(PROC_NET_ROUTE, "<$PROC_NET_ROUTE")
				or die "Could not open $PROC_NET_ROUTE";

			my @ifs = <PROC_NET_ROUTE>;

			close(PROC_NET_ROUTE);

			# Eat header line
			shift @ifs;
			chomp @ifs;
			foreach my $line(@ifs) {
				my ($Iface,$Destination,$Gateway,$Flags,$RefCnt,$Use,$Metric,$Mask,$MTU,$Window,$IRTT)=split(/\s/, $line);
				my $destination;
				my $mask;
				my ($d,$c,$b,$a)=unpack('a2 a2 a2 a2', $Destination);
				$destination= sprintf("%d.%d.%d.%d", hex($a), hex($b), hex($c), hex($d));
				($d,$c,$b,$a)=unpack('a2 a2 a2 a2', $Mask);
				$mask= sprintf("%d.%d.%d.%d", hex($a), hex($b), hex($c), hex($d));
				if(new NetAddr::IP($target_ip)->within(new NetAddr::IP($destination, $mask))) {
					# destination matches route, save mac and exit
					$result= 1;
					last;
				}
			}
		}
	} else {
		&main::daemon_log("Target ip $target_ip does not match Server ip $server_ip",1);
	}

	return $result;
}

1;
