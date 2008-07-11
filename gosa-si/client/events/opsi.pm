package opsi;
use Exporter;
@ISA = qw(Exporter);
my @events = (
    "get_events",
    "_opsi_get_client_status",
    "opsi_get_netboot_products",  
    "opsi_get_local_products",
    "opsi_get_client_hardware",
    "opsi_get_client_software",
    "opsi_get_product_properties",
    "opsi_set_product_properties",
    "opsi_list_clients",
    "opsi_del_client",
    "opsi_install_client",

    #"opsi_add_client",
    # Client hinzufügen
    # -> voll qualifizierter name
    # -> IP
    # -> MAC
    # -> Description
    # -> Notizen

    #"opsi_modify_client",

    #"opsi_add_product_to_client",
    # -> set product state auf "setup"
    # -> Abhängigkeit prüfen und evtl. erweitern
    # createProductDependency('productId', 'action', '*requiredProductId', '*requiredProductClassId', '*requiredAction', '*requiredInstallationStatus', '*requirementType', '*depotIds')

    #"opsi_del_product_from_client",
    # -> Abhängigkeit prüfen und evtl. verweigern
    # createProductDependency('productId', 'action', '*requiredProductId', '*requiredProductClassId', '*requiredAction', '*requiredInstallationStatus', '*requirementType', '*depotIds')
    # -> delete nur wenn eine "uninstall" methode existiert
    # -> set product state auf "none"

    );
@EXPORT = @events;

use strict;
use warnings;
use Data::Dumper;
use GOSA::GosaSupportDaemon;
use JSON::RPC::Client;
use XML::Quote qw(:all);

BEGIN {}

END {}

### Start ######################################################################

my $opsi_server;
my $opsi_admin;
my $opsi_password;

my %cfg_defaults = (
"opsi" => {
   "server" => [\$opsi_server, "localhost"],
   "admin" => [\$opsi_admin, "opsi-admin"],
   "password" => [\$opsi_password, "secret"],
   },
);
&read_configfile($main::cfg_file, %cfg_defaults);

# Assemble opsi URL
my $opsi_url= "https://".$opsi_admin.":".$opsi_password."@".$opsi_server.":4447/rpc";
my $client = new JSON::RPC::Client;

sub check_res {
  my $res= shift;
  # TODO: We need to return the outhash!?
  my $out_hash;

  if($res) {
    if ($res->is_error) {
      &main::daemon_log("ERROR: opsi configed communication failed: ".$res->error_message, 1);
      &add_content2xml_hash($out_hash, "error", $res->error_message);
    } else {
      return 1;
    }
  } else {
    &main::daemon_log("ERROR: opsi configed communication failed: ".$client->status_line, 1);
    &add_content2xml_hash($out_hash, "error", $client->status_line);
  }

  return 0;
}


sub read_configfile {
    my ($cfg_file, %cfg_defaults) = @_;
    my $cfg;

    if( defined( $cfg_file) && ( (-s $cfg_file) > 0 )) {
        if( -r $cfg_file ) {
            $cfg = Config::IniFiles->new( -file => $cfg_file );
        } else {
            &main::daemon_log("ERROR: opsi.pm couldn't read config file!", 1);
        }
    } else {
        $cfg = Config::IniFiles->new() ;
    }
    foreach my $section (keys %cfg_defaults) {
        foreach my $param (keys %{$cfg_defaults{ $section }}) {
            my $pinfo = $cfg_defaults{ $section }{ $param };
            ${@$pinfo[0]} = $cfg->val( $section, $param, @$pinfo[1] );
        }
    }
}


sub get_events { return \@events; }


sub opsi_get_netboot_products {
  my ($msg, $msg_hash) = @_;
  my $header = @{$msg_hash->{'header'}}[0];
  my $source = @{$msg_hash->{'source'}}[0];
  my $target = @{$msg_hash->{'target'}}[0];
  my $session_id = @{$msg_hash->{'session_id'}}[0];
  my $forward_to_gosa = @{$msg_hash->{'forward_to_gosa'}}[0];
  my $hostId;

  # build return message with twisted target and source
  my $out_hash = &main::create_xml_hash("answer_$header", $target, $source);
  &add_content2xml_hash($out_hash, "session_id", $session_id);

  # Get hostID if defined
  if (defined @{$msg_hash->{'hostId'}}[0]){
    $hostId = @{$msg_hash->{'hostId'}}[0];
    &add_content2xml_hash($out_hash, "hostId", $hostId);
  }

  if (defined $forward_to_gosa) {
    &add_content2xml_hash($out_hash, "forward_to_gosa", $forward_to_gosa);
  }
  &add_content2xml_hash($out_hash, "xxx", "");
  my $xml_msg= &create_xml_string($out_hash);

  # For hosts, only return the products that are or get installed
  my $callobj;
  $callobj = {
    method  => 'getNetBootProductIds_list',
    params  => [ ],
    id  => 1,
  };

  my $res = $client->call($opsi_url, $callobj);
  my %r = ();
  for (@{$res->result}) { $r{$_} = 1 }

  if (check_res($res)){

    if (defined $hostId){
      $callobj = {
        method  => 'getProductStates_hash',
        params  => [ $hostId ],
        id  => 1,
      };

      my $hres = $client->call($opsi_url, $callobj);
      if (check_res($hres)){
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

            my $sres = $client->call($opsi_url, $callobj);
            if (check_res($sres)){
              my $tres= $sres->result;

              my $name= xml_quote($tres->{'name'});
              my $r= $product->{'productId'};
              my $description= xml_quote($tres->{'description'});
              $name=~ s/\//\\\//;
              $description=~ s/\//\\\//;
              $xml_msg=~ s/<xxx><\/xxx>/<item><ProductId>$r<\/ProductId><name><\/name><description>$description<\/description><\/item>$state<xxx><\/xxx>/;
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

        my $sres = $client->call($opsi_url, $callobj);
        if (check_res($sres)){
          my $tres= $sres->result;

          my $name= xml_quote($tres->{'name'});
          my $description= xml_quote($tres->{'description'});
          $name=~ s/\//\\\//;
          $description=~ s/\//\\\//;
          $xml_msg=~ s/<xxx><\/xxx>/<item><ProductId>$r<\/ProductId><name><\/name><description>$description<\/description><\/item><xxx><\/xxx>/;
        }

      }

    }
  }

  $xml_msg=~ s/<xxx><\/xxx>//;

  return $xml_msg;
}


sub opsi_get_product_properties {
    my ($msg, $msg_hash) = @_;
    my $header = @{$msg_hash->{'header'}}[0];
    my $source = @{$msg_hash->{'source'}}[0];
    my $target = @{$msg_hash->{'target'}}[0];
    my $session_id = @{$msg_hash->{'session_id'}}[0];
    my $forward_to_gosa = @{$msg_hash->{'forward_to_gosa'}}[0];
    my $productId = @{$msg_hash->{'ProductId'}}[0];
    my $hostId;

    # build return message with twisted target and source
    my $out_hash = &main::create_xml_hash("answer_$header", $target, $source);
    &add_content2xml_hash($out_hash, "session_id", $session_id);

    # Get hostID if defined
    if (defined @{$msg_hash->{'hostId'}}[0]){
      $hostId = @{$msg_hash->{'hostId'}}[0];
      &add_content2xml_hash($out_hash, "hostId", $hostId);
    }

    if (defined $forward_to_gosa) {
      &add_content2xml_hash($out_hash, "forward_to_gosa", $forward_to_gosa);
    }
    &add_content2xml_hash($out_hash, "ProducId", "$productId");

    # Load actions
    my $callobj = {
      method  => 'getPossibleProductActions_list',
      params  => [ $productId ],
      id  => 1,
    };
    my $res = $client->call($opsi_url, $callobj);
    if (check_res($res)){
      foreach my $action (@{$res->result}){
        &add_content2xml_hash($out_hash, "action", $action);
      }
    }

    # Add place holder
    &add_content2xml_hash($out_hash, "xxx", "");

    # Move to XML string
    my $xml_msg= &create_xml_string($out_hash);

    # JSON Query
    $callobj = {
      method  => 'getProductProperties_hash',
      params  => [ $productId ],
      id  => 1,
    };

    $res = $client->call($opsi_url, $callobj);

    if (check_res($res)){
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

  return $xml_msg;
}


sub opsi_set_product_properties {
    my ($msg, $msg_hash) = @_;
    my $header = @{$msg_hash->{'header'}}[0];
    my $source = @{$msg_hash->{'source'}}[0];
    my $target = @{$msg_hash->{'target'}}[0];
    my $session_id = @{$msg_hash->{'session_id'}}[0];
    my $forward_to_gosa = @{$msg_hash->{'forward_to_gosa'}}[0];
    my $productId = @{$msg_hash->{'ProductId'}}[0];
    my $hostId;

    # build return message with twisted target and source
    my $out_hash = &main::create_xml_hash("answer_$header", $target, $source);
    &add_content2xml_hash($out_hash, "session_id", $session_id);
    &add_content2xml_hash($out_hash, "ProductId", $productId);

    # Get hostID if defined
    if (defined @{$msg_hash->{'hostId'}}[0]){
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

    if (defined $forward_to_gosa) {
        &add_content2xml_hash($out_hash, "forward_to_gosa", $forward_to_gosa);
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

      my $res = $client->call($opsi_url, $callobj);

      if (!check_res($res)){
        &main::daemon_log("ERROR: no communication failed while setting '".$item->{'name'}[0]."': ".$res->error_message, 1);
        &add_content2xml_hash($out_hash, "error", $res->error_message);
      }

    }

    # return message
    return &create_xml_string($out_hash);
}


sub opsi_get_client_hardware {
    my ($msg, $msg_hash) = @_;
    my $header = @{$msg_hash->{'header'}}[0];
    my $source = @{$msg_hash->{'source'}}[0];
    my $target = @{$msg_hash->{'target'}}[0];
    my $session_id = @{$msg_hash->{'session_id'}}[0];
    my $forward_to_gosa = @{$msg_hash->{'forward_to_gosa'}}[0];
    my $hostId = @{$msg_hash->{'hostId'}}[0];

    # build return message with twisted target and source
    my $out_hash = &main::create_xml_hash("answer_$header", $target, $source);
    &add_content2xml_hash($out_hash, "session_id", $session_id);

    if (defined $forward_to_gosa) {
      &add_content2xml_hash($out_hash, "forward_to_gosa", $forward_to_gosa);
    }
    &add_content2xml_hash($out_hash, "hostId", "$hostId");
    &add_content2xml_hash($out_hash, "xxx", "");
    my $xml_msg= &create_xml_string($out_hash);

    # JSON Query
    my $callobj = {
      method  => 'getHardwareInformation_hash',
      params  => [ $hostId ],
      id  => 1,
    };

    my $res = $client->call($opsi_url, $callobj);
    if (check_res($res)){
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

    return $xml_msg;
}


sub opsi_list_clients {
    my ($msg, $msg_hash) = @_;
    my $header = @{$msg_hash->{'header'}}[0];
    my $source = @{$msg_hash->{'source'}}[0];
    my $target = @{$msg_hash->{'target'}}[0];
    my $session_id = @{$msg_hash->{'session_id'}}[0];
    my $forward_to_gosa = @{$msg_hash->{'forward_to_gosa'}}[0];

    # build return message with twisted target and source
    my $out_hash = &main::create_xml_hash("answer_$header", $target, $source);
    &add_content2xml_hash($out_hash, "session_id", $session_id);

    if (defined $forward_to_gosa) {
      &add_content2xml_hash($out_hash, "forward_to_gosa", $forward_to_gosa);
    }

    &add_content2xml_hash($out_hash, "xxx", "");
    my $xml_msg= &create_xml_string($out_hash);

    # JSON Query
    my $callobj = {
      method  => 'getClients_listOfHashes',
      params  => [ ],
      id  => 1,
    };

    my $res = $client->call($opsi_url, $callobj);
    if (check_res($res)){

      foreach my $host (@{$res->result}){
        my $item= "<item><name>".$host->{'hostId'}."</name>";
        if (defined($host->{'description'})){
          $item.= "<description>".xml_quote($host->{'description'})."</description>";
        }
        $item.= "</item>";
        $xml_msg=~ s%<xxx></xxx>%$item<xxx></xxx>%;
      }

    }

    $xml_msg=~ s/<xxx><\/xxx>//;
    return $xml_msg;
}


sub opsi_get_client_software {
    my ($msg, $msg_hash) = @_;
    my $header = @{$msg_hash->{'header'}}[0];
    my $source = @{$msg_hash->{'source'}}[0];
    my $target = @{$msg_hash->{'target'}}[0];
    my $session_id = @{$msg_hash->{'session_id'}}[0];
    my $forward_to_gosa = @{$msg_hash->{'forward_to_gosa'}}[0];
    my $hostId = @{$msg_hash->{'hostId'}}[0];

    # build return message with twisted target and source
    my $out_hash = &main::create_xml_hash("answer_$header", $target, $source);
    &add_content2xml_hash($out_hash, "session_id", $session_id);

    if (defined $forward_to_gosa) {
      &add_content2xml_hash($out_hash, "forward_to_gosa", $forward_to_gosa);
    }
    &add_content2xml_hash($out_hash, "hostId", "$hostId");
    &add_content2xml_hash($out_hash, "xxx", "");
    my $xml_msg= &create_xml_string($out_hash);

    # JSON Query
    my $callobj = {
      method  => 'getSoftwareInformation_hash',
      params  => [ $hostId ],
      id  => 1,
    };

    my $res = $client->call($opsi_url, $callobj);
    if (check_res($res)){
      my $result= $res->result;
    }

    $xml_msg=~ s/<xxx><\/xxx>//;

    return $xml_msg;
}


sub opsi_get_local_products {
  my ($msg, $msg_hash) = @_;
  my $header = @{$msg_hash->{'header'}}[0];
  my $source = @{$msg_hash->{'source'}}[0];
  my $target = @{$msg_hash->{'target'}}[0];
  my $session_id = @{$msg_hash->{'session_id'}}[0];
  my $forward_to_gosa = @{$msg_hash->{'forward_to_gosa'}}[0];
  my $hostId;

  # build return message with twisted target and source
  my $out_hash = &main::create_xml_hash("answer_$header", $target, $source);
  &add_content2xml_hash($out_hash, "session_id", $session_id);

  # Get hostID if defined
  if (defined @{$msg_hash->{'hostId'}}[0]){
    $hostId = @{$msg_hash->{'hostId'}}[0];
    &add_content2xml_hash($out_hash, "hostId", $hostId);
  }

  if (defined $forward_to_gosa) {
    &add_content2xml_hash($out_hash, "forward_to_gosa", $forward_to_gosa);
  }
  &add_content2xml_hash($out_hash, "xxx", "");
  my $xml_msg= &create_xml_string($out_hash);

  # For hosts, only return the products that are or get installed
  my $callobj;
  $callobj = {
    method  => 'getLocalBootProductIds_list',
    params  => [ ],
    id  => 1,
  };

  my $res = $client->call($opsi_url, $callobj);
  my %r = ();
  for (@{$res->result}) { $r{$_} = 1 }

  if (check_res($res)){

    if (defined $hostId){
      $callobj = {
        method  => 'getProductStates_hash',
        params  => [ $hostId ],
        id  => 1,
      };

      my $hres = $client->call($opsi_url, $callobj);
      if (check_res($hres)){
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

            my $sres = $client->call($opsi_url, $callobj);
            if (check_res($sres)){
              my $tres= $sres->result;

              my $name= xml_quote($tres->{'name'});
              my $r= $product->{'productId'};
              my $description= xml_quote($tres->{'description'});
              $name=~ s/\//\\\//;
              $description=~ s/\//\\\//;
              $xml_msg=~ s/<xxx><\/xxx>/<item><ProductId>$r<\/ProductId><name><\/name><description>$description<\/description><\/item>$state<xxx><\/xxx>/;
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

        my $sres = $client->call($opsi_url, $callobj);
        if (check_res($sres)){
          my $tres= $sres->result;

          my $name= xml_quote($tres->{'name'});
          my $description= xml_quote($tres->{'description'});
          $name=~ s/\//\\\//;
          $description=~ s/\//\\\//;
          $xml_msg=~ s/<xxx><\/xxx>/<item><ProductId>$r<\/ProductId><name><\/name><description>$description<\/description><\/item><xxx><\/xxx>/;
        }

      }

    }
  }

  $xml_msg=~ s/<xxx><\/xxx>//;

  return $xml_msg;
}


sub _opsi_get_client_status {
  my $hostId = shift;
  my $result= {};

  # For hosts, only return the products that are or get installed
  my $callobj;
  $callobj = {
    method  => 'getProductStates_hash',
    params  => [ $hostId ],
    id  => 1,
  };

  my $hres = $client->call($opsi_url, $callobj);
  if (check_res($hres)){
    my $htmp= $hres->result->{$hostId};

    # check state != not_installed or action == setup -> load and add
    my $products= 0;
    my $installed= 0;
    my $error= 0;
    foreach my $product (@{$htmp}){

      if ($product->{'installationStatus'} ne "not_installed" or
          $product->{'actionRequest'} eq "setup"){

        # Increase number of products for this host
        $products++;

        if ($product->{'installationStatus'} eq "failed"){
          $result->{$product->{'productId'}}= "error";
          $error++;
        }
        if ($product->{'installationStatus'} eq "installed"){
          $result->{$product->{'productId'}}= "installed";
          $installed++;
        }
        if ($product->{'installationStatus'} eq "installing"){
          $result->{$product->{'productId'}}= "installing";
        }
      }
    }

    # Estimate "rough" progress
    $result->{'progress'}= int($installed * 100 / $products);
  }

  return $result;
}


sub opsi_del_client {
    my ($msg, $msg_hash) = @_;
    my $header = @{$msg_hash->{'header'}}[0];
    my $source = @{$msg_hash->{'source'}}[0];
    my $target = @{$msg_hash->{'target'}}[0];
    my $session_id = @{$msg_hash->{'session_id'}}[0];
    my $forward_to_gosa = @{$msg_hash->{'forward_to_gosa'}}[0];
    my $hostId = @{$msg_hash->{'hostId'}}[0];

    # build return message with twisted target and source
    my $out_hash = &main::create_xml_hash("answer_$header", $target, $source);
    &add_content2xml_hash($out_hash, "session_id", $session_id);

    if (defined $forward_to_gosa) {
      &add_content2xml_hash($out_hash, "forward_to_gosa", $forward_to_gosa);
    }
    &add_content2xml_hash($out_hash, "hostId", "$hostId");

    # JSON Query
    my $callobj = {
      method  => 'deleteClient',
      params  => [ $hostId ],
      id  => 1,
    };

    my $res = $client->call($opsi_url, $callobj);

    my $xml_msg= &create_xml_string($out_hash);
    return $xml_msg;
}


# Setze alles was auf "installed" steht auf setup
sub opsi_install_client {
    my ($msg, $msg_hash) = @_;
    my $header = @{$msg_hash->{'header'}}[0];
    my $source = @{$msg_hash->{'source'}}[0];
    my $target = @{$msg_hash->{'target'}}[0];
    my $session_id = @{$msg_hash->{'session_id'}}[0];
    my $forward_to_gosa = @{$msg_hash->{'forward_to_gosa'}}[0];
    my $hostId = @{$msg_hash->{'hostId'}}[0];

    # build return message with twisted target and source
    my $out_hash = &main::create_xml_hash("answer_$header", $target, $source);
    &add_content2xml_hash($out_hash, "session_id", $session_id);

    if (defined $forward_to_gosa) {
      &add_content2xml_hash($out_hash, "forward_to_gosa", $forward_to_gosa);
    }
    &add_content2xml_hash($out_hash, "hostId", "$hostId");

# Schaue nach produkten für diesen Host
# Setze alle Produkte dieses Hosts auf "setup"

#    # JSON Query
#    my $callobj = {
#      method  => 'deleteClient',
#      params  => [ $hostId ],
#      id  => 1,
#    };
#
#    my $res = $client->call($opsi_url, $callobj);

    my $xml_msg= &create_xml_string($out_hash);
    return $xml_msg;
}


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

  $client->call($opsi_url, $callobj);
}


sub _set_state {
  my $product = shift;
  my $hostId = shift;
  my $action = shift;
  my $callobj;

  $callobj = {
    method  => 'setProductState',
    params  => [ $product, $hostId, $action ],
    id  => 1,
  };

  $client->call($opsi_url, $callobj);
}

1;
