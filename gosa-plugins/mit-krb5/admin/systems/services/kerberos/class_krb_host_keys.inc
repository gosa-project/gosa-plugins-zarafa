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
  private $map  = array();

  private $namingAttr = "";

  public function __construct($config,$object)
  {
    plugin::plugin($config, NULL);
    $this->object = $object;
    $this->o_queue = new gosaSupportDaemon();
    $this->goKrbRealms =array("deactivated"=> _("Deactivated"));

    /* Check if the SI based kerberos method is available 
     */
    if(class_available("passwordMethodMIT")){
      $this->pwd_handle = new passwordMethodMIT($this->config);

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

        $principals = $this->pwd_handle->load_principals_for_server($attrs['macAddress'][0]);

        /* Create Realm/Server/Principal mapping.
         */
        foreach($principals as $principal){
          $this->map["PRINCIPAL_SERVER"][$principal] = $attrs['cn'][0];
          $this->map["PRINCIPAL_REALM"] [$principal] = $attrs['goKrbRealm'][0];
        }
        $this->map["MAC_SERVER"][$attrs['cn'][0]] = $attrs['macAddress'][0];
        $this->map["SERVER_REALM"][$attrs['cn'][0]] = $attrs['goKrbRealm'][0];
        $this->map["REALM_SERVER"][$attrs['goKrbRealm'][0]] = $attrs['cn'][0];

        $this->goKrbRealms[$attrs['goKrbRealm'][0]] = $attrs['goKrbRealm'][0];

        /* Set first realm as selected.
         */
        if($this->goKrbRealm == ""){
          $this->goKrbRealm = $attrs['goKrbRealm'][0];
        }

        /* Create Server list
         */
        $this->server_list[$attrs['cn'][0]] = array("macAddress" => $attrs['macAddress'][0],
            "description"=> $attrs['description'][0],
            "dn"         => $attrs['dn'],
            "principals" => $principals,
            "goKrbRealm" => $attrs['goKrbRealm'][0],
            "cn"         => $attrs['cn'][0]);
      }
  
      /* Everything seems to be OK, we have found at least one principal
       */
      if(isset($this->map['PRINCIPAL_SERVER']) && count($this->map['PRINCIPAL_SERVER'])){
        $this->kerberos_support = TRUE;
      }
    }
  }

  public function execute()
  {
    /* Skip if there is no kerberos support available 
     */
    if(!$this->kerberos_support) return("");
  
    $smarty = get_smarty(); 
    $smarty->assign("Realms",$this->goKrbRealms);
    $smarty->assign("goKrbRealm",$this->goKrbRealm);
    return($smarty->fetch (get_template_path('krb_host_keys.tpl', TRUE,dirname(__FILE__))));
  }


  private function generate_key()
  {

    /* Check if the source object is useable, we require the cn to be set 
     *  If this is not the case, do not display this plugin.
     */
    if(!isset($this->object->cn)){
      echo "Verdammt";
    }else{
      $name  = "host/".$this->object->cn;
      $realm = $this->goKrbRealm;
      $server= $this->map['REALM_SERVER'][$realm];
      $mac   = $this->map['MAC_SERVER'][$server];
      $this->o_queue ->krb5_add_principal($mac,$name,array());
    }
  }


  public function save_object()
  {
    /* Skip if there is no kerberos support available 
     */
    if(!$this->kerberos_support) return;

    if(isset($_POST['goKrbRealm']) && isset($this->goKrbRealms[$_POST['goKrbRealm']])){
      $this->goKrbRealm = get_post('goKrbRealm');
    }
    if(isset($_POST['host_key_generate'])){
      $this->generate_key();
    }
  } 
}











// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
