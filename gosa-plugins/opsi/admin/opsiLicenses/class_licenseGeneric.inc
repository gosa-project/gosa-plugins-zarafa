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


class licenseGeneric extends plugin 
{

  var $si = NULL;
  var $data = array();
  var $cn = "";
  var $description = "";  
  var $partner = "";

  var $conclusionDate = "";
  var $expirationDate = "";
  var $notificationDate = "";

  var $licenseModel = "";
  var $licenseKey = "";
  var $licensePoolId = "";
  var $boundToHost= array(); // Reserved for Host.   
  var $usedByHost = array(); // Used by Host.   

  var $maximumInstallations = 0;
  var $opsiHosts; 
  
  var $attributes = array("cn","description","partner","conclusionDate","expirationDate",
        "notificationDate","licenseModel","licenseKey","maximumInstallations","licensePoolId");

  function __construct(&$config, $license, $hosts = array())
  {
    $this->config = $config;
    $this->data = $license;
    $this->si = new opsiLicenceHandler($this->config);
    $this->opsiHosts = $hosts;

    // Detect account state.
    if(count($this->data) == 0){
      $this->initially_was_account = FALSE;
    }else{
      $this->initially_was_account = TRUE;
    }
    $this->is_account = TRUE;

    // Extract pool name out of the fake dn.
    $this->init();
  }

  
  function init()
  {

    $map = array(
        "BOUNDTOHOST"=> "boundToHost",
        "EXPIRATIONDATE"=> "expirationDate",
        "LICENSECONTRACTID"=> "contractId",
        "LICENSEPOOIDS"=> "licensePoolId",
        "LICENSETYPE"=> "licenseModel",
        "MAXINSTALLATIONS"=> "maximumInstallations",
        "SOFTWARELICENSEID"=> "cn");

    foreach($map as $source => $target){
      if(isset($this->data[$source][0]['VALUE'])){
        $this->$target = $this->data[$source][0]['VALUE'];
      }
    }

    // Get license key
    if(isset($this->data['LICENSEKEYS'][0][strtoupper($this->licensePoolId)][0]['VALUE'])){
      $this->licenseKey = $this->data['LICENSEKEYS'][0][strtoupper($this->licensePoolId)][0]['VALUE'];
    }

    // Get license usage
    $this->usedByHost = array();
    if(isset($this->data['HOSTIDS'])){
      foreach($this->data['HOSTIDS'] as $host){
        $this->usedByHost[] = $host['VALUE']; 
      }
    }

    $this->init_successfull = TRUE;
    return;    
  }


  function execute()
  {
    $this->init();

    // Handle initialization failures.
    if(isset($_POST['retry_init'])) $this->init();
    if(!$this->init_successfull){
      $smarty = get_smarty();
      $smarty->assign("init_successfull", $this->init_successfull);
      return($smarty->fetch(get_template_path('licenseGeneric.tpl',TRUE,dirname(__FILE__))));
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

    $smarty->assign("licenseModels",array(
          "RETAIL" => _("Retail"),
          "OEM"=>_("OEM"),
          "VOLUME" => _("Volume"),
          "CONCURRENT" => _("Concurrent")));


    $smarty->assign("init_successfull", $this->init_successfull);
    $smarty->assign("initially_was_account", $this->initially_was_account);
    $smarty->assign("hosts", $this->getHosts());

    $smarty->assign("notUsedHosts", array_diff($this->getHosts(), $this->usedByHost));
    $smarty->assign("usedByHost", $this->usedByHost);
    $smarty->assign("boundToHost", $this->boundToHost);
    return($smarty->fetch(get_template_path('licenseGeneric.tpl',TRUE,dirname(__FILE__))));
  }


  function getHosts()
  {
    $ret = array();
    foreach($this->opsiHosts as $host){
      $cn = $host['NAME'][0]['VALUE'];
      $ret[$cn] = $cn;
    }
    return($ret);
  }


 
  /* Save HTML inputs
   */
  function save_object()
  {
    if(isset($_POST['opsiLicensesPosted'])){
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
  

 
  /* Removes the object from the opsi database
   */ 
  function remove_from_parent()
  {
  echo "missing remove.";
#   $this->si->deletePool($this->orig_cn);
#   if($this->si->is_error()){
#     msg_dialog::display(_("Error"),msgPool::siError($this->si->get_error()),ERROR_DIALOG);
#   }else{
#
#     // Trigger remove signal
#     $this->handle_post_events("remove");
#   }
#
#   new log("remove","users/".get_class($this),$this->dn,array_keys($this->attrs),$this->si->get_error());
  }


  /* Saves object modifications
   */  
  function save()
  {

    echo "missing save";
#   plugin::save();
#
#   // Send modify/add events
#   $mode = "modify";
#   if($this->orig_dn == "new"){
#     $mode = "add";
#   }
#
#   $this->si->createPool($this->cn, $this->description,$this->productIds,$this->softwareIds);#
#   if($this->si->is_error()){
#     msg_dialog::display(_("Error"),msgPool::siError($this->si->get_error()),ERROR_DIALOG);
#   }else{
#     $this->handle_post_events($mode);
#   }
#
#   // Log action
#   if($mode == "modify"){
#     new log("modify","users/".get_class($this),$this->dn,array_keys($this->attrs),$this->si->get_error());
#   }else{
#     new log("create","users/".get_class($this),$this->dn,array_keys($this->attrs),$this->si->get_error());
#   }
#
#   return 0;
  }
 
  static function plInfo()
  {
    return (array(
          "plShortName"   => _("Generic"),
          "plDescription" => _("License generic"),
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
