<?php
/*
  This code is part of GOsa (https://gosa.gonicus.de)
  Copyright (C) 2003  Cajus Pollmeier

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

class dhcpAdvanced extends plugin
{
  /* Used attributes */
  var $options= array();
  var $statements= array();
  var $show_advanced= FALSE;
  var $autoStatements= array();
  var $autoOptions= array();

  /* attribute list for save action */
  var $attributes= array();
  var $objectclasses= array();
  var $parent;

  static function getOption($needle, $heystack, $value_only = false){
    foreach($heystack as $key => $val){
      if($val != $key){
        echo "<b>$key => $val</b>";
      }

      if(preg_match("/^{$needle} /i",$val)){
        if($value_only){
          return(trim(preg_replace('/^[^\s]*/')));
        }else{
          return($val);
        }
      }
    }
    return("");
  }

  function dhcpAdvanced()
  {
    /* This is always an account */
    $this->is_account= TRUE;
    $this->setAutoStatements();
    $this->setAutoOptions();
  }

  function execute()
  {
	$acl_writeable = preg_match("/w/",$this->parent->getacl(""));

    /* Check for interaction */
    if ($acl_writeable && isset($_POST['add_statement']) && $_POST['addstatement'] != ""){
      $key= preg_replace('/^([a-z0-9-]+)\s(.*)$/', '\\1', get_post('addstatement'));
      $val= preg_replace("/^$key\s*/", '', get_post('addstatement'));
      $this->statements[$key]= $val;
    }
    if ($acl_writeable && isset($_POST['delete_statement']) && isset($_POST['dhcpstatements'])){
      $key= preg_replace('/([a-z0-9-]+)\s(.*)$/', '\\1', get_post('dhcpstatements'));
      if (in_array($key, $this->autoStatements)){
        msg_dialog::display(_("Error"), _("Cannot delete automatic statements!"), ERROR_DIALOG);
      } else {
        unset($this->statements[$key]);	
      }
    }
    if ($acl_writeable && isset($_POST['add_option']) && $_POST['addoption'] != ""){
      $val= trim(get_post('addoption'));
      $this->options[$val]= $val;
    }
    if ($acl_writeable && isset($_POST['delete_option']) && isset($_POST['dhcpoptions'])){
      $key= preg_replace('/([a-z0-9-]+)\s(.*)$/', '\\1', get_post('dhcpoptions'));
      $val= get_post('dhcpoptions');
      if (in_array($key, $this->autoOptions)){
        msg_dialog::display(_("Error"), _("Cannot delete automatic statements!"), ERROR_DIALOG);
      } else {
        unset($this->options[$val]);	
      }
    }

    $smarty= get_smarty();

	/* Assign ACLs */
	$smarty->assign("acl",$this->parent->getacl(""));

    /* Assign arrays */
    $statements= array();
    foreach ($this->statements as $key => $val){
      if (in_array($key, $this->autoStatements)){
        $statements[$key]= "$key $val ["._("automatic")."]";
      } else {
        $statements[$key]= "$key $val";
      }
    }
    $smarty->assign("dhcpstatements", $statements);
    $options= array();
    foreach ($this->options as $key => $val){
      $key = preg_replace("/ .*$/","",$val);
      if (in_array($key, $this->autoOptions)){
        $options[$val]= "$val ["._("automatic")."]";
      } else {
        $options[$val]= "$val";
      }
    }
    $smarty->assign("dhcpoptions", $options);

    /* Show main page */
    $smarty->assign("show_advanced", $this->show_advanced);
    return ($smarty->fetch (get_template_path('dhcp_advanced.tpl', TRUE,dirname(__FILE__))));
  }

  function remove_from_parent()
  {
  }


  /* Save data to object */
  function save_object()
  {
    if (isset($_POST['show_advanced'])){
      $this->show_advanced= TRUE;
    }
    if (isset($_POST['hide_advanced'])){
      $this->show_advanced= FALSE;
    }
  }


  /* Check values */
  function check()
  {
    /* Nothing to check here */
    $message= array();
    return $message;
  }


  /* Save to LDAP */
  function save()
  {
  }


  function setAutoOptions($addopt= array())
  {
    $options= array("routers", "domain-name", "domain-name-servers", "subnet-mask", "broadcast-address");
    $this->autoOptions= array_merge($options, $addopt);
  }


  function setAutoStatements($addstat= array())
  {
    $statements= array("filename", "next-server", "get-lease-hostnames", "use-host-decl-names");
    $this->autoStatements= array_merge($statements, $addstat);
  }

}

?>
