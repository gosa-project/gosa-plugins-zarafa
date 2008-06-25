package opsi;
use Exporter;
@ISA = qw(Exporter);
my @events = (
    "get_events",
    "opsi_get_netboot_products",  
    "opsi_get_local_products",
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
my $opsi_url= "$opsi_admin:$opsi_password@$opsi_server:4447/rpc";


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

    # build return message with twisted target and source
    my $out_hash = &main::create_xml_hash("answer_$header", $target, $source);
    &add_content2xml_hash($out_hash, "session_id", $session_id);

    # Authenticate
    my $client = new JSON::RPC::Client;
    my $callobj = {
      method  => 'getPossibleProductActions_hash',
      params  => [ ],
      id  => 1,
    };

    my $res = $client->call($opsi_url, $callobj);

    if($res) {
      if ($res->is_error) {
        print STDERR "Error : ", $res->error_message;
      }
      else {
        print STDERR Dumper($res->result);
      }
    }
    else {
      print STDERR $client->status_line;
    }

    my $forward_to_gosa = @{$msg_hash->{'forward_to_gosa'}}[0];
    if (defined $forward_to_gosa) {
        &add_content2xml_hash($out_hash, "forward_to_gosa", $forward_to_gosa);
    }

#      my $data= $kadm5->get_policy(@{$msg_hash->{'policy'}}[0]) or &add_content2xml_hash($out_hash, "error", Authen::Krb5::Admin::error);
#      &add_content2xml_hash($out_hash, "name", $data->name);
#      &add_content2xml_hash($out_hash, "mask", $data->mask);
#      &add_content2xml_hash($out_hash, "pw_history_num", $data->pw_history_num);
#      &add_content2xml_hash($out_hash, "pw_max_life", $data->pw_max_life);
#      &add_content2xml_hash($out_hash, "pw_min_classes", $data->pw_min_classes);
#      &add_content2xml_hash($out_hash, "pw_min_length", $data->pw_min_length);
#      &add_content2xml_hash($out_hash, "pw_min_life", $data->pw_min_life);
#      &add_content2xml_hash($out_hash, "policy_refcnt", $data->policy_refcnt);

    # return message
    return &create_xml_string($out_hash);
}


sub opsi_set_product_properties {
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

    # build return message with twisted target and source
    my $out_hash = &main::create_xml_hash("answer_$header", $target, $source);
    &add_content2xml_hash($out_hash, "session_id", $session_id);

    if (defined $forward_to_gosa) {
        &add_content2xml_hash($out_hash, "forward_to_gosa", $forward_to_gosa);
    }

    # return message
    return &create_xml_string($out_hash);
}


sub opsi_get_client_software {
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

    # return message
    return &create_xml_string($out_hash);
}


sub opsi_get_local_products {
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

    # return message
    return &create_xml_string($out_hash);
}

1;
