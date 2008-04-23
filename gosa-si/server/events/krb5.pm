package krb5;
use Exporter;
@ISA = qw(Exporter);
my @events = (
    "get_events",
    "krb5_list_principals",
    "krb5_list_policies",
    "krb5_get_principal",
    "krb5_create_principal",
    "krb5_modify_principal",
    "krb5_set_password",
    "krb5_del_principal",
    "krb5_get_policy",
    "krb5_create_policy",
    "krb5_modify_policy",
    "krb5_del_policy",
    "answer_krb5_list_principals",
    "answer_krb5_list_policies",
    "answer_krb5_get_principal",
    "answer_krb5_create_principal",
    "answer_krb5_modify_principal",
    "answer_krb5_del_principal",
    "answer_krb5_get_policy",
    "answer_krb5_create_policy",
    "answer_krb5_modify_policy",
    "answer_krb5_del_policy",
    "answer_krb5_set_password",
   );
@EXPORT = @events;

use strict;
use warnings;
use GOSA::GosaSupportDaemon;


BEGIN {}

END {}

### Start ######################################################################

sub get_events {
    return \@events;
}

    
sub krb5_list_principals {
        my ($msg, $msg_hash, $session_id) = @_;
        $msg =~ s/gosa_krb5/krb5/g;
        return ( $msg );
}

sub krb5_set_password {
        my ($msg, $msg_hash, $session_id) = @_;
        $msg =~ s/gosa_krb5/krb5/g;
        return ( $msg );
}


sub krb5_create_principal {
        my ($msg, $msg_hash, $session_id) = @_;
        $msg =~ s/gosa_krb5/krb5/g;
        return ( $msg );
}


sub krb5_modify_principal {
        my ($msg, $msg_hash, $session_id) = @_;
        $msg =~ s/gosa_krb5/krb5/g;
        return ( $msg );
}


sub krb5_get_principal {
        my ($msg, $msg_hash, $session_id) = @_;
        $msg =~ s/gosa_krb5/krb5/g;
        return ( $msg );
}


sub krb5_del_principal {
        my ($msg, $msg_hash, $session_id) = @_;
        $msg =~ s/gosa_krb5/krb5/g;
        return ( $msg );
}


sub krb5_list_policies {
        my ($msg, $msg_hash, $session_id) = @_;
        $msg =~ s/gosa_krb5/krb5/g;
        return ( $msg );
}


sub krb5_get_policy {
        my ($msg, $msg_hash, $session_id) = @_;
        $msg =~ s/gosa_krb5/krb5/g;
        return ( $msg );
}


sub krb5_create_policy {
        my ($msg, $msg_hash, $session_id) = @_;
        $msg =~ s/gosa_krb5/krb5/g;
        return ( $msg );
}


sub krb5_modify_policy {
        my ($msg, $msg_hash, $session_id) = @_;
        $msg =~ s/gosa_krb5/krb5/g;
        return ( $msg );
}


sub krb5_del_policy {
        my ($msg, $msg_hash, $session_id) = @_;
        $msg =~ s/gosa_krb5/krb5/g;
        return ( $msg );
}

sub answer_krb5_list_principals {
    my ($msg, $msg_hash, $session_id) = @_ ;
    $msg =~ s/<target>\S+<\/target>/<target>GOSA<\/target>/g;
    return ($msg);
}

sub answer_krb5_get_principal {
    my ($msg, $msg_hash, $session_id) = @_ ;
    $msg =~ s/<target>\S+<\/target>/<target>GOSA<\/target>/g;
    return ($msg);
}

sub answer_krb5_create_principal {
    my ($msg, $msg_hash, $session_id) = @_ ;
    $msg =~ s/<target>\S+<\/target>/<target>GOSA<\/target>/g;
    return ($msg);
}

sub answer_krb5_modify_principal {
    my ($msg, $msg_hash, $session_id) = @_ ;
    $msg =~ s/<target>\S+<\/target>/<target>GOSA<\/target>/g;
    return ($msg);
}

sub answer_krb5_del_principal {
    my ($msg, $msg_hash, $session_id) = @_ ;
    $msg =~ s/<target>\S+<\/target>/<target>GOSA<\/target>/g;
    return ($msg);
}

sub answer_krb5_list_policies {
    my ($msg, $msg_hash, $session_id) = @_ ;
    $msg =~ s/<target>\S+<\/target>/<target>GOSA<\/target>/g;
    return ($msg);
}

sub answer_krb5_get_policy {
    my ($msg, $msg_hash, $session_id) = @_ ;
    $msg =~ s/<target>\S+<\/target>/<target>GOSA<\/target>/g;
    return ($msg);
}

sub answer_krb5_create_policy {
    my ($msg, $msg_hash, $session_id) = @_ ;
    $msg =~ s/<target>\S+<\/target>/<target>GOSA<\/target>/g;
    return ($msg);
}

sub answer_krb5_modify_policy {
    my ($msg, $msg_hash, $session_id) = @_ ;
    $msg =~ s/<target>\S+<\/target>/<target>GOSA<\/target>/g;
    return ($msg);
}

sub answer_krb5_del_policy {
    my ($msg, $msg_hash, $session_id) = @_ ;
    $msg =~ s/<target>\S+<\/target>/<target>GOSA<\/target>/g;
    return ($msg);
}

sub answer_krb5_set_password {
    my ($msg, $msg_hash, $session_id) = @_ ;
    $msg =~ s/<target>\S+<\/target>/<target>GOSA<\/target>/g;
    return ($msg);
}

1;
