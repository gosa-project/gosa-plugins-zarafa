package siTriggered;
use Exporter;
@ISA = qw(Exporter);
my @events = (
    "got_ping",
    "detected_hardware",
    "trigger_wake",
    "reload_ldap_config",
    );
@EXPORT = @events;

use strict;
use warnings;
use GOSA::GosaSupportDaemon;
use Socket;


BEGIN {}

END {}

### Start ######################################################################

my $ldap_uri;
my $ldap_base;
my $ldap_admin_dn;
my $ldap_admin_password;
my $mesg;

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

    if( defined( $cfg_file) && ( (-s $cfg_file) > 0 )) {
        if( -r $cfg_file ) {
            $cfg = Config::IniFiles->new( -file => $cfg_file );
        } else {
            &main::daemon_log("ERROR: siTriggered.pm couldn't read config file!", 1);
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


sub reload_ldap_config {
    my ($msg, $msg_hash, $session_id) = @_;
    my $header = @{$msg_hash->{header}}[0];
    my $target = @{$msg_hash->{$header}}[0];

    my $out_msg = &SIPackages::new_ldap_config($target, $session_id);
    my @out_msg_l = ( $out_msg );
    return @out_msg_l;
}


sub got_ping {
    my ($msg, $msg_hash, $session_id) = @_;

    my $source = @{$msg_hash->{source}}[0];
    my $target = @{$msg_hash->{target}}[0];
    my $header = @{$msg_hash->{header}}[0];
    my $act_time = &get_time;
    my @out_msg_l;
    my $out_msg;

    $session_id = @{$msg_hash->{'session_id'}}[0];

    # check known_clients_db
    my $sql_statement = "SELECT * FROM known_clients WHERE hostname='$source'";
    my $query_res = $main::known_clients_db->select_dbentry( $sql_statement );
    if( 1 == keys %{$query_res} ) {
         my $sql_statement= "UPDATE known_clients ".
            "SET status='$header', timestamp='$act_time' ".
            "WHERE hostname='$source'";
         my $res = $main::known_clients_db->update_dbentry( $sql_statement );
    } 
    
    # check known_server_db
    $sql_statement = "SELECT * FROM known_server WHERE hostname='$source'";
    $query_res = $main::known_server_db->select_dbentry( $sql_statement );
    if( 1 == keys %{$query_res} ) {
         my $sql_statement= "UPDATE known_server ".
            "SET status='$header', timestamp='$act_time' ".
            "WHERE hostname='$source'";
         my $res = $main::known_server_db->update_dbentry( $sql_statement );
    } 

    # create out_msg
    my $out_hash = &create_xml_hash($header, $source, "GOSA");
    &add_content2xml_hash($out_hash, "session_id", $session_id);
    $out_msg = &create_xml_string($out_hash);
    push(@out_msg_l, $out_msg);
    
    return @out_msg_l;
}


sub detected_hardware {
	my ($msg, $msg_hash, $session_id) = @_ ;
	my $address = $msg_hash->{source}[0];
	my $gotoHardwareChecksum= $msg_hash->{detected_hardware}[0]->{gotoHardwareChecksum};

	my $sql_statement= "SELECT * FROM known_clients WHERE hostname='$address'";
	my $res = $main::known_clients_db->select_dbentry( $sql_statement );

	# check hit
	my $hit_counter = keys %{$res};
	if( not $hit_counter == 1 ) {
		&main::daemon_log("$session_id ERROR: more or no hit found in known_clients_db by query by '$address'", 1);
		return;
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

	# We need to create a base entry first (if not done from ArpHandler)
	if($mesg->count == 0) {
		&main::daemon_log("INFO: Need to create a new LDAP Entry for client $address", 4);
		my $ipaddress= $1 if $address =~ /^([0-9\.]*?):.*$/;
		my $dnsname;
		if (defined($msg_hash->{'force-hostname'}) && 
			defined($msg_hash->{'force-hostname'}[0]) &&
			length($msg_hash->{'force-hostname'}[0]) > 0){
			$dnsname= $msg_hash->{'force-hostname'}[0];
			&main::daemon_log("INFO: Using forced hostname $dnsname for client $address", 4);
		} else {
			$dnsname= gethostbyaddr(inet_aton($ipaddress), AF_INET) || $ipaddress;
		}

		my $cn = (($dnsname =~ /^(\d){1,3}\.(\d){1,3}\.(\d){1,3}\.(\d){1,3}/) ? $dnsname : sprintf "%s", $dnsname =~ /([^\.]+)\.?/);
		my $dn = "cn=$cn,ou=incoming,$ldap_base";
		&main::daemon_log("INFO: Creating entry for $dn",5);
		my $entry= Net::LDAP::Entry->new( $dn );
		$entry->dn($dn);
		$entry->add("objectClass" => "goHard");
		$entry->add("cn" => $cn);
		$entry->add("macAddress" => $macaddress);
		$entry->add("gotomode" => "locked");
		$entry->add("gotoSysStatus" => "new-system");
		$entry->add("ipHostNumber" => $ipaddress);
		if(defined($main::gosa_unit_tag) && length($main::gosa_unit_tag) > 0) {
			$entry->add("objectClass" => "gosaAdministrativeUnitTag");
			$entry->add("gosaUnitTag" => $main::gosa_unit_tag);
		}
		my $res=$entry->update($ldap_handle);
		if(defined($res->{'errorMessage'}) &&
			length($res->{'errorMessage'}) >0) {
			&main::daemon_log("ERROR: can not add entries to LDAP: ".$res->{'errorMessage'}, 1);
			return;
		} else {
			# Fill $mesg again
			$mesg = $ldap_handle->search(
				base   => $ldap_base,
				scope  => 'sub',
				filter => "(&(objectClass=GOhard)(|(macAddress=$macaddress)(dhcpHWaddress=ethernet $macaddress)))"
			);
		}
	}

	if($mesg->count == 1) {
		my $entry= $mesg->entry(0);
		$entry->changetype("modify");
		foreach my $attribute (
			"gotoSndModule", "ghNetNic", "gotoXResolution", "ghSoundAdapter", "ghCpuType", "gotoXkbModel", 
			"ghGfxAdapter", "gotoXMousePort", "ghMemSize", "gotoXMouseType", "ghUsbSupport", "gotoXHsync", 
			"gotoXDriver", "gotoXVsync", "gotoXMonitor", "gotoHardwareChecksum") {
			if(defined($msg_hash->{detected_hardware}[0]->{$attribute}) &&
				length($msg_hash->{detected_hardware}[0]->{$attribute}) >0 ) {
				if(defined($entry->get_value($attribute))) {
					$entry->delete($attribute);
				}
				&main::daemon_log("INFO: Adding attribute $attribute with value ".$msg_hash->{detected_hardware}[0]->{$attribute},5);
				$entry->add($attribute => $msg_hash->{detected_hardware}[0]->{$attribute});	
			}
		}
		foreach my $attribute (
			"gotoModules", "ghScsiDev", "ghIdeDev") {
			if(defined($msg_hash->{detected_hardware}[0]->{$attribute}) &&
				length($msg_hash->{detected_hardware}[0]->{$attribute}) >0 ) {
				if(defined($entry->get_value($attribute))) {
					$entry->delete($attribute);
				}
				foreach my $array_entry (@{$msg_hash->{detected_hardware}[0]->{$attribute}}) {
					$entry->add($attribute => $array_entry);
				}
			}
		}

		my $res=$entry->update($ldap_handle);
		if(defined($res->{'errorMessage'}) &&
			length($res->{'errorMessage'}) >0) {
			&main::daemon_log("ERROR: can not add entries to LDAP: ".$res->{'errorMessage'}, 1);
		} else {
			&main::daemon_log("INFO: Added Hardware configuration to LDAP", 5);
		}

	}
	return ;
}


sub trigger_wake {
    my ($msg, $msg_hash, $session_id) = @_ ;

    foreach (@{$msg_hash->{macAddress}}){
        &main::daemon_log("$session_id INFO: trigger wake for $_", 5);
        my $host    = $_;
        my $ipaddr  = '255.255.255.255';
        my $port    = getservbyname('discard', 'udp');

        my ($raddr, $them, $proto);
        my ($hwaddr, $hwaddr_re, $pkt);

        # get the hardware address (ethernet address)
        $hwaddr_re = join(':', ('[0-9A-Fa-f]{1,2}') x 6);
        if ($host =~ m/^$hwaddr_re$/) {
          $hwaddr = $host;
        } else {
          &main::daemon_log("$session_id ERROR: trigger_wake called with non mac address", 1);
        }

        # Generate magic sequence
        foreach (split /:/, $hwaddr) {
                $pkt .= chr(hex($_));
        }
        $pkt = chr(0xFF) x 6 . $pkt x 16;

        # Allocate socket and send packet

        $raddr = gethostbyname($ipaddr);
        $them = pack_sockaddr_in($port, $raddr);
        $proto = getprotobyname('udp');

        socket(S, AF_INET, SOCK_DGRAM, $proto) or die "socket : $!";
        setsockopt(S, SOL_SOCKET, SO_BROADCAST, 1) or die "setsockopt : $!";

        send(S, $pkt, 0, $them) or die "send : $!";
        close S;
    }

    return;
}

1;
