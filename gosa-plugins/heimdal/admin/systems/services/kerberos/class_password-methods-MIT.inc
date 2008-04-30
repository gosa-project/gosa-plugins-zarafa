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


/*

   ATTRIBUTE INFORMATIONS taken from 
   http://www.mit.edu/~kerberos/krb5-1.5/krb5-1.5/doc/krb5-admin/Adding-or-Modifying-Principals.html

   expire date          
   -Sets the expiration date of the principal to 
   date.

   pwexpire date        
   -Sets the expiration date of the password to 
   date.

   maxlife maxlife      
   -Sets the maximum ticket life of the principal 
   to maxlife.

   maxrenewlife maxrenewlife
   -Sets the maximum renewable life of tickets 
   for the principal to maxrenewlife.

   kvno number
   -Explicity sets the key version number to number. 
   MIT does not recommend doing this unless there 
   is a specific reason.

   policy policy
   -Sets the policy used by this principal. (See Policies.) 
   With modify_principal, the current policy assigned to 
   the principal is set or changed. With add_principal, 
   if this option is not supplied, the -clearpolicy is 
   not specified, and the policy "default" exists, that 
   policy is assigned. If a principal is created with no 
   policy, kadmin will print a warning message.

   clearpolicy
   -For modify_principal, removes the current policy from a 
   principal. For add_principal, suppresses the automatic 
   assignment of the policy "default".


   {-|+}allow_postdated

   The "-allow_postdated" option prohibits this principal 
   from obtaining postdated tickets. "+allow_postdated" 
   clears this flag. In effect, "-allow_postdated" sets the 
   DISALLOW_POSTDATED flag on the principal in the 
   database.

   {-|+}allow_forwardable
   The "-allow_forwardable" option prohibits this principal 
   from obtaining forwardable tickets. "+allow_forwardable" 
   clears this flag. In effect, "-allow_forwardable" sets the 
   DISALLOW_FORWARDABLE flag on the principal in the 
   database.

   {-|+}allow_renewable
   The "-allow_renewable" option prohibits this principal 
   from obtaining renewable tickets. "+allow_renewable" 
   clears this flag. In effect, "-allow_renewable" sets the 
   DISALLOW_RENEWABLE flag on the principal in the 
   database.

   {-|+}allow_proxiable
   The "-allow_proxiable" option prohibits this principal 
   from obtaining proxiable tickets. "+allow_proxiable" 
   clears this flag. In effect, "-allow_proxiable" sets 
   the DISALLOW_PROXIABLE flag. on the principal 
   in the database.

   {-|+}allow_dup_skey
The "-allow_dup_skey" option disables user-to-user 
authentication for this principal by prohibiting this 
principal from obtaining a session key for another user. 
"+allow_dup_skey" clears this flag. In effect, "-allow_dup_skey" 
sets the DISALLOW_DUP_SKEY flag on the principal 
in the database.

{-|+}requires_preauth
The "+requires_preauth" option requires this principal 
to preauthenticate before being allowed to kinit. 
-requires_preauth clears this flag. In effect, +requires_preauth 
sets the REQUIRES_PRE_AUTH flag on the principal 
in the database.

{-|+}requires_hwauth
The "+requires_hwauth" flag requires the principal to 
preauthenticate using a hardware device before being 
allowed to kinit. "-requires_hwauth" clears this flag. 
In effect, "+requires_hwauth" sets the REQUIRES_HW_AUTH 
flag on the principal in the database.

{-|+}allow_svr
The "-allow_svr" flag prohibits the issuance of service 
tickets for this principal. "+allow_svr" clears this flag. 
In effect, "-allow_svr" sets the DISALLOW_SVR flag 
on the principal in the database.

{-|+}allow_tgs_req
The "-allow_tgs_req" option specifies that a Ticket-Granting 
Service (TGS) request for a service ticket for this principal 
is not permitted. You will probably never need to use this option. 
"+allow_tgs_req" clears this flag. The default is "+allow_tgs_req". 
In effect, "-allow_tgs_req" sets the DISALLOW_TGT_BASED 
flag on the principal in the database.

{-|+}allow_tix
The "-allow_tix" option forbids the issuance of any tickets for 
this principal. "+allow_tix" clears this flag. The default is 
"+allow_tix". In effect, "-allow_tix" sets the DISALLOW_ALL_TIX 
flag on the principal in the database.

{-|+}needchange
The "+needchange" option sets a flag in attributes field to force a 
password change; "-needchange" clears it. The default is "-needchange". 
In effect, "+needchange" sets the REQUIRES_PWCHANGE 
flag on the principal in the database.

{-|+}password_changing_service
The "+password_changing_service" option sets a flag in 
the attributes field marking this principal as a password 
change service. (Again, you will probably never need to use 
    this option.) "-password_changing_service" clears the flag. 
The default is "-password_changing_service". In effect, the 
"+password_changing_service" option sets the PWCHANGE_SERVICE 
flag on the principal in the database.

-randkey
Sets the key for the principal to a random value (add_principal only). 
MIT recommends using this option for host keys.

-pw password
Sets the key of the principal to the specified string and does not
prompt for a password (add_principal only). MIT does not recommend 
using this option.

*/



class passwordMethodMIT extends passwordMethod
{

  var $dn          = "new";     // DN of the current object
  var $parent_dn   = "new";     // parents DN
  var $is_account  = FALSE;     // This is TRUE if this object already has a krb extension
  var $server_list = array();   // A list with all configured servers
  var $map         = array();   // Mapping array, maps SERVER-REALM, REALM-SERVER ...

  var $goKrbRealm  = "";        // The realm name this principal belongs to
  var $principal   = "";        // The principals name (e.g. user@MY-DOMAIN.SYS)

  var $values = array(
      "PRINC_EXPIRE_TIME",      // Expiry date of this principal
      "PW_EXPIRATION",          // Password expiration 
      "MAX_LIFE",               // Ticket lifetime
      "MASK",                   // I'dont know 
      "MAX_RENEWABLE_LIFE",     // Max ticket lifetime when renewed
      "POLICY");                // The policy used by this principal

  var $flags = array(
      "DISALLOW_POSTDATED"  ,   // Pohibit postdated tickets
      "DISALLOW_FORWARDABLE",   // Prohibit forwardable tickets
      "DISALLOW_RENEWABLE"  ,   // Prohibit renewable tickets
      "DISALLOW_PROXIABLE"  ,   // Disallow proxiable tickets
      "DISALLOW_DUP_SKEY"   ,   // Disallow user to user authentification
      "REQUIRES_PRE_AUTH"   ,   // Preauthentication required
      "REQUIRES_HW_AUTH"    ,   // Hardware preauthentication
      "DISALLOW_SVR"        ,   // Prohibit issuance of service tickets
      "DISALLOW_TGT_BASED"  ,   // Disallow Ticket-Granting Service
      "DISALLOW_ALL_TIX"    ,   // Forbid ticket issuance
      "REQUIRES_PWCHANGE"   ,   // Force a password change
      "PWCHANGE_SERVICE"    );  // Password change service 
  
  var $readonly = array(
      "FAIL_AUTH_COUNT",        // The number of failed logins 
      "KVNO",                   // Key version number
      "LAST_FAILED",            // Last failed login time
      "LAST_PWD_CHANGE",        // Password last change time
      "LAST_SUCCESS",           // Last successful login 
      "MOD_DATE");              // Last modification time


  var $POLICY                   = "";
  var $POLICIES                 = array(); // Policies provided by the corrently selected realm/server

  var $PRINC_EXPIRE_TIME        = 0;
  var $PW_EXPIRATION            = 0;
  var $PRINC_EXPIRE_TIME_clear  = TRUE;
  var $PW_EXPIRATION_clear      = TRUE;
  var $MAX_LIFE                 = 0;
  var $MAX_RENEWABLE_LIFE       = 0;
  var $MASK                     = "";

  var $DISALLOW_POSTDATED       = FALSE;
  var $DISALLOW_FORWARDABLE     = FALSE;
  var $DISALLOW_RENEWABLE       = FALSE;
  var $DISALLOW_PROXIABLE       = FALSE;
  var $DISALLOW_DUP_SKEY        = FALSE;
  var $REQUIRES_PRE_AUTH        = FALSE;
  var $REQUIRES_HW_AUTH         = FALSE;
  var $DISALLOW_SVR             = FALSE;
  var $DISALLOW_TGT_BASED       = FALSE;
  var $DISALLOW_ALL_TIX         = FALSE;
  var $REQUIRES_PWCHANGE        = FALSE;
  var $PWCHANGE_SERVICE         = FALSE;

  var $FAIL_AUTH_COUNT          = 0;
  var $KVNO                     = "";
  var $LAST_FAILED              = 0;
  var $LAST_PWD_CHANGE          = 0;
  var $LAST_SUCCESS             = 0;
  var $MOD_DATE                 = 0;

  function __construct(&$config,$dn = "new")  
  {
    $this->config= $config;
    $this->parent_dn = $dn;

    /* No config object given, this may be the case 
        if there is only a is_available() request triggered.
     */
    if(!is_object($config)){
      return;
    }  

    /* Keep cache until we try to configure a principal 
     */
    if($dn != "new" && $dn != ""){
      session::un_set("MIT_CACHE");
    }

    /* Get a list of all kerberos servers, defined in ldap
        and get a list of principals they are providing. 
     */
    $ldap = $this->config->get_ldap_link();
    $ldap->cd($this->config->current['BASE']);
    $ldap->search("(&(objectClass=goServer)(objectClass=goKrbServer))",array("goKrbRealm","cn","description","macAddress"));
    $this->server_list = array();
    while($attrs = $ldap->fetch()){
      if(!isset($attrs['macAddress'][0])) continue;
      if(!isset($attrs['description'][0])) $attrs['description'][0] ="";

      $principals = $this->load_principals_for_server($attrs['macAddress'][0]);

      /* Create Realm/Server/Principal mapping.
       */
      foreach($principals as $principal){
        $this->map["PRINCIPAL_SERVER"][$principal] = $attrs['cn'][0];
        $this->map["PRINCIPAL_REALM"] [$principal] = $attrs['goKrbRealm'][0];
      }
      $this->map["SERVER_REALM"][$attrs['cn'][0]] = $attrs['goKrbRealm'][0];
      $this->map["REALM_SERVER"][$attrs['goKrbRealm'][0]] = $attrs['cn'][0];

      /* Create Server list
       */
      $this->server_list[$attrs['cn'][0]] = array("macAddress" => $attrs['macAddress'][0],
          "description"=> $attrs['description'][0],
          "dn"         => $attrs['dn'],
          "principals" => $principals,
          "goKrbRealm" => $attrs['goKrbRealm'][0],
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
        if($this->map['PRINCIPAL_SERVER'][$p_name]){
          $server= $this->map['PRINCIPAL_SERVER'][$p_name];
          $this->goKrbRealm = $this->map['SERVER_REALM'][$server];
          $this->principal  = $p_name;

          /* Load policies */
          $server_name = $this->map['REALM_SERVER'][$this->goKrbRealm];
          $server_mac  = $this->server_list[$server_name]['macAddress'];
          $this->POLICIES = $this->load_policies_for_server($server_mac);

          /* Load principal */
          $this->load_principal($this->server_list[$server]['macAddress'],$p_name);
        }
      }
    }
  }


  /*! \brief  Load this plugin with the values of the given principal
      @param  String  The macAddress of the kerberos server.
      @param  String  The name of the principal to load.
   */
  public function load_principal($server,$name)
  {
    $o = new gosaSupportDaemon();
    $tmp = array();
    $tmp = $o->krb5_get_principal($server,$name);
    if($o->is_error()){
      msg_dialog::display(_("Service infrastructure"),msgPool::siError($o->get_error()),ERROR_DIALOG);      
    }else{

      /* Load flags 
       */
      if(isset($tmp['ATTRIBUTES'])){
        foreach($this->flags as $flag){
          if(in_array($flag,$tmp['ATTRIBUTES'])){
            $this->$flag = TRUE;
          }
        } 
      }

      /* Load readonly attributes 
       */
      foreach($this->readonly as $attr){
        if(isset($tmp[$attr])){
          $this->$flag = $tmp[$attr];
        }
      } 

      /* Load modifyable attributes
       */
      foreach($this->values as $attr){
        if(isset($tmp[$attr])){
          $this->$flag = $tmp[$attr];
        }
      } 
    }
  }


  /*! \brief get list of all configured principals
    for a given server. 
    The results will cached.  
   */
  public function load_principals_for_server($server)
  {
    if(!session::is_set("MIT_PRINCIPAL_CACHE")){
      session::set("MIT_PRINCIPAL_CACHE",array());
    }
    $cache = session::get("MIT_PRINCIPAL_CACHE");
    if(!isset($cache[$server])){
      $o = new gosaSupportDaemon();
      $tmp = $o->krb5_list_principals($server);
      $cache[$server] = $tmp;
      session::set("MIT_PRINCIPAL_CACHE",$cache);
    }
    return($cache[$server]);
  }


  /*! \brief get list of all configured policies
    for a given server. 
    The results will cached.  
   */
  public function load_policies_for_server($server)
  {
    if(!session::is_set("MIT_POLICY_CACHE")){
      session::set("MIT_POLICY_CACHE",array());
    }
    $cache = session::get("MIT_POLICY_CACHE");
    if(!isset($cache[$server])){
      $o = new gosaSupportDaemon();
      $tmp = $o->krb5_list_policies($server);
      $cache[$server] = $tmp;
      session::set("MIT_POLICY_CACHE",$cache);
    }
    return($cache[$server]);
  }




  /*! \brief Check if this password method is useable. 
    This is the case if there is a si server running and at least one server configured.
    kerberos support. 
   */ 
  function is_available()
  {
    $o = new gosaSupportDaemon(FALSE);
    if(count($this->server_list) && $o->connect()){
      return TRUE; 
    }
    return(FALSE);  
  }


  function generate_hash($pwd)
  {
    $mode= "kerberos";
    if (isset($this->config->current['KRBSASL']) && preg_match('/^true$/i', $this->config->current['KRBSASL'])){
      $mode= "sasl";
    }
    return "{".$mode."}".$this->attrs['uid'][0]."@".$this->goKrbRealm;
  }


  function remove_from_parent()
  {
    if(!empty($this->principal) && $this->goKrbRealm){
      $server = $this->map['REALM_SERVER'][$this->goKrbRealm];
      $o = new gosaSupportDaemon();
      if(!$o->krb5_del_principal($this->server_list[$server]['macAddress'],$this->principal)){
        msg_dialog::display(_("Service infrastructure"),msgPool::siError($o->get_error()),ERROR_DIALOG);      
      }
    }
  }


  function set_password($password)
  {
    if(!empty($this->principal) && $this->goKrbRealm){
      $server = $this->map['REALM_SERVER'][$this->goKrbRealm];
      $o = new gosaSupportDaemon();

      echo "Not implemented yet";
      $attrs = array("password" => $password);

#      if(!$o->krb5_set_principal($this->server_list[$server]['macAddress'],$this->principal,$attrs)){
#        msg_dialog::display(_("Service infrastructure"),msgPool::siError($o->get_error()),ERROR_DIALOG);      
#      }
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


    /* Cancel heimdal options */
    if (isset($_POST['pw_abort']) || $this->display == FALSE){
      $this->display = FALSE;
      return("");
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


    $smarty = get_smarty();
    $smarty->assign("years",$years);
    $smarty->assign("month",$month);
    $smarty->assign("days",$days);
    $smarty->assign("hours",$hours);
    $smarty->assign("minutes",$minutes);

    $smarty->assign("server_list",$this->server_list);
    $smarty->assign("POLICY"  ,$this->POLICY);
    $smarty->assign("goKrbRealm" , $this->goKrbRealm);

    $server_name = $this->map['REALM_SERVER'][$this->goKrbRealm];
    $server_mac  = $this->server_list[$server_name]['macAddress'];
    $this->POLICIES = $this->load_policies_for_server($server_mac);
    $smarty->assign("POLICIES"  ,$this->POLICIES);
   
    foreach($this->values as $attr){
      $smarty->assign($attr ,$this->$attr);
    }
    foreach($this->readonly as $attr){
      $smarty->assign($attr ,$this->$attr);
    }
    foreach($this->flags as $attr){
      $smarty->assign($attr,$this->$attr);
    }

    $date_values = array("PRINC_EXPIRE_TIME","PW_EXPIRATION");
    foreach($date_values as $date_val){
      $clear = $date_val."_clear";
      $smarty->assign($date_val."_clear",$this->$clear);
      $smarty->assign($date_val."_y",substr($this->$date_val,0,4));
      $smarty->assign($date_val."_m",substr($this->$date_val,4,2));
      $smarty->assign($date_val."_d",substr($this->$date_val,6,2));
      $smarty->assign($date_val."_h",substr($this->$date_val,8,2));
      $smarty->assign($date_val."_i",substr($this->$date_val,10,2));
    }


    return($smarty->fetch(get_template_path("pwd_kerberos_mit.tpl")));
  }


  function save_object()
  {
    if(isset($_POST['pwd_heimdal_posted'])){

      if(isset($_POST['goKrbRealm'])){
        $this->goKrbRealm = get_post("goKrbRealm");
      }

      foreach($this->flags as $attr){
        $this->$attr = isset($_POST[$attr]);
      }

      foreach(array("MAX_LIFE","MAX_RENEWABLE_LIFE","POLICY") as $attr){
        if(isset($_POST[$attr])){
          $this->$attr = get_post($attr);
        }
      }

      $date_values = array("PW_EXPIRATION","PRINC_EXPIRE_TIME");
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
    }
  }

  function check()
  {
    $message = array();
    echo "Checks missing";
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

      foreach($this->attributes as $attr){
        $data[$attr] = array();
      }

      echo "Save missing";

      print_a($data);
      exit();
    }
  }
}

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
