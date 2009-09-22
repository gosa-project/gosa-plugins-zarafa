<?php
/*
* This code is part of GOsa (http://www.gosa-project.org)
* Copyright (C) 2003-2008 GONICUS GmbH
*
* ID: $$Id: class_licenseManagement.inc 13520 2009-03-09 14:54:13Z hickert $$
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

class opsiLicenses extends plugin
{
  /* Definitions */
  var $plHeadline= "Opsi";
  var $plDescription= "Opsi licenses";

  // The headpage list handler. 
  var $DivListLicenses    = NULL;

  // A list of currently visible licenses
  var $licenses = array();

  // A list of currently edited/removed/aso licenses.
  var $dns = array();

  // Permission modules to use.
  var $acl_module   = array("opsi");  

  // Construct and initialize the plugin 
  function __construct (&$config, $dn= NULL)
  {
    // Include config object 
    $this->config= &$config;
    $this->ui= get_userinfo();

    // Initialize the corresponding list class.
    $this->DivListLicenses = new divListLicense($this->config,$this);
  }


  function execute()
  {
    // Call parent execute 
    plugin::execute();

    // Variables to restore after 'entry locked' warning was displayed 
    session::set('LOCK_VARS_TO_USE',array('/^license_/','/^act/',
          '/^id/','/^menu_action/','/^item/'));

    $smarty     = get_smarty();

    /***************
     * Handle _POST/_GET variables
     ***************/
   
    // Get entry related posts 
    $s_action   = "";
    $s_entry    = "";
    foreach($_POST as $name => $value){
      if(preg_match("/^license_edit_/",$name)){
        $s_action = "edit";  
        $s_entry = preg_replace("/^license_edit_([0-9]*)_.*$/","\\1",$name);
        break;
      }
      if(preg_match("/^license_del_/",$name)){
        $s_action = "remove";  
        $s_entry = preg_replace("/^license_del_([0-9]*)_.*$/","\\1",$name);
        break;
      }
    }

    if(isset($_GET['act']) && $_GET['act'] == "edit_entry" && isset($_GET['id'])){
      $id = $_GET['id'];
      if(isset($this->licenses[$id])){
        $s_action = "edit";
        $s_entry = $id;
      }
    }
 
    // Get menu related posts 
    if(isset($_POST['menu_action'])) {
      if($_POST['menu_action'] == "remove_multiple_licenses"){
        $s_action = "remove_multiple";
      }
    }



    /***************
     * Remove handling
     ***************/

    if($s_action == "remove_multiple" || $s_action == "remove"){
    
      if($s_action == "remove_multiple"){
        $ids = $this->list_get_selected_items();
      }else{
        $ids = array($s_entry);
      }

      if(count($ids)){
        $this->dns = array();
        $disallowed = array();
        foreach($ids as $id){
          $dn = $this->licenses[$id]['dn'];
          $acl = $this->ui->get_permissions($dn, "opsi/licenseGeneric");
          if(preg_match("/d/",$acl)){
            $this->dns[$id] = $dn;
          }else{
            $disallowed[] = $dn;
          }
        }

        if(count($disallowed)){
          msg_dialog::display(_("Permission"),msgPool::permDelete($disallowed),INFO_DIALOG);
        }

        if(count($this->dns)){
          $smarty->assign("info", msgPool::deleteInfo($dns_names,_("License")));
          $smarty->assign("multiple", true);
          return($smarty->fetch(get_template_path('remove.tpl', TRUE)));
        }
      }
    } 


    /* Remove lock */
    if(isset($_POST['delete_multiple_licenses_cancel'])){

      /* Remove lock file after successfull deletion */
      $this->remove_lock();
      $this->dns = array();
    }


    /* Confirmation for deletion has been passed. Users should be deleted. */
    if (isset($_POST['delete_multiple_licenses_confirm'])){

      /* Remove user by user and check acls before removeing them */
      foreach($this->dns as $key => $dn){

        $acl = $this->ui->get_permissions($dn, "opsi/licenseGeneric");
        if (preg_match('/d/', $acl)){

          /* Delete request is permitted, perform LDAP action */
          $this->dialog= new licensetabs($this->config,$this->config->data['TABS']['LICENSETABS'], $dn);
          $this->dialog->delete();
          $this->dialog= NULL;
        } else {

          /* Normally this shouldn't be reached, send some extra
             logs to notify the administrator */
          msg_dialog::display(_("Permission error"), msgPool::permDelete(), INFO_DIALOG);
          new log("security","opsi/".get_class($this),$dn,array(),"Tried to trick deletion.");
        }
      }

      /* Remove lock file after successfull deletion */
      $this->remove_lock();
      $this->dns = array();
    }


    /***************
     * New handling
     ***************/

    if($s_action == "new" && !$this->dialog instanceOf tabs){
      $this->dialog = new licensetabs($this->config, $this->config->data['TABS']['LICENSETABS'], "new");
      $this->dialog->set_acl_base($this->config->current['BASE']);
    }
    
    /***************
     * Edit handling
     ***************/

    if($s_action == "edit" && !$this->dialog instanceOf tabs){
      if(!isset($this->licenses[$s_entry])){
        trigger_error("Unknown entry!"); 
      }else{

        $entry = $this->licenses[$s_entry];
        $this->dn = $entry['dn'];

        /* Open the dialog */
        $this->dialog = new licensetabs($this->config, $this->config->data['TABS']['LICENSETABS'], 
            $entry['dn'], "opsi");
        $this->dialog->set_acl_base($this->config->current['BASE']);
        set_object_info($this->dn);
      }
    }


    /***************
     * Dialog handling
     ***************/

    if ((isset($_POST['edit_finish']) || isset($_POST['edit_apply'])) && $this->dialog instanceOf tabs){
      $this->dialog->save_object();
      $msgs = $this->dialog->check();
      if(count($msgs)){
        msg_dialog::displayChecks($msgs);
      }else{
        $this->dialog->save();
        if (!isset($_POST['edit_apply'])){
          $this->remove_lock();
          $this->dialog= NULL;
          set_object_info();
        }else{
          $this->dialog->re_init();
        }
      }
    }

    if (isset($_POST['edit_cancel']) && $this->dialog instanceOf tabs){
      $this->remove_lock();
      $this->dialog= NULL;
      set_object_info();
    }

    if($this->dialog instanceOf tabs){
      $display= $this->dialog->execute();

      $dialog_opened = ($this->dialog->by_object[$this->dialog->current]->dialog instanceOf plugin);

      if(!$dialog_opened){
        if($this->dialog->read_only   == TRUE){
          $display.= "<p style=\"text-align:right\">
            <input type=submit name=\"edit_cancel\" value=\"".msgPool::cancelButton()."\">
            </p>";
        }else{

          $display.= "<p style=\"text-align:right\">\n";
          $display.= "<input type=submit name=\"edit_finish\" style=\"width:80px\" 
            value=\"".msgPool::okButton(). "\">\n";
          $display.= "&nbsp;\n";
          if ($this->dn != "new"){
            $display.= "<input type=submit name=\"edit_apply\" 
              value=\"".msgPool::applyButton()."\">\n";
            $display.= "&nbsp;\n";
          }
          $display.= "<input type=submit name=\"edit_cancel\" 
            value=\"".msgPool::cancelButton()."\">\n";
          $display.= "</p>";
        }
      }
      return ($display);
    }


    /***************
     * List handling
     ***************/

    // Display dialog with group list 
    $this->DivListLicenses->parent = $this;
    $this->DivListLicenses->execute();
    $this->reload ();
    $this->DivListLicenses->setEntries($this->licenses);
    return($this->DivListLicenses->Draw());
  }


  // Refreshes the list of known license objects. 
  function reload()
  {

    // Get current ldap base and filter settings.
    $base     = $this->DivListLicenses->selectedBase;
    $Regex    = $this->DivListLicenses->Regex;

    // Search and fetch all matching license objects.
    $this->licenses = array();
  }


  /* \brief  Returns a list of selected entry ids.
   *         E.g. remove multiple entries.
   * @return Array  A list of entry IDs
   */
  function list_get_selected_items()
  {
    $ids = array();
    foreach($_POST as $name => $value){
      if(preg_match("/^item_selected_[0-9]*$/",$name)){
        $id   = preg_replace("/^item_selected_/","",$name);
        $ids[$id] = $id;
      }
    }
    return($ids);
  }


  function remove_lock()
  {
    if (isset($this->dialog->dn)){
      del_lock ($this->dialog->dn);
    }elseif(isset($this->dn) && !empty($this->dn) && $this->dn != "new"){
      del_lock($this->dn);
    }
    if(isset($this->dns) && is_array($this->dns) && count($this->dns)){
      del_lock($this->dns);
    }
  }

 
  function save_object()
  {
    $this->DivListLicenses->save_object();
  }
}

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
