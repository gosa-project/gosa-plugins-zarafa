## @file
# @details A GOsa-SI-server event module containing all functions for message handling.
# @brief Implementation of an event module for GOsa-SI-server. 


package opsi_com;
use Exporter;
@ISA = qw(Exporter);
my @events = (
    "get_events",
    "opsi_install_client",
#    "opsi_get_netboot_products",
#    "opsi_get_local_products",
#    "opsi_get_product_properties",
#    "opsi_get_client_hardware",
#    "opsi_get_client_software",
   );
@EXPORT = @events;

use strict;
use warnings;
use GOSA::GosaSupportDaemon;
use Data::Dumper;


BEGIN {}

END {}

## @method get_events()
# A brief function returning a list of functions which are exported by importing the module.
# @return List of all provided functions
sub get_events {
    return \@events;
}

    
## @method opsi_install_client
# 
# @param msg - STRING - xml message with tags 
# @param msg_hash - HASHREF - message information parsed into a hash
# @param session_id - INTEGER - POE session id of the processing of this message
sub opsi_install_client {
    my ($msg, $msg_hash, $session_id) = @_ ;
    my $error = 0;
    my $out_msg;
    my $out_hash;

    # Prepare incoming message
    $msg =~ s/<header>gosa_/<header>/;
    $msg_hash->{'header'}[0] =~ s/gosa_//;


    # Assign variables
    my $header = @{$msg_hash->{'header'}}[0];
    my $source = @{$msg_hash->{'source'}}[0];
    my $target = @{$msg_hash->{'target'}}[0];


    # If no timestamp is specified in incoming message, use 19700101000000
    my $timestamp = "19700101000000";
    if( exists $msg_hash->{'timestamp'} ) {
        $timestamp = @{$msg_hash->{'timestamp'}}[0];
    }
     

    # If no macaddress is specified, raise error 
    my $macaddress;
    if ((exists $msg_hash->{'macaddress'}) &&
            ($msg_hash->{'macaddress'}[0] =~ /^([0-9a-f]{2}:[0-9a-f]{2}:[0-9a-f]{2}:[0-9a-f]{2}:[0-9a-f]{2}:[0-9a-f]{2})$/i)) { 
        $macaddress = $1;
    } else {
        $error ++;
        $out_msg = "<xml>".
            "<header>answer</header>".
            "<source>$main::server_address</source>".
            "<target>GOSA</target>".
            "<answer1>1</answer1>".
            "<error_string>no mac address specified in macaddres-tag</error_string>".
            "</xml>";
    }
    

    # Set hostID to plain_name
    my $plain_name;
    if (not $error) {
        if (exists $msg_hash->{'hostId'}) {
            $plain_name = $msg_hash->{'hostId'}[0];
        } else {
            $error++;
            $out_msg = "<xml>".
            "<header>answer</header>".
            "<source>$main::server_address</source>".
            "<target>GOSA</target>".
            "<answer1>1</answer1>".
            "<error_string>no hostId specified in hostId-tag</error_string>".
            "</xml>";
        }
    }


    # Add installation job to job queue
    if (not $error) {
        my $insert_dic = {table=>$main::job_queue_tn, 
            primkey=>['macaddress', 'headertag'],
            timestamp=>&get_time(),
            status=>'processing', 
            result=>'none',
            progress=>'none',
            headertag=>$header, 
            targettag=>$target,
            xmlmessage=>$msg,
            macaddress=>$macaddress,
            plainname=>$plain_name,
            siserver=>"windowsOpsi",
            modified=>"1",
        };
        my $res = $main::job_db->add_dbentry($insert_dic);
        if (not $res == 0) {
            &main::daemon_log("$session_id ERROR: Cannot add opsi-job to job_queue: $msg", 1);
        } else {
            &main::daemon_log("$session_id INFO: '$header'-job successfully added to job queue", 5);
        }
        $out_msg = $msg;   # forward GOsa message to client 
    }
    
    return ($out_msg);
}




#sub opsi_get_netboot_products {
#        my ($msg, $msg_hash, $session_id) = @_;
#        $msg =~ s/gosa_opsi/opsi/g;
#        return ( $msg );
#}
#
#
#sub opsi_set_product_properties {
#        my ($msg, $msg_hash, $session_id) = @_;
#        $msg =~ s/gosa_opsi/opsi/g;
#        return ( $msg );
#}
#
#
#sub opsi_get_product_properties {
#        my ($msg, $msg_hash, $session_id) = @_;
#        $msg =~ s/gosa_opsi/opsi/g;
#        return ( $msg );
#}
#
#
#sub opsi_get_local_products {
#        my ($msg, $msg_hash, $session_id) = @_;
#        $msg =~ s/gosa_opsi/opsi/g;
#        return ( $msg );
#}
#
#sub opsi_get_client_hardware {
#        my ($msg, $msg_hash, $session_id) = @_;
#        $msg =~ s/gosa_opsi/opsi/g;
#        return ( $msg );
#}
#
#sub opsi_get_client_software {
#        my ($msg, $msg_hash, $session_id) = @_;
#        $msg =~ s/gosa_opsi/opsi/g;
#        return ( $msg );
#}
#
1;
