package krb5;
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
use Data::Dumper;
use GOSA::GosaSupportDaemon;
use Authen::Krb5;
use Authen::Krb5::Admin qw(:constants);

BEGIN {}

END {}

### Start ######################################################################

Authen::Krb5::init_context;
Authen::Krb5::init_ets;

my $krb_admin;
my $krb_password;

my %cfg_defaults = (
"krb5" => {
   "admin" => [\$krb_admin, ""],
   "password" => [\$krb_password, ""],
   },
);
&read_configfile($main::cfg_file, %cfg_defaults);


sub read_configfile {
    my ($cfg_file, %cfg_defaults) = @_;
    my $cfg;

    if( defined( $cfg_file) && ( (-s $cfg_file) > 0 )) {
        if( -r $cfg_file ) {
            $cfg = Config::IniFiles->new( -file => $cfg_file );
        } else {
            &main::daemon_log("ERROR: krb5.pm couldn't read config file!", 1);
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


sub krb5_list_principals {
    my ($msg, $msg_hash) = @_;
    my $header = @{$msg_hash->{'header'}}[0];
    my $source = @{$msg_hash->{'source'}}[0];
    my $target = @{$msg_hash->{'target'}}[0];
    my $session_id = @{$msg_hash->{'session_id'}}[0];

    # build return message with twisted target and source
    my $out_hash = &main::create_xml_hash("answer_$header", $target, $source);
    &add_content2xml_hash($out_hash, "session_id", $session_id);

    # Authenticate
    my $kadm5 = Authen::Krb5::Admin->init_with_password($krb_admin, $krb_password);
    if (not defined $kadm5){
      &add_content2xml_hash($out_hash, "error", "Cannot connect to kadmin server");
    } else {
      my @principals= $kadm5->get_principals() or &add_content2xml_hash($out_hash, "error", Authen::Krb5::Admin::error);
      for my $principal (@principals) {
        &add_content2xml_hash($out_hash, "principal", $principal);
      }
    }

    # return message
    return &create_xml_string($out_hash);
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
    my $out_hash = &main::create_xml_hash("answer_$header", $target, $source);
    &add_content2xml_hash($out_hash, "session_id", $session_id);

    # Sanity check
    if (not defined @{$msg_hash->{'principal'}}[0]){
      &add_content2xml_hash($out_hash, "error", "No principal specified");
      return &create_xml_string($out_hash);
    }

    # Authenticate
    my $kadm5 = Authen::Krb5::Admin->init_with_password($krb_admin, $krb_password);
    my $principal;
    if (not defined $kadm5){
      &add_content2xml_hash($out_hash, "error", "Cannot connect to kadmin server");
    } else {
      $principal= Authen::Krb5::parse_name(@{$msg_hash->{'principal'}}[0]);
      if(not defined $principal) {
        &add_content2xml_hash($out_hash, "error", "Illegal principal name");
      } else {
        my $data= $kadm5->get_principal($principal) or &add_content2xml_hash($out_hash, "error", Authen::Krb5::Admin::error);
        &add_content2xml_hash($out_hash, "principal", @{$msg_hash->{'principal'}}[0]);
        &add_content2xml_hash($out_hash, "attributes", $data->attributes);
        &add_content2xml_hash($out_hash, "aux_attributes", $data->aux_attributes);
        &add_content2xml_hash($out_hash, "kvno", $data->kvno);
        &add_content2xml_hash($out_hash, "max_life", $data->max_life);
        &add_content2xml_hash($out_hash, "max_renewable_life", $data->max_renewable_life);
        &add_content2xml_hash($out_hash, "aux_attributes", $data->aux_attributes);
        &add_content2xml_hash($out_hash, "policy", $data->policy);
        &add_content2xml_hash($out_hash, "princ_expire_time", $data->princ_expire_time);
        &add_content2xml_hash($out_hash, "pw_expiration", $data->pw_expiration);
      }
    }

    # return message
    return &create_xml_string($out_hash);
}


sub krb5_del_principal {
    my ($msg, $msg_hash) = @_;
    my $header = @{$msg_hash->{'header'}}[0];
    my $source = @{$msg_hash->{'source'}}[0];
    my $target = @{$msg_hash->{'target'}}[0];
    my $session_id = @{$msg_hash->{'session_id'}}[0];

    # build return message with twisted target and source
    my $out_hash = &main::create_xml_hash("answer_$header", $target, $source);
    &add_content2xml_hash($out_hash, "session_id", $session_id);

    # Sanity check
    if (not defined @{$msg_hash->{'principal'}}[0]){
      &add_content2xml_hash($out_hash, "error", "No principal specified");
      return &create_xml_string($out_hash);
    }

    # Authenticate
    my $kadm5 = Authen::Krb5::Admin->init_with_password($krb_admin, $krb_password);
    my $principal;
    if (not defined $kadm5){
      &add_content2xml_hash($out_hash, "error", "Cannot connect to kadmin server");
    } else {
      $principal= Authen::Krb5::parse_name(@{$msg_hash->{'principal'}}[0]);
      if(not defined $principal) {
        &add_content2xml_hash($out_hash, "error", "Illegal principal name");
      } else {
        $kadm5->delete_principal($principal) or &add_content2xml_hash($out_hash, "error", Authen::Krb5::Admin::error);
      }
    }

    # return message
    return &create_xml_string($out_hash);
}


sub krb5_list_policies {
    my ($msg, $msg_hash) = @_;
    my $header = @{$msg_hash->{'header'}}[0];
    my $source = @{$msg_hash->{'source'}}[0];
    my $target = @{$msg_hash->{'target'}}[0];
    my $session_id = @{$msg_hash->{'session_id'}}[0];

    # build return message with twisted target and source
    my $out_hash = &main::create_xml_hash("answer_$header", $target, $source);
    &add_content2xml_hash($out_hash, "session_id", $session_id);

    # Authenticate
    my $kadm5 = Authen::Krb5::Admin->init_with_password($krb_admin, $krb_password);
    if (not defined $kadm5){
      &add_content2xml_hash($out_hash, "error", "Cannot connect to kadmin server");
    } else {
      my @policies= $kadm5->get_policies() or &add_content2xml_hash($out_hash, "error", Authen::Krb5::Admin::error);
      for my $policy (@policies) {
        &add_content2xml_hash($out_hash, "policy", $policy);
      }
    }

    # return message
    return &create_xml_string($out_hash);
}


sub krb5_get_policy {
    my ($msg, $msg_hash) = @_;
    my $header = @{$msg_hash->{'header'}}[0];
    my $source = @{$msg_hash->{'source'}}[0];
    my $target = @{$msg_hash->{'target'}}[0];
    my $session_id = @{$msg_hash->{'session_id'}}[0];

    # build return message with twisted target and source
    my $out_hash = &main::create_xml_hash("answer_$header", $target, $source);
    &add_content2xml_hash($out_hash, "session_id", $session_id);

    # Sanity check
    if (not defined @{$msg_hash->{'policy'}}[0]){
      &add_content2xml_hash($out_hash, "error", "No policy specified");
      return &create_xml_string($out_hash);
    }

    # Authenticate
    my $kadm5 = Authen::Krb5::Admin->init_with_password($krb_admin, $krb_password);
    my $principal;
    if (not defined $kadm5){
      &add_content2xml_hash($out_hash, "error", "Cannot connect to kadmin server");
    } else {
      my $data= $kadm5->get_principal($principal) or &add_content2xml_hash($out_hash, "error", Authen::Krb5::Admin::error);
      &add_content2xml_hash($out_hash, "principal", @{$msg_hash->{'principal'}}[0]);
      &add_content2xml_hash($out_hash, "attributes", $data->attributes);
      &add_content2xml_hash($out_hash, "aux_attributes", $data->aux_attributes);
      &add_content2xml_hash($out_hash, "kvno", $data->kvno);
      &add_content2xml_hash($out_hash, "max_life", $data->max_life);
      &add_content2xml_hash($out_hash, "max_renewable_life", $data->max_renewable_life);
      &add_content2xml_hash($out_hash, "aux_attributes", $data->aux_attributes);
      &add_content2xml_hash($out_hash, "policy", $data->policy);
      &add_content2xml_hash($out_hash, "princ_expire_time", $data->princ_expire_time);
      &add_content2xml_hash($out_hash, "pw_expiration", $data->pw_expiration);
    }

    # return message
    return &create_xml_string($out_hash);
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
    my $out_hash = &main::create_xml_hash("answer_$header", $target, $source);
    &add_content2xml_hash($out_hash, "session_id", $session_id);

    # Sanity check
    if (not defined @{$msg_hash->{'policy'}}[0]){
      &add_content2xml_hash($out_hash, "error", "No policy specified");
      return &create_xml_string($out_hash);
    }

    # Authenticate
    my $kadm5 = Authen::Krb5::Admin->init_with_password($krb_admin, $krb_password);
    my $policy;
    if (not defined $kadm5){
      &add_content2xml_hash($out_hash, "error", "Cannot connect to kadmin server");
    } else {
      $kadm5->delete_policy($policy) or &add_content2xml_hash($out_hash, "error", Authen::Krb5::Admin::error);
    }

    # return message
    return &create_xml_string($out_hash);
}

1;
