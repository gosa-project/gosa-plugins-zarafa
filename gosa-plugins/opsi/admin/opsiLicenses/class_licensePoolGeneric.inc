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


class licensePoolGeneric extends plugin 
{

  var $cn = "";
  var $orig_cn = "";
  var $description = "";
  var $orig_dn    = "";

  var $data       = array();
  var $orig_data  = array();
  var $productIds = array();
  var $softwareIds= array();
  var $licenses   = array();

  var $availableProductIds = array();
  var $attributes =array("cn","description");
  var $si = NULL;

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
    // Load local Boot Products 
    $res = $this->si->get_local_products();
    if($this->si->is_error()){
      $this->init_successfull = FALSE;
      return(FALSE);
    }
    $this->availableProductIds=array_keys($res);

    // Load Pool
    if(!$this->initially_was_account){
      $this->init_successfull = TRUE;
    }else{

      $res = $this->si->getPool($this->cn);
      if($this->si->is_error()){    
        $this->init_successfull = FALSE;
        return(FALSE);
      }else{
        $this->data = $this->orig_data = $res;
        $this->description = $this->data['description'][0];

        // Load software IDs 
        $this->softwareIds = array();
        if(isset($this->data['softwareId'])){
          for($i = 0; $i < $this->data['softwareId']['count']; $i++){
            $this->softwareIds[] = $this->data['softwareId'][$i];
          }
        }

        // Load product IDs 
        $this->productIds = array();
        if(isset($this->data['productId'])){
          for($i = 0; $i < $this->data['productId']['count']; $i++){
            $this->productIds[] = $this->data['productId'][$i];
          }
        }

        // Load Licences 
        $this->licenses = array();
        if(isset($this->data['licenses'])){
          $this->licenses = $this->data['licenses'];
        }
        $this->init_successfull = TRUE;
        return;
      }
    }
  }


  function execute()
  {
    // Handle initialization failures.
    if(isset($_POST['retry_init'])) $this->init();
    if(!$this->init_successfull){
      $smarty = get_smarty();
      $smarty->assign("init_successfull", $this->init_successfull);
      return($smarty->fetch(get_template_path('licensePoolGeneric.tpl',TRUE,dirname(__FILE__))));
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
    $smarty->assign("init_successfull", $this->init_successfull);
    $smarty->assign("availableProductIds", array_diff( $this->availableProductIds, $this->productIds));
    $smarty->assign("productIds", $this->productIds);
    $smarty->assign("softwareIds", $this->softwareIds);
    $smarty->assign("licenses", $this->getLicenseList());
    $smarty->assign("initially_was_account", $this->initially_was_account);
    return($smarty->fetch(get_template_path('licensePoolGeneric.tpl',TRUE,dirname(__FILE__))));
  }


  function getLicenseList()
  {
    $list = new divSelectBox("test");
    $list->setHeight(100);
    for($i=0 ; $i<$this->licenses['count'] ; $i++){
    
      $license = $this->licenses[$i];

      $keys = "";
      foreach($license['LICENSEKEYS'] as $key_by_pool){
        foreach($key_by_pool as $key){
          $keys .= $key[0]['VALUE'].", ";
        }
      }

      $link = "<input type='image' class='center' src='images/lists/edit.png' name='editLicense_{$i}'>";
      $link.= "<input type='image' class='center' src='images/lists/trash.png' name='removeLicense_{$i}'>";

      $f1 = array("string" => $license['SOFTWARELICENSEID'][0]['VALUE'], "attach" => "");
      $f2 = array("string" => $license['LICENSETYPE'][0]['VALUE'], "attach" => "");
      $f3 = array("string" => $license['EXPIRATIONDATE'][0]['VALUE'], "attach" => "");
      $f4 = array("string" => $license['MAXINSTALLATIONS'][0]['VALUE'], "attach" => "");
      $f5 = array("string" => rtrim($keys,", "));
      $f6 = array("string" => $link, "attach" => "style='border-right: 0px; width:32px;'");

      $list->addEntry(array($f1,$f2,$f3,$f4,$f5,$f6));
    }
    return($list->DrawList());
  }
 
 
  /* Save HTML inputs
   */
  function save_object()
  {
    if(isset($_POST['opsiLicensesPosted'])){
      plugin::save_object();  

      // Restore license cn, to avoid creating a copy...
      if($this->initially_was_account) $this->cn = $this->orig_cn;

      // We've to add prodcuts here 
      if(isset($_POST['availableProduct']) && isset($_POST['addProduct'])){
        $pro = get_post('availableProduct');
        if(isset($this->availableProductIds[$pro]) && !in_array($this->availableProductIds[$pro], $this->productIds)){
          $this->productIds[] =$this->availableProductIds[$pro];
        }
      }

      // We've to remove products here
      if(isset($_POST['productIds']) && isset($_POST['removeProduct'])){
        foreach($_POST['productIds'] as $key){
          if(isset($this->productIds[$key])){
            unset($this->productIds[$key]);
          }
        }
      }

      // We've to add software here 
      if(isset($_POST['newSoftwareId']) && isset($_POST['addSoftware'])){
        $soft = trim(get_post('newSoftwareId'));
        if(!empty($soft) && !in_array($soft, $this->softwareIds)){
          $this->softwareIds[] = $soft;
        }
      }

      // We've to remove software Ids here
      if(isset($_POST['softwareIds']) && isset($_POST['removeSoftware'])){
        foreach($_POST['softwareIds'] as $key){
          if(isset($this->softwareIds[$key])){
            unset($this->softwareIds[$key]);
          }
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
  

 
  /* Removes the object from the opsi database
   */ 
  function remove_from_parent()
  {
    $this->si->deletePool($this->orig_cn);
    if($this->si->is_error()){
      msg_dialog::display(_("Error"),msgPool::siError($this->si->get_error()),ERROR_DIALOG);
    }else{

      // Trigger remove signal
      $this->handle_post_events("remove");
    }

    new log("remove","users/".get_class($this),$this->dn,array_keys($this->attrs),$this->si->get_error());
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
