<?php
/*
 * This code is part of GOsa (http://www.gosa-project.org)
 * Copyright (C) 2003-2008 GONICUS GmbH
 *
 * ID: $$Id: class_gosaSupportDaemon.inc 10788 2008-05-06 11:15:57Z hickert $$
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */


class krb_host_keys extends plugin 
{

  public  $config;
  private $object;
  private $o_queue;
  private $goKrbRealm = "disabled";
  private $kerberos_support= FALSE;

  private $pwd_handle = NULL;
  private $server_list;

  private $init_namingAttr  = "";
  private $namingAttr       = "";
  private $is_service_key   = FALSE;

  private $prefix           = "host/";

  public function __construct($config,$object,$prefix = "host/")
  {
    plugin::plugin($config, NULL);
    $this->object = $object;
    $this->prefix = $prefix;
    $this->o_queue = new gosaSupportDaemon();

    /*  Create principal name, to detect whether there is a valid host key or not
     */
    $this->init_namingAttr = $this->namingAttr = $this->prefix.$this->object->cn;
    $this->init();
  }


  private function init()
  {
    /* Check if the SI based kerberos method is available 
     */
    $this->kerberos_support = FALSE;
    if(class_available("passwordMethodMIT")){

      $this->namingAttr = $this->prefix.$this->object->cn;

      /* Create password handle, it has already a working principal parsing 
       */
      $this->pwd_handle = new passwordMethodMIT($this->config);
      $this->pwd_handle->clear_cache();

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

        $mac        = $attrs['macAddress'][0];
        $realm      = $attrs['goKrbRealm'][0];
        $cn         = $attrs['cn'][0];
        $principals = $this->pwd_handle->load_principals_for_server($mac);
    
        $princ = FALSE;
        foreach($principals as $name){
          if(preg_match("/^".normalizePreg($this->namingAttr)."\@/i",$name)){
            $princ = TRUE;
            break;
          }
        }
    
        $this->server_list[$mac]['CN']    = $cn;
        $this->server_list[$mac]['MAC']   = $mac;
        $this->server_list[$mac]['REALM'] = $realm;
        $this->server_list[$mac]['PRINCIPAL'] = $princ;

        /* Set first realm as selected.
         */
        if($this->goKrbRealm == ""){
          $this->goKrbRealm = $attrs['goKrbRealm'][0];
        }
      }
  
      if(isset($this->server_list) && count($this->server_list)){
        $this->kerberos_support = TRUE;
      }
    }
  }


  public function execute()
  {
    /* Skip if there is no kerberos support available 
     */
    echo $this->namingAttr."<br>";

    if(!$this->kerberos_support) return("");
    $smarty = get_smarty(); 
    $smarty->assign("server_list",$this->server_list);
    $smarty->assign("is_service_key" , $this->is_service_key);
    return($smarty->fetch (get_template_path('krb_host_keys.tpl', TRUE,dirname(__FILE__))));
  }


  public function create_key($id,$name)
  {
    $this->o_queue->krb5_add_principal($id,$name,array());
    if($this->o_queue->is_error()){
      msg_dialog::display(_("Service infrastructure"),msgPool::siError($this->o_queue->get_error()),ERROR_DIALOG);
    }else{
      $this->recreate_key($id,$name);
    }
  }  


  public function recreate_key($id,$name)
  {
    $this->o_queue->krb5_ramdomize_key($id,  $name);
    if($this->o_queue->is_error()){
      msg_dialog::display(_("Service infrastructure"),msgPool::siError($this->o_queue->get_error()),ERROR_DIALOG);
    }
  }  


  public function delete_key($id,$name)
  {
    $this->o_queue->krb5_del_principal($id,  $name);
    if($this->o_queue->is_error()){
      msg_dialog::display(_("Service infrastructure"),msgPool::siError($this->o_queue->get_error()),ERROR_DIALOG);
    }
  }  


  public function save_object()
  {

    foreach($_POST as $name => $value){
      if(preg_match("/^create_/",$name)){
        $id = preg_replace("/^create_([a-z0-9\:]*)_.*/i","\\1",$name);
        $realm = $this->server_list[$id]['REALM'];
        $this->create_key($id,$this->namingAttr."@".$realm);
        $this->init();
        break;
      }

      /* Recreate host key */
      if(preg_match("/^recreate_/",$name)){
        $id = preg_replace("/^recreate_([a-z0-9\:]*)_.*/i","\\1",$name);
        $realm = $this->server_list[$id]['REALM'];
        $this->recreate_key($id,$this->namingAttr."@".$realm);
        $this->init();
        break;
      }

      /* Remove a host principal */
      if(preg_match("/^remove_/",$name)){
        $id = preg_replace("/^remove_([a-z0-9\:]*)_.*/i","\\1",$name);
        $realm = $this->server_list[$id]['REALM'];
        $this->delete_key($id,$this->namingAttr."@".$realm);
        $this->init();
        break;
      }
    }  
  } 


  public function ServiceKey($bool = TRUE)
  {
    $this->is_service_key = $bool;
  }
}











// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
