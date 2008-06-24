package opsi;
use Exporter;
@ISA = qw(Exporter);
my @events = (
    "get_events",
    "opsi_get_netboot_products",
    "opsi_get_local_products",
    "opsi_get_product_properties",
    "opsi_get_client_hardware",
    "opsi_get_client_software",
    "answer_opsi_get_netboot_products",
    "answer_opsi_get_local_products",
    "answer_opsi_get_product_properties",
    "answer_opsi_set_product_properties",
    "answer_opsi_get_client_hardware",
    "answer_opsi_get_client_software",
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

    
sub opsi_get_netboot_products {
        my ($msg, $msg_hash, $session_id) = @_;
        $msg =~ s/gosa_opsi/opsi/g;
        return ( $msg );
}


sub opsi_set_product_properties {
        my ($msg, $msg_hash, $session_id) = @_;
        $msg =~ s/gosa_opsi/opsi/g;
        return ( $msg );
}


sub opsi_get_product_properties {
        my ($msg, $msg_hash, $session_id) = @_;
        $msg =~ s/gosa_opsi/opsi/g;
        return ( $msg );
}


sub opsi_get_local_products {
        my ($msg, $msg_hash, $session_id) = @_;
        $msg =~ s/gosa_opsi/opsi/g;
        return ( $msg );
}

sub opsi_get_client_hardware {
        my ($msg, $msg_hash, $session_id) = @_;
        $msg =~ s/gosa_opsi/opsi/g;
        return ( $msg );
}

sub opsi_get_client_software {
        my ($msg, $msg_hash, $session_id) = @_;
        $msg =~ s/gosa_opsi/opsi/g;
        return ( $msg );
}

sub answer_opsi_get_netboot_products {
    my ($msg, $msg_hash, $session_id) = @_ ;
    $msg =~ s/<target>\S+<\/target>/<target>GOSA<\/target>/g;
    return ($msg);
}

sub answer_opsi_get_product_properties {
    my ($msg, $msg_hash, $session_id) = @_ ;
    $msg =~ s/<target>\S+<\/target>/<target>GOSA<\/target>/g;
    return ($msg);
}

sub answer_opsi_set_product_properties {
    my ($msg, $msg_hash, $session_id) = @_ ;
    $msg =~ s/<target>\S+<\/target>/<target>GOSA<\/target>/g;
    return ($msg);
}

sub answer_opsi_get_local_products {
    my ($msg, $msg_hash, $session_id) = @_ ;
    $msg =~ s/<target>\S+<\/target>/<target>GOSA<\/target>/g;
    return ($msg);
}

sub answer_opsi_get_client_hardware {
    my ($msg, $msg_hash, $session_id) = @_ ;
    $msg =~ s/<target>\S+<\/target>/<target>GOSA<\/target>/g;
    return ($msg);
}

sub answer_opsi_get_client_software {
    my ($msg, $msg_hash, $session_id) = @_ ;
    $msg =~ s/<target>\S+<\/target>/<target>GOSA<\/target>/g;
    return ($msg);
}

1;
