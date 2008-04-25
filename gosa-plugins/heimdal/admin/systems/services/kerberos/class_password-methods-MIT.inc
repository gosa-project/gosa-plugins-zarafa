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


class passwordMethodMIT extends passwordMethod
{

  var $krb5MaxLife      = "";
  var $krb5MaxRenew     = "";
  var $krb5ValidStart   = "";
  var $krb5ValidEnd     = "";
  var $krb5PasswordEnd  = "";
  var $krb5ValidStart_clear   = TRUE;
  var $krb5ValidEnd_clear     = TRUE;
  var $krb5PasswordEnd_clear  = TRUE;

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

  var $dn                     = "new";
  var $parent_dn              = "new";
  var $attributes             = array("krb5MaxLife","krb5MaxRenew","krb5KDCFlags",
                                      "krb5ValidStart","krb5ValidEnd","krb5PasswordEnd");
  var $attrs                  = array();
  var $is_account             = FALSE;

  var $server_list = array();

	function __construct(&$config,$dn = "new")  
	{
    $this->config= $config;
    $this->parent_dn = $dn;

    $this->is_account = FALSE;
    $this->krb5MaxLife    = 86400;
    $this->krb5MaxRenew   = 604800;
    $this->krb5ValidStart = date("Ymd",time())."0000Z";
    $this->krb5ValidEnd   = date("Ymd",time())."0000Z";
    $this->krb5PasswordEnd= date("Ymd",time())."0000Z;";

    if(!is_object($config)){
      return;
    }  

    /* Keep cache until we try to configure a principal 
     */
    if($dn != "new" && $dn != ""){
      echo "Reload cache";
      session::un_set("MIT_CACHE");
    }

    /* Get a list of all kerberos servers 
     */
    $ldap = $this->config->get_ldap_link();
    $ldap->cd($this->config->current['BASE']);
    $ldap->search("(&(objectClass=goServer)(objectClass=goKrbServer))",array("goKrbRealm","cn","description","macAddress"));
    $this->server_list = array();
    while($attrs = $ldap->fetch()){
      if(!isset($attrs['macAddress'][0])) continue;
      if(!isset($attrs['description'][0])) $attrs['description'][0] ="";
      $this->server_list[$attrs['cn'][0]] = array("macAddress" => $attrs['macAddress'][0],
                                                  "description"=> $attrs['description'][0],
                                                  "dn"         => $attrs['dn'],
                                                  "principals" => $this->load_principals_for_server($attrs['macAddress'][0]),
                                                  "goKrbRealm" => array($attrs['goKrbRealm'][0]),
                                                  "cn"         => $attrs['cn'][0]);
    }

    /* Load object data from ldap && initialize this class  
     */
    if($dn != "new" && $dn != ""){
      $ldap = $this->config->get_ldap_link();
      $ldap->cd($dn);
      $ldap->cat($dn);
      $this->attrs = $ldap->fetch();
      if(isset($this->attrs['userPassword'])){

        $p_name = preg_replace("/^\{".$this->get_hash_name()."\}/","",$this->attrs['userPassword'][0]);
   
        /* Try to detect server our principal is configured on
         */
        foreach($this->server_list as $server => $data){
          if(in_array($p_name,$data['principals'])){
            $info = $this->load_principal($data['macAddress'],$p_name);
            if(!count($info)){
            }
            break;
          }
        }
      }
    }
  }


  /*! \brief get list of all configured principals
        for a given server. 
       The results will cached.  
   */
  public function load_principal($server,$name)
  {
    return(array());
    $o = new gosaSupportDaemon();
    $tmp = array();
    $tmp = $o->krb5_get_principal($server,$name);
    if($o->is_error()){
      msg_dialog::display(_("Service infrastructure"),msgPool::siError($o->get_error()),ERROR_DIALOG);      
    }

    return($tmp); 
  }


  /*! \brief get list of all configured principals
        for a given server. 
       The results will cached.  
   */
  public function load_principals_for_server($server)
  {
    if(!session::is_set("MIT_CACHE")){
      session::set("MIT_CACHE",array());
    }
    $cache = session::get("MIT_CACHE");
    if(!isset($cache[$server])){
      $o = new gosaSupportDaemon();
      $tmp = $o->krb5_list_principals($server);
      $cache[$server] = $tmp;
      session::set("MIT_CACHE",$cache);
    }
    return($cache[$server]);
  }
 

  /*! \brief Check if this password method is useable. 
             This is the case if there is at least one si server which provides 
              kerberos support. 
   */ 
	function is_available()
	{
    foreach($this->server_list as $server => $data){
      if(count($data['principals'])){
        return(TRUE);
      }
    }
    return(FALSE);
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
        if (!$ldap->success()){
          msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $this->dn, LDAP_DEL, get_class()));
        }
      }
    }
  }


  function set_password($password)
  {
    #TODO
    # Add or modify kerberos entry below $this->dn
    # See https://oss.gonicus.de/labs/gosa/ticket/223
    # Order: create entries, then call the heimdal_keygen hook with the realm (returned by generate_hash)
    #        to let it add the missing kerberos keys.

    global $config;
    $cmd = "";
    if(isset($config->current['HEIMDAL_KEYGEN'])){
      $cmd = $config->current['HEIMDAL_KEYGEN'];
      if(!check_command($cmd)){
        new msg_dialog(_("Heimdal"), msgPool::cmdinvalid("HEIMDAL_KEYGEN",$cmd,_("Heimdal")),WARNING_DIALOG);
      }
    }
    if(isset($this->config->data['MAIN']['HEIMDAL_KEYGEN'])){
      $cmd = $this->config->data['MAIN']['HEIMDAL_KEYGEN'];
      if(!check_command($cmd)){
        new msg_dialog(_("Heimdal"), msgPool::cmdinvalid("HEIMDAL_KEYGEN",$cmd,_("Heimdal")),WARNING_DIALOG);
      }
    }
    if ($cmd != ""){

      /* Display in error message */
      $cmdd = $cmd." '".$this->generate_hash($password)."' 'PASSWORD'";

      /* Execute command and check return value */
      $cmd  = $cmd." '".$this->generate_hash($password)."' '".$password."'" ;
      exec($cmd,$out,$res);
      if($res != 0){
        new msg_dialog(_("Heimdal"), msgPool::cmdexecfailed("HEIMDAL_KEYGEN",$cmd,_("Heimdal")),WARNING_DIALOG);
      }
    }
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
    if (isset($_POST['pw_abort']) || $this->display == FALSE){
      $this->display = FALSE;
      return "";
    }

    /* Cancel heimdal options */
    if (isset($_POST['pw_save'])){
      $msgs = $this->check();
      if(count($msgs)){
        foreach($msgs as $msg){
          msg_dialog::display(_("Heimdal"),$msg,WARNING_DIALOG);
        }
      }else{
        $this->display = FALSE;
        return "";
      }
    }

    $years = array();
    $start = date("Y")-1;
    for($i = $start; $i < ($start +20) ; $i++){
      $years[$i] = $i;
    }
    $month= array();
    for($i = 1; $i <= 12  ; $i++){
      $month[str_pad($i,2,"0",STR_PAD_LEFT)] = $i;
    }
    $days= array();
    for($i = 1; $i <= 31  ; $i++){
      $days[str_pad($i,2,"0",STR_PAD_LEFT)] = $i;
    }
    $hours= array();
    for($i = 0; $i <= 23  ; $i++){
      $hours[str_pad($i,2,"0",STR_PAD_LEFT)] = $i;
    }
    $minutes= array();
    for($i = 0; $i <= 59  ; $i++){
      $minutes[str_pad($i,2,"0",STR_PAD_LEFT)] = $i;
    }
  

    $smarty = get_smarty();
    $smarty->assign("years",$years);
    $smarty->assign("month",$month);
    $smarty->assign("days",$days);
    $smarty->assign("hours",$hours);
    $smarty->assign("minutes",$minutes);

    $date_values = array("krb5ValidStart","krb5ValidEnd","krb5PasswordEnd");
    foreach($date_values as $date_val){
      $clear = $date_val."_clear";
      $smarty->assign($date_val."_clear",$this->$clear);
      $smarty->assign($date_val."_y",substr($this->$date_val,0,4));
      $smarty->assign($date_val."_m",substr($this->$date_val,4,2));
      $smarty->assign($date_val."_d",substr($this->$date_val,6,2));
      $smarty->assign($date_val."_h",substr($this->$date_val,8,2));
      $smarty->assign($date_val."_i",substr($this->$date_val,10,2));
    }

    foreach($this->attributes as $attr){
      $smarty->assign($attr ,$this->$attr);
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

      $date_values = array("krb5ValidStart","krb5ValidEnd","krb5PasswordEnd");
      foreach($date_values as $date_value){
        $clear = $date_value."_clear";
        if(isset($_POST[$date_value."_clear"])){
          $this->$clear = TRUE;
        }else{
          $this->$clear = FALSE;
          $str = "";
          foreach(array("y","m","d","h","i") as $val){
            if(isset($_POST[$date_value."_".$val])){
              $str .= $_POST[$date_value."_".$val]; 
            }
          }
          $this->$date_value = $str."Z";
        }
      }

      foreach($this->attributes as $attr){
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
    if(!is_numeric($this->krb5MaxLife) && !empty($this->krb5MaxLife)){
      $message[] = msgPool::invalid(_("Max life"),$this->krb5MaxLife,"/[0-9]/");
    }
    if(!is_numeric($this->krb5MaxRenew) && !empty($this->krb5MaxRenew)){
      $message[] = msgPool::invalid(_("Max renew"),$this->krb5MaxRenew,"/[0-9]/");
    }
   if(!$this->krb5ValidStart_clear && !$this->chk_times($this->krb5ValidStart)){
     $message[] = msgPool::invalid(_("Valid start"),$this->krb5ValidStart,"/[0-9]/");
   }
   if(!$this->krb5ValidEnd_clear && !$this->chk_times($this->krb5ValidEnd)){
     $message[] = msgPool::invalid(_("Valid end"),$this->krb5ValidEnd,"/[0-9]/");
   }
   if(!$this->krb5PasswordEnd_clear && !$this->chk_times($this->krb5PasswordEnd)){
     $message[] = msgPool::invalid(_("Valid password"),$this->krb5PasswordEnd,"/[0-9]/");
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

      if($this->is_account){ 
        foreach($this->attributes as $attr){
          $data[$attr] = array();
        }
      }

      /* Append Flags */
      $data['krb5KDCFlags']   = $this->krb5KDCFlags;
      if(!empty($this->krb5MaxLife)){
        $data['krb5MaxLife']    = $this->krb5MaxLife;
      }
      if(!empty($this->krb5MaxRenew)){
        $data['krb5MaxRenew']   = $this->krb5MaxRenew;
      }
      if(!$this->krb5ValidStart_clear){
        $data['krb5ValidStart'] = $this->krb5ValidStart;
      }
      if(!$this->krb5ValidEnd_clear){
        $data['krb5ValidEnd']   = $this->krb5ValidEnd;
      }
      if(!$this->krb5PasswordEnd_clear){
        $data['krb5PasswordEnd']= $this->krb5PasswordEnd;
      }

      /* Add / Updated data */
      $ldap->cd($dn);
      if(!$this->is_account){ 
        $ldap->add($data);
      }else{
        $ldap->modify($data);
      }
      if (!$ldap->success()){
        msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $dn, 0, get_class()));
      }
    }
  }
}

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
