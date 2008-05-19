package server_server_com;
use Exporter;
@ISA = qw(Exporter);
my @events = (
    'new_server',
    );
@EXPORT = @events;

use strict;
use warnings;
use Data::Dumper;
use GOSA::GosaSupportDaemon;


BEGIN {}

END {}

### Start ######################################################################

sub get_events {
    return \@events;
}


sub new_server {
print STDERR "hier kommt die logik von new_server\n"; 
}

1;
