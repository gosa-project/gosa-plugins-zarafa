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
use Net::LDAP;
use Socket;
use Net::hostent;

BEGIN{}
END {}

my ($known_clients_file_name);
my ($server_activ, $server_ip, $server_mac_address, $server_port, $server_passwd, $max_clients, $ldap_uri, $ldap_base, $ldap_admin_dn, $ldap_admin_password);
my ($bus_activ, $bus_passwd, $bus_ip, $bus_port);
my $server;
my $network_interface;
my $no_bus;
my (@ldap_cfg, @pam_cfg, @nss_cfg, $goto_admin, $goto_secret);


my %cfg_defaults =
(
"server" =>
    {"server_activ" => [\$server_activ, "on"],
    "server_ip" => [\$server_ip, "0.0.0.0"],
    "server_mac_address" => [\$server_mac_address, ""],
    "server_port" => [\$server_port, "20081"],
    "server_passwd" => [\$server_passwd, ""],
    "max_clients" => [\$max_clients, 100],
    "ldap_uri" => [\$ldap_uri, ""],
    "ldap_base" => [\$ldap_base, ""],
    "ldap_admin_dn" => [\$ldap_admin_dn, ""],
    "ldap_admin_password" => [\$ldap_admin_password, ""],
    },
"bus" =>
    {"bus_activ" => [\$bus_activ, "on"],
    "bus_passwd" => [\$bus_passwd, ""],
    "bus_ip" => [\$bus_ip, ""],
    "bus_port" => [\$bus_port, "20080"],
    },
);

### START #####################################################################

# read configfile and import variables
&read_configfile();

# detect interfaces and mac address
$network_interface= &get_interface_for_ip($server_ip);
$server_mac_address= &get_mac($network_interface); 

# complete addresses
if( $server_ip eq "0.0.0.0" ) {
    $server_ip = "127.0.0.1";
}
my $server_address = "$server_ip:$server_port";
my $bus_address = "$bus_ip:$bus_port";

# create general settings for this module
my $xml = new XML::Simple();

# register at bus
if ($main::no_bus > 0) {
    $bus_activ = "off"
}
if($bus_activ eq "on") {
    &register_at_bus();
}

### functions #################################################################


sub get_module_info {
    my @info = ($server_address,
                $server_passwd,
                $server,
                $server_activ,
                "socket",
                );
    return \@info;
}



sub do_wake {
        my $host    = shift;
        my $ipaddr  = shift || '255.255.255.255';
        my $port    = getservbyname('discard', 'udp');

        my ($raddr, $them, $proto);
        my ($hwaddr, $hwaddr_re, $pkt);

        # get the hardware address (ethernet address)

        $hwaddr_re = join(':', ('[0-9A-Fa-f]{1,2}') x 6);
        if ($host =~ m/^$hwaddr_re$/) {
                $hwaddr = $host;
        } else {
                # $host is not a hardware address, try to resolve it
                my $ip_re = join('\.', ('([0-9]|[1-9][0-9]|1[0-9]{2}|2([0-4][0-9]|5[0-5]))') x 4);
                my $ip_addr;
                if ($host =~ m/^$ip_re$/) {
                        $ip_addr = $host;
                } else {
                        my $h;
                        unless ($h = gethost($host)) {
                                return undef;
                        }
                        $ip_addr = inet_ntoa($h->addr);
                }
        }

        # Generate magic sequence
        foreach (split /:/, $hwaddr) {
                $pkt .= chr(hex($_));
        }
        $pkt = chr(0xFF) x 6 . $pkt x 16;

        # Allocate socket and send packet

        $raddr = gethostbyname($ipaddr)->addr;
        $them = pack_sockaddr_in($port, $raddr);
        $proto = getprotobyname('udp');

        socket(S, AF_INET, SOCK_DGRAM, $proto) or die "socket : $!";
        setsockopt(S, SOL_SOCKET, SO_BROADCAST, 1) or die "setsockopt : $!";

        send(S, $pkt, 0, $them) or die "send : $!";
        close S;
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
				if(get_ip($if) eq $ip) {
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
			if ($server_mac_address and length($server_mac_address) > 0) {
				$result= $server_mac_address;
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
#         NAME:  get_ip 
#   PARAMETERS:  interface name (i.e. eth0)
#      RETURNS:  (ip address) 
#  DESCRIPTION:  Uses ioctl to get ip address directly from system.
#===============================================================================
sub get_ip {
	my $ifreq= shift;
	my $result= "";
	my $SIOCGIFADDR= 0x8915;       # man 2 ioctl_list
	my $proto= getprotobyname('ip');

	socket SOCKET, PF_INET, SOCK_DGRAM, $proto
		or die "socket: $!";

	if(ioctl SOCKET, $SIOCGIFADDR, $ifreq) {
		my ($if, $sin)    = unpack 'a16 a16', $ifreq;
		my ($port, $addr) = sockaddr_in $sin;
		my $ip            = inet_ntoa $addr;

		if ($ip && length($ip) > 0) {
			$result = $ip;
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
                                                    primkey=>'hostname',
                                                    hostname=>$bus_address,
                                                    status=>'bus',
                                                    hostkey=>$bus_passwd,
                                                    timestamp=>&get_time,
                                                } );
    my $msg_hash = &create_xml_hash("here_i_am", $server_address, $bus_address);
    my $answer = "";
    $answer = &send_msg_hash2address($msg_hash, $bus_address, $bus_passwd);
    if ($answer == 0) {
        &main::daemon_log("register at bus: $bus_address", 1);
    } else {
        &main::daemon_log("unable to send 'register'-msg to bus '$bus_address': $answer", 1);
    }
    return;
}


#===  FUNCTION  ================================================================
#         NAME:  process_incoming_msg
#   PARAMETERS:  crypted_msg - string - incoming crypted message
#      RETURNS:  nothing
#  DESCRIPTION:  handels the proceeded distribution to the appropriated functions
#===============================================================================
sub process_incoming_msg {
    my ($msg, $msg_hash) = @_ ;
    my $error = 0;
    my $host_name;
    my $host_key;
    my @out_msg_l;

    # process incoming msg
    my $header = @{$msg_hash->{header}}[0]; 
    my $source = @{$msg_hash->{source}}[0];
    my @target_l = @{$msg_hash->{target}};

    &main::daemon_log("SIPackages: msg to process: $header", 3);
    &main::daemon_log("$msg", 8);

    if( 0 == length @target_l){     
        &main::daemon_log("ERROR: no target specified for msg $header", 1);
        $error++;
    }

    if( 1 == length @target_l) {
        my $target = $target_l[0];
        if( $target eq $server_address ) {  
            if ($header eq 'new_passwd') {
		@out_msg_l = &new_passwd($msg_hash)
	    } elsif ($header eq 'here_i_am') {
		@out_msg_l = &here_i_am($msg_hash)
	    } elsif ($header eq 'who_has') {
		@out_msg_l = &who_has($msg_hash)
	    } elsif ($header eq 'who_has_i_do') {
		@out_msg_l = &who_has_i_do($msg_hash)
	    } elsif ($header eq 'got_ping') {
		@out_msg_l = &got_ping($msg_hash)
	    } elsif ($header eq 'get_load') {
		@out_msg_l = &execute_actions($msg_hash)
            } elsif ($header eq 'detected_hardware') {
		@out_msg_l = &process_detected_hardware($msg_hash)
	    } elsif ($header eq 'trigger_wake') {
		my $in_hash= &transform_msg2hash($msg);
		foreach (@{$in_hash->{macAddress}}){
	            &main::daemon_log("SIPackages: trigger wake for $_", 1);
		    do_wake($_);
		}

            } else {
                &main::daemon_log("ERROR: $header is an unknown core function", 1);
                $error++;
            }
        }
		else {
			&main::daemon_log("msg is not for gosa-si-server '$server_address', deliver it to target '$target'", 5);
			push(@out_msg_l, $msg);
		}
    }

    if( $error == 0) {
        if( 0 == @out_msg_l ) {
			push(@out_msg_l, $msg);
        }
    }
    
    return \@out_msg_l;
}


#===  FUNCTION  ================================================================
#         NAME:  got_ping
#   PARAMETERS:  msg_hash - hash - hash from function create_xml_hash
#      RETURNS:  nothing
#  DESCRIPTION:  process this incoming message
#===============================================================================
sub got_ping {
    my ($msg_hash) = @_;
    
    my $source = @{$msg_hash->{source}}[0];
    my $target = @{$msg_hash->{target}}[0];
    my $header = @{$msg_hash->{header}}[0];
    
    if(exists $main::known_daemons->{$source}) {
        &main::add_content2known_daemons(hostname=>$source, status=>$header);
    } else {
        &main::add_content2known_clients(hostname=>$source, status=>$header);
    }
    
    return;
}


#===  FUNCTION  ================================================================
#         NAME:  new_passwd
#   PARAMETERS:  msg_hash - ref - hash from function create_xml_hash
#      RETURNS:  nothing
#  DESCRIPTION:  process this incoming message
#===============================================================================
sub new_passwd {
    my ($msg_hash) = @_;
    my @out_msg_l;
    
    my $header = @{$msg_hash->{header}}[0];
    my $source_name = @{$msg_hash->{source}}[0];
    my $source_key = @{$msg_hash->{new_passwd}}[0];
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

        my $hash = &create_xml_hash("confirm_new_passwd", $server_address, $source_name);
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

            my $hash = &create_xml_hash("confirm_new_passwd", $server_address, $source_name);
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
    my ($msg_hash) = @_;
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
        &main::daemon_log("WARNING: $source is already known as a client", 1);
        &main::daemon_log("WARNING: values for $source are being overwritten", 1);   
        $nu_clients --;
    }

    # number of actual activ clients
    my $act_nu_clients = $nu_clients;

    &main::daemon_log("number of actual activ clients: $act_nu_clients", 5);
    &main::daemon_log("number of maximal allowed clients: $max_clients", 5);

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
    my $res = $main::known_clients_db->add_dbentry( {table=>'known_clients', 
                                                primkey=>'hostname',
                                                hostname=>$source,
                                                events=>$events,
                                                macaddress=>$mac_address,
                                                status=>'registered',
                                                hostkey=>$new_passwd,
                                                timestamp=>&get_time,
                                                } );

    if ($res != 0)  {
        &main::daemon_log("ERROR: cannot add entry to known_clients: $res");
        return;
    }
    
    # return acknowledgement to client
    $out_hash = &create_xml_hash("registered", $server_address, $source);
    my $register_out = &create_xml_string($out_hash);
    push(@out_msg_l, $register_out);

    # notify registered client to bus
    if( $bus_activ eq "on") {
        # fetch actual bus key
        my $sql_statement= "SELECT * FROM known_server WHERE status='bus'";
        my $query_res = $main::known_server_db->select_dbentry( $sql_statement );
        my $hostkey = $query_res->{1}->{'hostkey'};

        # send update msg to bus
        $out_hash = &create_xml_hash("new_client", $server_address, $bus_address, $source);
        my $new_client_out = &create_xml_string($out_hash);
        push(@out_msg_l, $new_client_out);
        &main::daemon_log("send bus msg that client '$source' has registerd at server '$server_address'", 3);
    }

    # give the new client his ldap config
    my $new_ldap_config_out = &new_ldap_config($source);
    if( $new_ldap_config_out ) {
        push(@out_msg_l, $new_ldap_config_out);
    }

	my $hardware_config_out = &hardware_config($source, $gotoHardwareChecksum);
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
        my $out_msg = &create_xml_hash("who_has_i_do", $server_address, $source, "mac_address");
        &add_content2xml_hash($out_msg, "mac_address", $search_element);
        &send_msg_hash2address($out_msg, $bus_address);
    }
    return;
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
#      RETURNS:  nothing
#  DESCRIPTION:  send to address the ldap configuration found for dn gotoLdapServer
#===============================================================================
sub new_ldap_config {
	my ($address) = @_ ;

	my $sql_statement= "SELECT * FROM known_clients WHERE hostname='$address'";
	my $res = $main::known_clients_db->select_dbentry( $sql_statement );

	# check hit
	my $hit_counter = keys %{$res};
	if( not $hit_counter == 1 ) {
		&main::daemon_log("ERROR: more or no hit found in known_clients_db by query by '$address'", 1);
	}

	my $macaddress = $res->{1}->{macaddress};
	my $hostkey = $res->{1}->{hostkey};

	if (not defined $macaddress) {
		&main::daemon_log("ERROR: no mac address found for client $address", 1);
		return;
	}

	# Build LDAP connection
	my $ldap = Net::LDAP->new($ldap_uri);
	if( not defined $ldap ) {
		&main::daemon_log("ERROR: cannot connect to ldap: $ldap_uri", 1);
		return;
	} 


	# Bind to a directory with dn and password
	my $mesg= $ldap->bind($ldap_admin_dn, $ldap_admin_password);

	# Perform search
	$mesg = $ldap->search( base   => $ldap_base,
		scope  => 'sub',
		attrs => ['dn', 'gotoLdapServer', 'gosaUnitTag'],
		filter => "(&(objectClass=GOhard)(macaddress=$macaddress))");
	#$mesg->code && die $mesg->error;
	if($mesg->code) {
		&main::daemon_log($mesg->error, 1);
		return;
	}

	# Sanity check
	if ($mesg->count != 1) {
		&main::daemon_log("WARNING: client mac address $macaddress not found/not unique in ldap search", 1);
		&main::daemon_log("\tbase: $ldap_base", 1);
		&main::daemon_log("\tscope: sub", 1);
		&main::daemon_log("\tattrs: dn, gotoLdapServer", 1);
		&main::daemon_log("\tfilter: (&(objectClass=GOhard)(macaddress=$macaddress))", 1);
		return;
	}

	my $entry= $mesg->entry(0);
	my $dn= $entry->dn;
	my @servers= $entry->get_value("gotoLdapServer");
	my $unit_tag= $entry->get_value("gosaUnitTag");
	my @ldap_uris;
	my $server;
	my $base;

	# Do we need to look at an object class?
	if (length(@servers) < 1){
		$mesg = $ldap->search( base   => $ldap_base,
			scope  => 'sub',
			attrs => ['dn', 'gotoLdapServer'],
			filter => "(&(objectClass=gosaGroupOfNames)(member=$dn))");
		#$mesg->code && die $mesg->error;
		if($mesg->code) {
			&main::daemon_log($mesg->error, 1);
			return;
		}

		# Sanity check
		if ($mesg->count != 1) {
			&main::daemon_log("WARNING: no LDAP information found for client mac $macaddress", 1);
			return;
		}

		$entry= $mesg->entry(0);
		$dn= $entry->dn;
		@servers= $entry->get_value("gotoLdapServer");
	}

	@servers= sort (@servers);

	foreach $server (@servers){
		$base= $server;
		$server =~ s%^[^:]+:[^:]+:(ldap.*://[^/]+)/.*$%$1%;
		$base =~ s%^[^:]+:[^:]+:ldap.*://[^/]+/(.*)$%$1%;
		push (@ldap_uris, $server);
	}

	# Assemble data package
	my %data = ( 'ldap_uri'  => \@ldap_uris, 'ldap_base' => $base,
		'ldap_cfg' => \@ldap_cfg, 'pam_cfg' => \@pam_cfg,'nss_cfg' => \@nss_cfg );

	# Need to append GOto settings?
	if (defined $goto_admin and defined $goto_secret){
		$data{'goto_admin'}= $goto_admin;
		$data{'goto_secret'}= $goto_secret;
	}

	# Append unit tag if needed
	if (defined $unit_tag){

		# Find admin base and department name
		$mesg = $ldap->search( base   => $ldap_base,
			scope  => 'sub',
			attrs => ['dn', 'ou'],
			filter => "(&(objectClass=gosaAdministrativeUnit)(gosaUnitTag=$unit_tag))");
		#$mesg->code && die $mesg->error;
		if($mesg->code) {
			&main::daemon_log($mesg->error, 1);
			return;
		}

		# Sanity check
		if ($mesg->count != 1) {
			&main::daemon_log("WARNING: cannot find administrative unit for client with tag $unit_tag", 1);
			return;
		}

		$entry= $mesg->entry(0);
		$data{'admin_base'}= $entry->dn;
		$data{'department'}= $entry->get_value("ou");

		# Append unit Tag
		$data{'unit_tag'}= $unit_tag;
	}

	# Unbind
	$mesg = $ldap->unbind;

	# Send information
	return send_msg("new_ldap_config", $server_address, $address, \%data);
}

sub process_detected_hardware {
	my $msg_hash = shift;


	return;
}
#===  FUNCTION  ================================================================
#         NAME:  hardware_config
#   PARAMETERS:  address - string - ip address and port of a host
#      RETURNS:  
#  DESCRIPTION:  
#===============================================================================
sub hardware_config {
    my ($address, $gotoHardwareChecksum, $detectedHardware) = @_ ;
    
    my $sql_statement= "SELECT * FROM known_clients WHERE hostname='$address'";
    my $res = $main::known_clients_db->select_dbentry( $sql_statement );

    # check hit
    my $hit_counter = keys %{$res};
    if( not $hit_counter == 1 ) {
        &main::daemon_log("ERROR: more or no hit found in known_clients_db by query by '$address'", 1);
    }

    my $macaddress = $res->{1}->{macaddress};
    my $hostkey = $res->{1}->{hostkey};

    if (not defined $macaddress) {
        &main::daemon_log("ERROR: no mac address found for client $address", 1);
        return;
    }

    # Build LDAP connection
    my $ldap = Net::LDAP->new($ldap_uri);
    if( not defined $ldap ) {
        &main::daemon_log("ERROR: cannot connect to ldap: $ldap_uri", 1);
        return;
    } 


    # Bind to a directory with dn and password
    my $mesg= $ldap->bind($ldap_admin_dn, $ldap_admin_password);

    # Perform search
    $mesg = $ldap->search( base   => $ldap_base,
		    scope  => 'sub',
		    attrs => ['dn', 'gotoHardwareChecksum'],
		    filter => "(&(objectClass=GOhard)(macaddress=$macaddress))");
		#$mesg->code && die $mesg->error;

    	#my $entry= $mesg->entry(0);
    	#my $dn= $entry->dn;
    	#my @servers= $entry->get_value("gotoHardwareChecksum");

    # Assemble data package
    my %data = ();

    # Need to append GOto settings?
    if (defined $goto_admin and defined $goto_secret){
	    $data{'goto_admin'}= $goto_admin;
	    $data{'goto_secret'}= $goto_secret;
    }

    # Unbind
    $mesg = $ldap->unbind;

	&main::daemon_log("Send detect_hardware message to $address", 4);
    # Send information
    return send_msg("detect_hardware", $server_address, $address, \%data);
}


#===  FUNCTION  ================================================================
#         NAME:  execute_actions
#   PARAMETERS:  msg_hash - hash - hash from function create_xml_hash
#      RETURNS:  nothing
#  DESCRIPTION:  invokes the script specified in msg_hash which is located under
#                /etc/gosad/actions
#===============================================================================
sub execute_actions {
    my ($msg_hash) = @_ ;
    my $configdir= '/etc/gosad/actions/';
    my $result;

    my $header = @{$msg_hash->{header}}[0];
    my $source = @{$msg_hash->{source}}[0];
    my $target = @{$msg_hash->{target}}[0];
 
    if((not defined $source)
            && (not defined $target)
            && (not defined $header)) {
        &main::daemon_log("ERROR: Entries missing in XML msg for gosad actions under /etc/gosad/actions");
    } else {
        my $parameters="";
        my @params = @{$msg_hash->{$header}};
        my $params = join(", ", @params);
        &main::daemon_log("execute_actions: got parameters: $params", 5);

        if (@params) {
            foreach my $param (@params) {
                my $param_value = (&get_content_from_xml_hash($msg_hash, $param))[0];
                &main::daemon_log("execute_actions: parameter -> value: $param -> $param_value", 7);
                $parameters.= " ".$param_value;
            }
        }

        my $cmd= $configdir.$header."$parameters";
        &main::daemon_log("execute_actions: executing cmd: $cmd", 7);
        $result= "";
        open(PIPE, "$cmd 2>&1 |");
        while(<PIPE>) {
            $result.=$_;
        }
        close(PIPE);
    }

    # process the event result


    return;
}


1;
