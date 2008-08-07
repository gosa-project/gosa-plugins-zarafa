## @file
# @details A GOsa-SI-server event module containing all functions for message handling.
# @brief Implementation of an event module for GOsa-SI-server. 


package opsi_com;
use Exporter;
@ISA = qw(Exporter);
my @events = (
    "get_events",
    "opsi_install_client",
    "opsi_get_netboot_products",  
    "opsi_get_local_products",
    "opsi_get_client_hardware",
    "opsi_get_client_software",
    "opsi_get_product_properties",
    "opsi_set_product_properties",
    "opsi_list_clients",
    "opsi_del_client",
    "opsi_add_client",
    "opsi_modify_client",
    "opsi_add_product_to_client",
    "opsi_del_product_from_client",
   );
@EXPORT = @events;

use strict;
use warnings;
use GOSA::GosaSupportDaemon;
use Data::Dumper;
use XML::Quote qw(:all);


BEGIN {}

END {}

## @method get_events()
# A brief function returning a list of functions which are exported by importing the module.
# @return List of all provided functions
sub get_events {
    return \@events;
}

## @method opsi_add_product_to_client
# Adds an Opsi product to an Opsi client.
# @param msg - STRING - xml message with tags hostId and productId
# @param msg_hash - HASHREF - message information parsed into a hash
# @param session_id - INTEGER - POE session id of the processing of this message
# @return out_msg - STRING - feedback to GOsa in success and error case
sub opsi_add_product_to_client {
    my ($msg, $msg_hash, $session_id) = @_;
    my $header = @{$msg_hash->{'header'}}[0];
    my $source = @{$msg_hash->{'source'}}[0];
    my $target = @{$msg_hash->{'target'}}[0];
    my $forward_to_gosa = @{$msg_hash->{'forward_to_gosa'}}[0];
    my ($hostId, $productId);
    my $error = 0;

    # Build return message
    my $out_hash = &main::create_xml_hash("answer_$header", $main::server_address, $source);
    if (defined $forward_to_gosa) {
        &add_content2xml_hash($out_hash, "forward_to_gosa", $forward_to_gosa);
    }

    # Sanity check of needed parameter
    if ((not exists $msg_hash->{'hostId'}) || (@{$msg_hash->{'hostId'}} != 1))  {
        $error++;
        &add_content2xml_hash($out_hash, "hostId_error", "no hostId specified or hostId tag invalid");
        &add_content2xml_hash($out_hash, "error", "hostId");
        &main::daemon_log("$session_id ERROR: no hostId specified or hostId tag invalid: $msg", 1); 

    }
    if ((not exists $msg_hash->{'productId'}) || (@{$msg_hash->{'productId'}} != 1)) {
        $error++;
        &add_content2xml_hash($out_hash, "productId_error", "no productId specified or productId tag invalid");
        &add_content2xml_hash($out_hash, "error", "productId");
        &main::daemon_log("$session_id ERROR: no productId specified or procutId tag invalid: $msg", 1); 
    }

    if (not $error) {
        # Get hostID
        $hostId = @{$msg_hash->{'hostId'}}[0];
        &add_content2xml_hash($out_hash, "hostId", $hostId);

        # Get productID
        $productId = @{$msg_hash->{'productId'}}[0];
        &add_content2xml_hash($out_hash, "productId", $productId);

        # Do an action request for all these -> "setup".
        my $callobj = {
            method  => 'setProductActionRequest',
            params  => [ $productId, $hostId, "setup" ],
            id  => 1, }; 

        my $sres = $main::opsi_client->call($main::opsi_url, $callobj);
        my ($sres_err, $sres_err_string) = &check_opsi_res($sres);
        if ($sres_err){
            &main::daemon_log("$session_id ERROR: cannot add product: ".$sres_err_string, 1);
            &add_content2xml_hash($out_hash, "error", $sres_err_string);
        }
    } 

    # return message
    return ( &create_xml_string($out_hash) );
}

## @method opsi_del_product_from_client
# Deletes an Opsi-product from an Opsi-client. 
# @param msg - STRING - xml message with tags hostId and productId
# @param msg_hash - HASHREF - message information parsed into a hash
# @param session_id - INTEGER - POE session id of the processing of this message
# @return out_msg - STRING - feedback to GOsa in success and error case
sub opsi_del_product_from_client {
    my ($msg, $msg_hash, $session_id) = @_;
    my $header = @{$msg_hash->{'header'}}[0];
    my $source = @{$msg_hash->{'source'}}[0];
    my $target = @{$msg_hash->{'target'}}[0];
    my $forward_to_gosa = @{$msg_hash->{'forward_to_gosa'}}[0];
    my ($hostId, $productId);
    my $error = 0;
    my ($sres, $sres_err, $sres_err_string);

    # Build return message
    my $out_hash = &main::create_xml_hash("answer_$header", $main::server_address, $source);
    if (defined $forward_to_gosa) {
        &add_content2xml_hash($out_hash, "forward_to_gosa", $forward_to_gosa);
    }

    # Sanity check of needed parameter
    if ((not exists $msg_hash->{'hostId'}) || (@{$msg_hash->{'hostId'}} != 1))  {
        $error++;
        &add_content2xml_hash($out_hash, "hostId_error", "no hostId specified or hostId tag invalid");
        &add_content2xml_hash($out_hash, "error", "hostId");
        &main::daemon_log("$session_id ERROR: no hostId specified or hostId tag invalid: $msg", 1); 

    }
    if ((not exists $msg_hash->{'productId'}) || (@{$msg_hash->{'productId'}} != 1)) {
        $error++;
        &add_content2xml_hash($out_hash, "productId_error", "no productId specified or productId tag invalid");
        &add_content2xml_hash($out_hash, "error", "productId");
        &main::daemon_log("$session_id ERROR: no productId specified or procutId tag invalid: $msg", 1); 
    }

    # All parameter available
    if (not $error) {
        # Get hostID
        $hostId = @{$msg_hash->{'hostId'}}[0];
        &add_content2xml_hash($out_hash, "hostId", $hostId);

        # Get productID
        $productId = @{$msg_hash->{'productId'}}[0];
        &add_content2xml_hash($out_hash, "productId", $productId);


        #TODO: check the results for more than one entry which is currently installed
        #$callobj = {
        #    method  => 'getProductDependencies_listOfHashes',
        #    params  => [ $productId ],
        #    id  => 1, };
        #
        #my $sres = $main::opsi_client->call($main::opsi_url, $callobj);
        #my ($sres_err, $sres_err_string) = &check_opsi_res($sres);
        #if ($sres_err){
        #  &main::daemon_log("ERROR: cannot perform dependency check: ".$sres_err_string, 1);
        #  &add_content2xml_hash($out_hash, "error", $sres_err_string);
        #  return ( &create_xml_string($out_hash) );
        #}


    # Check to get product action list 
        my $callobj = {
            method  => 'getPossibleProductActions_list',
            params  => [ $productId ],
            id  => 1, };
        $sres = $main::opsi_client->call($main::opsi_url, $callobj);
        ($sres_err, $sres_err_string) = &check_opsi_res($sres);
        if ($sres_err){
            &main::daemon_log("$session_id ERROR: cannot get product action list: ".$sres_err_string, 1);
            &add_content2xml_hash($out_hash, "error", $sres_err_string);
            $error++;
        }
    }

    # Check action uninstall of product
    if (not $error) {
        my $uninst_possible= 0;
        foreach my $r (@{$sres->result}) {
            if ($r eq 'uninstall') {
                $uninst_possible= 1;
            }
        }
        if (!$uninst_possible){
            &main::daemon_log("$session_id ERROR: cannot uninstall product '$productId', product do not has the action 'uninstall'", 1);
            &add_content2xml_hash($out_hash, "error", "cannot uninstall product '$productId', product do not has the action 'uninstall'");
            $error++;
        }
    }

    # Set product state to "none"
    # Do an action request for all these -> "setup".
    if (not $error) {
        my $callobj = {
            method  => 'setProductActionRequest',
            params  => [ $productId, $hostId, "none" ],
            id  => 1, 
        }; 
        $sres = $main::opsi_client->call($main::opsi_url, $callobj);
        ($sres_err, $sres_err_string) = &check_opsi_res($sres);
        if ($sres_err){
            &main::daemon_log("$session_id ERROR: cannot delete product: ".$sres_err_string, 1);
            &add_content2xml_hash($out_hash, "error", $sres_err_string);
        }
    }

    # Return message
    return ( &create_xml_string($out_hash) );
}

## @method opsi_add_client
# Adds an Opsi client to Opsi.
# @param msg - STRING - xml message with tags hostId and macaddress
# @param msg_hash - HASHREF - message information parsed into a hash
# @param session_id - INTEGER - POE session id of the processing of this message
# @return out_msg - STRING - feedback to GOsa in success and error case
sub opsi_add_client {
    my ($msg, $msg_hash, $session_id) = @_;
    my $header = @{$msg_hash->{'header'}}[0];
    my $source = @{$msg_hash->{'source'}}[0];
    my $target = @{$msg_hash->{'target'}}[0];
    my $forward_to_gosa = @{$msg_hash->{'forward_to_gosa'}}[0];
    my ($hostId, $mac);
    my $error = 0;
    my ($sres, $sres_err, $sres_err_string);

    # build return message with twisted target and source
    my $out_hash = &main::create_xml_hash("answer_$header", $main::server_address, $source);
    if (defined $forward_to_gosa) {
        &add_content2xml_hash($out_hash, "forward_to_gosa", $forward_to_gosa);
    }

    # Sanity check of needed parameter
    if ((not exists $msg_hash->{'hostId'}) || (@{$msg_hash->{'hostId'}} != 1))  {
        $error++;
        &add_content2xml_hash($out_hash, "hostId_error", "no hostId specified or hostId tag invalid");
        &add_content2xml_hash($out_hash, "error", "hostId");
        &main::daemon_log("$session_id ERROR: no hostId specified or hostId tag invalid: $msg", 1); 
    }
    if ((not exists $msg_hash->{'macaddress'}) || (@{$msg_hash->{'macaddress'}} != 1))  {
        $error++;
        &add_content2xml_hash($out_hash, "macaddress_error", "no macaddress specified or macaddress tag invalid");
        &add_content2xml_hash($out_hash, "error", "macaddress");
        &main::daemon_log("$session_id ERROR: no macaddress specified or macaddress tag invalid: $msg", 1); 
    }

    if (not $error) {
        # Get hostID
        $hostId = @{$msg_hash->{'hostId'}}[0];
        &add_content2xml_hash($out_hash, "hostId", $hostId);

        # Get macaddress
        $mac = @{$msg_hash->{'macaddress'}}[0];
        &add_content2xml_hash($out_hash, "macaddress", $mac);

        my $name= $hostId;
        $name=~ s/^([^.]+).*$/$1/;
        my $domain= $hostId;
        $domain=~ s/^[^.]+\.(.*)$/$1/;
        my ($description, $notes, $ip);

        if (defined @{$msg_hash->{'description'}}[0]){
            $description = @{$msg_hash->{'description'}}[0];
        }
        if (defined @{$msg_hash->{'notes'}}[0]){
            $notes = @{$msg_hash->{'notes'}}[0];
        }
        if (defined @{$msg_hash->{'ip'}}[0]){
            $ip = @{$msg_hash->{'ip'}}[0];
        }

        my $callobj;
        $callobj = {
            method  => 'createClient',
            params  => [ $name, $domain, $description, $notes, $ip, $mac ],
            id  => 1,
        };

        $sres = $main::opsi_client->call($main::opsi_url, $callobj);
        ($sres_err, $sres_err_string) = &check_opsi_res($sres);
        if ($sres_err){
            &main::daemon_log("$session_id ERROR: cannot create client: ".$sres_err_string, 1);
            &add_content2xml_hash($out_hash, "error", $sres_err_string);
        }
    }

    # Return message
    return ( &create_xml_string($out_hash) );
}

## @method opsi_modify_client
# Modifies the parameters description, mac or notes for an Opsi client if the corresponding message tags are given.
# @param msg - STRING - xml message with tag hostId and optional description, mac or notes
# @param msg_hash - HASHREF - message information parsed into a hash
# @param session_id - INTEGER - POE session id of the processing of this message    
# @return out_msg - STRING - feedback to GOsa in success and error case
sub opsi_modify_client {
    my ($msg, $msg_hash, $session_id) = @_;
    my $header = @{$msg_hash->{'header'}}[0];
    my $source = @{$msg_hash->{'source'}}[0];
    my $target = @{$msg_hash->{'target'}}[0];
    my $forward_to_gosa = @{$msg_hash->{'forward_to_gosa'}}[0];
    my $hostId;
    my $error = 0;
    my ($sres, $sres_err, $sres_err_string);

    # Build return message with twisted target and source
    my $out_hash = &main::create_xml_hash("answer_$header", $main::server_address, $source);
    if (defined $forward_to_gosa) {
        &add_content2xml_hash($out_hash, "forward_to_gosa", $forward_to_gosa);
    }

    # Sanity check of needed parameter
    if ((not exists $msg_hash->{'hostId'}) || (@{$msg_hash->{'hostId'}} != 1))  {
        $error++;
        &add_content2xml_hash($out_hash, "hostId_error", "no hostId specified or hostId tag invalid");
        &add_content2xml_hash($out_hash, "error", "hostId");
        &main::daemon_log("$session_id ERROR: no hostId specified or hostId tag invalid: $msg", 1); 
    }

    if (not $error) {
        # Get hostID
        $hostId = @{$msg_hash->{'hostId'}}[0];
        &add_content2xml_hash($out_hash, "hostId", $hostId);
        my $name= $hostId;
        $name=~ s/^([^.]+).*$/$1/;
        my $domain= $hostId;
        $domain=~ s/^[^.]+(.*)$/$1/;

        # Modify description, notes or mac if defined
        my ($description, $notes, $mac);
        my $callobj;
        if ((exists $msg_hash->{'description'}) && (@{$msg_hash->{'description'}} == 1) ){
            $description = @{$msg_hash->{'description'}}[0];
            $callobj = {
                method  => 'setHostDescription',
                params  => [ $hostId, $description ],
                id  => 1,
            };
            my $sres = $main::opsi_client->call($main::opsi_url, $callobj);
            my ($sres_err, $sres_err_string) = &check_opsi_res($sres);
            if ($sres_err){
                &main::daemon_log("ERROR: cannot set description: ".$sres_err_string, 1);
                &add_content2xml_hash($out_hash, "error", $sres_err_string);
            }
        }
        if ((exists $msg_hash->{'notes'}) && (@{$msg_hash->{'notes'}} == 1)) {
            $notes = @{$msg_hash->{'notes'}}[0];
            $callobj = {
                method  => 'setHostNotes',
                params  => [ $hostId, $notes ],
                id  => 1,
            };
            my $sres = $main::opsi_client->call($main::opsi_url, $callobj);
            my ($sres_err, $sres_err_string) = &check_opsi_res($sres);
            if ($sres_err){
                &main::daemon_log("ERROR: cannot set notes: ".$sres_err_string, 1);
                &add_content2xml_hash($out_hash, "error", $sres_err_string);
            }
        }
        if ((exists $msg_hash->{'mac'}) && (@{$msg_hash->{'mac'}} == 1)){
            $mac = @{$msg_hash->{'mac'}}[0];
            $callobj = {
                method  => 'setMacAddress',
                params  => [ $hostId, $mac ],
                id  => 1,
            };
            my $sres = $main::opsi_client->call($main::opsi_url, $callobj);
            my ($sres_err, $sres_err_string) = &check_opsi_res($sres);
            if ($sres_err){
                &main::daemon_log("ERROR: cannot set mac address: ".$sres_err_string, 1);
                &add_content2xml_hash($out_hash, "error", $sres_err_string);
            }
        }
    }

    # Return message
    return ( &create_xml_string($out_hash) );
}

    
## @method opsi_get_netboot_products
# Get netboot products for specific host.
# @param msg - STRING - xml message with tag hostId
# @param msg_hash - HASHREF - message information parsed into a hash
# @param session_id - INTEGER - POE session id of the processing of this message
# @return out_msg - STRING - feedback to GOsa in success and error case
sub opsi_get_netboot_products {
    my ($msg, $msg_hash, $session_id) = @_;
    my $header = @{$msg_hash->{'header'}}[0];
    my $source = @{$msg_hash->{'source'}}[0];
    my $target = @{$msg_hash->{'target'}}[0];
    my $forward_to_gosa = @{$msg_hash->{'forward_to_gosa'}}[0];
    my $hostId;
    my $error = 0;
    my $xml_msg;

    # Build return message with twisted target and source
    my $out_hash = &main::create_xml_hash("answer_$header", $main::server_address, $source);
    if (defined $forward_to_gosa) {
        &add_content2xml_hash($out_hash, "forward_to_gosa", $forward_to_gosa);
    }

    # Sanity check of needed parameter
    if ((not exists $msg_hash->{'hostId'}) || (@{$msg_hash->{'hostId'}} != 1))  {
        $error++;
        &add_content2xml_hash($out_hash, "hostId_error", "no hostId specified or hostId tag invalid");
        &add_content2xml_hash($out_hash, "error", "hostId");
        &main::daemon_log("$session_id ERROR: no hostId specified or hostId tag invalid: $msg", 1); 
    }

    if (not $error) {

        # Get hostID if defined
        $hostId = @{$msg_hash->{'hostId'}}[0];
        &add_content2xml_hash($out_hash, "hostId", $hostId);

        &add_content2xml_hash($out_hash, "xxx", "");
        $xml_msg= &create_xml_string($out_hash);

        # For hosts, only return the products that are or get installed
        my $callobj;
        $callobj = {
            method  => 'getNetBootProductIds_list',
            params  => [ ],
            id  => 1,
        };

        my $res = $main::opsi_client->call($main::opsi_url, $callobj);
        my %r = ();
        for (@{$res->result}) { $r{$_} = 1 }

        if (not &check_opsi_res($res)){

            if (defined $hostId){
                $callobj = {
                    method  => 'getProductStates_hash',
                    params  => [ $hostId ],
                    id  => 1,
                };

                my $hres = $main::opsi_client->call($main::opsi_url, $callobj);
                if (not &check_opsi_res($hres)){
                    my $htmp= $hres->result->{$hostId};

                    # check state != not_installed or action == setup -> load and add
                    foreach my $product (@{$htmp}){

                        if (!defined ($r{$product->{'productId'}})){
                            next;
                        }

                        # Now we've a couple of hashes...
                        if ($product->{'installationStatus'} ne "not_installed" or
                                $product->{'actionRequest'} eq "setup"){
                            my $state= "<state>".$product->{'installationStatus'}."</state><action>".$product->{'actionRequest'}."</action>";

                            $callobj = {
                                method  => 'getProduct_hash',
                                params  => [ $product->{'productId'} ],
                                id  => 1,
                            };

                            my $sres = $main::opsi_client->call($main::opsi_url, $callobj);
                            if (not &check_opsi_res($sres)){
                                my $tres= $sres->result;

                                my $name= xml_quote($tres->{'name'});
                                my $r= $product->{'productId'};
                                my $description= xml_quote($tres->{'description'});
                                $name=~ s/\//\\\//;
                                $description=~ s/\//\\\//;
                                $xml_msg=~ s/<xxx><\/xxx>/<item><productId>$r<\/productId><name><\/name><description>$description<\/description><\/item>$state<xxx><\/xxx>/;
                            }
                        }
                    }

                }

            } else {
                foreach my $r (@{$res->result}) {
                    $callobj = {
                        method  => 'getProduct_hash',
                        params  => [ $r ],
                        id  => 1,
                    };

                    my $sres = $main::opsi_client->call($main::opsi_url, $callobj);
                    if (not &check_opsi_res($sres)){
                        my $tres= $sres->result;

                        my $name= xml_quote($tres->{'name'});
                        my $description= xml_quote($tres->{'description'});
                        $name=~ s/\//\\\//;
                        $description=~ s/\//\\\//;
                        $xml_msg=~ s/<xxx><\/xxx>/<item><productId>$r<\/productId><name><\/name><description>$description<\/description><\/item><xxx><\/xxx>/;
                    }
                }

            }
        }
        $xml_msg=~ s/<xxx><\/xxx>//;
    }

    # Return message
    return ( $xml_msg );
}


## @method opsi_get_product_properties
# Get product properties for a product and a specific host or gobally for a product.
# @param msg - STRING - xml message with tags productId and optional hostId
# @param msg_hash - HASHREF - message information parsed into a hash
# @param session_id - INTEGER - POE session id of the processing of this message
# @return out_msg - STRING - feedback to GOsa in success and error case
sub opsi_get_product_properties {
    my ($msg, $msg_hash, $session_id) = @_;
    my $header = @{$msg_hash->{'header'}}[0];
    my $source = @{$msg_hash->{'source'}}[0];
    my $target = @{$msg_hash->{'target'}}[0];
    my $forward_to_gosa = @{$msg_hash->{'forward_to_gosa'}}[0];
    my ($hostId, $productId);
    my $error = 0;
    my $xml_msg;

    # build return message with twisted target and source
    my $out_hash = &main::create_xml_hash("answer_$header", $main::server_address, $source);
    if (defined $forward_to_gosa) {
        &add_content2xml_hash($out_hash, "forward_to_gosa", $forward_to_gosa);
    }

    # Sanity check of needed parameter
    if ((not exists $msg_hash->{'productId'}) || (@{$msg_hash->{'productId'}} != 1))  {
        $error++;
        &add_content2xml_hash($out_hash, "productId_error", "no productId specified or productId tag invalid");
        &add_content2xml_hash($out_hash, "error", "productId");
        &main::daemon_log("$session_id ERROR: no productId specified or productId tag invalid: $msg", 1); 
    }

    if (not $error) {

        # Get productid
        $productId = @{$msg_hash->{'productId'}}[0];
        &add_content2xml_hash($out_hash, "producId", "$productId");


        $hostId = @{$msg_hash->{'hostId'}}[0];
        &add_content2xml_hash($out_hash, "hostId", $hostId);

        # Load actions
        my $callobj = {
            method  => 'getPossibleProductActions_list',
            params  => [ $productId ],
            id  => 1,
        };
        my $res = $main::opsi_client->call($main::opsi_url, $callobj);
        if (not &check_opsi_res($res)){
            foreach my $action (@{$res->result}){
                &add_content2xml_hash($out_hash, "action", $action);
            }
        }

        # Add place holder
        &add_content2xml_hash($out_hash, "xxx", "");

    }

    # Move to XML string
    $xml_msg= &create_xml_string($out_hash);

    if (not $error) {

        # JSON Query
        my $callobj = {
            method  => 'getProductProperties_hash',
            params  => [ $productId ],
            id  => 1,
        };
        my $res = $main::opsi_client->call($main::opsi_url, $callobj);
        if (not &check_opsi_res($res)){
            my $r= $res->result;
            foreach my $key (keys %{$r}) {
                my $item= "<item>";
                my $value= $r->{$key};
                if (UNIVERSAL::isa( $value, "ARRAY" )){
                    foreach my $subval (@{$value}){
                        $item.= "<$key>".xml_quote($subval)."</$key>";
                    }
                } else {
                    $item.= "<$key>".xml_quote($value)."</$key>";
                }
                $item.= "</item>";
                $xml_msg=~ s/<xxx><\/xxx>/$item<xxx><\/xxx>/;
            }
        }

        $xml_msg=~ s/<xxx><\/xxx>//;
    }

    # Return message
    return ( $xml_msg );
}


## @method opsi_set_product_properties
# Set product properities for a specific host or globaly. Message needs one xml tag 'item' and within one xml tag 'name' and 'value'. The xml tags action and state are optional.
# @param msg - STRING - xml message with tags productId, action, state and optional hostId, action and state
# @param msg_hash - HASHREF - message information parsed into a hash
# @param session_id - INTEGER - POE session id of the processing of this message
# @return out_msg - STRING - feedback to GOsa in success and error case
sub opsi_set_product_properties {
    my ($msg, $msg_hash, $session_id) = @_;
    my $header = @{$msg_hash->{'header'}}[0];
    my $source = @{$msg_hash->{'source'}}[0];
    my $target = @{$msg_hash->{'target'}}[0];
    my $forward_to_gosa = @{$msg_hash->{'forward_to_gosa'}}[0];
    my ($productId, $hostId);
    my $error = 0;

    # Build return message with twisted target and source
    my $out_hash = &main::create_xml_hash("answer_$header", $main::server_address, $source);
    if (defined $forward_to_gosa) {
        &add_content2xml_hash($out_hash, "forward_to_gosa", $forward_to_gosa);
    }

    # Sanity check of needed parameter
    if ((not exists $msg_hash->{'productId'}) || (@{$msg_hash->{'productId'}} != 1))  {
        $error++;
        &add_content2xml_hash($out_hash, "productId_error", "no productId specified or productId tag invalid");
        &add_content2xml_hash($out_hash, "error", "productId");
        &main::daemon_log("$session_id ERROR: no productId specified or productId tag invalid: $msg", 1); 
    }
    if ((not exists $msg_hash->{'item'}) || (@{$msg_hash->{'item'}}[0] != 1)) {
        $error++;
        &add_content2xml_hash($out_hash, "item_error", "message needs one xml-tag 'item' and within the xml-tags 'name' and 'value'");
        &add_content2xml_hash($out_hash, "error", "item");
        &main::daemon_log("$session_id ERROR: message needs one xml-tag 'item' and within the xml-tags 'name' and 'value': $msg", 1); 
    } else {
        if ((not exists $msg_hash->{'item'}->{'name'}) || (@{$msg_hash->{'item'}->{'name'}}[0] != 1)) {
            $error++;
            &add_content2xml_hash($out_hash, "name_error", "message needs within the xml-tag 'item' one xml-tags 'name'");
            &add_content2xml_hash($out_hash, "error", "name");
            &main::daemon_log("$session_id ERROR: message needs within the xml-tag 'item' one xml-tags 'name': $msg", 1); 
        }
        if ((not exists $msg_hash->{'item'}->{'value'}) || (@{$msg_hash->{'item'}->{'value'}}[0] != 1)) {
            $error++;
            &add_content2xml_hash($out_hash, "value_error", "message needs within the xml-tag 'item' one xml-tags 'value'");
            &add_content2xml_hash($out_hash, "error", "value");
            &main::daemon_log("$session_id ERROR: message needs within the xml-tag 'item' one xml-tags 'value': $msg", 1); 
        }
    }
    if ((exists $msg_hash->{'hostId'}) && (@{$msg_hash->{'hostId'}} != 1))  {
        $error++;
        &add_content2xml_hash($out_hash, "hostId_error", "hostId contains no or more than one values");
        &add_content2xml_hash($out_hash, "error", "hostId");
        &main::daemon_log("$session_id ERROR: hostId contains no or more than one values: $msg", 1); 
    }

    if (not $error) {
        
    # Get productId
        $productId =  @{$msg_hash->{'productId'}}[0];
        &add_content2xml_hash($out_hash, "productId", $productId);

    # Get hostID if defined
        if (exists $msg_hash->{'hostId'}){
            $hostId = @{$msg_hash->{'hostId'}}[0];
            &add_content2xml_hash($out_hash, "hostId", $hostId);
        }

    # Set product states if requested
        if (defined @{$msg_hash->{'action'}}[0]){
            &_set_action($productId, @{$msg_hash->{'action'}}[0], $hostId);
        }
        if (defined @{$msg_hash->{'state'}}[0]){
            &_set_state($productId, @{$msg_hash->{'state'}}[0], $hostId);
        }

    # Find properties
        foreach my $item (@{$msg_hash->{'item'}}){
            # JSON Query
            my $callobj;

            if (defined $hostId){
                $callobj = {
                    method  => 'setProductProperty',
                    params  => [ $productId, $item->{'name'}[0], $item->{'value'}[0], $hostId ],
                    id  => 1,
                };
            } else {
                $callobj = {
                    method  => 'setProductProperty',
                    params  => [ $productId, $item->{'name'}[0], $item->{'value'}[0] ],
                    id  => 1,
                };
            }

            my $res = $main::opsi_client->call($main::opsi_url, $callobj);
            my ($res_err, $res_err_string) = &check_opsi_res($res);
# TODO : This error message sounds strange
            if ($res_err){
                &man::daemon_log("$session_id ERROR: no communication failed while setting '".$item->{'name'}[0]."': ".$res_err_string, 1);
                &add_content2xml_hash($out_hash, "error", $res_err_string);
            }
        }

    }

    # Return message
    return ( &create_xml_string($out_hash) );
}


## @method opsi_get_client_hardware
# Reports client hardware inventory.
# @param msg - STRING - xml message with tag hostId
# @param msg_hash - HASHREF - message information parsed into a hash
# @param session_id - INTEGER - POE session id of the processing of this message
# @return out_msg - STRING - feedback to GOsa in success and error case
sub opsi_get_client_hardware {
    my ($msg, $msg_hash, $session_id) = @_;
    my $header = @{$msg_hash->{'header'}}[0];
    my $source = @{$msg_hash->{'source'}}[0];
    my $target = @{$msg_hash->{'target'}}[0];
    my $forward_to_gosa = @{$msg_hash->{'forward_to_gosa'}}[0];
    my $hostId;
    my $error = 0;
    my $xml_msg;

    # build return message with twisted target and source
    my $out_hash = &main::create_xml_hash("answer_$header", $main::server_address, $source);
    if (defined $forward_to_gosa) {
      &add_content2xml_hash($out_hash, "forward_to_gosa", $forward_to_gosa);
    }

    # Sanity check of needed parameter
    if ((exists $msg_hash->{'hostId'}) && (@{$msg_hash->{'hostId'}} != 1))  {
        $error++;
        &add_content2xml_hash($out_hash, "hostId_error", "hostId contains no or more than one values");
        &add_content2xml_hash($out_hash, "error", "hostId");
        &main::daemon_log("$session_id ERROR: hostId contains no or more than one values: $msg", 1); 
    }

    if (not $error) {

    # Get hostID
        $hostId = @{$msg_hash->{'hostId'}}[0];
        &add_content2xml_hash($out_hash, "hostId", "$hostId");
        &add_content2xml_hash($out_hash, "xxx", "");
    }    

    # Move to XML string
    $xml_msg= &create_xml_string($out_hash);
    
    if (not $error) {

    # JSON Query
        my $callobj = {
            method  => 'getHardwareInformation_hash',
            params  => [ $hostId ],
            id  => 1,
        };

        my $res = $main::opsi_client->call($main::opsi_url, $callobj);
        if (not &check_opsi_res($res)){
            my $result= $res->result;
            foreach my $r (keys %{$result}){
                my $item= "<item><id>".xml_quote($r)."</id>";
                my $value= $result->{$r};
                foreach my $sres (@{$value}){

                    foreach my $dres (keys %{$sres}){
                        if (defined $sres->{$dres}){
                            $item.= "<$dres>".xml_quote($sres->{$dres})."</$dres>";
                        }
                    }

                }
                $item.= "</item>";
                $xml_msg=~ s%<xxx></xxx>%$item<xxx></xxx>%;

            }
        }

        $xml_msg=~ s/<xxx><\/xxx>//;

    }

    # Return message
    return ( $xml_msg );
}


## @method opsi_list_clients
# Reports all Opsi clients. 
# @param msg - STRING - xml message 
# @param msg_hash - HASHREF - message information parsed into a hash
# @param session_id - INTEGER - POE session id of the processing of this message
# @return out_msg - STRING - feedback to GOsa in success and error case
sub opsi_list_clients {
    my ($msg, $msg_hash, $session_id) = @_;
    my $header = @{$msg_hash->{'header'}}[0];
    my $source = @{$msg_hash->{'source'}}[0];
    my $target = @{$msg_hash->{'target'}}[0];
    my $forward_to_gosa = @{$msg_hash->{'forward_to_gosa'}}[0];
    my $error = 0;

    # build return message with twisted target and source
    my $out_hash = &main::create_xml_hash("answer_$header", $main::server_address, $source);
    if (defined $forward_to_gosa) {
      &add_content2xml_hash($out_hash, "forward_to_gosa", $forward_to_gosa);
    }
    &add_content2xml_hash($out_hash, "xxx", "");

    # Move to XML string
    my $xml_msg= &create_xml_string($out_hash);

    if (not $error) {

    # JSON Query
        my $callobj = {
            method  => 'getClients_listOfHashes',
            params  => [ ],
            id  => 1,
        };
        my $res = $main::opsi_client->call($main::opsi_url, $callobj);
        if (not &check_opsi_res($res)){
            foreach my $host (@{$res->result}){
                my $item= "<item><name>".$host->{'hostId'}."</name>";
                if (defined($host->{'description'})){
                    $item.= "<description>".xml_quote($host->{'description'})."</description>";
                }
                if (defined($host->{'ip'})){
                    $item.= "<ip>".xml_quote($host->{'ip'})."</ip>";
                }
                if (defined($host->{'mac'})){
                    $item.= "<mac>".xml_quote($host->{'mac'})."</mac>";
                }
                if (defined($host->{'notes'})){
                    $item.= "<notes>".xml_quote($host->{'notes'})."</notes>";
                }
                $item.= "</item>";
                $xml_msg=~ s%<xxx></xxx>%$item<xxx></xxx>%;
            }
        }
    }

    $xml_msg=~ s/<xxx><\/xxx>//;
    return ( $xml_msg );
}


## @method opsi_get_client_software
# Reports client software inventory.
# @param msg - STRING - xml message with tag hostId
# @param msg_hash - HASHREF - message information parsed into a hash
# @param session_id - INTEGER - POE session id of the processing of this message
# @return out_msg - STRING - feedback to GOsa in success and error case
sub opsi_get_client_software {
    my ($msg, $msg_hash, $session_id) = @_;
    my $header = @{$msg_hash->{'header'}}[0];
    my $source = @{$msg_hash->{'source'}}[0];
    my $target = @{$msg_hash->{'target'}}[0];
    my $forward_to_gosa = @{$msg_hash->{'forward_to_gosa'}}[0];
    my $error = 0;
    my $hostId;
    my $xml_msg;

    # build return message with twisted target and source
    my $out_hash = &main::create_xml_hash("answer_$header", $main::server_address, $source);
    if (defined $forward_to_gosa) {
      &add_content2xml_hash($out_hash, "forward_to_gosa", $forward_to_gosa);
    }

    # Sanity check of needed parameter
    if ((exists $msg_hash->{'hostId'}) && (@{$msg_hash->{'hostId'}} != 1))  {
        $error++;
        &add_content2xml_hash($out_hash, "hostId_error", "hostId contains no or more than one values");
        &add_content2xml_hash($out_hash, "error", "hostId");
        &main::daemon_log("$session_id ERROR: hostId contains no or more than one values: $msg", 1); 
    }

    if (not $error) {

    # Get hostID
        $hostId = @{$msg_hash->{'hostId'}}[0];
        &add_content2xml_hash($out_hash, "hostId", "$hostId");
        &add_content2xml_hash($out_hash, "xxx", "");
    }

    $xml_msg= &create_xml_string($out_hash);

    if (not $error) {

    # JSON Query
        my $callobj = {
            method  => 'getSoftwareInformation_hash',
            params  => [ $hostId ],
            id  => 1,
        };

        my $res = $main::opsi_client->call($main::opsi_url, $callobj);
        if (not &check_opsi_res($res)){
            my $result= $res->result;
# TODO : fertig???   
        }

        $xml_msg=~ s/<xxx><\/xxx>//;

    }

    # Return message
    return ( $xml_msg );
}


## @method opsi_get_local_products
# Reports product for given hostId or globally.
# @param msg - STRING - xml message with optional tag hostId
# @param msg_hash - HASHREF - message information parsed into a hash
# @param session_id - INTEGER - POE session id of the processing of this message
# @return out_msg - STRING - feedback to GOsa in success and error case
sub opsi_get_local_products {
    my ($msg, $msg_hash, $session_id) = @_;
    my $header = @{$msg_hash->{'header'}}[0];
    my $source = @{$msg_hash->{'source'}}[0];
    my $target = @{$msg_hash->{'target'}}[0];
    my $forward_to_gosa = @{$msg_hash->{'forward_to_gosa'}}[0];
    my $hostId;
    my $error = 0;
    my $xml_msg;

    # Build return message with twisted target and source
    my $out_hash = &main::create_xml_hash("answer_$header", $main::server_address, $source);
    if (defined $forward_to_gosa) {
        &add_content2xml_hash($out_hash, "forward_to_gosa", $forward_to_gosa);
    }
    &add_content2xml_hash($out_hash, "xxx", "");

    # Get hostID if defined
    if ((exists $msg_hash->{'hostId'}) && (@{$msg_hash->{'hostId'}} == 1))  {
        $hostId = @{$msg_hash->{'hostId'}}[0];
        &add_content2xml_hash($out_hash, "hostId", $hostId);
    }

    # Move to XML string
    my $xml_msg= &create_xml_string($out_hash);

    # For hosts, only return the products that are or get installed
    my $callobj;
    $callobj = {
        method  => 'getLocalBootProductIds_list',
        params  => [ ],
        id  => 1,
    };

    my $res = $main::opsi_client->call($main::opsi_url, $callobj);
    my %r = ();
    for (@{$res->result}) { $r{$_} = 1 }

    if (not &check_opsi_res($res)){

        if (defined $hostId){
            $callobj = {
                method  => 'getProductStates_hash',
                params  => [ $hostId ],
                id  => 1,
            };

            my $hres = $main::opsi_client->call($main::opsi_url, $callobj);
            if (not &check_opsi_res($hres)){
                my $htmp= $hres->result->{$hostId};

                # Check state != not_installed or action == setup -> load and add
                foreach my $product (@{$htmp}){

                    if (!defined ($r{$product->{'productId'}})){
                        next;
                    }

                    # Now we've a couple of hashes...
                    if ($product->{'installationStatus'} ne "not_installed" or
                            $product->{'actionRequest'} eq "setup"){
                        my $state= "<state>".$product->{'installationStatus'}."</state><action>".$product->{'actionRequest'}."</action>";

                        $callobj = {
                            method  => 'getProduct_hash',
                            params  => [ $product->{'productId'} ],
                            id  => 1,
                        };

                        my $sres = $main::opsi_client->call($main::opsi_url, $callobj);
                        if (not &check_opsi_res($sres)){
                            my $tres= $sres->result;

                            my $name= xml_quote($tres->{'name'});
                            my $r= $product->{'productId'};
                            my $description= xml_quote($tres->{'description'});
                            $name=~ s/\//\\\//;
                            $description=~ s/\//\\\//;
                            $xml_msg=~ s/<xxx><\/xxx>/<item><productId>$r<\/productId><name><\/name><description>$description<\/description><\/item>$state<xxx><\/xxx>/;
                        }

                    }
                }

            }

        } else {
            foreach my $r (@{$res->result}) {
                $callobj = {
                    method  => 'getProduct_hash',
                    params  => [ $r ],
                    id  => 1,
                };

                my $sres = $main::opsi_client->call($main::opsi_url, $callobj);
                if (not &check_opsi_res($sres)){
                    my $tres= $sres->result;

                    my $name= xml_quote($tres->{'name'});
                    my $description= xml_quote($tres->{'description'});
                    $name=~ s/\//\\\//;
                    $description=~ s/\//\\\//;
                    $xml_msg=~ s/<xxx><\/xxx>/<item><productId>$r<\/productId><name><\/name><description>$description<\/description><\/item><xxx><\/xxx>/;
                }

            }

        }
    }

    $xml_msg=~ s/<xxx><\/xxx>//;

    # Retrun Message
    return ( $xml_msg );
}


## @method opsi_del_client
# Deletes a client from Opsi.
# @param msg - STRING - xml message with tag hostId
# @param msg_hash - HASHREF - message information parsed into a hash
# @param session_id - INTEGER - POE session id of the processing of this message
# @return out_msg - STRING - feedback to GOsa in success and error case
sub opsi_del_client {
    my ($msg, $msg_hash, $session_id) = @_;
    my $header = @{$msg_hash->{'header'}}[0];
    my $source = @{$msg_hash->{'source'}}[0];
    my $target = @{$msg_hash->{'target'}}[0];
    my $forward_to_gosa = @{$msg_hash->{'forward_to_gosa'}}[0];
    my $hostId;
    my $error = 0;

    # Build return message with twisted target and source
    my $out_hash = &main::create_xml_hash("answer_$header", $main::server_address, $source);
    if (defined $forward_to_gosa) {
      &add_content2xml_hash($out_hash, "forward_to_gosa", $forward_to_gosa);
    }

    # Sanity check of needed parameter
    if ((exists $msg_hash->{'hostId'}) && (@{$msg_hash->{'hostId'}} != 1))  {
        $error++;
        &add_content2xml_hash($out_hash, "hostId_error", "hostId contains no or more than one values");
        &add_content2xml_hash($out_hash, "error", "hostId");
        &main::daemon_log("$session_id ERROR: hostId contains no or more than one values: $msg", 1); 
    }

    if (not $error) {

    # Get hostID
        $hostId = @{$msg_hash->{'hostId'}}[0];
        &add_content2xml_hash($out_hash, "hostId", "$hostId");

    # JSON Query
        my $callobj = {
            method  => 'deleteClient',
            params  => [ $hostId ],
            id  => 1,
        };
        my $res = $main::opsi_client->call($main::opsi_url, $callobj);
    }

    # Move to XML string
    my $xml_msg= &create_xml_string($out_hash);

    # Return message
    return ( $xml_msg );
}


## @method opsi_install_client
# Set a client in Opsi to install and trigger a wake on lan message (WOL).  
# @param msg - STRING - xml message with tags hostId, macaddress
# @param msg_hash - HASHREF - message information parsed into a hash
# @param session_id - INTEGER - POE session id of the processing of this message
# @return out_msg - STRING - feedback to GOsa in success and error case
sub opsi_install_client {
    my ($msg, $msg_hash, $session_id) = @_;
    my $header = @{$msg_hash->{'header'}}[0];
    my $source = @{$msg_hash->{'source'}}[0];
    my $target = @{$msg_hash->{'target'}}[0];
    my $forward_to_gosa = @{$msg_hash->{'forward_to_gosa'}}[0];


    my ($hostId, $macaddress);

    my $error = 0;
    my @out_msg_l;

    # Build return message with twisted target and source
    my $out_hash = &main::create_xml_hash("answer_$header", $main::server_address, $source);
    if (defined $forward_to_gosa) {
        &add_content2xml_hash($out_hash, "forward_to_gosa", $forward_to_gosa);
    }

    # Sanity check of needed parameter
    if ((not exists $msg_hash->{'hostId'}) || (@{$msg_hash->{'hostId'}} != 1))  {
        $error++;
        &add_content2xml_hash($out_hash, "hostId_error", "no hostId specified or hostId tag invalid");
        &add_content2xml_hash($out_hash, "error", "hostId");
        &main::daemon_log("$session_id ERROR: no hostId specified or hostId tag invalid: $msg", 1); 
    }
    if ((not exists $msg_hash->{'macaddress'}) || (@{$msg_hash->{'macaddress'}} != 1))  {
        $error++;
        &add_content2xml_hash($out_hash, "macaddress_error", "no macaddress specified or macaddress tag invalid");
        &add_content2xml_hash($out_hash, "error", "macaddress");
        &main::daemon_log("$session_id ERROR: no macaddress specified or macaddress tag invalid: $msg", 1); 
    } else {
        if ((exists $msg_hash->{'macaddress'}) && 
                ($msg_hash->{'macaddress'}[0] =~ /^([0-9a-f]{2}:[0-9a-f]{2}:[0-9a-f]{2}:[0-9a-f]{2}:[0-9a-f]{2}:[0-9a-f]{2})$/i)) {  
            $macaddress = $1; 
        } else { 
            $error ++; 
            &add_content2xml_hash($out_hash, "macaddress_error", "given mac address is not correct");
            &add_content2xml_hash($out_hash, "error", "macaddress");
            &main::daemon_log("$session_id ERROR: given mac address is not correct: $msg", 1); 
        }
    }

    if (not $error) {

    # Get hostId
        $hostId = @{$msg_hash->{'hostId'}}[0];
        &add_content2xml_hash($out_hash, "hostId", "$hostId");

        # Load all products for this host with status != "not_installed" or actionRequest != "none"
        my $callobj = {
            method  => 'getProductStates_hash',
            params  => [ $hostId ],
            id  => 1,
        };

        my $hres = $main::opsi_client->call($main::opsi_url, $callobj);
        if (not &check_opsi_res($hres)){
            my $htmp= $hres->result->{$hostId};

            # check state != not_installed or action == setup -> load and add
            foreach my $product (@{$htmp}){
                # Now we've a couple of hashes...
                if ($product->{'installationStatus'} ne "not_installed" or
                        $product->{'actionRequest'} ne "none"){

                    # Do an action request for all these -> "setup".
                    $callobj = {
                        method  => 'setProductActionRequest',
                        params  => [ $product->{'productId'}, $hostId, "setup" ],
                        id  => 1,
                    };
                    my $res = $main::opsi_client->call($main::opsi_url, $callobj);
                    my ($res_err, $res_err_string) = &check_opsi_res($res);
                    if ($res_err){
                        &main::daemon_log("$session_id ERROR: cannot set product action request for '$hostId': ".$product->{'productId'}, 1);
                    } else {
                        &main::daemon_log("$session_id INFO: requesting 'setup' for '".$product->{'productId'}."' on $hostId", 1);
                    }
                }
            }
        }
        push(@out_msg_l, &create_xml_string($out_hash));
    

    # Build wakeup message for client
        if (not $error) {
            my $wakeup_hash = &create_xml_hash("trigger_wake", "GOSA", "KNOWN_SERVER");
            &add_content2xml_hash($wakeup_hash, 'macAddress', $macaddress);
            my $wakeup_msg = &create_xml_string($wakeup_hash);
            push(@out_msg_l, $wakeup_msg);

            # invoke trigger wake for this gosa-si-server
            &main::server_server_com::trigger_wake($wakeup_msg, $wakeup_hash, $session_id);
        }
    }
    
    # Return messages
    return @out_msg_l;
}


## @method _set_action
# Set action for an Opsi client
# @param product - STRING - Opsi product
# @param action - STRING - action
# @param hostId - STRING - Opsi hostId
sub _set_action {
  my $product= shift;
  my $action = shift;
  my $hostId = shift;
  my $callobj;

  $callobj = {
    method  => 'setProductActionRequest',
    params  => [ $product, $hostId, $action],
    id  => 1,
  };

  $main::opsi_client->call($main::opsi_url, $callobj);
}

## @method _set_state
# Set state for an Opsi client
# @param product - STRING - Opsi product
# @param action - STRING - state
# @param hostId - STRING - Opsi hostId
sub _set_state {
  my $product = shift;
  my $state = shift;
  my $hostId = shift;
  my $callobj;

  $callobj = {
    method  => 'setProductState',
    params  => [ $product, $hostId, $state ],
    id  => 1,
  };

  $main::opsi_client->call($main::opsi_url, $callobj);
}

1;
