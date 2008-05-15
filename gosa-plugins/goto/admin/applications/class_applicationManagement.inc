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

class applicationManagement extends plugin
{
  /* Definitions */
  var $plHeadline     = "Applications";
  var $plDescription  = "This does something";

  /* Dialog attributes */
  var $apptabs                  = NULL;
  var $ui                       = NULL;
  var $CopyPasteHandler         = NULL;
  var $DivListApplication       = NULL;
  var $applications             = array();
  var $enableReleaseManagement  = false;
  var $start_pasting_copied_objects = FALSE;

  var $app_base     ="";
  var $app_release  ="";

  var $dns = array();

  function IsReleaseManagementActivated()
  {
    /* Check if we should enable the release selection */
    $tmp = $this->config->search("faiManagement", "CLASS",array('menu','tabs'));
    if(!empty($tmp)){
      return(true);
    }
    return(false);
  }


  function applicationManagement (&$config, &$ui)
  {
    /* Save configuration for internal use */
    $this->config   = &$config;
    $this->ui       = &$ui;

    /* Check if copy & paste is activated */
    if($this->config->boolValueIsTrue("MAIN","ENABLECOPYPASTE")){
      $this->CopyPasteHandler = new CopyPasteHandler($this->config);
    }

    /* Creat dialog object */
    $this->DivListApplication = new divListApplication($this->config,$this);

    if($this->IsReleaseManagementActivated()){
    /* Check if we should enable the release selection */
      $this->enableReleaseManagement = true;

      /* Hide SubSearch checkbox */
      $this->DivListApplication->DisableCheckBox("SubSearch");
    }

    /* Set default release */
    if(!$this->IsReleaseManagementActivated()){
      $this->app_base = get_ou("applicationou").$this->config->current['BASE'];
      if(!session::is_set("app_filter")){
        session::set("app_filter",array("app_base" => $this->app_base));
      }
      $app_filter     = session::get("app_filter");
      $this->app_base = $app_filter['app_base'];
    }else{
      $this->app_base = get_ou("applicationou").$this->config->current['BASE'];
      if(!session::is_set("app_filter")){
        session::set("app_filter",array("app_base" => $this->app_base,"app_release" => $this->app_base));
      }
      $app_filter         = session::get("app_filter");
      $this->app_base     = $app_filter['app_base'];
      $this->app_release  = $app_filter['app_release'];
    }
  }


  function getReleases()
  {
    $ldap = $this->config->get_ldap_link();
    $ret  = array();
    $base = $this->app_base; 

    $ret[$this->app_base] = "/";
    $ldap->cd($base);
    $ldap->search("(&(objectClass=organizationalUnit)(objectClass=FAIbranch))",array("ou"));
    while($attrs = $ldap->fetch()){
      $str = str_replace($base,"",$attrs['dn']);
      $tmp = array_reverse( split("ou=",$str));
      $str = "";
      foreach($tmp as $val){
        $val = trim(preg_replace("/,/","",$val));
        if(empty($val)) break;
        $str .= "/".$val;
      } 
      if(!empty($str)){
        $ret[$attrs['dn']]= preg_replace("/^\//","",$str);
      }
    }
    asort($ret);
    return($ret);
  }

  function execute()
  {
    /* Call parent execute */
    plugin::execute();


    /**************** 
      Variable init 
     ****************/

    /* These vars will be stored if you try to open a locked app, 
        to be able to perform your last requests after showing a warning message */
    session::set('LOCK_VARS_TO_USE',array("/^act$/","/^id$/","/^appl_edit_/","/^appl_del_/","/^item_selected/","/^remove_multiple_applications/","/^menu_action/"));

    $smarty       = get_smarty();             // Smarty instance
    $s_action     = "";                       // Contains the action to proceed
    $s_entry      = "";                       // The value for s_action
    $base_back    = "";                       // The Link for Backbutton
    
    /* Test Posts */
    foreach($_POST as $key => $val){
      // Post for delete
      if(preg_match("/appl_del.*/",$key)){
        $s_action = "del";
        $s_entry  = preg_replace("/appl_".$s_action."_/i","",$key);
        // Post for edit
      }elseif(preg_match("/appl_edit_.*/",$key)){
        $s_action="edit";
        $s_entry  = preg_replace("/appl_".$s_action."_/i","",$key);
        // Post for new
      }elseif(preg_match("/^copy_.*/",$key)){
        $s_action="copy";
        $s_entry  = preg_replace("/^copy_/i","",$key);
      }elseif(preg_match("/^cut_.*/",$key)){
        $s_action="cut";
        $s_entry  = preg_replace("/^cut_/i","",$key);
        // Post for new
      }elseif(preg_match("/^appl_new.*/",$key)){
        $s_action="new";
      }elseif(preg_match("/^remove_multiple_applications/",$key)){
        $s_action="del_multiple";
      }elseif(preg_match("/^editPaste.*/i",$key)){
        $s_action="editPaste";
      }elseif(preg_match("/^multiple_copy_groups/",$key)){
        $s_action = "copy_multiple";
      }elseif(preg_match("/^multiple_cut_groups/",$key)){
        $s_action = "cut_multiple";
      }
    }

    if((isset($_GET['act']))&&($_GET['act']=="edit_entry")){
      $s_action ="edit";
      $s_entry  = $_GET['id'];
    }

    $s_entry  = preg_replace("/_.$/","",$s_entry);


    /* handle C&P from layers menu */
    if(isset($_POST['menu_action']) && preg_match("/^multiple_copy_systems/",$_POST['menu_action'])){
      $s_action = "copy_multiple";
    }
    if(isset($_POST['menu_action']) && preg_match("/^multiple_cut_systems/",$_POST['menu_action'])){
      $s_action = "cut_multiple";
    }
    if(isset($_POST['menu_action']) && preg_match("/^editPaste/",$_POST['menu_action'])){
      $s_action = "editPaste";
    }

    /* Create options */
    if(isset($_POST['menu_action']) && $_POST['menu_action'] == "appl_new"){
      $s_action = "new";
    }

    /* handle remove from layers menu */
    if(isset($_POST['menu_action']) && preg_match("/^remove_multiple/",$_POST['menu_action'])){
      $s_action = "del_multiple";
    }

    /**************** 
      Copy & Paste handling  
     ****************/

    /* Display the copy & paste dialog, if it is currently open */
    $ret = $this->copyPasteHandling_from_queue($s_action,$s_entry);
    if($ret){
      return($ret);
    }

    /**************** 
      Create a new app 
     ****************/

    /* New application? */
    if ($s_action=="new"){

      /* By default we set 'dn' to 'new', all relevant plugins will
         react on this. */
      $this->dn= "new";

      /* Create new usertab object */
      $this->apptabs= new apptabs($this->config,$this->config->data['TABS']['APPSTABS'], $this->dn,"application");
      $this->apptabs->parent = &$this;
      $this->apptabs->set_acl_base($this->app_base);
    }


    /**************** 
      Edit entry canceled 
     ****************/

    /* Cancel dialogs */
    if (isset($_POST['edit_cancel']) || isset($_POST['password_cancel'])){
      $this->remove_lock();
      $this->apptabs= NULL;
      session::un_set('objectinfo');
    }


    /**************** 
      Edit entry finished 
     ****************/

    /* Finish apps edit is triggered by the tabulator dialog, so
       the user wants to save edited data. Check and save at this
       point. */
    if ((isset($_POST['edit_finish']) || isset($_POST['edit_apply']) ) && (isset($this->apptabs->config))){

      /* Check tabs, will feed message array */
      $this->apptabs->last= $this->apptabs->current;
      $this->apptabs->save_object();
      $message= $this->apptabs->check();

      /* Save, or display error message? */
      if (count($message) == 0){

        /* Save data data to ldap */
        $this->apptabs->save();

        if (!isset($_POST['edit_apply'])){
          /* Application has been saved successfully, remove lock from
             LDAP. */
          if ($this->dn != "new"){
            $this->remove_lock();
          }
          $this->apptabs= NULL;
          session::un_set('objectinfo');
        }else{

          /* Reinitialize tab */
          if($this->apptabs instanceof tabs){
            $this->apptabs->re_init();
          }
        }
      } else {
        /* Ok. There seem to be errors regarding to the tab data,
           show message and continue as usual. */
        msg_dialog::displayChecks($message);
      }
    }


    /**************** 
      Edit entry  
     ****************/

    /* User wants to edit data? */
    if (($s_action=="edit") && (!isset($this->apptabs->config))){

      /* Get 'dn' from posted 'applist', must be unique */
      $this->dn= $this->applications[$s_entry]['dn'];

      /* Check locking, save current plugin in 'back_plugin', so
         the dialog knows where to return. */
      if (($user= get_lock($this->dn)) != ""){
        return(gen_locked_message ($user, $this->dn));
      }

      /* Lock the current entry, so everyone will get the
         above dialog */
      add_lock ($this->dn, $this->ui->dn);

      /* Register apptabs to trigger edit dialog */
      $this->apptabs= new apptabs($this->config,$this->config->data['TABS']['APPSTABS'], $this->dn,"application");
      $this->apptabs->parent = &$this;
      $this->apptabs->set_acl_base($this->dn);
      session::set('objectinfo',$this->dn);
    }



    /********************
      Delete MULTIPLE entries requested, display confirm dialog
     ********************/

    if ($s_action=="del_multiple"){
      $ids = $this->list_get_selected_items();

      if(count($ids)){

        foreach($ids as $id){
          $dn = $this->applications[$id]['dn'];
          $this->dns[$id] = $dn;
        }
        if ($user= get_multiple_locks($this->dns)){
          return(gen_locked_message($user,$this->dns));
        }

        $dns_names = array();
        foreach($this->dns as $dn){
          add_lock ($dn, $this->ui->dn);
          $dns_names[] =@LDAP::fix($dn);
        }

        /* Lock the current entry, so nobody will edit it during deletion */
        $smarty->assign("intro",  msgPool::deleteInfo($dns_names,_("application")));
        $smarty->assign("multiple", true);
        return($smarty->fetch(get_template_path('remove.tpl', TRUE)));
      }
    }


    /********************
      Delete MULTIPLE entries confirmed
     ********************/

    /* Confirmation for deletion has been passed. Users should be deleted. */
    if (isset($_POST['delete_multiple_application_confirm'])){

      /* Remove user by user and check acls before removeing them */
      foreach($this->dns as $key => $dn){

        $ui = get_userinfo();
        $acl = $ui->get_permissions($dn ,"application/application");
        if (preg_match('/d/', $acl)){

          /* Delete request is permitted, perform LDAP action */
          $this->apptabs= new apptabs($this->config, $this->config->data['TABS']['APPSTABS'], $dn,"application");
          $this->apptabs->parent = &$this;
          $this->apptabs->set_acl_base($dn);
          $this->apptabs->delete ();
          unset ($this->apptabs);
          $this->apptabs= NULL;

        } else {
          /* Normally this shouldn't be reached, send some extra
             logs to notify the administrator */
          msg_dialog::display(_("Permission error"), msgPool::permDelete(), INFO_DIALOG);
          new log("security","application/".get_class($this),$dn,array(),"Tried to trick deletion.");
        }
      }

      /* Remove lock file after successfull deletion */
      $this->remove_lock();
      $this->dns = array();
    }


    /********************
      Delete MULTIPLE entries Canceled
     ********************/

    /* Remove lock */
    if(isset($_POST['delete_multiple_application_cancel'])){

      /* Remove lock file after successfull deletion */
      $this->remove_lock();
      $this->dns = array();
    }

    /**************** 
      Delete app 
     ****************/

    /* Remove user was requested */
    if ($s_action == "del"){

      /* Get 'dn' from posted 'uid' */
      $this->dn= $this->applications[$s_entry]['dn'];

      /* Load permissions for selected 'dn' and check if
         we're allowed to remove this 'dn' */
      $ui = get_userinfo();
      $acl = $ui->get_permissions($this->dn ,"application/application");

      if(preg_match("/d/",$acl)){
        /* Check locking, save current plugin in 'back_plugin', so
           the dialog knows where to return. */
        if (($user= get_lock($this->dn)) != ""){
          return (gen_locked_message ($user, $this->dn));
        }

        /* Lock the current entry, so nobody will edit it during deletion */
        add_lock ($this->dn, $this->ui->dn);
        $smarty= get_smarty();
        $smarty->assign("intro",msgPool::deleteInfo(@LDAP::fix($this->dn),_("application")));
        $smarty->assign("multiple", false);
        return($smarty->fetch (get_template_path('remove.tpl', TRUE)));
      } else {

        /* Obviously the user isn't allowed to delete. Show message and
           clean session. */
        msg_dialog::display(_("Permission error"), msgPool::permDelete(), INFO_DIALOG);
      }
    }


    /**************** 
      Delete app confirmed 
     ****************/

    /* Confirmation for deletion has been passed. Group should be deleted. */
    if (isset($_POST['delete_app_confirm'])){

      /* Some nice guy may send this as POST, so we've to check
         for the permissions again. */
      $ui = get_userinfo();
      $acl = $ui->get_permissions($this->dn ,"application/application");

      if(preg_match("/d/",$acl)){

        /* Delete request is permitted, perform LDAP action */
        $this->apptabs= new apptabs($this->config, $this->config->data['TABS']['APPSTABS'], $this->dn,"application");
        $this->apptabs->parent = &$this;
        $this->apptabs->set_acl_base($this->dn);
        $this->apptabs->delete ();
        unset ($this->apptabs);
        $this->apptabs= NULL;

      } else {

        /* Normally this shouldn't be reached, send some extra
           logs to notify the administrator */
        msg_dialog::display(_("Permission error"), msgPool::permDelete(), INFO_DIALOG);
        new log("security","application/".get_class($this),$dn,array(),"Tried to trick deletion.");
      }

      /* Remove lock file after successfull deletion */
      $this->remove_lock();
    }


    /**************** 
      Delete app canceled 
     ****************/

    /* Delete application canceled? */
    if (isset($_POST['delete_cancel'])){
      $this->remove_lock();
      session::un_set('objectinfo');
    }

    /* Show tab dialog if object is present */
    if (($this->apptabs) && (isset($this->apptabs->config))){
      $display= $this->apptabs->execute();

      /* Don't show buttons if tab dialog requests this */
      if (!$this->apptabs->by_object[$this->apptabs->current]->dialog){
        $display.= "<p style=\"text-align:right\">\n";
        $display.= "<input type=\"submit\" name=\"edit_finish\" style=\"width:80px\" value=\"".msgPool::okButton()."\">\n";
        $display.= "&nbsp;\n";
        if ($this->dn != "new"){
          $display.= "<input type=submit name=\"edit_apply\" value=\"".msgPool::applyButton()."\">\n";
          $display.= "&nbsp;\n";
        }
        $display.= "<input type=\"submit\" name=\"edit_cancel\" value=\"".msgPool::cancelButton()."\">\n";
        $display.= "</p>";
      }
      return ($display);
    }


    /****************
      Dialog display
     ****************/

    /* Check if there is a snapshot dialog open */
    if($this->IsReleaseManagementActivated()){
      $base = $this->app_release;  
    }else{
      $base = $this->app_base;  
    }
    if($str = $this->showSnapshotDialog($base,$this->get_used_snapshot_bases())){
      return($str);
    }

    /* Display dialog with system list */
    $this->DivListApplication->parent = $this;
    $this->DivListApplication->execute();
    if(!$this->IsReleaseManagementActivated()){
      $this->DivListApplication->AddDepartments($this->DivListApplication->selectedBase,3,1);
    } 
    $this->reload();
    $this->DivListApplication->setEntries($this->applications);
    return($this->DivListApplication->Draw());
  }


  /* Return departments, that will be included within snapshot detection */
  function get_used_snapshot_bases()
  {
    if($this->IsReleaseManagementActivated()){
      return(array($this->app_release));  
    }else{
      return(array($this->app_base));  
    }
  }


  function reload()
  {
    $this->applications= array();

    /* Set base for all searches */
    $Regex      = $this->DivListApplication->Regex;
    $SubSearch  = $this->DivListApplication->SubSearch; 
    $Flags      =  GL_NONE | GL_SIZELIMIT;
    $Filter     = "(&(cn=".$Regex.")(objectClass=gosaApplication))";
    $tmp        = array();

    if(!$this->IsReleaseManagementActivated()){
      $use_base = $this->app_base;
      if($SubSearch){
        $use_base = preg_replace("/^".normalizePreg(get_ou("applicationou"))."/","",$use_base);
      }
    }else{
      $use_base = $this->app_release;
      $SubSearch= FALSE;
    }
  
    if($SubSearch){ 
      $res= get_sub_list($Filter, "application",get_ou("applicationou"), $use_base, array("cn","description","dn","objectClass"), $Flags);
    }else{
      $res= get_list($Filter, "application",$use_base, array("cn","description","dn","objectClass"), $Flags);
    }
    $tmp2 = array();
    foreach ($res as $val){
      $tmp[strtolower($val['cn'][0]).$val['cn'][0].$val['dn']]=$val;
      $tmp2[strtolower($val['cn'][0]).$val['cn'][0].$val['dn']] = strtolower($val['cn'][0]).$val['cn'][0].$val['dn'];
    }

    natcasesort($tmp2);
    $this->applications=array();
    foreach($tmp2 as $val){
      $this->applications[]=$tmp[$val];
    }
    reset ($this->applications);
  }

  function remove_from_parent()
  {
    /* Optionally execute a command after we're done */
    $this->postremove();
  }


  function copyPasteHandling_from_queue($s_action,$s_entry)
  {
    /* Check if Copy & Paste is disabled */
    if(!is_object($this->CopyPasteHandler)){
      return("");
    }

    /* Add a single entry to queue */
    if($s_action == "cut" || $s_action == "copy"){

      /* Cleanup object queue */
      $this->CopyPasteHandler->cleanup_queue();
      $dn = $this->applications[$s_entry]['dn'];
      $this->CopyPasteHandler->add_to_queue($dn,$s_action,"apptabs","APPSTABS","application");
    }


    /* Add entries to queue */
    if($s_action == "copy_multiple" || $s_action == "cut_multiple"){

      /* Cleanup object queue */
      $this->CopyPasteHandler->cleanup_queue();

      /* Add new entries to CP queue */
      foreach($this->list_get_selected_items() as $id){
        $dn = $this->applications[$id]['dn'];

        if($s_action == "copy_multiple"){
          $this->CopyPasteHandler->add_to_queue($dn,"copy","apptabs","APPSTABS","application");
        }
        if($s_action == "cut_multiple"){
          $this->CopyPasteHandler->add_to_queue($dn,"cut","apptabs","APPSTABS","application");
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
      $this->CopyPasteHandler->SetVar("base",$this->app_base);
      $this->CopyPasteHandler->SetVar("parent",$this);
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


  /* Save to LDAP */
  function save()
  {
    /* Optionally execute a command after we're done */
    $this->postcreate();
  }


  function remove_lock()
  {
    if (isset($this->apptabs->dn)){
      del_lock ($this->apptabs->dn);
    }
    if(isset($this->dn) && !empty($this->dn) && $this->dn != "new"){
      del_lock($this->dn);
    }
    if(isset($this->dns) && is_array($this->dns) && count($this->dns)){
      del_lock($this->dns);
    }
  }


  function save_object() 
  {
    $this->DivListApplication->save_object();
    if(is_object($this->CopyPasteHandler)){
      $this->CopyPasteHandler->save_object();
    }
    
    if($this->IsReleaseManagementActivated() && isset($_POST['app_release'])){
      $sel_rel = get_post('app_release');
      $releases = array_flip($this->getReleases());
      if(isset($releases[$sel_rel])){
        $this->app_release = $releases[$sel_rel];
      }
      $app_filter     = session::get("app_filter");
      $app_filter['app_release'] = $this->app_release;
      session::set("app_filter",$app_filter);
    }elseif(!$this->IsReleaseManagementActivated()){
      $this->app_base = get_ou("applicationou").$this->DivListApplication->selectedBase;
      $app_filter     = session::get("app_filter");
      $app_filter['app_base'] = $this->app_base;
      session::set("app_filter",$app_filter);
    }
  }

  function check() {}
  function adapt_from_template($dn, $skip= array()) {}
  function password_change_needed() {}
}
// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
