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


class faiManagement extends plugin
{
	/* Definitions */
  var $plHeadline     = "Software deployment";
  var $plDescription  = "Manage software packages and deployment reciepes";
  var $plIcon         = "plugins/fai/images/plugin.png";

	/* Headpage attributes */
  var $lock_type    = "";    // should be branch/freeze
  var $lock_name    = "";
  var $lock_dn      = "";  

	/* attribute list for save action */
	var $attributes		= array("lock_type","lock_name","lock_dn"); 	// 	Attributes Managed by this plugin 
	var $objectclasses= array();	// 	ObjectClasses which the attributes are related to
	var $dialog				= array();	// 	This object contains every dialog we have currently opened

	var $objects			= array();	// 	This array contains all available objects shown in divlist
	var $is_dialog		= false;

  var $dispNewBranch= false;
  var $dispNewFreeze= false;

  var $DivListFai;
  var $start_pasting_copied_objects = FALSE;
  var $CopyPasteHandler = FALSE;

  /* Allow inserting of new elements if freezed releases 
  */
  var $allow_freeze_object_attach = TRUE;
  var $no_save;
  var $acl_base     ="";
  var $fai_base     ="";
  var $fai_release  ="";
  var $acl_module = array("fai");
  var $opsi = NULL;

	/* construction/reconstruction 
	 */
	function faiManagement (&$config, $ui)
	{
		/* Set defaults */
		$this->dn			= "";
		$this->config	= $config;
		$this->ui			= $ui;	

    /* Check if the opsi plugin is installed.
     */
    if(class_available("opsi")){
      $this->opsi = new opsi($this->config);;
    }
    
    /* Creat dialog object */
    $this->DivListFai = new divListFai($this->config,$this);

    /* Copy & Paste handler */
    if ($this->config->boolValueIsTrue("main", "copyPaste")){
      $this->CopyPasteHandler= new CopyPasteHandler($this->config);
    }

    /* Set default release 
     */
    $this->acl_base = $this->config->current['BASE'];
    $this->fai_base = get_ou("faiBaseRDN").$this->config->current['BASE'];

    if(!session::is_set("fai_filter")){

      /* Set intial release */
      $rel = $config->search("faiManagement","DEFAULTFAIRELEASE",array("menu"));
      $rels = array_flip($this->getBranches());
      if(isset($rels[$rel])){
        $rel = $rels[$rel];
      }else{
        $rel = $this->fai_base;
      }

      session::set("fai_filter",array("fai_release" => $rel));
    }

    $fai_filter = session::get("fai_filter");
    $this->fai_release = $fai_filter['fai_release'];
	}

	function execute()
	{
    /* Call parent execute */
    plugin::execute();

    /* Initialise vars and smarty */
		$smarty 	= get_smarty();
		$smarty->assign("BranchName","");
    
		$display 	= "";
    $s_action	= "";
		$s_entry 	= "";
    $no_save = FALSE;   // hide Apply / Save buttons
    
    /* If an entry was locked, these vars will be stored in a session to allow direct edit */
    session::set('LOCK_VARS_TO_USE',array("/^edit_freeze_entry$/","/^edit_entry$/","/^id$/","/^entry_edit_/","/^entry_delete_/","/^item_selected/","/^remove_multiple_fai_objects/","/^menu_action/","/^faiGroupHandle_apply$/"));

    /****************
      Handle posts 
     ****************/

		/* Check ImageButton posts
		 * Create new tab ich new_xx is posted
		 */
    $posts = array( "/^remove_branch/"                =>"remove_branch",    
                    "/^branch_branch/"                =>"branch_branch",
                    "/^freeze_branch/"                =>"freeze_branch",   
 
                    "/^create_partition/i"            =>"new_partition",
                    "/^create_script/i"               =>"new_script",      
                    "/^create_hook/i"                 =>"new_hook",
                    "/^create_variable/i"             =>"new_variable",  
                    "/^create_template/i"             =>"new_template",
                    "/^create_package/i"              =>"new_package",    
                    "/^create_profile/i"              =>"new_profile",

                    "/^edit_continue$/"               => "select_class_name_finished",

                    "/^group_copy/"                   => "group_copy",
                    "/^group_cut/"                    => "group_cut",
                    "/^group_edit/"                   => "group_edit",
                    "/^group_remove/"                 => "group_remove");
		foreach($_POST as $name => $value){
      foreach($posts as $reg => $act ){
        if(preg_match($reg,$name)){
          $s_action = $act;
          $s_entry = ltrim(preg_replace($reg,"",$name),"_");
          $s_entry = preg_replace("/_.*$/","",$s_entry);
          break;
        }
      }
      if(preg_match("/^edit_[0-9]*_.*$/",$name)){
        $i_entryID  = preg_replace("/^edit_([0-9]*)_.*$/i","\\1",$name);
        $s_entryType= preg_replace("/^edit_[0-9]*_([^_]*)_.*$/i","\\1",$name);
        $s_action = "edit";
      }
    }

    if(isset($_GET['act']) && $_GET['act'] == "edit" && isset($_GET['id'])){
      if(isset($this->objects[$_GET['id']])){
        $s_action = "group_edit";
        $s_entry = $_GET['id'];
      }
    }

    foreach(array("freeze_branch" => "freeze_branch",
          "branch_branch" => "branch_branch",
          "remove_branch" => "remove_branch") as $from => $to){
      if(isset($_GET['act']) && $_GET['act'] == $from){
        $s_action = $to;
      }
    }
  
    /* handle C&P from layer menu */
    if(isset($_POST['menu_action']) && preg_match("/^multiple_copy_systems/",$_POST['menu_action'])){
      $s_action = "copy_multiple";
    }
    if(isset($_POST['menu_action']) && preg_match("/^multiple_cut_systems/",$_POST['menu_action'])){
      $s_action = "cut_multiple";
    }
    if(isset($_POST['menu_action']) && preg_match("/^editPaste/",$_POST['menu_action'])){
      $s_action = "editPaste";
    }


    if((isset($_POST['CancelBranchName'])) || (isset($_POST['CloseIFrame']))){
      $this->dispNewBranch = false;
      $this->dispNewFreeze = false;
    }

    /* Create options */
    if(isset($_POST['menu_action']) && preg_match("/^Create_/",$_POST['menu_action'])){
      $s_action = "new_".preg_replace("/^Create_/","",$_POST['menu_action']);;
      $s_entry  = preg_replace("/^Create_/","",$_POST['menu_action']);
    }

    /* handle remove from layers menu */
    if(isset($_POST['menu_action']) && preg_match("/^remove_multiple/",$_POST['menu_action'])){
      $s_action = "del_multiple";
    }


    if(!empty($s_action)){
      $this->no_save = $no_save;
    }


    /*******************
      Opsi extension 
     *******************/

    if($this->opsi instanceof opsi && $this->opsi->enabled()){
      if($s_action == "opsi_edit"){
        $name = $this->objects[$s_entry]['cn'];
        $cfg = $this->opsi->get_product_properties($name);
        $this->dialog = new tabs_opsiProdConfig($this->config, $this->config->data['TABS']['OPSIPRODCONFIG'],"","opsi");
        if(isset($this->dialog->by_object['opsiProperties'])){
          $this->dialog->by_object['opsiProperties']->set_cfg($cfg);
          $this->dialog->by_object['opsiProperties']->set_product($name);
        }else{
          trigger_error("Unknown tab, please check config.");
        }
      }
      if($this->dialog instanceof tabs_opsiProdConfig && isset($_POST['cancel_properties'])){
        $this->dialog = NULL;
      }
      if($this->dialog instanceof tabs_opsiProdConfig && isset($_POST['save_properties'])){
        $this->dialog->save_object();
        $op    = $this->dialog->by_object['opsiProperties'];
        $name  = $op->get_product();
        $cfg   = $op->get_cfg();
        $this->opsi->set_product_properties($name,$cfg); 
        if($this->opsi->is_error()){
          msg_dialog::display(_("Error"),msgPool::siError($this->opsi->get_error()),ERROR_DIALOG);
        }else{
          $this->dialog = NULL;
        }
      }
      if($this->dialog instanceof tabs_opsiProdConfig){
        $this->dialog->save_object();
        return($this->dialog->execute());
      }
    }


    /****************
      Delete a group of FAI objects 
        (Group = same name & different classes)
     ****************/

    if($s_action == "group_remove"){
      if(isset($this->objects[$s_entry])){
        $group = $this->objects[$s_entry];

        /* Do not allow to remove opsi products */
        foreach(array("opsi_netboot","opsi_local") as $type){
          if(isset($group[$type])) unset($group[$type]);
        }
        if(count($group)){
          $this->dialog = new faiGroupHandle($group,"remove");
        }
      }
    }elseif($s_action == "group_edit"){
      if(isset($this->objects[$s_entry])){
        $group = $this->objects[$s_entry];
        $this->dialog = new faiGroupHandle($group,"edit");
      }
    }elseif($s_action == "group_cut"){
      if(isset($this->objects[$s_entry])){
        $group = $this->objects[$s_entry];
        $this->dialog = new faiGroupHandle($group,"cut");
      }
    }elseif($s_action == "group_copy"){
      if(isset($this->objects[$s_entry])){
        $group = $this->objects[$s_entry];
        $this->dialog = new faiGroupHandle($group,"copy");
      }
    }
    if($this->dialog instanceOf faiGroupHandle){
      $this->dialog->save_object();
      if($this->dialog->is_open()){
        return($this->dialog->execute());
      }elseif($this->dialog->is_canceled() || isset($_POST['cancel_lock'])){
        $this->dialog = FALSE;
      }else{
        if(!count($this->dialog->get_selected())){
          $this->dialog = FALSE;
        }
      }
    }

    /********************
      Copy & Paste
     ********************/

    /* Display the copy & paste dialog, if it is currently open */
    $ret = $this->copyPasteHandling_from_queue($s_action,$s_entry);
    if($ret){
      return($ret);
    }


    /********************
      Delete MULTIPLE entries confirmed
     ********************/

    /* Confirmation for deletion has been passed. Users should be deleted. */
    if (isset($_POST['delete_multiple_fai_object_confirm'])){

      /* Find out more about the object type */
      $ldap   = $this->config->get_ldap_link();

      /* Remove user by user and check acls before removeing them */
      foreach($this->dns as $key => $dn){
        $ldap->cat($dn, array('objectClass'));
        $attrs  = $ldap->fetch();
        $type   = $this->get_type($attrs);
        $acl  = $this->ui->get_permissions($dn,"fai/".$type[1]);
        if(preg_match("/d/",$acl)){
          $this->dialog = new $type[0]($this->config, $this->config->data['TABS'][$type[2]], $dn,"fai");
          $this->dialog->parent = &$this;
          $this->dialog->set_acl_base($dn);
          $this->dialog->by_object[$type[1]]->remove_from_parent ();
          $this->dialog= FALSE;
          FAI::save_release_changes_now();
          $to_del = FAI::clean_up_releases($dn);
          /* Remove sub-objects (e.g. the variable key/value of a FAIvariable) from LDAP entirely */
          $children = FAI::get_child_objects($dn);
          if ($children) {
            $to_del += $children;
          }
          foreach($to_del as $dn){
            $ldap->rmdir_recursive($dn);
          }
        } else {

          /* Normally this shouldn't be reached, send some extra
             logs to notify the administrator */
          msg_dialog::display(_("Permission error"), msgPool::permDelete(), ERROR_DIALOG);
          new log("security","fai/".get_class($this),$dn,array(),"Tried to trick deletion.");
        }
      }

      /* Remove lock file after successfull deletion */
      $this->remove_lock();
      $this->dns = array();
      $this->dialog = FALSE;
    }


    /****************
      Delete confirme dialog 
     ****************/

    if ($s_action=="del_multiple" || 
        $this->dialog instanceOf faiGroupHandle && 
        $this->dialog->get_mode() == "remove" &&
        !isset($_POST['delete_multiple_fai_object_cancel'])){
 
      /* Collect objects to delete and check if objects are freezed
       */ 
      $dns = array();
      $errors = "";
      $this->dns = array();
      if($this->dialog instanceOf faiGroupHandle){
        $to_delete = $this->dialog->get_selected();
      }else{
        $ids = $this->list_get_selected_items();
        $to_delete = array();
        foreach($ids as $id){
          foreach($this->objects[$id] as $obj) {
            array_push($to_delete, $obj);
          }
        }
      } 

      foreach($to_delete as $obj){
        if(isset($obj['type']) && in_array($obj['type'],array("opsi_netboot","opsi_local"))){
          continue;
        }
        if(isset($obj['FAIstate']) && preg_match('/^freeze/', $obj['FAIstate'])){
          $errors .= $obj['cn'].", ";
        }else{
          $this->dns[] = $obj['dn'];
        }
      }

      if($errors != ""){
        msg_dialog::display(_("Branch locked"),sprintf(_("The following entries are locked, you can't remove them %s."),
              "<br><br>".trim($errors,", ")),INFO_DIALOG);
      }

      /* Check locking 
       */
      if(count($this->dns)){
        if ($user= get_multiple_locks($this->dns)){
          return(gen_locked_message($user,$this->dns));
        }
        if(count($this->dns)){
          $dns_names = array();
          foreach($this->dns as $dn){
            add_lock ($dn, $this->ui->dn);
            $dns_names[] = LDAP::fix($dn);
          }
				  $smarty->assign("warning",msgPool::deleteInfo($dns_names,_("FAI object")));
          $smarty->assign("multiple", true);
          return($smarty->fetch(get_template_path('remove.tpl', TRUE)));
        }
      }
    }



    /********************
      Delete MULTIPLE entries Canceled
     ********************/

    /* Remove lock */
    if(isset($_POST['delete_multiple_fai_object_cancel'])){
      $this->remove_lock();
      $this->dns = array();
      $this->dialog = FALSE;
    }

    /****************
      Edit entry 
     ****************/

		if($s_action == "edit" || $this->dialog instanceOf faiGroupHandle && $this->dialog->get_mode() == "edit"){

      $entry = array();
      if($this->dialog instanceOf faiGroupHandle){
        $entries = $this->dialog->get_selected();
        $entry = array_pop($entries);
      }elseif(isset($this->objects[$i_entryID][$s_entryType])){
        $entry = $this->objects[$i_entryID][$s_entryType];
      }

      if(count($entry)){
        $a_setup  = $this->get_type($entry);

        /* Special handling for opsi products 
         */

        if(in_array($entry['type'],array("opsi_local","opsi_netboot")) && 
            $this->opsi instanceof opsi && $this->opsi->enabled() ){

          $name = $entry['cn'];
          $cfg = $this->opsi->get_product_properties($name);
          $this->dialog = new tabs_opsiProdConfig($this->config, $this->config->data['TABS']['OPSIPRODCONFIG'],"","opsi");
          if(isset($this->dialog->by_object['opsiProperties'])){
            $this->dialog->by_object['opsiProperties']->set_cfg($cfg);
            $this->dialog->by_object['opsiProperties']->set_product($name);
          }else{
            trigger_error("Unknown tab, please check config.");
          }
        }elseif(count($a_setup)){

          $this->dn = $entry['dn'];
          /* Check locking, save current plugin in 'back_plugin', so the dialog knows where to return. */
          if (($user= get_lock($this->dn)) != ""){
            return(gen_locked_message ($user, $this->dn, TRUE));
          }
          add_lock ($this->dn, $this->ui->dn);

          $this->dialog     = new $a_setup[0]($this->config,$this->config->data['TABS'][$a_setup[2]],$this->dn,"fai");
          $this->dialog->parent = &$this;
          $this->dialog->by_object[$a_setup[1]]->FAIstate = $entry['FAIstate'];
          $this->dialog->set_acl_base($this->dn);
          $this->is_dialog  = true;
          set_object_info($this->dn);
        }
      }
    }


    /*  Branch handling 
        09.01.2006
    */

    /****************
      Remove branch
     ****************/

    /* Remove branch 
     */
    if($s_action == "remove_branch"){
      $base= $this->fai_release;

      /* Check if we have a post remove method configured
       *  else skip this operation. (Skip:Button in the ui should be disabled in this case too)
       */
      if("" != $this->config->search("faiManagement", "POSTREMOVE",array('menu','tabs'))){
        /* Load permissions for selected 'dn' and check if
           we're allowed to remove this 'dn' */
        if(preg_match("/d/",$this->ui->get_permissions($this->acl_base,"fai/faiManagement"))){
          $smarty->assign("release_hidden",base64_encode($this->fai_release));
          $smarty->assign("info", msgPool::deleteInfo(LDAP::fix($this->fai_release),_("FAI branch/freeze")));
          return($smarty->fetch(get_template_path('remove_branch.tpl',TRUE)));
        } else {
          msg_dialog::display(_("Permission error"), _("You have no permission to delete this release!"), ERROR_DIALOG);
        }
      }
    }

    
    /****************
      Remove branch confirmed
     ****************/

    if(isset($_POST['delete_branch_confirm'])){

      /* Check if we have a post remove method configured
       *  else skip this operation. (Skip:Button in the ui should be disabled in this case too)
       */
      if("" != $this->config->search("faiManagement", "POSTREMOVE",array('menu','tabs'))){

        if(!isset($_POST['release_hidden']) || base64_decode($_POST['release_hidden']) != $this->fai_release){
          msg_dialog::display(_("Warning"),_("Release remove aborted because the release name check failed!"));
        }else{

          $bb =  $this->fai_release;
          if(!isset($ldap)){
            $ldap = $this->config->get_ldap_link();
          }

          $br = $this->getBranches();

          if(isset($br[$bb]) && preg_match("/d/",$this->ui->get_permissions($this->acl_base,"fai/faiManagement"))){
            $name = $br[$bb];

            $ldap->cd($bb);
            $ldap->recursive_remove();
            $ldap->cd(preg_replace('/,'.preg_quote(get_ou('faiBaseRDN'), '/').'/', ','.get_ou('applicationRDN'), $bb));
            $ldap->recursive_remove();
            $ldap->cd(preg_replace('/,'.preg_quote(get_ou('faiBaseRDN'), '/').'/', ','.get_ou('mimetypeRDN'), $bb));
            $ldap->recursive_remove();

            /* Search for all groups with configured application menus.
              - First search all groups, to ensure that we only remove entries form whithin groups. 
              - The search für menu configuration for the specified release and collect all those dns.
              - Remove entries
             */
            $release_ou = preg_replace("/".preg_quote(get_ou("faiBaseRDN"), '/').".*$/","",$bb);
            $ldap->cd($this->config->current['BASE']);
            $ldap->search("(objectClass=posixGroup)",array("dn"));
          
            /* Collect all group dns 
             */
            $groups = array();
            while($attrs = $ldap->fetch()){
              $groups[] = $attrs['dn'];
            }

            /* Collect all group menu release dns that match the release we have removed 
             */
            $dns = array();
            foreach($groups as $dn){
              $ldap->cd($dn);
              $ldap->search("(objectClass=FAIbranch)",array("dn"));
              while($attrs = $ldap->fetch()){
                if(preg_match("/^".preg_quote($release_ou, '/')."/",$attrs['dn'])){
                  $dns[] = $attrs['dn'];
                }
              }
            }
            
            /* Finally remove collected release dns 
             */
            foreach($dns as $dn){
              $ldap->cd($dn);
              $ldap->recursive_remove();
            }

            /* Post remove */
            $this->fai_release = $this->fai_base;
            $this->lock_name   = $name;
            $this->lock_dn     = $bb;
            $this->postremove();

            $fai_filter = session::get("fai_filter");
            $fai_filter['fai_release'] = $this->fai_release;
            session::set("fai_filter",$fai_filter);

            new log("remove","fai/".get_class($this),$br[$bb],array(),"Release removed");
          }
        }
      }
    }


    /****************
      Create a new branch "insert Name"
     ****************/

    if((isset($_POST['UseBranchName']))&&(($this->dispNewBranch)||($this->dispNewFreeze))){
      session::set('LASTPOST',$_POST);

      if($this->dispNewBranch){
        $type = "branch";
      }else{
        $type = "freeze";
      }

      /* Check branch name */
      $name = $_POST['BranchName'];
      $is_ok = true;
      $smarty->assign("BranchName",$name);
      $base= $this->fai_base;

      /* Check used characters */
      if(!preg_match("/^[0-9a-z\.]*$/",$name)){
        msg_dialog::display(_("Error"), msgPool::invalid(_("Name"),$name,"/[0-9a-z\.]/"), ERROR_DIALOG);
        $is_ok = false;
      }

      /* Check if this name is already in use */
      if(!$this->CheckNewBranchName($_POST['BranchName'],$this->fai_release)){
        msg_dialog::display(_("Error"), msgPool::duplicated(_("Name")), ERROR_DIALOG);
        $is_ok = false;
      }

      if($is_ok){

        if(session::is_set('LASTPOST')){
          $LASTPOST = session::get('LASTPOST');
        }else{
          $LASTPOST = array();
        }
        $LASTPOST['base'] = $base;
        $LASTPOST['type'] = $type;
        session::set('LASTPOST',$LASTPOST);
        $smarty->assign("iframe", true);
        $smarty->assign("plugID", $_GET['plug']);
        $display 	= $smarty->fetch(get_template_path('faiNewBranch.tpl', TRUE, dirname(__FILE__)));
        return($display);
      }
    }


    /****************
      Create a new branch 
     ****************/

    if(isset($_GET['PerformBranch'])){
    
      if(!preg_match("/c/",$this->ui->get_permissions($this->acl_base,"fai/faiManagement"))){
        msg_dialog::display(_("Permission error"), msgPool::permCreate(_("Branch")), ERROR_DIALOG);
      }else{

        /* In order to see error messages we have to reset the error handler.
            Due to the exit(); 
         */
        restore_error_handler();

        /* Create it know */
        $this->dispNewBranch = false;
        $this->dispNewFreeze = false;

        $LASTPOST = session::get('LASTPOST');
        $base = $LASTPOST['base'];
        $_POST  = session::get('LASTPOST');      
        $name = $_POST['BranchName'];

        $type = $LASTPOST['type'];
        $ldap = $this->config->get_ldap_link();

        $baseToUse = $base;
        if($this->fai_release !=  $this->fai_base){
          $baseToUse = $this->fai_release;
        }

        /* Create new Release name to be able to set faidebianRelease for FAIpackageList */

        $CurrentReleases  = $this->getBranches();
        $NewReleaseName   = $name;
        if(isset($CurrentReleases[$this->fai_release])) {
          if($this->fai_release != $this->fai_base){
            $NewReleaseName = $CurrentReleases[$this->fai_release]."/".$name;
            $NewReleaseName = preg_replace("#\/#","/",$NewReleaseName); 
          }else{
            $NewReleaseName   = $name;
          }
        }

        $appsrc = preg_replace("/".preg_quote(get_ou('faiBaseRDN'), '/')."/",get_ou('applicationRDN'),$baseToUse); 
        $appdst = preg_replace("/".preg_quote(get_ou('faiBaseRDN'), '/')."/",get_ou('applicationRDN'),"ou=".$name.",".$baseToUse) ; 

        $mimesrc = preg_replace("/".preg_quote(get_ou('faiBaseRDN'), '/')."/",get_ou('mimetypeRDN'),$baseToUse); 
        $mimedst = preg_replace("/".preg_quote(get_ou('faiBaseRDN'), '/')."/",get_ou('mimetypeRDN'),"ou=".$name.",".$baseToUse) ; 

        /* Check if source depeartments exist */
        foreach(array($baseToUse,$appsrc,$mimesrc) as $dep){
          $ldap->cd($this->config->current['BASE']);
          $ldap->cat($dep);
          if(!$ldap->count()){
            $ldap->create_missing_trees($dep);
          }
        }

        /* Print header to have styles included */
        echo '  <!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 3.2//EN">
          <html>
          <head>
          <title></title>
          <style type="text/css">@import url("themes/default/style.css");</style>
          <script language="javascript" src="include/focus.js" type="text/javascript"></script>
          </head>
          <body style="background: none;margin:3px;color:black">
          ';

        new log("create","fai/".get_class($this),$NewReleaseName,array(),"New $type created");

        /* Duplicate group application releases 
         */
        FAI::copy_FAI_group_releases($CurrentReleases[$this->fai_release],$name,$type);

        /* Duplicate applications 
         */
        $ldap->cat($appsrc,array("dn")) ;
        if($ldap->count()){
          $ldap->cd ($appdst);
          $ldap->recursive_remove();
          FAI::copy_FAI_resource_recursive($appsrc,$appdst,$NewReleaseName,$type,true);
        }

        /* Duplicate mime types 
         */
        $ldap->cat($mimesrc,array("dn")) ;
        if($ldap->count()){
          $ldap->cd ($mimedst);
          $ldap->recursive_remove();
          FAI::copy_FAI_resource_recursive($mimesrc,$mimedst,$NewReleaseName,$type,true);
        }

        $attr = array();
        $attr['objectClass'] = array("organizationalUnit","FAIbranch");
        $attr['ou']       = $name;
        $attr['FAIstate'] = $type;
        $ldap->cd($this->config->current['BASE']);
        $ldap->cd("ou=".$name.",".$baseToUse);
        $ldap->cat("ou=".$name.",".$baseToUse);
        if($ldap->count()){
          $ldap->modify($attr);
        }else{
          $ldap->add($attr);
        }

        /* Duplicate fai objects 
         */
        //      $ldap->cd ("ou=".$name.",".$baseToUse);
        //      $ldap->recursive_remove();
        //      FAI::copy_FAI_resource_recursive($baseToUse,"ou=".$name.",".$baseToUse,$NewReleaseName,$type,true);

        echo "<div style='width:100%;text-align:right;'><form name='form' method='post' action='?plug=".$_GET['plug']."' target='_parent'>
          <br><input type='submit' name='CloseIFrame' value='"._("Continue")."'>
          <input type='hidden' name='php_c_check' value='1'>
          </form></div>";

        echo "<script language=\"javascript\" type=\"text/javascript\">scrollDown2();</script>" ;

        /* Print footer to have valid html */
        echo "</body></html>";

        $this->dispNewFreeze = false; 

        /* Postcreate */ 

        /* Assign possible attributes */
        $this->lock_type  = $type; 
        $this->lock_name  = $name; 
        $this->lock_dn    = $baseToUse;
        $this->postcreate();


        /* Send daemon event to reload the fai release database 
         */
        if(class_available("DaemonEvent") && class_available("gosaSupportDaemon")){
          $events = DaemonEvent::get_event_types(SYSTEM_EVENT | HIDDEN_EVENT);
          if(isset($events['TRIGGERED']['DaemonEvent_recreate_fai_release_db'])){
            $evt = $events['TRIGGERED']['DaemonEvent_recreate_fai_release_db'];
            $tmp = new $evt['CLASS_NAME']($this->config);
            $tmp->set_type(TRIGGERED_EVENT);
            $tmp->add_targets(array("GOSA"));
            $o_queue = new gosaSupportDaemon();
            if(!$o_queue->append($tmp)){
              msg_dialog::display(_("Service infrastructure"),msgPool::siError($o_queue->get_error()),ERROR_DIALOG);
            }
          }
        }else{  
          trigger_error("Unknown class DaemonEvent / gosaSupportDaemon");
          msg_dialog::display(_("Fatal error"),
              "Daemon events called but classes where not accessible, DaemonEvent gosaSupportDaemon",
              FATAL_ERROR_DIALOG);
        }
        exit();
      }
    }

    /****************
      Display dialog to enter new Branch name
     ****************/

    /* Check if we have a post create method configured
     *  else skip this operation. (Skip:Button in the ui should be disabled in this case too)
     */
    if("" != $this->config->search("faiManagement", "POSTCREATE",array('menu','tabs'))){
      if(($s_action == "branch_branch")||($this->dispNewBranch)){
        if(!preg_match("/c/",$this->ui->get_permissions($this->acl_base,"fai/faiManagement"))){
          msg_dialog::display(_("Permission error"), msgPool::permCreate(_("Branch")), ERROR_DIALOG);
        }else{
          $this->dispNewBranch=true;
          $smarty->assign("iframe",false);
          $display 	.= $smarty->fetch(get_template_path('faiNewBranch.tpl', TRUE, dirname(__FILE__)));
          return($display);
        }
      } 
    }

 
    /****************
      Display dialog to enter new Freeze name
     ****************/

    /* Check if we have a post create method configured
     *  else skip this operation. (Skip:Button in the ui should be disabled in this case too)
     */
    if("" != $this->config->search("faiManagement", "POSTCREATE",array('menu','tabs'))){
      if(($s_action == "freeze_branch")||($this->dispNewFreeze)){
        if(!preg_match("/c/",$this->ui->get_permissions($this->acl_base,"fai/faiManagement"))){
          msg_dialog::display(_("Permission error"), msgPool::permCreate(_("Branch")), ERROR_DIALOG);
        }else{
          $this->dispNewFreeze = true;
          $smarty->assign("iframe",false);
          $display 	.= $smarty->fetch(get_template_path('faiNewBranch.tpl', TRUE, dirname(__FILE__)));
          return($display);
        }
      }
    }


    /****************
      Create a new object 
     ****************/

    $types = array( "new_partition"     =>  "FAIpartitionTable",
                    "new_script"        =>  "FAIscript",
                    "new_hook"          =>  "FAIhook",
                    "new_variable"      =>  "FAIvariable",
                    "new_template"      =>  "FAItemplate",
                    "new_package"       =>  "FAIpackageList");
    $types_i18n = array( "new_partition"     =>  _("partition table"),
                    "new_script"        =>  _("script"),
                    "new_hook"          =>  _("hook"),
                    "new_variable"      =>  _("variable"),
                    "new_template"      =>  _("template"),
                    "new_package"       =>  _("package list"));

    if(isset($types[$s_action])){

      $type_acl_mapping = array(
          "FAIpartitionTable"  => "faiPartitionTable", 
          "FAIpackageList"     => "faiPackage",
          "FAIscript"          => "faiScript",
          "FAIvariable"        => "faiVariable",
          "FAIhook"            => "faiHook",
          "FAIprofile"         => "faiProfile",
          "FAItemplate"        => "faiTemplate");

      $acl = $this->ui->get_permissions($this->acl_base,"fai/".$type_acl_mapping[$types[$s_action]]);
      if(preg_match("/c/",$acl)){
        $this->dialog = new askClassName($this->config,$this->dn,$this->ui,$types[$s_action]);
        $this->dialog->parent = &$this;
      }else{
        msg_dialog::display(_("Permission error"), sprintf(_("You have no permission to create a new %s!"), $types_i18n[$s_action]), ERROR_DIALOG);
      }
    }

    /* New Profile */
    if($s_action == "new_profile"){
      $this->dn = "new" ;

      $acl = $this->ui->get_permissions($this->acl_base,"fai/faiProfile");
      if(preg_match("/c/",$acl)){
        $a_setup= $this->get_type(array("objectClass"=>array("FAIprofile")));
        $this->dialog = new $a_setup[0]($this->config,$this->config->data['TABS'][$a_setup[2]],$this->dn,"fai");
        $this->dialog->set_acl_base($this->acl_base);
        $this->dialog->parent = &$this;

        $this->is_dialog = false;
      }else{
        msg_dialog::display(_("Permission error"), sprintf(_("You have no permission to create a new %s!"), _("profile")), ERROR_DIALOG);
      }
    }


    /****************
      Get name from ask_class_name dialog 
     ****************/

    if($s_action == "select_class_name_finished"){
      $this->dialog->save_object();
      if(count($this->dialog->check())!=0){
        foreach($this->dialog->check() as $msg){
          msg_dialog::display(_("Error"), $msg, ERROR_DIALOG);
        }		
      }elseif(isset($this->dialog->objectClass)){
        $this->dn = "new" ;
        $a_setup= $this->get_type(array("objectClass"=>array($this->dialog->objectClass)));
        $name = $this->dialog->save();

        if(class_exists($a_setup[0])){
          $this->dialog = new $a_setup[0]($this->config,$this->config->data['TABS'][$a_setup[2]],$this->dn,"fai");
          $this->dialog->set_acl_base($this->acl_base);
          $this->dialog->by_object[$a_setup[1]]->cn = $name;
          $this->dialog->parent = &$this;
          $this->is_dialog = true;
        }
      }		
    }	


    /****************
     Cancel dialogs 
     ****************/

		if(isset($_POST['edit_cancel'])){
			$this->dialog=FALSE;
			$this->is_dialog = false;
			set_object_info();
      $this->remove_lock();
		}


    /****************
     Handle opsi dialogs  
     ****************/
      
    if($this->dialog instanceof tabs_opsiProdConfig && isset($_POST['cancel_properties'])){
      $this->dialog = NULL;
    }
    if($this->dialog instanceof tabs_opsiProdConfig && isset($_POST['save_properties'])){
      $this->dialog->save_object();
      $op    = $this->dialog->by_object['opsiProperties'];
      $name  = $op->get_product();
      $cfg   = $op->get_cfg();
      $this->opsi->set_product_properties($name,$cfg);
      if($this->opsi->is_error()){
        msg_dialog::display(_("Error"),msgPool::siError($this->opsi->get_error()),ERROR_DIALOG);
      }else{
        $this->dialog = NULL;
      }
    }
    if($this->dialog instanceof tabs_opsiProdConfig){
      $this->dialog->save_object();
      return($this->dialog->execute());
    }


    /****************
      Save sub dialogs 
     ****************/

		/* This check if the given tab could be saved 
		 * If it was possible to save it, remove dialog object. 
		 * If it wasn't possible, show errors and keep dialog.
		 */
		if((isset($_POST['edit_finish']) || isset($_POST['edit_apply'])) && (isset($this->dialog->config))){
			$this->dialog->save_object();
      $msgs= $this->dialog->check();
			if(count($msgs)!=0){
				foreach($msgs as $msg){
          msg_dialog::display(_("Error"), $msg, ERROR_DIALOG);
				}
			}else{
				$this->dialog->save();
        FAI::save_release_changes_now();
        $to_del = FAI::clean_up_releases($this->dn, false);
        $ldap = $this->config->get_ldap_link();
        $ldap->cd($this->dn);
        foreach($to_del as $dn){
          /* Only delete subobjects of the current FAI class */
          $ldap->cd($this->dn);
          $ldap->search("(|(objectClass=FAIclass)(objectClass=FAIdebconfInfo))", array('dn'));
          if ($ldap->count()){
            while($attrs = $ldap->fetch()){
              if ($attrs['dn'] == $dn) {
                $ldap->rmdir_recursive($dn);
              }
            }
          }
        }
        if (!isset($_POST['edit_apply'])){
          $this->remove_lock();
          $this->dialog=FALSE;
          $this->is_dialog=false;
          set_object_info();
        }else{

          /* Reinitialize tab */
          if($this->dialog instanceof tabs){
            $this->dialog->re_init();
          }
        }
			}
		}


    /****************
      Display currently open dialog 
     ****************/

		/* If dialog is set, but $this->is_dialog==false, then 
		 *  only the "abort" button is shown, this are dialogs that must not be saved.  
		 * If is_dialog == true, we are currently editing tab objects.
		 *  Here we need both, save and cancel
		 */ 

		if(is_object($this->dialog)){
			$display .= $this->dialog->execute();
			/* Don't show buttons if tab dialog requests this */

      if(isset($this->dialog->current)){

        $obj = $this->dialog->by_object[$this->dialog->current];

        if(($this->dialog instanceOf tabs || $this->dialog instanceOf plugin) && $this->dialog->read_only == TRUE){
          $display.= "<p style=\"text-align:right\">
            <input type=submit name=\"edit_cancel\" value=\"".msgPool::cancelButton()."\">
            </p>";
        }elseif((isset($obj->is_dialog) && (!$obj->is_dialog)) || (isset($obj->dialog) && (!$obj->dialog))){

          $display.= "<p style=\"text-align:right\">\n";
          if(!$this->no_save){
            $display.= "<input type=\"submit\" name=\"edit_finish\" style=\"width:80px\" value=\"".msgPool::okButton()."\">\n";
            $display.= "&nbsp;\n";
            if ($this->dn != "new"){
              $display.= "<input type=submit name=\"edit_apply\" value=\"".msgPool::applyButton()."\">\n";
              $display.= "&nbsp;\n";
            }
          }
          $display.= "<input type=\"submit\" name=\"edit_cancel\" value=\"".msgPool::cancelButton()."\">\n";
          $display.= "</p>";
        }elseif(!isset($this->dialog->current)){
          $display.= "<p style=\"text-align:right\">\n";
          $display.= "<input type=\"submit\" name=\"edit_continue\" value=\""._("Continue")."\">&nbsp;";
          $display.= "<input type=\"submit\" name=\"edit_cancel\" value=\"".msgPool::cancelButton()."\">\n";
          $display.= "</p>";
        }
      }else{
        $display.= "<p style=\"text-align:right\">\n";
        $display.= "<input type=\"submit\" name=\"edit_continue\" value=\""._("Continue")."\">&nbsp;";
        $display.= "<input type=\"submit\" name=\"edit_cancel\" value=\"".msgPool::cancelButton()."\">\n";
        $display.= "</p>";

      }
      return($display);
		}
		

    /****************
      Dialog display
     ****************/

    /* Check if there is a snapshot dialog open */
    $base = $this->fai_base;
    if($str = $this->showSnapshotDialog($base,$this->get_used_snapshot_bases(),$this)){
      return($str);
    }

    /* Display dialog with system list */
    $this->reload();
    $this->DivListFai->parent = &$this;
    $this->DivListFai->execute();
    $this->DivListFai->setEntries($this->objects);
    return($this->DivListFai->Draw());
	}


  /* Return departments, that will be included within snapshot detection */
  function get_used_snapshot_bases()
  {
    $tmp = array();
    $types = array("faiPartitionRDN","faiScriptRDN","faiTemplateRDN","faiHookRDN","faiProfileRDN","faiVariableRDN","faiPackageRDN");
    foreach($types as $type){
      $tmp[] = get_ou($type).$this->fai_release;
    }
    return($tmp);
  }


  /* Get available branches for current base */
  function getBranches($base = false,$prefix = "")
  {
    $ret = array("/"=>$this->fai_base);
    $ldap = $this->config->get_ldap_link();
    if(!$base){
      $base = $this->fai_base;
    }
    $tmp = FAI::get_all_releases_from_base($base,true);
    foreach($tmp as $dn => $name){
      $ret[$name]=$dn;
    }
    ksort($ret);
    $ret = array_flip($ret);

    return ($ret);
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


  /* reload list of objects */
  function reload()
  {
    /* Variable initialisation */
    $str            = "";
    $Regex          = $this->DivListFai->Regex;
    $this->objects  = array();

    /* Get base */
    $base = $this->fai_base;
    if($this->fai_release != $this->fai_base){
      $br = $this->getBranches();
      if(isset($br[$this->fai_release])){
        $base = $this->fai_release;
      }else{
        $base = $this->fai_base;
      }
    }
    $this->base = $base;
    $this->set_acl_base($this->acl_base);

    $this->lock_type = FAI::get_release_tag(FAI::get_release_dn($base));

    /* Create a new list of FAI object 
     * Generate List of Partitions,Hooks,Scripts,Templates,Profiles ... 
     */
    $ObjectTypes = array(
        "FAIpartitionTable"  => array("OU"=> get_ou('faiPartitionRDN') , "CHKBOX"=>"ShowPartitions"  ,"ACL" => "faiPartitionTable"),
        "FAIpackageList"     => array("OU"=> get_ou('faiPackageRDN')   , "CHKBOX"=>"ShowPackages"    ,"ACL" => "faiPackage"),
        "FAIscript"          => array("OU"=> get_ou('faiScriptRDN')    , "CHKBOX"=>"ShowScripts"     ,"ACL" => "faiScript"),
        "FAIvariable"        => array("OU"=> get_ou('faiVariableRDN')  , "CHKBOX"=>"ShowVariables"   ,"ACL" => "faiVariable"),
        "FAIhook"            => array("OU"=> get_ou('faiHookRDN')      , "CHKBOX"=>"ShowHooks"       ,"ACL" => "faiHook"),
        "FAIprofile"         => array("OU"=> get_ou('faiProfileRDN')   , "CHKBOX"=>"ShowProfiles"    ,"ACL" => "faiProfile"),
        "FAItemplate"        => array("OU"=> get_ou('faiTemplateRDN')  , "CHKBOX"=>"ShowTemplates"   ,"ACL" => "faiTemplate"));

    $filter = "";
    foreach($ObjectTypes as $key => $data){
      if($this->DivListFai->$data['CHKBOX']){
        $filter.= "(objectClass=".$key.")";
      }
    }
    $filter = "(&(|".$filter.")(cn=$Regex))";
    
    /* Get resolved release dependencies */
    $tmp = FAI::get_all_objects_for_given_base($base,$filter);

    /* Ge listed ldap objects */
    $ldap = $this->config->get_ldap_link();
    $ldap->cd($this->config->current['BASE']);

    foreach($tmp as $entry){

      /* Get some more informations about the object */ 
      $ldap->cat($entry['dn'], array("cn","description","objectClass","FAIclass","FAIstate","objectClass"));
      $object  = $ldap->fetch();

      /* Walk through possible types */
      foreach($ObjectTypes as $type => $rest){  

        $acl = $this->ui->get_permissions($object['dn'],"fai/".$rest ['ACL']);

        if(in_array($type,$object['objectClass']) && preg_match("/r/",$acl)){

          /* Prepare object */
          unset($object['objectClass']['count']);
          if(!isset($object['description'][0])){
            $object['description'][0]="";
          }

          /* Clean up object informations */
          $obj                  = array();
          $obj['cn'] 			      = $object['cn'][0];
          $obj['dn'] 			      = $object['dn'];
          $obj['acl'] 			    = $acl;
          $obj['class'] 			  = $rest ['ACL'];
          $obj['FAIstate']      = $entry['FAIstate'];
          $obj['description']   = $object['description'][0];
          $obj['objectClass']   = $object['objectClass'];

          $this->objects[$obj['cn']][$type] = $obj;
          $this->objects[$obj['cn']][$type]['type']=$type;
        }
			}
		}

    /*  Append opsi objects, if opsi is available and if we are on the fai_base
     */
    if($this->opsi instanceof opsi && $this->opsi->enabled()){  
      $opsi_acl = $this->ui->get_permissions($base,"opsi/opsiProperties");
      if(preg_match("/r/",$opsi_acl)){
        $err = FALSE;
        if(!$err && $this->DivListFai->ShowOpsiNetboot){
          $n_pro = $this->opsi->get_netboot_products();
          $err |= $this->opsi->is_error();
          foreach($n_pro as $name => $data){
            $entry = array(
                "cn" => $name,
                "description" => $data['DESC'],
                "type" => "opsi_netboot");
            $this->objects[$name]['opsi_netboot'] = $entry;
          }  
        }
        if(!$err && $this->DivListFai->ShowOpsiLocal){
          $l_pro = $this->opsi->get_local_products();
          $err |= $this->opsi->is_error();
          foreach($l_pro as $name => $data){
            $entry = array("cn" => $name,
                "description" => $data['DESC'],
                "type" => "opsi_local");
            $this->objects[$name]["opsi_local"] = $entry;
          }  
        }  
        if($err){
          msg_dialog::display(_("Error"),msgPool::siError($this->opsi->get_error()),ERROR_DIALOG);
        }
      }
    }
 
    uksort($this->objects, 'strnatcasecmp');
		reset ($this->objects);
		$this->objects = array_values($this->objects);
	}

	function remove_lock()
	{
		if (isset($this->dn) && !empty($this->dn)){
			del_lock ($this->dn);
		}
    if(isset($this->dns) && is_array($this->dns) && count($this->dns)){
      del_lock ($this->dns);
    }
	}

	function get_type($array){
    if(!isset($array['objectClass'])) return(array());
		if(in_array("FAIpartitionTable",$array['objectClass'])){
			return(array("tabsPartition","faiPartitionTable","FAIPARTITIONTABS"));
		}
		if(in_array("FAIscript",$array['objectClass'])){
			return(array("tabsScript","faiScript","FAISCRIPTTABS"));
		}
		if(in_array("FAItemplate",$array['objectClass'])){
			return(array("tabsTemplate","faiTemplate","FAITEMPLATETABS"));
		}
		if(in_array("FAIhook",$array['objectClass'])){
			return(array("tabsHook","faiHook","FAIHOOKTABS"));
		}
		if(in_array("FAIvariable",$array['objectClass'])){
			return(array("tabsVariable","faiVariable","FAIVARIABLETABS"));
		}
		if(in_array("FAIprofile",$array['objectClass'])){
			return(array("tabsProfile","faiProfile","FAIPROFILETABS"));
		}
		
		if(in_array("FAIpackageList",$array['objectClass'])){
			return(array("tabsPackage","faiPackage","FAIPACKAGETABS"));
		}
    return(array());
	}

  function CheckNewBranchName($name,$base)
  {
    $f = $this->fai_release;
    if($name == ""){
      return(false);
    }elseif(in_array($name,$this->getBranches($f))) {
      return(false);
    }elseif(tests::is_department_name_reserved($name,$base)){
      return(false);
    }
    return(true);
  }

  function save_object()
  {
    $this->DivListFai->save_object();

    /* Get posted release */
    $r_releases = array_flip($this->getBranches());
    if(isset($_POST['fai_release']) && isset($r_releases[get_post('fai_release')])){

      /* Ensure that we have a valid release selected */
      if(!isset($r_releases[get_post('fai_release')])){
        msg_dialog::display(_("Warning"),_("The selected release is not available anymore. All triggered actions are skipped."));
        $_POST = array();
        $plug =$_GET['plug'];
        $_GET  = array("plug" => $plug);
        $this->fai_release = $this->fai_base;
      }else{
        $this->fai_release = $r_releases[get_post('fai_release')];
      }

      $fai_filter = session::get("fai_filter");
      $fai_filter['fai_release'] = $this->fai_release;
      session::set("fai_filter",$fai_filter);
    }

    if(is_object($this->CopyPasteHandler)){
      $this->CopyPasteHandler->save_object();
    }
  }


  function copyPasteHandling_from_queue($s_action,$s_entry)
  {
    /* Check if Copy & Paste is disabled */
    if(!is_object($this->CopyPasteHandler)){
      return("");
    }

    $ui = get_userinfo();

    /* Add a single entry to queue */
    if($this->dialog instanceOf faiGroupHandle && !$this->dialog->is_open()){

      if(in_array($this->dialog->get_mode(),array("copy"))){      

        /* Cleanup object queue */
        $this->CopyPasteHandler->cleanup_queue();
        $group = $this->dialog->get_selected();
        foreach($group as $entry){
          $a_setup  = $this->get_type($entry);
          $dn = $entry['dn'];
          if($ui->is_copyable($dn,"fai",$a_setup[1])){
            $this->CopyPasteHandler->add_to_queue($dn,$this->dialog->get_mode(),$a_setup[0],$a_setup[2],"fai");//$a_setup[1]);
          }
        }
        $this->dialog = FALSE;
      }
    }

    /* Add entries to queue */
    if($s_action == "copy_multiple"){

      /* Cleanup object queue */
      $this->CopyPasteHandler->cleanup_queue();

      /* Add new entries to CP queue */
      foreach($this->list_get_selected_items() as $id){
        $group    = $this->objects[$id];
        foreach($group as $entry){
          $a_setup  = $this->get_type($entry);
          $dn = $entry['dn'];

          if($s_action == "copy_multiple" && $ui->is_copyable($dn,"fai",$a_setup[1])){
            $this->CopyPasteHandler->add_to_queue($dn,"copy",$a_setup[0],$a_setup[2],"fai");//$a_setup[1]);
          }
        }
      }
    }

    /* Start pasting entries */
    if($s_action == "editPaste" && !($this->lock_type == "freeze" && !$this->allow_freeze_object_attach)){
      $this->start_pasting_copied_objects = TRUE;
    }

    /* Return C&P dialog */
    if($this->start_pasting_copied_objects && $this->CopyPasteHandler->entries_queued()){

      /* Get dialog */
      $this->CopyPasteHandler->SetVar("parent",$this);
      $data = $this->CopyPasteHandler->execute();

      FAI::save_release_changes_now();

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


  /* Check if the given FAI class is used in this release 
   */
  static function check_class_name($oc,$name,$dn)
  {
    $base = FAI::get_release_dn($dn);

    if($oc == "FAIprofile"){
      $f = "";
      $ocs = array("FAIprofile","FAItemplate","FAIhook","FAIpartitionTable","FAIpackageList","FAIscript","FAIvariable");
      foreach($ocs as $oc){
        $f .= "(objectClass=".$oc.")";
      } 
      $res  = FAI::get_all_objects_for_given_base($base,"(|".$f.")",TRUE);    
    }else{
      $res  = FAI::get_all_objects_for_given_base($base,"(|(objectClass=FAIprofile)(objectClass=".$oc."))",TRUE);
    }
    $delete = array();
    $used   = array();
    foreach($res as $object){
      $used[$object['cn'][0]]= $object['cn'][0];
    }
    return($used);
  }


  /* Return plugin informations for acl handling */ 
  static function plInfo()
  {
    return (array( 
          "plShortName"   => _("FAI releases"),
          "plDescription" => _("FAI release management"),
          "plSelfModify"  => FALSE,
          "plDepends"     => array(),
          "plPriority"    => 0,
          "plSection"     => array("administration"),           
          "plCategory"    => array("fai"=> array("description" => _("FAI"),
                                                 "objectClass" => "FAIclass")),
          "plProvidedAcls"=> array()));
  }
}
// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
