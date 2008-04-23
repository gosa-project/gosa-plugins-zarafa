package kerberos;
use Exporter;
@ISA = qw(Exporter);
my @events = (
    "get_events",
    "krb5_list_principals",  
    "krb5_list_policies",
    "krb5_get_principal",
    "krb5_set_principal",
    "krb5_del_principal",
    "krb5_get_policy",
    "krb5_set_policy",
    "krb5_del_policy",

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
    &add_content2xml_hash($out_hash, "principal", 'pollmeier@GONICUS.DE');
    &add_content2xml_hash($out_hash, "principal", 'hickert@GONICUS.DE');
    my $out_msg = &create_xml_string($out_hash);

    # return message
    return $out_msg;

}


sub krb5_set_principal {
    my ($msg, $msg_hash) = @_;
    my $header = @{$msg_hash->{'header'}}[0];
    my $source = @{$msg_hash->{'source'}}[0];
    my $target = @{$msg_hash->{'target'}}[0];
    my $session_id = @{$msg_hash->{'session_id'}}[0];

    # build return message with twisted target and source
    my $out_hash = &main::create_xml_hash("answer_krb5_list_principals", $target, $source);
    my $out_msg = &create_xml_string($out_hash);

    # return message
    return $out_msg;


}


sub krb5_get_principal {
    my ($msg, $msg_hash) = @_;
    my $header = @{$msg_hash->{'header'}}[0];
    my $source = @{$msg_hash->{'source'}}[0];
    my $target = @{$msg_hash->{'target'}}[0];
    my $session_id = @{$msg_hash->{'session_id'}}[0];

    # build return message with twisted target and source
    my $out_hash = &main::create_xml_hash("answer_krb5_list_principals", $target, $source);
    my $out_msg = &create_xml_string($out_hash);

    # return message
    return $out_msg;


}


sub krb5_del_principal {
    my ($msg, $msg_hash) = @_;
    my $header = @{$msg_hash->{'header'}}[0];
    my $source = @{$msg_hash->{'source'}}[0];
    my $target = @{$msg_hash->{'target'}}[0];
    my $session_id = @{$msg_hash->{'session_id'}}[0];

    # build return message with twisted target and source
    my $out_hash = &main::create_xml_hash("answer_krb5_list_principals", $target, $source);
    my $out_msg = &create_xml_string($out_hash);

    # return message
    return $out_msg;


}


sub krb5_list_policies {
    my ($msg, $msg_hash) = @_;
    my $header = @{$msg_hash->{'header'}}[0];
    my $source = @{$msg_hash->{'source'}}[0];
    my $target = @{$msg_hash->{'target'}}[0];
    my $session_id = @{$msg_hash->{'session_id'}}[0];

    # build return message with twisted target and source
    my $out_hash = &main::create_xml_hash("answer_krb5_list_principals", $target, $source);
    my $out_msg = &create_xml_string($out_hash);

    # return message
    return $out_msg;


}


sub krb5_get_policy {
    my ($msg, $msg_hash) = @_;
    my $header = @{$msg_hash->{'header'}}[0];
    my $source = @{$msg_hash->{'source'}}[0];
    my $target = @{$msg_hash->{'target'}}[0];
    my $session_id = @{$msg_hash->{'session_id'}}[0];

    # build return message with twisted target and source
    my $out_hash = &main::create_xml_hash("answer_krb5_list_principals", $target, $source);
    my $out_msg = &create_xml_string($out_hash);

    # return message
    return $out_msg;


}


sub krb5_set_policy {
    my ($msg, $msg_hash) = @_;
    my $header = @{$msg_hash->{'header'}}[0];
    my $source = @{$msg_hash->{'source'}}[0];
    my $target = @{$msg_hash->{'target'}}[0];
    my $session_id = @{$msg_hash->{'session_id'}}[0];

    # build return message with twisted target and source
    my $out_hash = &main::create_xml_hash("answer_krb5_list_principals", $target, $source);
    my $out_msg = &create_xml_string($out_hash);

    # return message
    return $out_msg;


}


sub krb5_del_policy {
    my ($msg, $msg_hash) = @_;
    my $header = @{$msg_hash->{'header'}}[0];
    my $source = @{$msg_hash->{'source'}}[0];
    my $target = @{$msg_hash->{'target'}}[0];
    my $session_id = @{$msg_hash->{'session_id'}}[0];

    # build return message with twisted target and source
    my $out_hash = &main::create_xml_hash("answer_krb5_list_principals", $target, $source);
    my $out_msg = &create_xml_string($out_hash);

    # return message
    return $out_msg;


}

1;
