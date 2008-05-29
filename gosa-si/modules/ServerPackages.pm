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
            $main::foreign_server_key,            
            );
    return \@info;
}

sub process_incoming_msg {
    my ($msg, $msg_hash, $session_id) = @_ ;
    my $header = @{$msg_hash->{header}}[0];
    my $source = @{$msg_hash->{source}}[0]; 
    my $target = @{$msg_hash->{target}}[0];
    my $sql_events;

    my @msg_l;
    my @out_msg_l;

    &main::daemon_log("$session_id DEBUG: ServerPackages: msg to process '$header'", 7);
    if( exists $event_hash->{$header} ) {
        # a event exists with the header as name
        &main::daemon_log("$session_id INFO: found event '$header' at event-module '".$event_hash->{$header}."'", 5);
        no strict 'refs';
        @out_msg_l = &{$event_hash->{$header}."::$header"}($msg, $msg_hash, $session_id);

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
        &main::daemon_log("$session_id ERROR: ServerPackages: no event handler defined for '$header'", 1);
        @out_msg_l = ();
    } elsif ($out_msg_l[0] eq 'knownclienterror') {
        &main::daemon_log("$session_id ERROR: no or more than 1 hits are found at known_clients_db with sql query: '$sql_events'", 1);
        &main::daemon_log("$session_id WARNING: processing is aborted and message will not be forwarded", 3);
        @out_msg_l = ();
    } elsif ($out_msg_l[0] eq 'noeventerror') {
        &main::daemon_log("$session_id WARNING: client '$target' is not registered for event '$header', processing is aborted", 3); 
        @out_msg_l = ();
    }
      
    return \@out_msg_l;
}

1;
