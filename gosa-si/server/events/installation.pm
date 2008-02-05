package installation;
use Exporter;
@ISA = qw(Exporter);
my @events = qw(get_events set_activated_for_installation reboot halt softupdate reinstall);
@EXPORT = @events;

use strict;
use warnings;
use GOSA::GosaSupportDaemon;


BEGIN {}

END {}

### Start ######################################################################

#&main::read_configfile($main::cfg_file, %cfg_defaults);

sub get_events {
    return \@events;
}

sub set_activated_for_installation {
    my ($msg, $msg_hash) = @_;

    my $header = @{$msg_hash->{header}}[0];
    my $source = @{$msg_hash->{source}}[0];
    my $target = @{$msg_hash->{target}}[0];

    my $out_hash = &create_xml_hash("set_activated_for_installation", $source, $target);
    my $out_msg = &create_xml_string($out_hash);

    return $out_msg;
}

sub reboot {
    my ($msg, $msg_hash) = @_;
    $msg =~ s/<header>gosa_reboot<\/header>/<header>reboot<\/header>/;

    return $msg;
}


sub halt {
    my ($msg, $msg_hash) = @_;
    $msg =~ s/<header>gosa_halt<\/header>/<header>halt<\/header>/;

    return $msg;
}


sub reinstall {
    my ($msg, $msg_hash) = @_;
    $msg =~ s/<header>gosa_reinstall<\/header>/<header>reinstall<\/header>/;

    return $msg;
}


sub softupdate {
    my ($msg, $msg_hash) = @_;
    $msg =~ s/<header>gosa_softupdate<\/header>/<header>softupdate<\/header>/;

    return $msg;
}

1;
