package ServerPackages;

use Exporter;
@ISA = ("Exporter");

# Each module has to have a function 'process_incoming_msg'. This function works as a interface to gosa-sd and recieves the msg hash from gosa-sd. 'process_incoming_function checks, wether it has a function to process the incoming msg and forward the msg to it. 


use strict;
use warnings;
use GOSA::GosaSupportDaemon;
use IO::Socket::INET;
use XML::Simple;
use Data::Dumper;
use Net::LDAP;
use Socket qw/PF_INET SOCK_DGRAM inet_ntoa sockaddr_in/;

BEGIN{}
END {}

my ($known_clients_file_name);
my ($server_activ, $server_ip, $server_mac_address, $server_port, $server_passwd, $max_clients, $ldap_uri, $ldap_base, $ldap_admin_dn, $ldap_admin_password);
my ($bus_activ, $bus_passwd, $bus_ip, $bus_port);
my $server;
my @network_interfaces;
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

&main::daemon_log("server ip address detected: $server_ip", 1);
&main::daemon_log("server mac address detected: $server_mac_address", 1);

# complete addresses
my $server_address = "$server_ip:$server_port";
my $bus_address = "$bus_ip:$bus_port";

# create general settings for this module
my $xml = new XML::Simple();

# open server socket
if($server_activ eq "on"){
    &main::daemon_log(" ", 1);
    $server = IO::Socket::INET->new(LocalPort => $server_port,
            Type => SOCK_STREAM,
            Reuse => 1,
            Listen => 20,
            ); 
    if(not defined $server){
        &main::daemon_log("cannot be a tcp server at $server_port : $@");
    } else {
        &main::daemon_log("start server: $server_address", 1);
    }
}

# TODO
# füge den server selbst zu known_server hinzu, msgs können nämlich auch von sich selbst kommen (gosa!!!)


# register at bus
if ($main::no_bus > 0) {
    $bus_activ = "off"
}
if($bus_activ eq "on") {
    &main::daemon_log(" ", 1);
    &register_at_bus();
}

### functions #################################################################

#sub get_module_tags {
#    
#    # lese config file aus dort gibt es eine section Basic
#    # dort stehen drei packettypen, für die sich das modul anmelden kann, gosa-admin-packages, 
#    #   server-packages, client-packages
#    my %tag_hash = (gosa_admin_packages => "yes", 
#                    server_packages => "yes", 
#                    client_packages => "yes",
#                    );
#    return \%tag_hash;
#}


sub get_module_info {
    my @info = ($server_address,
                $server_passwd,
                $server,
                $server_activ,
                "socket",
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
			# TODO
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
		my $SIOCGIFHWADDR= 0x8927;     # man 2 ioctl_list

		# A configured MAC Address should always override a guessed value
		if ($server_mac_address and length($server_mac_address) > 0) {
			return $server_mac_address;
		}

		socket SOCKET, PF_INET, SOCK_DGRAM, getprotobyname('ip')
			or die "socket: $!";

		if(ioctl SOCKET, $SIOCGIFHWADDR, $ifreq) {
			my ($if, $mac)= unpack 'h36 H12', $ifreq;

			if (length($mac) > 0) {
				$result = $mac
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
#         NAME:  open_socket
#   PARAMETERS:  PeerAddr string something like 192.168.1.1 or 192.168.1.1:10000
#                [PeerPort] string necessary if port not appended by PeerAddr
#      RETURNS:  socket IO::Socket::INET
#  DESCRIPTION:  open a socket to PeerAddr
#===============================================================================
sub open_socket {
    my ($PeerAddr, $PeerPort) = @_ ;
    if(defined($PeerPort)){
        $PeerAddr = $PeerAddr.":".$PeerPort;
    }
    my $socket;
    $socket = new IO::Socket::INET(PeerAddr => $PeerAddr ,
            Porto => "tcp" ,
            Type => SOCK_STREAM,
            Timeout => 5,
            );
    if(not defined $socket) {
        return;
    }
    &main::daemon_log("open_socket to: $PeerAddr", 7);
    return $socket;
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
#    if ($res == 3) {
#        my $update_hash = { table=>'known_server' };
#        $update_hash->{where} = [ { hostname=>[$bus_address] } ];
#        $update_hash->{update} = [ { 
#            hostkey=>[$bus_passwd],
#            timestamp=>[&get_time],
#        } ];
#        $res = $main::known_server_db->update_dbentry( $update_hash );
#    }

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
    my ($crypted_msg) = @_ ;
    if(not defined $crypted_msg) {
        &main::daemon_log("function 'process_incoming_msg': got no msg", 7);
    }

    &main::daemon_log("ServerPackages: incoming msg: \n$crypted_msg", 7);

    $crypted_msg =~ /^([\s\S]*?)\.(\d{1,3}?)\.(\d{1,3}?)\.(\d{1,3}?)\.(\d{1,3}?)$/;
    $crypted_msg = $1;
    my $host = sprintf("%s.%s.%s.%s", $2, $3, $4, $5);

    my $msg;
    my $msg_hash;
    my $host_name;
    my $host_key;

    # check wether incoming msg is a new msg
    $host_name = $server_address;
    $host_key = $server_passwd;
    &main::daemon_log("ServerPackage: host_name: $host_name", 7);
    &main::daemon_log("ServerPackage: host_key: $host_key", 7);
    eval{
        my $key_cipher = &create_ciphering($host_key);
        $msg = &decrypt_msg($crypted_msg, $key_cipher);
        $msg_hash = &transform_msg2hash($msg);
    };
    if($@) {
        &main::daemon_log("ServerPackage: deciphering raise error", 7);
        &main::daemon_log("$@", 8);
        $msg = undef;
        $msg_hash = undef;
        $host_name = undef;
        $host_key = undef;
    } 

    # check wether incoming msg is from a known_server
    if( not defined $msg ) {
        my $query_res = $main::known_server_db->select_dbentry( {table=>'known_server'} ); 
        while( my ($hit_num, $hit) = each %{ $query_res } ) {  
            $host_name = $hit->{hostname};
            if( not $host_name =~ "^$host") {
                next;
            }
            $host_key = $hit->{hostkey};
            &main::daemon_log("ServerPackage: host_name: $host_name", 7);
            &main::daemon_log("ServerPackage: host_key: $host_key", 7);
            eval{
                my $key_cipher = &create_ciphering($host_key);
                $msg = &decrypt_msg($crypted_msg, $key_cipher);
                $msg_hash = &transform_msg2hash($msg);
            };
            if($@) {
                &main::daemon_log("ServerPackage: deciphering raise error", 7);
                &main::daemon_log("$@", 8);
                $msg = undef;
                $msg_hash = undef;
                $host_name = undef;
                $host_key = undef;
            } else {
                last;
            }
        }
    }

    # check wether incoming msg is from a known_client
    if( not defined $msg ) {
        my $query_res = $main::known_clients_db->select_dbentry( {table=>'known_clients'} ); 
        while( my ($hit_num, $hit) = each %{ $query_res } ) {    
            $host_name = $hit->{hostname};
            if( not $host_name =~ "^$host") {
                next;
            }
            $host_key = $hit->{hostkey};
            &main::daemon_log("ServerPackage: host_name: $host_name", 7);
            &main::daemon_log("ServerPackage: host_key: $host_key", 7);
            eval{
                my $key_cipher = &create_ciphering($host_key);
                $msg = &decrypt_msg($crypted_msg, $key_cipher);
                $msg_hash = &transform_msg2hash($msg);
            };
            if($@) {
                &main::daemon_log("ServerPackage: deciphering raise error", 7);
                &main::daemon_log("$@", 8);
                $msg = undef;
                $msg_hash = undef;
                $host_name = undef;
                $host_key = undef;
            } else {
                last;
            }
        }
    }

    if( not defined $msg ) {
        &main::daemon_log("WARNING: ServerPackage do not understand the message:", 5);
        &main::daemon_log("$@", 7);
        return;
    }

    # process incoming msg
    my $header = @{$msg_hash->{header}}[0]; 
    my $source = @{$msg_hash->{source}}[0];

    &main::daemon_log("recieve '$header' at ServerPackages from $host", 1);
    &main::daemon_log("ServerPackages: msg to process: \n$msg", 5);

    my @targets = @{$msg_hash->{target}};
    my $len_targets = @targets;
    if ($len_targets == 0){     
        &main::daemon_log("ERROR: ServerPackages: no target specified for msg $header", 1);

    }  elsif ($len_targets == 1){
        # we have only one target symbol
        my $target = $targets[0];
        &main::daemon_log("SeverPackages: msg is for: $target", 7);

        if ($target eq $server_address) {
            # msg is for server
            if ($header eq 'new_passwd'){ &new_passwd($msg_hash)}
            elsif ($header eq 'here_i_am') { &here_i_am($msg_hash)}
            elsif ($header eq 'who_has') { &who_has($msg_hash) }
            elsif ($header eq 'who_has_i_do') { &who_has_i_do($msg_hash)}
            elsif ($header eq 'update_status') { &update_status($msg_hash) }
            elsif ($header eq 'got_ping') { &got_ping($msg_hash)}
            elsif ($header eq 'get_load') { &execute_actions($msg_hash)}
            else { &main::daemon_log("ERROR: ServerPackages: no function assigned to this msg", 5) }

       } elsif ($target eq "*") {
            # msg is for all clients
            my $query_res = $main::known_clients_db->select_dbentry( {table=>'known_clients'} ); 
            while( my ($hit_num, $hit) = each %{ $query_res } ) {    
                $host_name = $hit->{hostname};
                $host_key = $hit->{hostkey};
                $msg_hash->{target} = [$host_name];
                &send_msg_hash2address($msg_hash, $host_name, $host_key);
            }
            return;
         
        } else {
            # msg is for one host
            my $query_res;
            $query_res = $main::known_clients_db->select_dbentry( {table=>'known_clients', hostname=>$target} );
            if( 1 == keys %{$query_res} ) {
                $host_key = $query_res->{1}->{host_key};
                &send_msg_hash2address($msg_hash, $target, $host_key);
                return;
            }

            $query_res = $main::known_server_db->select_dbentry( {table=>'known_server', hostname=>$target} );
            if( 1 == keys %{$query_res} ) {
                $host_key = $query_res->{1}->{host_key};
                &send_msg_hash2address($msg_hash, $target, $host_key);
                return;
            }

            &main::daemon_log("ERROR: ServerPackages: target '$target' is not known neither in known_clients nor in known_server",1);
            return;
        }

    } elsif ($len_targets > 1 ) {
        # we have more than one target 
        return;
    }

    return ;
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

    my $header = @{$msg_hash->{header}}[0];
    my $source_name = @{$msg_hash->{source}}[0];
    my $source_key = @{$msg_hash->{new_passwd}}[0];
    my $query_res;

    # check known_clients_db
    $query_res = $main::known_clients_db->select_dbentry( {table=>'known_clients', hostname=>$source_name} );
    if( 1 == keys %{$query_res} ) {
        my $update_hash = { table=>'known_clients' };
        $update_hash->{where} = [ { hostname=>[$source_name] } ];
        $update_hash->{update} = [ {
            hostkey=>[$source_key],
            timestamp=>[&get_time],
        } ];
        my $res = $main::known_clients_db->update_dbentry( $update_hash );

        my $hash = &create_xml_hash("confirm_new_passwd", $server_address, $source_name);
        &send_msg_hash2address($hash, $source_name, $source_key);
        return;
    }

    # check known_server_db
    $query_res = $main::known_server_db->select_dbentry( {table=>'known_server', hostname=>$source_name } );
    if( 1 == keys %{$query_res} ) {
        my $update_hash = { table=>'known_server' };
        $update_hash->{where} = [ { hostname=>[$source_name] } ];
        $update_hash->{update} = [ {
            hostkey=>[$source_key],
                timestamp=>[&get_time],
        } ];
        my $res = $main::known_server_db->update_dbentry( $update_hash );

        my $hash = &create_xml_hash("confirm_new_passwd", $server_address, $source_name);
        &send_msg_hash2address($hash, $source_name, $source_key);
        return;
    }

    &main::daemon_log("ERROR: $source_name not known for '$header'-msg", 1);
    return;
}


sub send_msg_hash {
    my ($hash, $host_name, $host_key);

    
    my $answer = &send_msg_hash2address($hash, $host_name, $host_key);
    
    return;
}


#===  FUNCTION  ================================================================
#         NAME:  here_i_am
#   PARAMETERS:  msg_hash - hash - hash from function create_xml_hash
#      RETURNS:  nothing
#  DESCRIPTION:  process this incoming message
#===============================================================================
sub here_i_am {
    my ($msg_hash) = @_;

    my $source = @{$msg_hash->{source}}[0];
    my $mac_address = @{$msg_hash->{mac_address}}[0];
    my $out_hash;

    # number of known clients
    my $nu_clients = keys %{$main::known_clients_db->select_dbentry( {table=>'known_clients'} )};

    # check wether client address or mac address is already known
    if (exists $main::known_clients->{$source}) {
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
    &send_msg_hash2address($out_hash, $source, $new_passwd);

    # notify registered client to bus
    if( $bus_activ eq "on") {
        # fetch actual bus key
        my $query_res = $main::known_server_db->select_dbentry( {table=>'known_server'} );
        my $hostkey = $query_res->{1}->{hostkey};
        
        # send update msg to bus
        $out_hash = &create_xml_hash("new_client", $server_address, $bus_address, $source);
        &send_msg_hash2address($out_hash, $bus_address, $hostkey);
        
        &main::daemon_log("send bus msg that client '$source' has registerd at server '$server_address'", 3);
    }

    # give the new client his ldap config
    &new_ldap_config($source);

    return;
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
    
    my $res = $main::known_clients_db->select_dbentry( { table=>'known_clients', hostname=>$address } );

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
		    attrs => ['dn', 'gotoLdapServer'],
		    filter => "(&(objectClass=GOhard)(macaddress=$macaddress))");
    $mesg->code && die $mesg->error;

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
    my @ldap_uris;
    my $server;
    my $base;

    # Do we need to look at an object class?
    if ($#servers < 1){
	    $mesg = $ldap->search( base   => $ldap_base,
			    scope  => 'sub',
			    attrs => ['dn', 'gotoLdapServer'],
			    filter => "(&(objectClass=gosaGroupOfNames)(member=$dn))");
	    $mesg->code && die $mesg->error;

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

    # Unbind
    $mesg = $ldap->unbind;

    # Assemble data package
    my %data = ( 'ldap_uri'  => \@ldap_uris, 'ldap_base' => $base,
	             'ldap_cfg' => \@ldap_cfg, 'pam_cfg' => \@pam_cfg,'nss_cfg' => \@nss_cfg );

    # Need to append GOto settings?
    if (defined $goto_admin and defined $goto_secret){
	    $data{'goto_admin'}= $goto_admin;
	    $data{'goto_secret'}= $goto_secret;
    }

    # Send information
    send_msg("new_ldap_config", $server_address, $address, \%data, $hostkey);

    return;
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
