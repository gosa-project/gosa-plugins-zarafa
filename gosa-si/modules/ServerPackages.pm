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

my $event_dir = "/usr/lib/gosa-si/server/ServerPackages";
use lib "/usr/lib/gosa-si/server/ServerPackages";

BEGIN{}
END {}


### START #####################################################################

# import local events
my ($error, $result, $event_hash) = &import_events($event_dir);
if ($error == 0) {
    foreach my $log_line (@$result) {
        &main::daemon_log("0 DEBUG: ServerPackages - $log_line", 7);
    }
} else {
    foreach my $log_line (@$result) {
        &main::daemon_log("0 ERROR: ServerPackages - $log_line", 1);
    }
}

### FUNCTIONS #####################################################################

sub get_module_info {
    my @info = ($main::server_address,
            );
    return \@info;
}


1;
