<?php
/*
* This code is part of GOsa (http://www.gosa-project.org)
* Copyright (C) 2003-2008 GONICUS GmbH
*
* ID: $$Id: class_opsiLicenses.inc 13520 2009-03-09 14:54:13Z hickert $$
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


class licenseUsage extends plugin 
{

  var $cn = "";
  var $licenseUses = array();
  var $init_successfull = FALSE;


  function __construct(&$config,$dn)
  {
    $this->config = $config;
    $this->dn = $this->orig_dn = $dn;
    $this->si = new opsiLicenceHandler($this->config);

    $this->is_account=TRUE;
    if($this->dn == "new"){
      $this->initially_was_account = FALSE;
    }else{
      $this->initially_was_account = TRUE;
      $this->cn = $this->orig_cn = preg_replace("/^opsi:cn=([^,]*),.*$/","\\1",$dn);
    }

    // Extract pool name out of the fake dn.
    $this->init();
  }

  
  function init()
  {
    $this->licenseUses = array();
    if(!$this->initially_was_account){
      $this->init_successfull = TRUE;
    }else{

      $res = $this->si->getLicenseUsage("", $this->cn);
      if($this->si->is_error()){
        $this->init_successfull = FALSE;
        return;
      }

      $this->licenseUses = $res;
      $this->init_successfull = TRUE;
    }
  }


  function execute()
  {
    // Handle initialization failures.
    if(isset($_POST['retry_init'])) $this->init();
    if(!$this->init_successfull){
      $smarty = get_smarty();
      $smarty->assign("init_successfull", $this->init_successfull);
      return($smarty->fetch(get_template_path('licenseGeneric.tpl',TRUE,dirname(__FILE__))));
    }
    
    // Create usage list
    $list = new divSelectBox("licenseUsage");
    foreach($this->licenseUses as $license){

      $f1 = array("string" => $license['hostId'][0]);
      $f2 = array("string" => $license['licenseKey'][0]);
      $f3 = array("string" => $license['licensePoolId'][0]);
      $f4 = array("string" => $license['softwareLicenseId'][0],
                  "attach" => "style='border-right:0px;'");
      $list->addEntry(array($f1,$f2,$f3,$f4));
    } 

    $smarty = get_smarty();

    // Assign ACls 
    $plInfo = $this->plInfo();
    foreach($plInfo['plProvidedAcls'] as $name => $desc){
      $smarty->assign("{$name}ACL",$this->getacl($name));
    }
    foreach($this->attributes as $attr){
      $smarty->assign($attr,$this->$attr);
    }

    $smarty->assign("licenseUses", $list->DrawList());
    $smarty->assign("init_successfull", $this->init_successfull);
    $smarty->assign("initially_was_account", $this->initially_was_account);
    return($smarty->fetch(get_template_path('licenseUsage.tpl',TRUE,dirname(__FILE__))));
  }

 
  /* Save HTML inputs
   */
  function save_object()
  {
    if(isset($_POST['opsiLicenseUsagePosted'])){
      plugin::save_object();  
    }
  }  


  /* Check user input and return a list of 'invalid input' messages.
   */
  function check()
  {
    $message = plugin::check();
    return($message);
  }
  

  /* Saves object modifications
   */  
  function save()
  {
    plugin::save();

    // Send modify/add events
    $mode = "modify";
    if($this->orig_dn == "new"){
      $mode = "add";
    }

    $this->si->createPool($this->cn, $this->description,$this->productIds,$this->softwareIds);#
    if($this->si->is_error()){
      msg_dialog::display(_("Error"),msgPool::siError($this->si->get_error()),ERROR_DIALOG);
    }else{
      $this->handle_post_events($mode);
    }

    // Log action
    if($mode == "modify"){
      new log("modify","users/".get_class($this),$this->dn,array_keys($this->attrs),$this->si->get_error());
    }else{
      new log("create","users/".get_class($this),$this->dn,array_keys($this->attrs),$this->si->get_error());
    }

    return 0;
  }

 
  function remove_from_parent(){ return; }

 
  static function plInfo()
  {
    return (array(
          "plShortName"   => _("Usage"),
          "plDescription" => _("License usage"),
          "plSelfModify"  => FALSE,
          "plDepends"     => array(),
          "plPriority"    => 1,
          "plSection"     => array("administration"),
          "plCategory"    => array("opsi"),
          "plProvidedAcls"=> array(
            "cn"                => _("Name"),
            "description" => _("Description"))
          ));
  }
}


// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
