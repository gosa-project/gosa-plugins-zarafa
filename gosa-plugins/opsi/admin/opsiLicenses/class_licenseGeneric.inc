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
  var $orig_cn = "";
  var $description = "";  
  var $partner = "";

  var $conclusionDate = "";
  var $expirationDate = "";
  var $notificationDate = "";

  var $licenseModel = "";
  var $licenseKey = array();
  var $orig_licenseModel = "";
  var $licensePoolId = "";
  var $boundToHost= array(); // Reserved for Host.   
  var $usedByHost = array(); // Used by Host.   

  var $maximumInstallations = 0;
  var $opsiHosts; 
  
  var $attributes = array(
        "cn","description","partner","conclusionDate","expirationDate",
        "notificationDate","licenseModel","licenseKey","maximumInstallations",
        "licensePoolId", "usedByHost","boundToHost");

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
    $this->boundToHost = array('0'=>"");
    $this->usedByHost = array('0'=>"");
    $this->licenseKey = array('0'=>"");
    if($this->initially_was_account){
      foreach($this->attributes as $attr){
        $this->$attr = $this->data[$attr];
      }
    }

    $this->orig_cn = $this->cn;
    $this->orig_licenseModel = $this->licenseModel;
    $this->init_successfull = TRUE;
    return;    
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
    $smarty->assign("boundToHost", $this->boundToHost[0]);
    $smarty->assign("licenseKey", $this->licenseKey[0]);
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

      if(isset($_POST['addLicenseUsage']) && isset($_POST['selectedHostToAdd'])){
        $host = get_post('selectedHostToAdd');
        if(!empty($host) && 
            in_array($host,$this->getHosts()) && 
            !in_array($host, $this->usedByHost)){
          $this->usedByHost[] = $host;
        }
      }

      if(isset($_POST['removeLicenseUsage']) && isset($_POST['selectedUsedHosts'])){
        $todel = $_POST['selectedUsedHosts'];
        foreach($todel as $host){
          if(isset($this->usedByHost[$host])){
            unset($this->usedByHost[$host]);
          }
        }
      }

      // Force licenseKey to be of type array.
      if(!is_array($this->licenseKey)){
        $this->licenseKey = array($this->licenseKey);
      }

      // BoundToHost maybe multiple too, later.
      if(!is_array($this->boundToHost)){
        $this->boundToHost = array($this->boundToHost);
      }    

      if($this->initially_was_account){
        $this->cn = $this->orig_cn;
        $this->licenseModel = $this->orig_licenseModel;
      }
    }
  }  


  /* Check user input and return a list of 'invalid input' messages.
   */
  function check()
  {
    $message = plugin::check();

    // Very simple date input checks
    if(!empty($this->expirationDate) && 
       !preg_match("/^[0-9]{4}\-[0-9]{2}\-[0-9]{2}$/",$this->expirationDate)){
      $message[] = msgPool::invalid(_("Expiration date"),$this->expirationDate,"","2009-02-23");
    }
    if(!empty($this->conclusionDate) && 
       !preg_match("/^[0-9]{4}\-[0-9]{2}\-[0-9]{2}$/",$this->conclusionDate)){
      $message[] = msgPool::invalid(_("Expiration date"),$this->conclusionDate,"","2009-02-23");
    }
    if(!empty($this->notificationDate) && 
       !preg_match("/^[0-9]{4}\-[0-9]{2}\-[0-9]{2}$/",$this->notificationDate)){
      $message[] = msgPool::invalid(_("Expiration date"),$this->notificationDate,"","2009-02-23");
    }

    if(empty($this->cn)){
      $message[] = msgPool::required(_("Name"));
    }

    if(empty($this->licenseKey[0])){
      $message[] = msgPool::required(_("License key"));
    }

    return($message);
  }
  
 
  /* Removes the object from the opsi database
   */ 
  function remove_from_parent() {}


  /* Saves object modifications
   */  
  function save()
  {
    $data = array();
    foreach($this->attributes as $target){
      $data[$target] = $this->$target;
    }
    return($data);
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
