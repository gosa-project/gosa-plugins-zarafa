<?php
/*
   This code is part of GOsa (https://gosa.gonicus.de)
   Copyright (C) 2007 Cajus Pollmeier

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */


class passwordMethodheimdal extends passwordMethod
{

  var $krb5MaxLife      = 86400;
  var $krb5MaxRenew     = 604800;
  var $krb5ValidStart   = "200712310000Z";
  var $krb5ValidEnd     = "201012310000Z";
  var $krb5PasswordEnd  = "201012310000Z";

  var $unlimited_krb5MaxLife    = FALSE;
  var $unlimited_krb5MaxRenew   = FALSE;
  var $unlimited_krb5ValidStart = FALSE;
  var $unlimited_krb5ValidEnd   = FALSE;
  var $unlimited_krb5PasswordEnd= FALSE;

  var $display = TRUE;

  var $flag_list = array(
      "0"=>"initial" , 
      "1"=>"forwardable" , 
      "2"=>"proxiable" , 
      "3"=>"renewable" , 
      "4"=>"postdate" , 
      "5"=>"server" , 
      "6"=>"client" , 
      "7"=>"invalid" , 
      "8"=>"require-preauth" , 
      "9"=>"change-pw" , 
      "10"=>"require-hwauth" , 
      "11"=>"ok-as-delegate" , 
      "12"=>"user-to-user" , 
      "13"=>"immutable");
  var $krb5KDCFlags     = 123;

  /* Dummy */
  var $unlimited_krb5KDCFlags = FALSE;
  var $dn                     = "new";
  var $parent_dn              = "new";
  var $attributes             = array("krb5MaxLife","krb5MaxRenew","krb5KDCFlags",
                                      "krb5ValidStart","krb5ValidEnd","krb5PasswordEnd");
  var $attrs                  = array();
  var $is_account             = FALSE;

	function passwordMethodheimdal(&$config,$dn = "new")  
	{
    $this->config= $config;
    $this->parent_dn = $dn;

    /* Load existing entries */
    if($dn != "new"){
      $ldap = $this->config->get_ldap_link();
      $ldap->cd($dn);
      $ldap->ls("objectClass=krb5Principal",$dn,array("*"));
      if($ldap->count()==1){
        $this->is_account = TRUE;
        $this->attrs = $ldap->fetch();
        $this->dn = $this->attrs['dn'];
        foreach($this->attributes as $attr){
          $uattr = "unlimited_".$attr;
          if(isset($this->attrs[$attr][0])){
            $this->$attr = $this->attrs[$attr][0];
          }else{
            $this->$uattr = TRUE;
          }
        }
      }elseif($ldap->count() >= 2){
        new msg_dialog(_("Error initialising heimdal"),_("Error loading heimdal principals, more than one entry found."));
      }
    }
	}


	function is_available()
	{
    #TODO
    # Check if heimdal information is available
    # in configuration and return true/false
    #
    # In particular, we need the "heimdal_keygen" to feed kadmin in order to return "true".
    #
    return true;
	}


	function generate_hash($pwd)
	{
    $mode= "kerberos";
    if (isset($this->config->current['KRBSASL']) && preg_match('/^true$/i', $this->config->current['KRBSASL'])){
      $mode= "sasl";
    }

    return "{".$mode."}".$this->attrs['uid'][0]."@".$cfg= $this->config->data['SERVERS']['KERBEROS']['REALM'];
	}


  function remove_from_parent()
  {
    if($this->is_account && $this->dn != "new"){
      $ldap = $this->config->get_ldap_link();
      $ldap->cat($this->dn,array("dn"));
      if($ldap->count()){
        $ldap->rmdir($this->dn);
        show_ldap_error($ldap->get_error(),_("Tried to remove heimdal principal extension."));
      }
    }
  }


  function set_password()
  {
    #TODO
    # Add or modify kerberos entry below $this->dn
    # See https://oss.gonicus.de/labs/gosa/ticket/223
    # Order: create entries, then call the heimdal_keygen hook with the realm (returned by generate_hash)
    #        to let it add the missing kerberos keys.
  }


  function get_hash_name()
  {
    $mode= "kerberos";
    if (isset($this->config->current['KRBSASL']) && preg_match('/^true$/i', $this->config->current['KRBSASL'])){
      $mode= "sasl";
    }
    return "$mode";
  }


  function is_configurable()
  {
    return TRUE;
  }


  function configure()
  {
    $this->save_object();

    /* Cancel heimdal options */
    if (isset($_POST['pw_abort'])){
      return "";
    }

    /* Cancel heimdal options */
    if (isset($_POST['pw_save'])){
      $msgs = $this->check();
      if(count($msgs)){
        foreach($msgs as $msg){
          msg_dialog::display(_("Heimdal properties"),$msg,WARNING_DIALOG);
        }
      }else{
        $this->display = FALSE;
        return "";
      }
    }

    $smarty = get_smarty();
    foreach($this->attributes as $attr){
      $uattr = "unlimited_".$attr;
      $smarty->assign($attr ,$this->$attr);
      $smarty->assign($uattr,$this->$uattr);
    }
    foreach($this->flag_list as $key => $name){
      $val = pow(2,$key);
      if($this->krb5KDCFlags & $val){
        $smarty->assign("krb5KDCFlags_".$key,TRUE);
      }else{
        $smarty->assign("krb5KDCFlags_".$key,FALSE);
      }
    }

    return($smarty->fetch(get_template_path("pwd_heimdal.tpl")));
  }

  
  function save_object()
  {
    if(isset($_POST['pwd_heimdal_posted'])){
      foreach($this->attributes as $attr){
        $uattr = "unlimited_".$attr;
        if(isset($_POST[$uattr])){
          $this->$uattr = TRUE;
        }else{
          $this->$uattr = FALSE;
        }

        if(isset($_POST[$attr])){
          $this->$attr = get_post($attr);
        }
      }
  
      $int = "";
      foreach($this->flag_list as $key => $name){
        $post = "krb5KDCFlags_".$key;
        if(isset($_POST[$post])){
          $int |= pow(2,$key);
        }
      }
      $this->krb5KDCFlags = $int;
    }
  }

  function check()
  {
    $message = array();
    if(!is_numeric($this->krb5MaxLife) && !$this->unlimited_krb5MaxLife){
      $message[] = sprintf(_("Please specify a numeric value for %s."),_("Max life"));
    }
    if(!is_numeric($this->krb5MaxRenew) && !$this->unlimited_krb5MaxRenew){
      $message[] = sprintf(_("Please specify a numeric value for %s."),_("Max renew"));
    }
   if((empty($this->krb5ValidStart) || !$this->chk_times($this->krb5ValidStart)) && !$this->unlimited_krb5ValidStart){
     $message[] = sprintf(_("Please specify a numeric value for %s."),_("Valid start"));
   }
   if((empty($this->krb5ValidEnd) || !$this->chk_times($this->krb5ValidEnd)) && !$this->unlimited_krb5ValidEnd){
     $message[] = sprintf(_("Please specify a numeric value for %s."),_("Valid end"));
   }
   if((empty($this->krb5PasswordEnd) || !$this->chk_times($this->krb5PasswordEnd)) && !$this->unlimited_krb5PasswordEnd){
     $message[] = sprintf(_("Please specify a numeric value for %s."),_("Valid password"));
   }
    return($message);
  }


  function chk_times($str)
  {
    if(preg_match("/^([0-9]){12,12}[a-z]$/i",$str)){
      return(true);
    }
    return(false);
  }
 
 
  function save($dn)
  {
    $realm = $this->config->data['SERVERS']['KERBEROS']['REALM'];

    $ldap = $this->config->get_ldap_link();
    $ldap->cd($dn);
    $ldap->cat($dn,array('uid'));
    $attrs = $ldap->fetch();
    if(isset($attrs['uid'][0])){

      $uid  = $attrs['uid'][0];
      $name = $uid."@".strtoupper($realm); 
      $dn   = "krb5PrincipalName=".$name.",".$dn;

      $data = array();
      $data['krb5PrincipalName'] = $name;
      $data['objectClass']  = array("top","account","krb5Principal","krb5KDCEntry");
      $data['krb5PrincipalName'] =$name;
      $data['uid'] = $uid;
      $data['krb5KeyVersionNumber'] = rand(100000,99999999);

      if(!$this->is_account){ 
        foreach($this->attributes as $attr){
          $data[$attr] = array();
        }
      }

      /* Append Flags */
      $data['krb5KDCFlags']   = $this->krb5KDCFlags;
      if(!$this->unlimited_krb5MaxLife){
        $data['krb5MaxLife']    = $this->krb5MaxLife;
      }
      if(!$this->unlimited_krb5MaxRenew){
        $data['krb5MaxRenew']   = $this->krb5MaxRenew;
      }
      if(!$this->unlimited_krb5ValidStart){
        $data['krb5ValidStart'] = $this->krb5ValidStart;
      }
      if(!$this->unlimited_krb5ValidEnd){
        $data['krb5ValidEnd']   = $this->krb5ValidEnd;
      }
      if(!$this->unlimited_krb5PasswordEnd){
        $data['krb5PasswordEnd']= $this->krb5PasswordEnd;
      }

      /* This should not happen, because the UID can't be modified via GOsa ui */
      if(!$this->is_account& $dn != $this->dn){  
        plugin::move($this->dn,$dn);
      }

      /* Add / Updated data */
      $ldap->cd($dn);
      if($this->is_account){
        $ldap->add($data);
      }else{
        $ldap->modify($data);
      }
      show_ldap_error($ldap->get_error(),"Mist");   
    }
  }
}

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
