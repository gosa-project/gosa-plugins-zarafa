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

class opsiLicences extends plugin
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

    // Variables to restore after 'entry locked' warning was displayed 
    session::set('LOCK_VARS_TO_USE',array('/^copy/','/^cut/','/^role_/','/^act/',
          '/^id/','/^menu_action/','/^item/'));

    $smarty     = get_smarty();

    /***************
     * Handle _POST/_GET variables
     ***************/
   
    // Get entry related posts 
    $s_action   = "";
    $s_entry    = "";
    foreach($_POST as $name => $value){
      if(preg_match("/^cut_/",$name)){
        $s_action = "cut";  
        $s_entry = preg_replace("/^cut_([0-9]*)_.*$/","\\1",$name);
        break;
      }
      if(preg_match("/^copy_/",$name)){
        $s_action = "copy";  
        $s_entry = preg_replace("/^copy_([0-9]*)_.*$/","\\1",$name);
        break;
      }
      if(preg_match("/^role_edit_/",$name)){
        $s_action = "edit";  
        $s_entry = preg_replace("/^role_edit_([0-9]*)_.*$/","\\1",$name);
        break;
      }
      if(preg_match("/^role_del_/",$name)){
        $s_action = "remove";  
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
      if($_POST['menu_action'] == "editPaste"){
        $s_action = "editPaste";
      }elseif($_POST['menu_action'] == "role_new"){
        $s_action = "new";
      }elseif($_POST['menu_action'] == "remove_multiple_roles"){
        $s_action = "remove_multiple";
      }
    }

    /***************
     * Copy & Paste handling
     ***************/

    /* Display the copy & paste dialog, if it is currently open */
    $ret = $this->copyPasteHandling_from_queue($s_action,$s_entry);
    if($ret){
      return($ret);
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
          $dn = $this->roles[$id]['dn'];
          $acl = $this->ui->get_permissions($dn, "roles/roleGeneric");
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
          if ($user= get_multiple_locks($this->dns)){
            return(gen_locked_message($user,$this->dns));
          }
          $dns_names = array();
          foreach($this->dns as $dn){
            $dns_names[] = LDAP::fix($dn);
          }

          /* Lock the current entry, so nobody will edit it during deletion */
          add_lock ($this->dns, $this->ui->dn);

          $smarty->assign("info", msgPool::deleteInfo($dns_names,_("Role")));
          $smarty->assign("multiple", true);
          return($smarty->fetch(get_template_path('remove.tpl', TRUE)));
        }
      }
    } 


    /* Remove lock */
    if(isset($_POST['delete_multiple_roles_cancel'])){

      /* Remove lock file after successfull deletion */
      $this->remove_lock();
      $this->dns = array();
    }


    /* Confirmation for deletion has been passed. Users should be deleted. */
    if (isset($_POST['delete_multiple_roles_confirm'])){

      /* Remove user by user and check acls before removeing them */
      foreach($this->dns as $key => $dn){

        $acl = $this->ui->get_permissions($dn, "roles/roleGeneric");
        if (preg_match('/d/', $acl)){

          /* Delete request is permitted, perform LDAP action */
          $this->dialog= new roletabs($this->config,$this->config->data['TABS']['ROLETABS'], $dn);
          $this->dialog->delete();
          $this->dialog= NULL;
        } else {

          /* Normally this shouldn't be reached, send some extra
             logs to notify the administrator */
          msg_dialog::display(_("Permission error"), msgPool::permDelete(), INFO_DIALOG);
          new log("security","roles/".get_class($this),$dn,array(),"Tried to trick deletion.");
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
      $this->dialog = new roletabs($this->config, $this->config->data['TABS']['ROLETABS'], "new");
      $this->dialog->set_acl_base($this->DivListRoles->selectedBase);
    }
    
    /***************
     * Edit handling
     ***************/

    if($s_action == "edit" && !$this->dialog instanceOf tabs){
      if(!isset($this->roles[$s_entry])){
        trigger_error("Unknown entry!"); 
      }else{

        $entry = $this->roles[$s_entry];
        $this->dn = $entry['dn'];

        /* Check locking, save current plugin in 'back_plugin', so
           the dialog knows where to return. */
        if (($user= get_lock($this->dn)) != ""){
          return(gen_locked_message ($user, $this->dn,TRUE));
        }

        /* Lock the current entry, so everyone will get the above dialog */
        add_lock ($this->dn, $this->ui->dn);

        /* Open the dialog */
        $this->dialog = new roletabs($this->config, $this->config->data['TABS']['ROLETABS'], 
            $entry['dn'], "roles");
        $this->dialog->set_acl_base($this->dn);
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

    // Check if there is a snapshot dialog open 
    $base = $this->DivListRoles->selectedBase;
    if($str = $this->showSnapshotDialog($base,$this->get_used_snapshot_bases(),$this)){
      return($str);
    }

    // Display dialog with group list 
    $this->DivListRoles->parent = $this;
    $this->DivListRoles->execute();

    // Add departments if subsearch is disabled 
    if(!$this->DivListRoles->SubSearch){
      $this->DivListRoles->AddDepartments($this->DivListRoles->selectedBase,3,1);
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
    $filter= "(&(objectClass=organizationalRole)(cn=$Regex))";
    $attrs = array("cn","description","objectClass");

    if($this->DivListRoles->SubSearch){
      $res= get_sub_list($filter, "roles",array(), $base, $attrs, GL_SIZELIMIT | GL_SUBSEARCH);
    }else{
      $res= get_sub_list($filter, "roles",get_ou('roleRDN'), get_ou('roleRDN').$base, $attrs, GL_SIZELIMIT );
    }

    $tmp = array();
    foreach($res as $attrs){
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
    if (isset($this->dialog->dn)){
      del_lock ($this->dialog->dn);
    }elseif(isset($this->dn) && !empty($this->dn) && $this->dn != "new"){
      del_lock($this->dn);
    }
    if(isset($this->dns) && is_array($this->dns) && count($this->dns)){
      del_lock($this->dns);
    }
  }


  /* Return departments, that will be included within snapshot detection 
   */
  function get_used_snapshot_bases()
  {
    return(array(get_ou('roleRDN').$this->DivListRoles->selectedBase));
  }


  function copyPasteHandling_from_queue($s_action,$s_entry)
  {
    /* Check if Copy & Paste is disabled */
    if(!is_object($this->CopyPasteHandler)){
      return("");
    }

    $ui = get_userinfo();

    /* Add a single entry to queue */
    if($s_action == "cut" || $s_action == "copy"){

      /* Cleanup object queue */
      $this->CopyPasteHandler->cleanup_queue();
      $dn = $this->roles[$s_entry]['dn'];
      if($s_action == "copy" && $ui->is_copyable($dn,"roles","roleGeneric")){
        $this->CopyPasteHandler->add_to_queue($dn,$s_action,"roletabs","ROLETABS","roles");
      }
      if($s_action == "cut" && $ui->is_cutable($dn,"roles","roleGeneric")){
        $this->CopyPasteHandler->add_to_queue($dn,$s_action,"roletabs","ROLETABS","roles");
      }
    }

    /* Add entries to queue */
    if($s_action == "copy_multiple" || $s_action == "cut_multiple"){

      /* Cleanup object queue */
      $this->CopyPasteHandler->cleanup_queue();

      /* Add new entries to CP queue */
      foreach($this->list_get_selected_items() as $id){
        $dn = $this->roles[$id]['dn'];

        if($s_action == "copy_multiple" && $ui->is_copyable($dn,"roles","roleGeneric")){
          $this->CopyPasteHandler->add_to_queue($dn,"copy","roletabs","ROLETABS","roles");
        }
        if($s_action == "cut_multiple" && $ui->is_cutable($dn,"roles","roleGeneric")){
          $this->CopyPasteHandler->add_to_queue($dn,"cut","roletabs","ROLETABS","roles");
        }
      }
    }

    /* Start pasting entries */
    if($s_action == "editPaste"){
      $this->start_pasting_copied_objects = TRUE;
    }

    /* Return C&P dialog */
    if($this->start_pasting_copied_objects && $this->CopyPasteHandler->entries_queued()){

      /* Get dialog */
      $this->CopyPasteHandler->SetVar("base",$this->DivListRoles->selectedBase);
      $data = $this->CopyPasteHandler->execute();

      /* Return dialog data */
      if(!empty($data)){
        return($data);
      }
    }
    /* Automatically disable status for pasting */
    if(!$this->CopyPasteHandler->entries_queued()){
      $this->start_pasting_copied_objects = FALSE;
    }
    return("");
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
