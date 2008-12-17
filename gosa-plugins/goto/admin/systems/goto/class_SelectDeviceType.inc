<?php

class SelectDeviceType extends plugin
{
  /* attribute list for save action */
  var $ignore_account= TRUE;
  var $attributes= array("ObjectGroup","SystemType");
  var $objectclasses= array("whatever");

  var $ObjectGroups   = array();
  var $SystemTypes       = array();

  var $ObjectGroup    = "";
  var $SystemType        = "";

  var $dn;

  function SelectDeviceType  (&$config, $dn= NULL)
  {
    if(!is_array($dn)){
      $this->dns = array($dn);
      plugin::plugin ($config, NULL);
    }else{
      $this->dns = $dn;
      plugin::plugin ($config, $dn);
    }

    /* Get object groups */
    $ldap = $this->config->get_ldap_link();
    $ldap->cd ($this->config->current['BASE']);
    $ldap->search("(&(objectClass=gosaGroupOfNames)(cn=*))",array("cn"));
    $tmp= array();
    while($attrs = $ldap->fetch()){
      $tmp[$attrs['dn']]= $attrs['cn'][0];
    }
    asort($tmp, SORT_LOCALE_STRING);
    $this->ObjectGroups= $tmp;

    $this->SystemTypes =array("workstation"=>_("Workstation"), "terminal"=>_("Terminal"), "server"=>_("Server"), "opsi_client"=>_("Windows workstation"));
  }

  function execute()
  {
    /* Call parent execute */
    plugin::execute();

    /* Fill templating stuff */
    $smarty= get_smarty();
    $display= "";

    $smarty->assign("dns_cnt" , count($this->dns));

    /* Add none to the list of object groups, if there is only one 
        object that currently edited. This will allow to modify attributes manually and 
        skip object group inherit.
     */
    $ogs = $this->ObjectGroups;
    if(count($this->dns) == 1){
      $tmpa['none']=_("none");
      $ogs = array_merge($tmpa,$ogs);
    }

    $smarty->assign("ObjectGroups"    ,$ogs);
    $smarty->assign("ObjectGroupKeys" ,array_flip($ogs));
    $smarty->assign("SystemTypes"     ,$this->SystemTypes);
    $smarty->assign("SystemTypeKeys"  ,array_flip($this->SystemTypes)); 

    $smarty->assign("ObjectGroup",$this->ObjectGroup);
    $smarty->assign("SystemType",$this->SystemType);

    $display.= $smarty->fetch(get_template_path('SelectDeviceType.tpl', TRUE,dirname(__FILE__)));
    return($display);
  }

  /* Save data to object */
  function save_object()
  {
    plugin::save_object();
    foreach($this->attributes as $attr){
      if(isset($_POST[$attr])){
        $this->$attr = $_POST[$attr];
      }
    }
  }

  /* Check supplied data */
  function check()
  {
    /* Call common method to give check the hook */
    $message= plugin::check();

    return ($message);
  }

  /* Save to LDAP */
  function save()
  {
  }
}

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
