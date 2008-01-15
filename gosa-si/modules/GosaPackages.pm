package GosaPackages;

use Exporter;
@ISA = ("Exporter");

use strict;
use warnings;
use GOSA::GosaSupportDaemon;
use IO::Socket::INET;
use XML::Simple;
use File::Spec;
use Data::Dumper;
use GOSA::DBsqlite;
use MIME::Base64;

BEGIN{}
END{}

my ($server_activ, $server_ip, $server_mac_address, $server_port, $server_passwd, $max_clients, $server_event_dir);
my ($bus_activ, $bus_passwd, $bus_ip, $bus_port);
my ($gosa_activ, $gosa_ip, $gosa_mac_address, $gosa_port, $gosa_passwd, $network_interface);
my ($job_queue_timeout, $job_queue_file_name);

my $gosa_server;

my %cfg_defaults = 
("general" =>
    {"job_queue_file_name" => [\$job_queue_file_name, '/var/lib/gosa-si/jobs.db'],
    },
"server" =>
    {"server_activ" => [\$server_activ, "on"],
    "server_ip" => [\$server_ip, "0.0.0.0"],
    "server_port" => [\$server_port, "20081"],
    "server_passwd" => [\$server_passwd, ""],
    "max_clients" => [\$max_clients, 100],
    "server_event_dir" => [\$server_event_dir, '/usr/lib/gosa-si/server/events'],
    },
"bus" =>
    {"bus_activ" => [\$bus_activ, "on"],
    "bus_passwd" => [\$bus_passwd, ""],
    "bus_ip" => [\$bus_ip, "0.0.0.0"],
    "bus_port" => [\$bus_port, "20080"],
    },
"gosa" =>
    {"gosa_activ" => [\$gosa_activ, "on"],
    "gosa_ip" => [\$gosa_ip, "0.0.0.0"],
    "gosa_port" => [\$gosa_port, "20082"],
    "gosa_passwd" => [\$gosa_passwd, "none"],
    },
);
 

## START ##########################

# read configfile and import variables
&read_configfile();
$network_interface= &get_interface_for_ip($server_ip);
$gosa_mac_address= &get_mac($network_interface);

# complete addresses
my $server_address = "$server_ip:$server_port";
my $bus_address = "$bus_ip:$bus_port";
my $gosa_address = "$gosa_ip:$gosa_port";

# create general settings for this module
my $gosa_cipher = &create_ciphering($gosa_passwd);
my $xml = new XML::Simple();

# open gosa socket
if ($gosa_activ eq "on") {
    &main::daemon_log(" ",1);
    $gosa_server = IO::Socket::INET->new(LocalPort => $gosa_port,
            Type => SOCK_STREAM,
            Reuse => 1,
            Listen => 1,
            );
    if (not defined $gosa_server) {
        &main::daemon_log("cannot start tcp server at $gosa_port for communication to gosa: $@", 1);
        die;
    } else {
        &main::daemon_log("start server for communication to gosa: $gosa_address", 1);
        
    }
}

# create gosa job queue as a SQLite DB 
my $table_name = "jobs";
my $sqlite = GOSA::DBsqlite->new($job_queue_file_name);




## FUNCTIONS #################################################################

sub get_module_info {
    my @info = ($gosa_address,
                $gosa_passwd,
                $gosa_server,
                $gosa_activ,
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
#         NAME:  process_incoming_msg
#   PARAMETERS:  crypted_msg - string - incoming crypted message
#      RETURNS:  nothing
#  DESCRIPTION:  handels the proceeded distribution to the appropriated functions
#===============================================================================
sub process_incoming_msg {
    my ($crypted_msg) = @_ ;
    if( (not(defined($crypted_msg))) || (length($crypted_msg) <= 0)) {
        &main::daemon_log("function 'process_incoming_msg': got no msg", 7);
        return;
    }
    &main::daemon_log("GosaPackages: incoming msg: \n>>>$crypted_msg<<<", 7);

    $crypted_msg =~ /^([\s\S]*?)\.(\d{1,3}?)\.(\d{1,3}?)\.(\d{1,3}?)\.(\d{1,3}?)$/;
    $crypted_msg = $1;
    my $host = sprintf("%s.%s.%s.%s", $2, $3, $4, $5);
 
    # collect addresses from possible incoming clients
    # only gosa is allowd as incoming client
    &main::daemon_log("GosaPackages: host_key: $host", 7);
    &main::daemon_log("GosaPackages: key_passwd: $gosa_passwd", 7);

    $gosa_cipher = &create_ciphering($gosa_passwd);

    # determine the correct passwd for deciphering of the incoming msgs
    my $msg = "";
    my $msg_hash;
    eval{
        $msg = &decrypt_msg($crypted_msg, $gosa_cipher);
        &main::daemon_log("GosaPackages: decrypted_msg: \n$msg", 7);

        $msg_hash = $xml->XMLin($msg, ForceArray=>1);
    };
    if($@) {
        &main::daemon_log("WARNING: GosaPackages do not understand the message:", 5);
        &main::daemon_log("$@", 7);
        return;
    }

    my $header = @{$msg_hash->{header}}[0];
    
    &main::daemon_log("GosaPackages: recieve '$header' from $host", 1);
    
    my $out_msg;
    if ($header =~ /^job_/) {
        $out_msg = &process_job_msg($msg, $msg_hash);
    } elsif ($header =~ /^gosa_/) {
        $out_msg = &process_gosa_msg($msg, $header);
    } else {
        &main::daemon_log("ERROR: $header is not a valid GosaPackage-header, need a 'job_' or a 'gosa_' prefix");
    }
    
    if (not defined $out_msg) {
        return;
    }

    if ($out_msg =~ /<jobdb_id>(\d*?)<\/jobdb_id>/) {
        my $job_id = $1;
        my $sql = "BEGIN TRANSATION; UPDATE '$main::job_queue_table_name' SET status='done', result='$out_msg' WHERE id='$job_id'; COMMIT;";
        my $res = $main::job_db->exec_statement($sql);
        return;

    } else {

        &main::daemon_log("antwort von query jobs db:", 1);
        &main::daemon_log("$out_msg", 1);
        my $out_cipher = &create_ciphering($gosa_passwd);
        $out_msg = &encrypt_msg($out_msg, $out_cipher);
        return $out_msg;
    }

}

sub process_gosa_msg {
    my ($msg, $header) = @_ ;
    my $out_msg;
    $header =~ s/gosa_//;
    &main::daemon_log("GosaPackages: got a gosa msg $header", 5);

    # decide wether msg is a core function or a event handler
    if ( $header eq 'query_jobdb') { $out_msg = &query_jobdb }
    elsif ($header eq 'delete_jobdb_entry') { $out_msg = &delete_jobdb_entry }
    elsif ($header eq 'clear_jobdb') { $out_msg = &clear_jobdb }
    elsif ($header eq 'update_status_jobdb_entry' ) { $out_msg = &update_status_jobdb_entry }
    elsif ($header eq 'update_timestamp_jobdb_entry' ) { $out_msg = &update_timestamp_jobdb_entry }
    else {
        # msg could not be assigned to core function
        # fetch all available eventhandler under $server_event_dir
        opendir (DIR, $server_event_dir) or &main::daemon_log("ERROR cannot open $server_event_dir: $!\n", 1) and return;
        while (defined (my $file = readdir (DIR))) {
            if (not $file eq $header) {
                next;
            }
            # try to deliver incoming msg to eventhandler
            my $cmd = File::Spec->join($server_event_dir, $header)." '$msg'";
            &main::daemon_log("GosaPackages: execute event_handler $header", 3);
            &main::daemon_log("GosaPackages: cmd: $cmd", 7);

            $out_msg = "";
            open(PIPE, "$cmd 2>&1 |");
            while(<PIPE>) {
                $out_msg.=$_;
            }
            close(PIPE);
            &main::daemon_log("GosaPackages: answer of cmd: $out_msg", 5);
            last;
        }
    }

    # if delivery not possible raise error and return 
    if (not defined $out_msg) {
        &main::daemon_log("ERROR: GosaPackages: no event handler or core function defined for $header", 1);
    } elsif ($out_msg eq "") {
        &main::daemon_log("ERROR: GosaPackages got not answer from event_handler $header", 1);
    }
    return $out_msg;
    
}


sub process_job_msg {
    my ($msg, $msg_hash)= @_ ;    

    my $header = @{$msg_hash->{header}}[0];
    $header =~ s/job_//;
    &main::daemon_log("GosaPackages: got a job msg for queue: $header", 5);
    
    # check wether mac address is already known in known_daemons or known_clients
    my $target = 'none';

    # add job to job queue
    my $func_dic = {table=>$main::job_queue_table_name, 
                    primkey=>'id',
                    timestamp=>@{$msg_hash->{timestamp}}[0],
                    status=>'waiting', 
                    result=>'none',
                    headertag=>$header, 
                    targettag=>$target,
                    xmlmessage=>$msg,
                    macaddress=>@{$msg_hash->{mac}}[0],
                    };
    my $res = $main::job_db->add_dbentry($func_dic);
    if (not $res == 0) {
        &main::daemon_log("ERROR: GosaPackages: process_job_msg: $res", 1);
    }
    
    &main::daemon_log("GosaPackages: $header job successfully added to job queue", 3);
    return "<xml><1>$res</1></xml>";

}


sub db_res_2_xml {
    my ($db_res) = @_ ;
    my $xml = "<xml>";

    while ( my ($hit, $hash) = each %{ $db_res } ) {
        $xml .= "\n<answer$hit>";

        while ( my ($column_name, $column_value) = each %{$hash} ) {
            $xml .= "<$column_name>";
            my $xml_content;
            if( $column_name eq "xmlmessage" ) {
                $xml_content = &encode_base64($column_value);
            } else {
                $xml_content = $column_value;
            }
            $xml .= $xml_content;
            $xml .= "</$column_name>"; 
        }

        $xml .= "</answer$hit>";
    }

    $xml .= "</xml>";
    return $xml;
}


## CORE FUNCTIONS ############################################################

sub query_jobdb {
    my ($msg) = @_;
    my $msg_hash = &transform_msg2hash($msg);

    # prepare query sql statement
    my @where = @{$msg_hash->{where}};
    my $where_hash = {table=>$main::job_queue_table_name };
    foreach my $where_pram (@where) {
        my $where_val = @{$msg_hash->{$where_pram}}[0];
        if (defined $where_val) {
            $where_hash->{$where_pram} = $where_val;
        }
    }
 
    # execute db query   
    my $res_hash = $main::job_db->select_dbentry($where_hash);

    my $out_xml = &db_res_2_xml($res_hash);
    return $out_xml;
}

sub delete_jobdb_entry {
    my ($msg) = @_ ;
    my $msg_hash = &transform_msg2hash($msg);
    
    # prepare query sql statement
    my @where = @{$msg_hash->{where}};
    my $where_hash = {table=>$main::job_queue_table_name };
    foreach my $where_pram (@where) {
        my $where_val = @{$msg_hash->{$where_pram}}[0];
        if (defined $where_val) {
            $where_hash->{$where_pram} = $where_val;
        }
    }
    
    # execute db query
    my $db_res = $main::job_db->del_dbentry($where_hash);

    my $res;
    if( $db_res > 0 ) { 
        $res = 0 ;
    } else {
        $res = 1;
    }

    # prepare xml answer
    my $out_xml = "<xml><answer1>$res</answer1></xml>";
    return $out_xml;

}

sub clear_jobdb {
    my ($msg) = @_ ;
    my $msg_hash = &transform_msg2hash($msg);
    
    my $where_hash = {table=>$main::job_queue_table_name };
    
    # execute db query
    my $db_res = $main::job_db->del_dbentry($where_hash);
    print STDERR "db_res=$db_res\n";
    my $res;
    if( $db_res eq '0E0' ) { 
        $res = 0 ;
    } else {
        $res = 1;
    }

    # prepare xml answer
    my $out_xml = "<xml><answer1>$res</answer1></xml>";
    return $out_xml;
}

sub update_status_jobdb_entry {
    my ($msg) = @_ ;
    my $msg_hash = &transform_msg2hash($msg);
    
    # prepare query sql statement
    my $update_hash = {table=>$main::job_queue_table_name };
    if( exists $msg_hash->{where} ) {
        $update_hash->{where} = $msg_hash->{where};
    } else {
        $update_hash->{where} = [];
    }

    if( not exists $msg_hash->{update}[0]->{status} ) {
        return "<xml><answer1>1</answer1></xml>";
    }
    $update_hash->{update} = [ { status=>$msg_hash->{update}[0]->{status} } ];

    # execute db query
    my $db_res = $main::job_db->update_dbentry($update_hash);

    # transform db answer to error returnment
    my $res;
    if( $db_res > 0 ) { 
        $res = 0 ;
    } else {
        $res = 1;
    }

    # prepare xml answer
    my $out_xml = "<xml><answer1>$res</answer1></xml>";
    return $out_xml;
}

sub update_timestamp_jobdb_entry {
    my ($msg) = @_ ;
    my $msg_hash = &transform_msg2hash($msg);
    
    # prepare query sql statement
    my $update_hash = {table=>$main::job_queue_table_name };
    if( exists $msg_hash->{where} ) {
        $update_hash->{where} = $msg_hash->{where};
    } else {
        $update_hash->{where} = [];
    }

    if( not exists $msg_hash->{update}[0]->{timestamp} ) {
        return "<xml><answer1>1</answer1></xml>";
    }

    $update_hash->{update} = [ { timestamp=>$msg_hash->{update}[0]->{timestamp} } ];

    # execute db query
    my $db_res = $main::job_db->update_dbentry($update_hash);

    # transform db answer to error returnment
    my $res;
    if( $db_res > 0 ) { 
        $res = 0 ;
    } else {
        $res = 1;
    }

    # prepare xml answer
    my $out_xml = "<xml><answer1>$res</answer1></xml>";
    return $out_xml;

}


1;










