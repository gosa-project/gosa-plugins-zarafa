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


class licenseUsageByHost extends plugin 
{

  var $cn = "";
  var $licenseUses = array();
  var $reservedLicenses = array();
  var $init_successfull = FALSE;

  var $availableLicenses = array();

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
      $this->cn = $this->orig_cn = preg_replace("/^opsi:=([^,]*),.*$/","\\1",$dn);
    }

    // Extract pool name out of the fake dn.
    $this->init();
  }

  
  function init()
  {
    $this->licenseUses = array();
    $this->reservedLicenses = array();
    $this->init_reservedLicenses = array();
    if(!$this->initially_was_account){
      $this->init_successfull = TRUE;
    }else{

      // Get license usage  
      $res = $this->si->getLicenseUsage($this->cn);
      if($this->si->is_error()){
        $this->init_successfull = FALSE;
        return;
      }
      $this->licenseUses = $res;

      // Get reservations
      $res = $this->si->getReservedLicensesForHost($this->cn);
      if($this->si->is_error()){
        $this->init_successfull = FALSE;
        return;
      }
      foreach($res as $pool){
        $this->reservedLicenses[] = $pool['licensePoolId'][0];
      }
      $this->init_reservedLicenses = $this->reservedLicenses;

      // Get list of license pools 
      $res = $this->si->listPools();
      if($this->si->is_error()){
        $this->init_successfull = FALSE;
        return;
      }
      $this->availableLicenses = array();
      foreach($res as $pool){
        $this->availableLicenses[] = $pool['cn'][0];
      }

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
    $list->setHeight(150);
    foreach($this->licenseUses as $license){
      $f3 = array("string" => $license['licenseKey'][0]);
      $f2 = array("string" => $license['licensePoolId'][0]);
      $f4 = array("string" => $license['softwareLicenseId'][0]);
      $list->addEntry(array($f2,$f3,$f4));
    } 

    // Create reserved list
    $list2 = new divSelectBox("reservedLicenses");
    $list2->setHeight(150);
    foreach($this->reservedLicenses as $key => $license){
      $action = "<input class='center' type='image' src='images/lists/trash.png' 
        name='removeReservation_{$key}'>";
      $f1 = array("string" => $license);
      $f4 = array("string" => $action,
                  "attach" => "style='border-right:0px; width:16px;'");
      $list2->addEntry(array($f1,$f4));
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
    $smarty->assign("licenseReserved", $list2->DrawList());
    $smarty->assign("init_successfull", $this->init_successfull);
    $smarty->assign("availableLicenses", array_diff($this->availableLicenses, $this->reservedLicenses));
    $smarty->assign("initially_was_account", $this->initially_was_account);
    return($smarty->fetch(get_template_path('licenseUsageByHost.tpl',TRUE,dirname(__FILE__))));
  }

 
  /* Save HTML inputs
   */
  function save_object()
  {
    if(isset($_POST['opsiLicenseUsagePosted'])){
      plugin::save_object();  
  
      // Check if we've to remove reservations 
      foreach($_POST as $name => $value){
        if(preg_match("/^removeReservation_/", $name)){
          $id = preg_replace("/^removeReservation_(.*)_.$/", "\\1" ,$name);
          if(isset($this->reservedLicenses[$id])) {
            unset($this->reservedLicenses[$id]);
          }
          break;
        }
      } 

      // Check if we've to add reservations
      if(isset($_POST['availableLicense']) && isset($_POST['addReservation'])){
        $id = get_post('availableLicense');
        if(isset($this->availableLicenses[$id])){
          $this->reservedLicenses[] =  $this->availableLicenses[$id];
        }
      }
    }
  }  


  /* Check user input and return a list of 'invalid input' messages.
   */
  function check()
  {
    $message = plugin::check();
    return($message);
  }
  

  function save()
  {
    $del = array_diff($this->init_reservedLicenses, $this->reservedLicenses);
    $add = array_diff($this->reservedLicenses, $this->init_reservedLicenses);

    foreach($del as $pool){
      $this->si->removeLicenseFromHost($pool, $this->cn);
      if($this->si->is_error()){
        msg_dialog::display(_("Error"),msgPool::siError($this->si->get_error()),ERROR_DIALOG);
      }
    }

    foreach($add as $pool){
      $this->si->addLicenseToHost($pool, $this->cn);
      if($this->si->is_error()){
        msg_dialog::display(_("Error"),msgPool::siError($this->si->get_error()),ERROR_DIALOG);
      }
    }
  }


  function remove_from_parent(){ }

 
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
