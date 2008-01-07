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

my ($server_activ, $server_port, $server_passwd, $max_clients, $server_event_dir);
my ($bus_activ, $bus_passwd, $bus_ip, $bus_port);
my ($gosa_activ, $gosa_ip, $gosa_port, $gosa_passwd);
my ($job_queue_timeout, $job_queue_file_name);

my $gosa_server;

my %cfg_defaults = 
("general" =>
    {"job_queue_file_name" => [\$job_queue_file_name, '/var/lib/gosa-si/jobs.db'],
    },
"server" =>
    {"server_activ" => [\$server_activ, "on"],
    "server_port" => [\$server_port, "20081"],
    "server_passwd" => [\$server_passwd, ""],
    "max_clients" => [\$max_clients, 100],
    "server_event_dir" => [\$server_event_dir, '/usr/lib/gosa-si/server/events'],
    },
"bus" =>
    {"bus_activ" => [\$bus_activ, "on"],
    "bus_passwd" => [\$bus_passwd, ""],
    "bus_ip" => [\$bus_ip, ""],
    "bus_port" => [\$bus_port, "20080"],
    },
"gosa" =>
    {"gosa_activ" => [\$gosa_activ, "on"],
    "gosa_ip" => [\$gosa_ip, ""],
    "gosa_port" => [\$gosa_port, "20082"],
    "gosa_passwd" => [\$gosa_passwd, "none"],
    },
);
 

## START ##########################

# read configfile and import variables
&read_configfile();

# detect own ip and mac address
my ($server_ip, $server_mac_address) = &get_ip_and_mac(); 

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
    } else {
        &main::daemon_log("start server for communication to gosa: $gosa_address", 1);
        
    }
}

# create gosa job queue as a SQLite DB 
my @col_names = ("id", "timestamp", "status", "result", "header", 
                "target", "xml", "mac");
my $table_name = "jobs";
my $sqlite = GOSA::DBsqlite->new($job_queue_file_name);
#$sqlite->create_table($table_name, \@col_names);




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
#         NAME:  get_ip_and_mac 
#   PARAMETERS:  nothing
#      RETURNS:  (ip, mac) 
#  DESCRIPTION:  executes /sbin/ifconfig and parses the output, the first occurence 
#                of a inet address is returned as well as the mac address in the line
#                above the inet address
#===============================================================================
sub get_ip_and_mac {
    my $ip = "0.0.0.0.0"; # Defualt-IP
    my $mac = "00:00:00:00:00:00";  # Default-MAC
    my @ifconfig = qx(/sbin/ifconfig);
    foreach(@ifconfig) {
        if (/Hardware Adresse (\S{2}):(\S{2}):(\S{2}):(\S{2}):(\S{2}):(\S{2})/) {
            $mac = "$1:$2:$3:$4:$5:$6";
            next;
        }
        if (/inet Adresse:(\d+).(\d+).(\d+).(\d+)/) {
            $ip = "$1.$2.$3.$4";
            last;
        }
    }
    return ($ip, $mac);
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
    if(not defined $crypted_msg) {
        &main::daemon_log("function 'process_incoming_msg': got no msg", 7);
    }
    &main::daemon_log("GosaPackages: incoming msg: \n$crypted_msg", 7);

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
    
    &main::daemon_log("recieve '$header' at GosaPackages from $host", 1);
    &main::daemon_log("$msg", 7);
    
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
        my $sql = "UPDATE '$main::job_queue_table_name' SET status='done', result='$out_msg' WHERE id='$job_id'";
        my $res = $main::job_db->exec_statement($sql);
        return;

    } else {
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
    &main::daemon_log("GosaPackages: got a job msg $header", 5);
    
    # check wether mac address is already known in known_daemons or known_clients
    my $target = 'not known until now';

    # add job to job queue
    my $func_dic = {table=>$main::job_queue_table_name, 
                    timestamp=>@{$msg_hash->{timestamp}}[0],
                    status=>'waiting', 
                    result=>'none',
                    header=>$header, 
                    target=>$target,
                    xmlmessage=>$msg,
                    macaddress=>@{$msg_hash->{mac}}[0],
                    };
    my $res = $main::job_db->add_dbentry($func_dic);
    if (not $res == 0) {
        &main::daemon_log("ERROR: GosaPackages: process_job_msg: $res", 1);
    }
    
    &main::daemon_log("GosaPackages: $header job successfully added to job queue", 3);
    return;

}


sub db_res_2_xml {
    my ($db_res) = @_ ;
    my $xml = "<xml>";

    while ( my ($hit, $hash) = each %{ $db_res } ) {
        $xml .= "<$hit>";

        while ( my ($column_name, $column_value) = each %{$hash} ) {
            $xml .= "<$column_name>";
            my $xml_content = $column_value;
            if( $column_name eq "xml" ) {
                $xml_content = &encode_base64($column_value);
            }
            $xml .= $xml_content;
            $xml .= "</$column_name>"; 
        }

        $xml .= "</$hit>";
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



1;










