package ServerPackages;

use Exporter;
@ISA = ("Exporter");

# Each module has to have a function 'process_incoming_msg'. This function works as a interface to gosa-sd and receives the msg hash from gosa-sd. 'process_incoming_function checks, wether it has a function to process the incoming msg and forward the msg to it. 

use strict;
use warnings;
use GOSA::GosaSupportDaemon;

#use IO::Socket::INET;
#use XML::Simple;
#use Data::Dumper;
#use NetAddr::IP;
#use Net::LDAP;
#use Socket;
#use Net::hostent;

my $event_dir = "/usr/lib/gosa-si/server/events";
use lib "/usr/lib/gosa-si/server/events";

BEGIN{}
END {}


### START #####################################################################

# read configfile and import variables
#&read_configfile();

sub get_module_info {
    my @info = ($server_address,
                $SIPackages_key,
                );
    return \@info;
}


1;
