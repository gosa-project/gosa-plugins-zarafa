package gosaTriggered;
use Exporter;
@ISA = qw(Exporter);
my @events = (
    "get_events", 
    "ping",
    "set_activated_for_installation",
    "new_key_for_client",
    "detect_hardware",
    "trigger_action_localboot",
    "trigger_action_reboot",
    "trigger_action_halt",
    "trigger_action_update", 
    "trigger_action_reinstall",
    "trigger_action_memcheck", 
    "trigger_action_sysinfo",
    "trigger_action_instant_update",
    "trigger_action_rescan",
    );
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

sub ping {
     my ($msg, $msg_hash, $session_id) = @_ ;
     my $source = @{$msg_hash->{source}}[0];
     my $target = @{$msg_hash->{target}}[0];
     my $out_hash =  &create_xml_hash("ping", $source, $target);
     &add_content2xml_hash($out_hash, "session_id", $session_id);
     my $out_msg = &create_xml_string($out_hash);
    
     return ( $out_msg );
}

sub detect_hardware {
    my ($msg, $msg_hash) = @_ ;
    # just forward msg to client, but dont forget to split off 'gosa_' in header
    my $header = @{$msg_hash->{header}}[0];
    my $source = @{$msg_hash->{source}}[0];
    my $target = @{$msg_hash->{target}}[0];

    my $out_hash = &create_xml_hash("detect_hardware", $source, $target);
    my $out_msg = &create_xml_string($out_hash);

    return ( $out_msg );
}


sub set_activated_for_installation {
    my ($msg, $msg_hash) = @_;

    my $header = @{$msg_hash->{header}}[0];
    my $source = @{$msg_hash->{source}}[0];
    my $target = @{$msg_hash->{target}}[0];

    my $out_hash = &create_xml_hash("set_activated_for_installation", $source, $target);
    my $out_msg = &create_xml_string($out_hash);

    return ( $out_msg );
}

sub trigger_action_localboot {
    my ($msg, $msg_hash) = @_;
    $msg =~ s/<header>gosa_trigger_action_localboot<\/header>/<header>trigger_action_localboot<\/header>/;

    return ( $msg );
}

sub trigger_action_halt {
    my ($msg, $msg_hash) = @_;
    $msg =~ s/<header>gosa_trigger_action_halt<\/header>/<header>trigger_action_halt<\/header>/;
    return ( $msg );
}


sub trigger_action_reboot {
    my ($msg, $msg_hash) = @_;
    $msg =~ s/<header>gosa_trigger_action_reboot<\/header>/<header>trigger_action_reboot<\/header>/;
    return ( $msg );
}


sub trigger_action_memcheck {
    my ($msg, $msg_hash) = @_ ;
    $msg =~ s/<header>gosa_trigger_action_memcheck<\/header>/<header>trigger_action_memcheck<\/header>/;
    return ( $msg );
}


sub trigger_action_reinstall {
    my ($msg, $msg_hash) = @_;
    $msg =~ s/<header>gosa_trigger_action_reinstall<\/header>/<header>trigger_action_reinstall<\/header>/;
    return ( $msg );
}


sub trigger_action_update {
    my ($msg, $msg_hash) = @_;
    $msg =~ s/<header>gosa_trigger_action_update<\/header>/<header>trigger_action_update<\/header>/;
    return ( $msg );
}


sub trigger_action_instant_update {
    my ($msg, $msg_hash) = @_;
    $msg =~ s/<header>gosa_trigger_action_instant_update<\/header>/<header>trigger_action_instant_update<\/header>/;
    return ( $msg );
}


sub trigger_action_sysinfo {
    my ($msg, $msg_hash) = @_;
    $msg =~ s/<header>gosa_trigger_action_sysinfo<\/header>/<header>trigger_action_sysinfo<\/header>/;
    return ( $msg );
}



sub new_key_for_client {
    my ($msg, $msg_hash) = @_;
    $msg =~ s/<header>gosa_new_key_for_client<\/header>/<header>new_key<\/header>/;
    return ( $msg );
}

sub trigger_action_rescan {
    my ($msg, $msg_hash) = @_;
    $msg =~ s/<header>gosa_trigger_action_rescan<\/header>/<header>trigger_action_rescan<\/header>/;
    return ( $msg );
}

1;
