<?php
/*
* This code is part of GOsa (http://www.gosa-project.org)
* Copyright (C) 2003-2008 GONICUS GmbH
*
* ID: $$Id: class_roleManagement.inc 13520 2009-03-09 14:54:13Z hickert $$
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

class roleManagement extends plugin
{
  /* Definitions */
  var $plHeadline= "Roles";
  var $plDescription= "Manage roles";

  // Copy and paste handler 
  var $CopyPasteHandler = NULL;

  // The headpage list handler. 
  var $DivListRoles    = NULL;

  // A list of currently visible roles
  var $roles = array();

  // A list of currently edited/removed/aso roles.
  var $dns = array();

  // Permission modules to use.
  var $acl_module   = array("roles");  

  // Internal: Is truw while objects are pasted.
  var $start_pasting_copied_objects = FALSE;

  
  // Construct and initialize the plugin 
  function __construct (&$config, $dn= NULL)
  {
    // Include config object 
    $this->config= &$config;
    $this->ui= get_userinfo();

    // Copy & Paste enabled ?
    if ($this->config->get_cfg_value("copyPaste") == "true"){
      $this->CopyPasteHandler = new CopyPasteHandler($this->config);
    }

    // Initialize the corresponding list class.
    $this->DivListRoles = new divListRole($this->config,$this);
  }


  function execute()
  {
    // Call parent execute 
    plugin::execute();

    /* Variables to restore after 'entry locked' warning was displayed */
    session::set('LOCK_VARS_TO_USE',array('/^role_/'));

    $smarty     = get_smarty();
    $s_action   = "";
    $s_entry    = "";

    /***************
     * Handle _POST/_GET variables
     ***************/
   
    // Get entry related posts 
    foreach($_POST as $name => $value){
      if(preg_match("/^role_edit_/",$name)){
        $s_action = "edit";  
        $s_entry = preg_replace("/^role_edit_([0-9]*)_.*$/","\\1",$name);
        break;
      }
      if(preg_match("/^role_del_/",$name)){
        $s_action = "del";  
        $s_entry = preg_replace("/^role_del_([0-9]*)_.*$/","\\1",$name);
        break;
      }
    }
    if(isset($_GET['act']) && $_GET['act'] == "edit_entry" && isset($_GET['id'])){
      $id = $_GET['id'];
      if(isset($this->roles[$id])){
        $s_action = "edit";
        $s_entry = $id;
      }
    }
 
    // Get menu related posts 
    if(isset($_POST['menu_action'])) {
      if($_POST['menu_action'] == "role_new"){
        $s_action = "new";
      }elseif($_POST['menu_action'] == "remove_multiple_roles"){
        $s_action = "remove_multiple";
      }
    }


    /***************
     * New handling
     ***************/

    if($s_action == "new"){
      $this->dialog = new roletabs($this->config, $this->config->data['TABS']['ROLETABS'], "new");
    }
    
    /***************
     * Edit handling
     ***************/

    if($s_action == "edit"){
      if(!isset($this->roles[$s_entry])){
        trigger_error("Unknown entry!"); 
      }else{
        $entry = $this->roles[$s_entry];
        print_a($entry);
      }
    }

    /***************
     * Dialog handling
     ***************/

    if (isset($_POST['edit_cancel'])){
      if(isset($this->grouptab->dn)){
        $this->remove_lock();
      }
      $this->dialog= NULL;
      set_object_info();
    }

    if($this->dialog instanceOf tabs){
      $this->dialog->save_object();
      $display= $this->dialog->execute();

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
      return ($display);
    }


    /***************
     * List handling
     ***************/

    // Display dialog with group list 
    $this->DivListRoles->parent = $this;
    $this->DivListRoles->execute();

    // Add departments if subsearch is disabled 
    if(!$this->DivListRoles->SubSearch){
      $this->DivListRoles->AddDepartments($this->DivListRoles->selectedBase,4,1);
    }
    $this->reload ();
    $this->DivListRoles->setEntries($this->roles);
    return($this->DivListRoles->Draw());
  }


  // Refreshes the list of known role objects. 
  function reload()
  {

    // Get current ldap base and filter settings.
    $base     = $this->DivListRoles->selectedBase;
    $Regex    = $this->DivListRoles->Regex;

    // Search and fetch all matching role objects.
    $this->roles = array();
    $ldap = $this->config->get_ldap_link();
    $ldap->search("(objectClass=gosaGroupOfNames)",array("cn","description"));
    $tmp = array();
    while($attrs = $ldap->fetch()){
      $tmp[$attrs['cn'][0].$attrs['dn']] = $attrs;
    }
    
    uksort($tmp, 'strnatcasecmp');
    $this->roles = array_values($tmp);
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
  }


  /* Return departments, that will be included within snapshot detection 
   */
  function get_used_snapshot_bases()
  {
    return(array(get_ou('roleRDN').$this->DivListRoles->selectedBase));
  }

  
  function save_object()
  {
    $this->DivListRoles->save_object();
    if(is_object($this->CopyPasteHandler)){
      $this->CopyPasteHandler->save_object();
    }
  }
}

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
