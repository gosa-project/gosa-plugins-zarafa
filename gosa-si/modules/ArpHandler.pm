package ArpHandler;

use Exporter;
@ISA = ("Exporter");

use strict;
use warnings;
use GOSA::GosaSupportDaemon;
use Getopt::Long;
use Config::IniFiles;
use POSIX;
use Fcntl;
use Net::LDAP;
use Net::LDAP::LDIF;
use Net::LDAP::Entry;
use Net::DNS;
use Switch;
use Data::Dumper;
use Socket qw/PF_INET SOCK_DGRAM inet_ntoa sockaddr_in/;
use POE qw(Component::Pcap Component::ArpWatch);

BEGIN{}
END{}

my ($timeout, $mailto, $mailfrom, $user, $group);
my %daemon_children;
my ($ldap, $bind_phrase, $password, $ldap_base, $interface) ;
my $hosts_database={};
my $resolver=Net::DNS::Resolver->new;

$ldap_base = "dc=gonicus,dc=de" ;
$interface = "all";

sub get_module_info {
	my @info = (undef,
		undef,
		undef,
		undef,
		"socket",
	);

	$ldap = Net::LDAP->new("ldap.intranet.gonicus.de") or die "$@";

	# When interface is not configured (or 'all'), start arpwatch on all possible interfaces
	if ((!defined($interface)) || $interface eq 'all') {
		foreach my $device(&get_interfaces) {
			# TODO: Need a better workaround for IPv4-to-IPv6 bridges
			if($device =~ m/^sit.$/) {
				next;
			}

			# If device has a valid mac address
			if(not(&get_mac($device) eq "00:00:00:00:00:00")) {
				&main::daemon_log("Starting ArpWatch on $device", 1);
				POE::Session->create( 
					inline_states => {
						_start => sub {
							&start(@_,$device);
						},
						_stop => sub {
							$_[KERNEL]->post( sprintf("arp_watch_$device") => 'shutdown' )
						},
						got_packet => \&got_packet,
					},
				);
			}
		}
	} else {
		&main::daemon_log("Starting ArpWatch on $interface", 1);
		POE::Session->create( 
			inline_states => {
				_start => \&start,
				_stop => sub {
					$_[KERNEL]->post( sprintf("arp_watch_$interface") => 'shutdown' )
				},
				got_packet => \&got_packet,
			},
		);
	}

	return \@info;
}

sub process_incoming_msg {
	return 1;
}

sub start {
	my $device = (exists($_[ARG0])?$_[ARG0]:'eth0');
	POE::Component::ArpWatch->spawn( 
		Alias => sprintf("arp_watch_$device"),
		Device => $device, 
		Dispatch => 'got_packet',
		Session => $_[SESSION],
	);

	$_[KERNEL]->post( sprintf("arp_watch_$device") => 'run' );
}

sub got_packet {
	my $packet = $_[ARG0];

	if(	$packet->{source_haddr} eq "00:00:00:00:00:00" || 
		$packet->{source_haddr} eq "ff:ff:ff:ff:ff:ff" || 
		$packet->{source_ipaddr} eq "0.0.0.0") {
		return;
	}

	if(!exists($hosts_database->{$packet->{source_haddr}})) {
		my $dnsresult= $resolver->search($packet->{source_ipaddr});
		my $dnsname= (defined($dnsresult))?$dnsresult->{answer}[0]->{ptrdname}:$packet->{source_ipaddr};
		my $ldap_result=&get_host_from_ldap($packet->{source_haddr});
		if(exists($ldap_result->{dn})) {
			$hosts_database->{$packet->{source_haddr}}=$ldap_result;
			if(!exists($ldap_result->{ipHostNumber})) {
				$hosts_database->{$packet->{source_haddr}}->{ipHostNumber}=$packet->{source_ipaddr};
			} else {
				if(!($ldap_result->{ipHostNumber} eq $packet->{source_ipaddr})) {
					&main::daemon_log(
						"Current IP Address ".$packet->{source_ipaddr}.
						" of host ".$ldap_result->{dnsname}.
						" differs from LDAP (".$ldap_result->{ipHostNumber}.")", 4);
				}
			}
			$hosts_database->{$packet->{source_haddr}}->{dnsname}=$dnsname;
			&main::daemon_log("Host was found in LDAP as ".$ldap_result->{dn}, 6);
		} else {
			$hosts_database->{$packet->{source_haddr}}={
				macAddress => $packet->{source_haddr},
				ipHostNumber => $packet->{source_ipaddr},
				dnsname => $dnsname,
			};
			&main::daemon_log("Host was not found in LDAP (".($hosts_database->{$packet->{source_haddr}}->{dnsname}).")",6);
			&main::daemon_log(
				"New Host ".($hosts_database->{$packet->{source_haddr}}->{dnsname}).
				": ".$hosts_database->{$packet->{source_haddr}}->{ipHostNumber}.
				"/".$hosts_database->{$packet->{source_haddr}}->{macAddress},4);
		}
	} else {
		if(!($hosts_database->{$packet->{source_haddr}}->{ipHostNumber} eq $packet->{source_ipaddr})) {
			&main::daemon_log(
				"IP Address change of MAC ".$packet->{source_haddr}.
				": ".$hosts_database->{$packet->{source_haddr}}->{ipHostNumber}.
				"->".$packet->{source_ipaddr}, 4);
			$hosts_database->{$packet->{source_haddr}}->{ipHostNumber}= $packet->{source_ipaddr};
		}
		&main::daemon_log("Host already in cache (".($hosts_database->{$packet->{source_haddr}}->{dnsname}).")",6);
	}
} 

sub get_host_from_ldap {
	my $mac=shift;
	my $result={};
		
	my $ldap_result= search_ldap_entry(
		$ldap,
		$ldap_base,
		"(|(macAddress=$mac)(dhcpHWAddress=ethernet $mac))"
	);

	if($ldap_result->count==1) {
		if(exists($ldap_result->{entries}[0]) && 
			exists($ldap_result->{entries}[0]->{asn}->{objectName}) && 
			exists($ldap_result->{entries}[0]->{asn}->{attributes})) {

			for my $attribute(@{$ldap_result->{entries}[0]->{asn}->{attributes}}) {
				if($attribute->{type} eq 'cn') {
					$result->{cn} = $attribute->{vals}[0];
				}
				if($attribute->{type} eq 'macAddress') {
					$result->{macAddress} = $attribute->{vals}[0];
				}
				if($attribute->{type} eq 'dhcpHWAddress') {
					$result->{dhcpHWAddress} = $attribute->{vals}[0];
				}
				if($attribute->{type} eq 'ipHostNumber') {
					$result->{ipHostNumber} = $attribute->{vals}[0];
				}
			}
		}
		$result->{dn} = $ldap_result->{entries}[0]->{asn}->{objectName};
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
#         NAME:  add_ldap_entry
#      PURPOSE:  adds an element to ldap-tree
#   PARAMETERS:  
#      RETURNS:  none
#  DESCRIPTION:  ????
#       THROWS:  no exceptions
#     COMMENTS:  none
#     SEE ALSO:  n/a
#===============================================================================
#sub add_ldap_entry {
#    my ($ldap_tree, $ldap_base, $mac, $gotoSysStatus, $ip, $interface, $desc) = @_;
#    my $dn = "cn=$mac,ou=incoming,$ldap_base";
#    my $s_res = &search_ldap_entry($ldap_tree, $ldap_base, "(|(macAddress=$mac)(dhcpHWAddress=ethernet $mac))");
#    my $c_res = $s_res->count;
#    if($c_res == 1) {
#        daemon_log("WARNING: macAddress $mac already in LDAP", 1);
#        return;
#    } elsif($c_res > 0) {
#        daemon_log("ERROR: macAddress $mac exists $c_res times in LDAP", 1);
#        return;
#    }
#
#    # create LDAP entry 
#    my $entry = Net::LDAP::Entry->new( $dn );
#    $entry->dn($dn);
#    $entry->add("objectClass" => "goHard");
#    $entry->add("cn" => $mac);
#    $entry->add("macAddress" => $mac);
#    if(defined $gotoSysStatus) {$entry->add("gotoSysStatus" => $gotoSysStatus)}
#    if(defined $ip) {$entry->add("ipHostNumber" => $ip) }
#    #if(defined $interface) { }
#    if(defined $desc) {$entry->add("description" => $desc) }
#    
#    # submit entry to LDAP
#    my $result = $entry->update ($ldap_tree); 
#        
#    # for $result->code constants please look at Net::LDAP::Constant
#    my $log_time = localtime( time );
#    if($result->code == 68) {   # entry already exists 
#        daemon_log("WARNING: $log_time: $dn ".$result->error, 3);
#    } elsif($result->code == 0) {   # everything went fine
#        daemon_log("$log_time: add entry $dn to ldap", 1);
#    } else {  # if any other error occur
#        daemon_log("ERROR: $log_time: $dn, ".$result->code.", ".$result->error, 1);
#    }
#    return;
#}


#===  FUNCTION  ================================================================
#         NAME:  change_ldap_entry
#      PURPOSE:  ????
#   PARAMETERS:  ????
#      RETURNS:  ????
#  DESCRIPTION:  ????
#       THROWS:  no exceptions
#     COMMENTS:  none
#     SEE ALSO:  n/a
#===============================================================================
#sub change_ldap_entry {
#    my ($ldap_tree, $ldap_base, $mac, $gotoSysStatus ) = @_;
#    
#    # check if ldap_entry exists or not
#    my $s_res = &search_ldap_entry($ldap_tree, $ldap_base, "(|(macAddress=$mac)(dhcpHWAddress=ethernet $mac))");
#    my $c_res = $s_res->count;
#    if($c_res == 0) {
#        daemon_log("WARNING: macAddress $mac not in LDAP", 1);
#        return;
#    } elsif($c_res > 1) {
#        daemon_log("ERROR: macAddress $mac exists $c_res times in LDAP", 1);
#        return;
#    }
#
#    my $s_res_entry = $s_res->pop_entry();
#    my $dn = $s_res_entry->dn();
#    my $result = $ldap->modify( $dn, replace => {'gotoSysStatus' => $gotoSysStatus } );
#
#    # for $result->code constants please look at Net::LDAP::Constant
#    my $log_time = localtime( time );
#    if($result->code == 32) {   # entry doesnt exists 
#        &add_ldap_entry($mac, $gotoSysStatus);
#    } elsif($result->code == 0) {   # everything went fine
#        daemon_log("$log_time: entry $dn changed successful", 1);
#    } else {  # if any other error occur
#        daemon_log("ERROR: $log_time: $dn, ".$result->code.", ".$result->error, 1);
#    }
#
#    return;
#}

#===  FUNCTION  ================================================================
#         NAME:  search_ldap_entry
#      PURPOSE:  ????
#   PARAMETERS:  [Net::LDAP] $ldap_tree - object of an ldap-tree
#                string $sub_tree - dn of the subtree the search is performed
#                string $search_string - either a string or a Net::LDAP::Filter object
#      RETURNS:  [Net::LDAP::Search] $msg - result object of the performed search
#  DESCRIPTION:  ????
#       THROWS:  no exceptions
#     COMMENTS:  none
#     SEE ALSO:  n/a
#===============================================================================
sub search_ldap_entry {
    my ($ldap_tree, $sub_tree, $search_string) = @_;
    my $msg = $ldap_tree->search( # perform a search
                        base   => $sub_tree,
                        filter => $search_string,
                      ) or daemon_log("cannot perform search at ldap: $@", 1);
#    if(defined $msg) {
#        print $sub_tree."\t".$search_string."\t";
#        print $msg->count."\n";
#        foreach my $entry ($msg->entries) { $entry->dump; };
#    }

    return $msg;
}



#========= MAIN = main ========================================================
#daemon_log( "####### START DAEMON ######\n", 1 );
#&check_cmdline_param ;
#&check_pid;
#&open_fifo($fifo_path);
#
## Just fork, if we"re not in foreground mode
#if( ! $foreground ) { $pid = fork(); }
#else { $pid = $$; }
#
## Do something useful - put our PID into the pid_file
#if( 0 != $pid ) {
#    open( LOCK_FILE, ">$pid_file" );
#    print LOCK_FILE "$pid\n";
#    close( LOCK_FILE );
#    if( !$foreground ) { exit( 0 ) };
#}
#
#
#if( not -p $fifo_path ) { die "fifo file disappeared\n" }
#if($c_res == 1) {
#        daemon_log("WARNING: macAddress $mac already in LDAP", 1);
#        return;
#    } elsif($c_res > 0) {
#        daemon_log("ERROR: macAddress $mac exists $c_res times in LDAP", 1);
#        return;
#    }
#
#    # create LDAP entry 
#    my $entry = Net::LDAP::Entry->new( $dn );
#    $entry->dn($dn);
#    $entry->add("objectClass" => "goHard");
#    $entry->add("cn" => $mac);
#    $entry->add("macAddress" => $mac);
#    if(defined $gotoSysStatus) {$entry->add("gotoSysStatus" => $gotoSysStatus)}
#    if(defined $ip) {$entry->add("ipHostNumber" => $ip) }
#    #if(defined $interface) { }
#    if(defined $desc) {$entry->add("description" => $desc) }
#    
#    # submit entry to LDAP
#    my $result = $entry->update ($ldap_tree); 
#        
#    # for $result->code constants please look at Net::LDAP::Constant
#    my $log_time = localtime( time );
#    if($result->code == 68) {   # entry already exists 
#        daemon_log("WARNING: $log_time: $dn ".$result->error, 3);
#    } elsif($result->code == 0) {   # everything went fine
#        daemon_log("$log_time: add entry $dn to ldap", 1);
#    } else {  # if any other error occur
#        daemon_log("ERROR: $log_time: $dn, ".$result->code.", ".$result->error, 1);
#    }
#    return;
#}


#===  FUNCTION  ================================================================
#         NAME:  change_ldap_entry
#      PURPOSE:  ????
#   PARAMETERS:  ????
#      RETURNS:  ????
#  DESCRIPTION:  ????
#       THROWS:  no exceptions
#     COMMENTS:  none
#     SEE ALSO:  n/a
#===============================================================================
#sub change_ldap_entry {
#    my ($ldap_tree, $ldap_base, $mac, $gotoSysStatus ) = @_;
#    
#    # check if ldap_entry exists or not
#    my $s_res = &search_ldap_entry($ldap_tree, $ldap_base, "(|(macAddress=$mac)(dhcpHWAddress=ethernet $mac))");
#    my $c_res = $s_res->count;
#    if($c_res == 0) {
#        daemon_log("WARNING: macAddress $mac not in LDAP", 1);
#        return;
#    } elsif($c_res > 1) {
#        daemon_log("ERROR: macAddress $mac exists $c_res times in LDAP", 1);
#        return;
#    }
#
#    my $s_res_entry = $s_res->pop_entry();
#    my $dn = $s_res_entry->dn();
#    my $result = $ldap->modify( $dn, replace => {'gotoSysStatus' => $gotoSysStatus } );
#
#    # for $result->code constants please look at Net::LDAP::Constant
#    my $log_time = localtime( time );
#    if($result->code == 32) {   # entry doesnt exists 
#        &add_ldap_entry($mac, $gotoSysStatus);
#    } elsif($result->code == 0) {   # everything went fine
#        daemon_log("$log_time: entry $dn changed successful", 1);
#    } else {  # if any other error occur
#        daemon_log("ERROR: $log_time: $dn, ".$result->code.", ".$result->error, 1);
#    }
#
#    return;
#}

#===  FUNCTION  ================================================================
#         NAME:  search_ldap_entry
#      PURPOSE:  ????
#   PARAMETERS:  [Net::LDAP] $ldap_tree - object of an ldap-tree
#                string $sub_tree - dn of the subtree the search is performed
#                string $search_string - either a string or a Net::LDAP::Filter object
#      RETURNS:  [Net::LDAP::Search] $msg - result object of the performed search
#  DESCRIPTION:  ????
#       THROWS:  no exceptions
#     COMMENTS:  none
#     SEE ALSO:  n/a
#===============================================================================
#sub search_ldap_entry {
#    my ($ldap_tree, $sub_tree, $search_string) = @_;
#    my $msg = $ldap_tree->search( # perform a search
#                        base   => $sub_tree,
#                        filter => $search_string,
#                      ) or daemon_log("cannot perform search at ldap: $@", 1);
##    if(defined $msg) {
##        print $sub_tree."\t".$search_string."\t";
##        print $msg->count."\n";
##        foreach my $entry ($msg->entries) { $entry->dump; };
##    }
#
#    return $msg;
#}



#========= MAIN = main ========================================================
#daemon_log( "####### START DAEMON ######\n", 1 );
#&check_cmdline_param ;
#&check_pid;
#&open_fifo($fifo_path);
#
## Just fork, if we"re not in foreground mode
#if( ! $foreground ) { $pid = fork(); }
#else { $pid = $$; }
#
## Do something useful - put our PID into the pid_file
#if( 0 != $pid ) {
#    open( LOCK_FILE, ">$pid_file" );
#    print LOCK_FILE "$pid\n";
#    close( LOCK_FILE );
#    if( !$foreground ) { exit( 0 ) };
#}
#
#
#if( not -p $fifo_path ) { die "fifo file disappeared\n" }
#sysopen(FIFO, $fifo_path, O_RDONLY) or die "can't read from $fifo_path: $!" ;
#
#while( 1 ) {
#    # checke alle prozesse im hash daemon_children ob sie noch aktiv sind, wenn
#    # nicht, dann entferne prozess aus hash
#    while( (my $key, my $val) = each( %daemon_children) ) {
#        my $status = waitpid( $key, &WNOHANG) ;
#        if( $status == -1 ) { 
#            delete $daemon_children{$key} ; 
#            daemon_log("childprocess finished: $key", 3) ;
#        }
#    }
#
#    # ist die max_process anzahl von prozesskindern erreicht, dann warte und 
#    # prüfe erneut, ob in der zwischenzeit prozesse fertig geworden sind
#    if( keys( %daemon_children ) >= $max_process ) { 
#        sleep($max_process_timeout) ;
#        next ;
#    }
#
#    my $msg = <FIFO>;
#    if( not defined( $msg )) { next ; }
#    
#    chomp( $msg );
#    if( length( $msg ) == 0 ) { next ; }
#
#    my $forked_pid = fork();
##=== PARENT = parent ==========================================================
#    if ( $forked_pid != 0 ) { 
#        daemon_log("childprocess forked: $forked_pid", 3) ;
#        $daemon_children{$forked_pid} = 0 ;
#    }
##=== CHILD = child ============================================================
#    else {
#        # parse the incoming message from arp, split the message and return 
#        # the values in an array. not defined values are set to "none" 
#        #my ($mac, $ip, $interface, $arp_sig, $desc) = &parse_input( $msg ) ;
#        daemon_log( "childprocess read from arp: $fifo_path\nline: $msg", 3);
#        my ($mac, $ip, $interface, $arp_sig, $desc) = split('\s', $msg, 5);
#
#        # create connection to LDAP
#        $#sysopen(FIFO, $fifo_path, O_RDONLY) or die "can't read from $fifo_path: $!" ;
#
#while( 1 ) {
#    # checke alle prozesse im hash daemon_children ob sie noch aktiv sind, wenn
#    # nicht, dann entferne prozess aus hash
#    while( (my $key, my $val) = each( %daemon_children) ) {
#        my $status = waitpid( $key, &WNOHANG) ;
#        if( $status == -1 ) { 
#            delete $daemon_children{$key} ; 
#            daemon_log("childprocess finished: $key", 3) ;
#        }
#    }
#
#    # ist die max_process anzahl von prozesskindern erreicht, dann warte und 
#    # prüfe erneut, ob in der zwischenzeit prozesse fertig geworden sind
#    if( keys( %daemon_children ) >= $max_process ) { 
#        sleep($max_process_timeout) ;
#        next ;
#    }
#
#    my $msg = <FIFO>;
#    if( not defined( $msg )) { next ; }
#    
#    chomp( $msg );
#    if( length( $msg ) == 0 ) { next ; }
#
#    my $forked_pid = fork();
##=== PARENT = parent ==========================================================
#    if ( $forked_pid != 0 ) { 
#        daemon_log("childprocess forked: $forked_pid", 3) ;
#        $daemon_children{$forked_pid} = 0 ;
#    }
##=== CHILD = child ============================================================
#    else {
#        # parse the incoming message from arp, split the message and return 
#        # the values in an array. not defined values are set to "none" 
#        #my ($mac, $ip, $interface, $arp_sig, $desc) = &parse_input( $msg ) ;
#        daemon_log( "childprocess read from arp: $fifo_path\nline: $msg", 3);
#        my ($mac, $ip, $interface, $arp_sig, $desc) = split('\s', $msg, 5);
#
#        # create connection to LDAP
#        $ldap = Net::LDAP->new( "localhost" ) or die "$@";
#        $ldap->bind($bind_phrase,
#                    password => $password,
#                    ) ;
#        
#        switch($arp_sig) {
#            case 0 {&change_ldap_entry($ldap, $ldap_base, 
#                                      $mac, "ip-changed",
#                                      )} 
#            case 1 {&change_ldap_entry($ldap, $ldap_base, 
#                                      $mac, "mac-not-whitelisted",
#                                      )}
#            case 2 {&change_ldap_entry($ldap, $ldap_base, 
#                                      $mac, "mac-in-blacklist",
#                                      )}
#            case 3 {&add_ldap_entry($ldap, $ldap_base, 
#                                   $mac, "new-mac-address", $ip, 
#                                   $interface, $desc, 
#                                   )}
#            case 4 {&change_ldap_entry($ldap, $ldap_base, 
#                                      $mac, "unauthorized-arp-request",
#                                      )}
#            case 5 {&change_ldap_entry($ldap, $ldap_base, 
#                                      $mac, "abusive-number-of-arp-requests",
#                                      )}
#            case 6 {&change_ldap_entry($ldap, $ldap_base, 
#                                      $mac, "ether-and-arp-mac-differs",
#                                      )}
#            case 7 {&change_ldap_entry($ldap, $ldap_base, 
#                                      $mac, "flood-detected",
#                                      )}
#            case 8 {&add_ldap_entry($ldap, $ldap_base, 
#                                   $mac, $ip, "new-system",
#                                   )}
#            case 9 {&change_ldap_entry($ldap, $ldap_base, 
#                                      $mac, "mac-changed",
#                                      )}
#        }
#
#
        # ldap search
#        my $base_phrase = "dc=gonicus,dc=de";
#        my $filter_phrase = "cn=keinesorge";
#        my $attrs_phrase = "cn macAdress";
#        my $msg_search = $ldap->search( base   => $base_phrase,
#                                        filter => $filter_phrase,
#                                        attrs => $attrs_phrase,
#                                        );
#        $msg_search->code && die $msg_search->error;
#        
#        my @entries = $msg_search->entries;
#        my $max = $msg_search->count;
#        print "anzahl der entries: $max\n";
#        my $i;
#        for ( $i = 0 ; $i < $max ; $i++ ) {
#            my $entry = $msg_search->entry ( $i );
#            foreach my $attr ( $entry->attributes ) {
#                if( not $attr eq "cn") {
#                    next;
#                }
#                print join( "\n ", $attr, $entry->get_value( $attr ) ), "\n\n";
#            }
#        }
		#
		#        # ldap add
		#       
		#        
		#        $ldap->unbind;
		#        exit;
		#    }
		#
		#}

1;
