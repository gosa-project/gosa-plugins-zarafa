package registered;
use Exporter;
@ISA = qw(Exporter);
@EXPORT = qw(get_events registered set_activated_for_installation);


use strict;
use warnings;
use Data::Dumper;
use Fcntl;

BEGIN {}

END {}

### Start ######################################################################


sub get_events {
    my @events = ('registered', 'set_activated_for_installation');
    return \@events;
}

sub registered {
    my ($msg, $msg_hash) = @_ ;
    my $header = @{$msg_hash->{'header'}}[0];
   
    if( $header eq "registered" ) {
        my $source = @{$msg_hash->{'source'}}[0];
        &main::daemon_log("registration at $source",1);
    }
    
    # set registration_flag to true 
    return 0;

}


sub set_activated_for_installation {
    my ($msg, $msg_hash) = @_ ;
 my $header = @{$msg_hash->{'header'}}[0];
 my $target = @{$msg_hash->{'target'}}[0];
 my $source = @{$msg_hash->{'source'}}[0];

    my $Datei = "/tmp/set_activated_for_installation";
    open(DATEI, ">$Datei");
    print DATEI "set_activated_for_installation\n";
    close DATEI;
    
    return 0;
}



1;
