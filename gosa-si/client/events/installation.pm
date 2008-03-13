package installation;
use Exporter;
@ISA = qw(Exporter);
my @events = qw(get_events set_activated_for_installation);
@EXPORT = @events;

use strict;
use warnings;
use Data::Dumper;
use Fcntl;

BEGIN {}

END {}

### Start ######################################################################


sub get_events {
    return \@events;
}

sub set_activated_for_installation {
    my ($msg, $msg_hash) = @_ ;
    my $header = @{$msg_hash->{'header'}}[0];
    my $target = @{$msg_hash->{'target'}}[0];
    my $source = @{$msg_hash->{'source'}}[0];

    my $Datei = "/var/run/gosa-si-client.activated";
    open(DATEI, ">$Datei");
    print DATEI "$msg\n";
    close DATEI;

    return;
}



1;
