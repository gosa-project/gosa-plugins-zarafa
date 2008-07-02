package opsi;
use Exporter;
@ISA = qw(Exporter);
my @events = (
    "get_events",
    "opsi_get_netboot_products",  
    "opsi_get_local_products",
    "opsi_get_product_properties",
    "opsi_set_product_properties",
    "opsi_get_client_hardware",
    "opsi_get_client_software",
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

  if($res) {
    if ($res->is_error) {
      &main::daemon_log("ERROR: opsi configed communication failed: ".$res->error_message, 1);
    } else {
      return 1;
    }
  } else {
    &main::daemon_log("ERROR: opsi configed communication failed: ".$client->status_line, 1);
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

    # Get hostID if defined
    if (defined @{$msg_hash->{'hostId'}}[0]){
	$hostId = @{$msg_hash->{'hostId'}}[0];
    }

    # build return message with twisted target and source
    my $out_hash = &main::create_xml_hash("answer_$header", $target, $source);
    &add_content2xml_hash($out_hash, "session_id", $session_id);

    if (defined $forward_to_gosa) {
      &add_content2xml_hash($out_hash, "forward_to_gosa", $forward_to_gosa);
    }
    &add_content2xml_hash($out_hash, "xxx", "");
    my $xml_msg= &create_xml_string($out_hash);

    # Authenticate
    my $callobj;
    if (defined $hostId){
	    $callobj = {
	      method  => 'getNetBootProductIds_list',
	      params  => [ $hostId ],
	      id  => 1,
	    };
    } else {
	    $callobj = {
	      method  => 'getNetBootProductIds_list',
	      params  => [ ],
	      id  => 1,
	    };
    }

    my $res = $client->call($opsi_url, $callobj);
    if (check_res($res)){
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

    # Get hostID if defined
    if (defined @{$msg_hash->{'hostId'}}[0]){
	$hostId = @{$msg_hash->{'hostId'}}[0];
    }

    # build return message with twisted target and source
    my $out_hash = &main::create_xml_hash("answer_$header", $target, $source);
    &add_content2xml_hash($out_hash, "session_id", $session_id);

    if (defined $forward_to_gosa) {
      &add_content2xml_hash($out_hash, "forward_to_gosa", $forward_to_gosa);
    }
    &add_content2xml_hash($out_hash, "ProducId", "$productId");
    &add_content2xml_hash($out_hash, "xxx", "");
    my $xml_msg= &create_xml_string($out_hash);


# Move to getProductProperties_hash  prod + objectid

    # JSON Query
    my $callobj = {
      method  => 'getProductPropertyDefinitions_listOfHashes',
      params  => [ $productId ],
      id  => 1,
    };

    my $res = $client->call($opsi_url, $callobj);
    if (check_res($res)){

      foreach my $r (@{$res->result}) {
        my $item= "<item>";
        foreach my $key (keys %{$r}) {
          my $value = $r->{$key};
          if (UNIVERSAL::isa( $value, "ARRAY" )){
            foreach my $subval (@{$value}){
              $item.= "<$key>".xml_quote($subval)."</$key>";
            }
          } else {
            $item.= "<$key>".xml_quote($value)."</$key>";
          }
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
    my $hostId;

    # Get hostID if defined
    if (defined @{$msg_hash->{'hostId'}}[0]){
	$hostId = @{$msg_hash->{'hostId'}}[0];
    }

    # build return message with twisted target and source
    my $out_hash = &main::create_xml_hash("answer_$header", $target, $source);
    &add_content2xml_hash($out_hash, "session_id", $session_id);

    if (defined $forward_to_gosa) {
        &add_content2xml_hash($out_hash, "forward_to_gosa", $forward_to_gosa);
    }

# Params += objectId ='hostId'

# Produkt
# Property
# Wert
#   <ProductId>ntfs-restore-image</ProductId>
#   <item>
#    <name>askbeforeinst</name>
#    <value>false</value>
#   </item>

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
        my $item= "<item><name>".xml_quote($r)."</name>";
        my $value= $result->{$r};
        foreach my $sres (@{$value}){

          foreach my $dres (keys %{$sres}){
            if (defined $sres->{$dres}){
              $item.= "<$dres>".xml_quote($sres->{$dres})."</$dres>";
            }
          }

          $item.= "</item>";
          $xml_msg=~ s/<xxx><\/xxx>/$item<xxx><\/xxx>/;

        }
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

    # Get hostID if defined
    if (defined @{$msg_hash->{'hostId'}}[0]){
	$hostId = @{$msg_hash->{'hostId'}}[0];
    }

    # build return message with twisted target and source
    my $out_hash = &main::create_xml_hash("answer_$header", $target, $source);
    &add_content2xml_hash($out_hash, "session_id", $session_id);

    if (defined $forward_to_gosa) {
      &add_content2xml_hash($out_hash, "forward_to_gosa", $forward_to_gosa);
    }
    &add_content2xml_hash($out_hash, "xxx", "");
    my $xml_msg= &create_xml_string($out_hash);

    # Append hostId
    my $callobj;
    if (defined $hostId){
	    $callobj = {
	      method  => 'getLocalBootProductIds_list',
	      params  => [ $hostId ],
	      id  => 1,
	    };
    } else {
	    $callobj = {
	      method  => 'getLocalBootProductIds_list',
	      params  => [ ],
	      id  => 1,
	    };
    }

    my $res = $client->call($opsi_url, $callobj);
    if (check_res($res)){
      foreach my $r (@{$res->result}) {
        $callobj = {
          method  => 'getProduct_hash',
          params  => [ $r ],
          id  => 1,
        };

        my $sres = $client->call($opsi_url, $callobj);
        if (check_res($sres)){
          my $tres= $sres->result;

          my $name= $tres->{'name'};
          my $description= $tres->{'description'};
          $name=~ s/\//\\\//;
          $description=~ s/\//\\\//;
          $xml_msg=~ s/<xxx><\/xxx>/<item><id>".xml_quote($r).<\/id><name><\/name><description>".xml_quote($description)."<\/description><\/item><xxx><\/xxx>/;
        }

      }
  }

  $xml_msg=~ s/<xxx><\/xxx>//;

  return $xml_msg;
}

1;
