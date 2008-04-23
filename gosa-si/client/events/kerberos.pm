package kerberos;
use Exporter;
@ISA = qw(Exporter);
my @events = (
    "get_events",
    "krb5_list_principals",
    );
@EXPORT = @events;

use strict;
use warnings;
use GOSA::GosaSupportDaemon;

BEGIN {}

END {}
sub get_events { return \@events; }


sub krb5_list_principals {
    my ($msg, $msg_hash) = @_;
    my $header = @{$msg_hash->{'header'}}[0];
    my $source = @{$msg_hash->{'source'}}[0];
    my $target = @{$msg_hash->{'target'}}[0];
    my $session_id = @{$msg_hash->{'session_id'}}[0];

    # do now whatever kerb5_list_pricipals has to do 
    
    # build return message with twisted target and source
    my $out_hash = &main::create_xml_hash("answer_krb5_list_principals", $target, $source);
    &add_content2xml_hash($out_hash, "session_id", $session_id);
    &add_content2xml_hash($out_hash, "principal", 'rettenberger@GONICUS.DE');
    my $out_msg = &create_xml_string($out_hash);

    return $out_msg;

}


1;
