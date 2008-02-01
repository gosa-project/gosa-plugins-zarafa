package registered;
use Exporter;
@ISA = qw(Exporter);
@EXPORT = qw(get_events registered);


use strict;
use warnings;
use Data::Dumper;

BEGIN {}

END {}

### Start ######################################################################


sub get_events {
    my @events = ('registered');
    return \@events;
}

sub registered {
    my ($msg, $msg_hash) = @_ ;
    my $header = @{$msg_hash->{'header'}}[0];
   
    if( $header eq "registered" ) {
        my $source = @{$msg_hash->{'source'}}[0];
        &main::daemon_log("Registration at $source",1);
    }
    
    # set registration_flag to true 
    return 0;

}






1;
