<?php
/*
 * This code is part of GOsa (http://www.gosa-project.org)
 * Copyright (C) 2003-2008 GONICUS GmbH
 *
 * ID: $$Id: class_roleManagement.inc 14742 2009-11-04 13:18:33Z hickert $$
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

class faiManagement extends management
{
  var $plHeadline     = "Software deployment";
  var $plDescription  = "Manage software packages and deployment reciepes";
  var $plIcon  = "plugins/fai/images/plugin.png";

  // Tab definition 
  protected $tabClass = "roletabs";
  protected $tabType = "ROLETABS";
  protected $aclCategory = "roles";
  protected $aclPlugin   = "role";
  protected $objectName   = "role";

  //  Attributes Managed by this plugin can be used in post events
  protected $attributes   = array("lock_type","lock_name","lock_dn");  

  var $dispNewBranch=false;
  var $dispNewFreeze=false;

  var $fai_release = ""; // The currently selected release while in release management mode!
  var $fai_base = ""; 
  var $acl_base = ""; 

  var $lock_type = "";
  var $lock_dn = "";
  var $lock_name = "";


  function __construct($config,$ui)
  {
    $this->config = $config;
    $this->fai_base = get_ou("faiBaseRDN").$this->config->current['BASE'];
    $this->fai_release = $this->fai_base;
    $this->acl_base = $this->config->current['BASE'];
    $this->ui = $ui;
    $this->storagePoints = array(
        get_ou('faiPartitionRDN'),
        get_ou('faiPackageRDN'),
        get_ou('faiScriptRDN'),
        get_ou('faiVariableRDN'),
        get_ou('faiHookRDN'),
        get_ou('faiProfileRDN'),get_ou('faiTemplateRDN'));

#    // Build filter
#    if (session::global_is_set(get_class($this)."_filter")){
#      $filter= session::global_get(get_class($this)."_filter");
#    } else {
      $filter = new filter(get_template_path("fai-filter.xml", true));
      $filter->setObjectStorage($this->storagePoints);
#    }
    $this->setFilter($filter);

    // Build headpage
    $headpage = new listing(get_template_path("fai-list.xml", true));
    $headpage->setFilter($filter);
    $headpage->setBase($this->fai_release);
    $headpage->registerElementFilter("filterProperties", "faiManagement::filterProperties");

    // Add copy&paste and snapshot handler.
    if ($this->config->boolValueIsTrue("main", "copyPaste")){
      $this->cpHandler = new CopyPasteHandler($this->config);
    }
    if($this->config->get_cfg_value("enableSnapshots") == "true"){
      $this->snapHandler = new SnapshotHandler($this->config);
    }

    $this->registerAction("new_profile","newEntry");
    $this->registerAction("new_template","newEntry");
    $this->registerAction("new_script","newEntry");
    $this->registerAction("new_hook","newEntry");
    $this->registerAction("new_variable","newEntry");
    $this->registerAction("new_package","newEntry");
    $this->registerAction("new_partition","newEntry");

    $this->registerAction("newClassNameSelected","newClassNameSelected");

    $this->registerAction("editByGroup","editByGroup");
    $this->registerAction("createBranch","createBranch");
    $this->registerAction("createFreeze","createFreeze");
    $this->registerAction("removeBranch","removeBranch");
    $this->registerAction("removeBranchConfirmed","removeBranchConfirmed");
    $this->registerAction("saveBranch","saveBranch");
    $this->registerAction("PerformBranch","PerformBranch");
    parent::__construct($config, $ui, "roles", $headpage);
  }


  function newEntry($action="",$target=array(),$all=array(), $altTabClass ="", $altTabType = "", $altAclCategory="")
  {
    /****************
      Create a new object
     ****************/

    $types = array( 
        "new_partition"     =>  "FAIpartitionTable",
        "new_script"        =>  "FAIscript",
        "new_hook"          =>  "FAIhook",
        "new_variable"      =>  "FAIvariable",
        "new_template"      =>  "FAItemplate",
        "new_package"       =>  "FAIpackageList");
    $types_i18n = array( 
        "new_partition"     =>  _("partition table"),
        "new_script"        =>  _("script"),
        "new_hook"          =>  _("hook"),
        "new_variable"      =>  _("variable"),
        "new_template"      =>  _("template"),
        "new_package"       =>  _("package list"));

    if(isset($types[$action])){
      $type_acl_mapping = array(
          "FAIpartitionTable"  => "faiPartitionTable",
          "FAIpackageList"     => "faiPackage",
          "FAIscript"          => "faiScript",
          "FAIvariable"        => "faiVariable",
          "FAIhook"            => "faiHook",
          "FAIprofile"         => "faiProfile",
          "FAItemplate"        => "faiTemplate");

      $acl = $this->ui->get_permissions($this->acl_base,"fai/".$type_acl_mapping[$types[$action]]);
      if(preg_match("/c/",$acl)){
        $this->dialogObject = new askClassName($this->config,$this->dn,$this->ui,$types[$action]);
        $this->dialogObject->parent = &$this;
      }else{
        msg_dialog::display(_("Permission error"), sprintf(_("You have no permission to create a new %s!"), $types_i18n[$action]),      ERROR_DIALOG);
      }
    }
    if($action == "new_profile"){
      $this->dn = "new" ;

      $acl = $this->ui->get_permissions($this->acl_base,"fai/faiProfile");
      if(preg_match("/c/",$acl)){
        $a_setup= $this->get_type(array("objectClass"=>array("FAIprofile")));
        $this->dialog = new $a_setup[0]($this->config,$this->config->data['TABS'][$a_setup[2]],$this->dn,"fai");
        $this->dialog->set_acl_base($this->base);
        $this->dialog->parent = &$this;

        $this->is_dialog = false;
      }else{
        msg_dialog::display(_("Permission error"), sprintf(_("You have no permission to create a new %s!"), _("profile")), ERROR_DIALOG);
      }
    }
  }


  function newClassNameSelected()
  {
    $this->dialogObject->save_object();
    if(count($this->dialogObject->check())!=0){
      foreach($this->dialogObject->check() as $msg){
        msg_dialog::display(_("Error"), $msg, ERROR_DIALOG);
      }
    }elseif(isset($this->dialogObject->objectClass)){
      $this->dn = "new" ;
      $type= $this->get_type(array("objectClass"=>array($this->dialogObject->objectClass)));
      $name = $this->dialogObject->save();

      if(class_exists($type[0])){
        $this->closeDialogs();
        management::newEntry('newEntry',array(),array(),$type[0],$type[2],$type[1]);
        $this->tabObject->set_acl_base($this->acl_base);
        $this->tabObject->by_object[$type[1]]->cn = $name;
      }
    }
  }



  function editEntry($action="",$target=array(),$all=array(), $altTabClass ="", $altTabType = "", $altAclCategory="")
  {
    $headpage = $this->getHeadpage();
    if(count($target) == 1){
      $entry = $headpage->getEntry($target[0]);
      if($entry){
        if(count($entry['GROUPS']) == 1){
          $data = array_pop($entry['GROUPS']);
          $type = $this->get_type($data);
          management::editEntry('editEntry',array($data['dn']),array(),$type[0],$type[2],$type[1]);
          $this->tabObject->by_object[$type[1]]->FAIstate = $data['FAIstate'];
          $this->tabObject->read_only = preg_match("/freeze/i", $data['FAIstate']);

        }else{
          $this->dialogObject = new faiGroupHandle($entry['GROUPS'],"edit");
        }
      }
    }
  }


  function removeEntryRequested($action="",$target=array(),$all=array(), $altTabClass ="", $altTabType = "", $altAclCategory="")
  {
    $headpage = $this->getHeadpage();
    if(count($target) == 1){
      $entry = $headpage->getEntry($target[0]);
      if($entry){
        $this->dialogObject = new faiGroupHandle($entry['GROUPS'],"remove");
      }
    }
  }

  /*! \brief  Object removal was confirmed, now remove the requested entries.
   *
   *  @param  String  'action'  The name of the action which was the used as trigger.
   *  @param  Array   'target'  A list of object dns, which should be affected by this method.
   *  @param  Array   'all'     A combination of both 'action' and 'target'.
   */
  function removeEntryConfirmed($action="",$target=array(),$all=array(),
      $altTabClass="",$altTabType="",$altAclCategory="")
  {
    $ldap = $this->config->get_ldap_link();
    $ldap->cd($this->config->current['BASE']);  
    foreach($this->dns as $key => $dn){
      $ldap->cat($dn);
      if($ldap->count()){
        $attrs = $ldap->fetch();
        $type= $this->get_type($attrs);
        management::removeEntryConfirmed($action,array($dn),$all,$type[0],$type[2],$type[1]);

        // Now save changes
        FAI::save_release_changes_now();
        $to_del = FAI::clean_up_releases($dn);
        foreach($to_del as $dn){
          $ldap->rmdir_recursive($dn);
        }
      }
    }
  }


  function editByGroup()
  {
    if($this->dialogObject instanceOf faiGroupHandle && $this->dialogObject->get_mode() == "edit"){
      $this->dialogObject->save_object();
      $entry = $this->dialogObject->get_selected();
      $this->closeDialogs();
      $data = array_pop($entry);
      $type = $this->get_type($data);
      management::editEntry('editEntry',array($data['dn']),array(),$type[0],$type[2],$type[1]);
      $this->tabObject->by_object[$type[1]]->FAIstate = $data['FAIstate'];
      $this->tabObject->read_only = preg_match("/freeze/i", $data['FAIstate']);

    }elseif($this->dialogObject instanceOf faiGroupHandle && $this->dialogObject->get_mode() == "remove"){
      $this->dialogObject->save_object();
      $to_delete = $entry = $this->dialogObject->get_selected();
      $dns = array();


      // Check FAIstate to ensure that we do not remove frozen objects
      //  additionally check ACLs
      $locked = array();
      $disallowed = array();
      foreach($to_delete as $obj){
        if(isset($obj['FAIstate']) && preg_match('/^freeze/', $obj['FAIstate'])){
          $locked[] = $obj['dn'];
        }else{
          $type = $this->get_type($obj);
          $acl = $this->ui->get_permissions($obj['dn'], 'acl/'.$type[1]);
          if(!preg_match("/d/",$acl)){
            $disallowed[] = $obj['dn']; 
          }else{
            $dns[] = $obj['dn'];
          }
        }
      }

      // Display info dialog about locked and not removeable entries
      if(count($locked)){
        msg_dialog::display(_("Branch locked"),sprintf(_("The following entries are locked, you can't remove them %s."),
            msgPool::buildList($locked)));
      }

      // Display info dialog about not removeable entries due to permissions
      if(count($disallowed)){
        msg_dialog::display(_("Permission"),msgPool::permDelete($disallowed),INFO_DIALOG);
      }

      // There are entries left to be removed
      if(count($dns)){  
        $this->closeDialogs();
        $this->dns = $dns;

        @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__,$target,"Entry removel requested!");

        // check locks
        if ($user= get_multiple_locks($this->dns)){
          return(gen_locked_message($user,$this->dns));
        }

        // Add locks
        $dns_names = array();
        foreach($this->dns as $dn){
          $dns_names[] =LDAP::fix($dn);
        }
        add_lock ($this->dns, $this->ui->dn);

        // Display confirmation dialog.
        $smarty = get_smarty();
        $smarty->assign("info", msgPool::deleteInfo($dns_names,_($this->objectName)));
        $smarty->assign("multiple", true);
        return($smarty->fetch(get_template_path('remove.tpl', TRUE)));
      }else{
        $this->closeDialogs();
      }
    }
  }

  
  protected function saveChanges()
  {
    management::saveChanges();

    // Now save changes
    FAI::save_release_changes_now();
    $to_del = FAI::clean_up_releases($this->last_dn);
    foreach($to_del as $dn){
      $ldap->rmdir_recursive($dn);
    }
  }

  protected function applyChanges()
  {
    management::applyChanges();

    // Now save changes
    FAI::save_release_changes_now();
    $to_del = FAI::clean_up_releases($this->last_dn);
    foreach($to_del as $dn){
      $ldap->rmdir_recursive($dn);
    }
  }

  function detectPostActions()
  {
    $action = management::detectPostActions();
    if(isset($_POST['new_profile'])) $action['action'] = "new_profile";
    if(isset($_POST['new_template'])) $action['action'] = "new_template";
    if(isset($_POST['new_script'])) $action['action'] = "new_script";
    if(isset($_POST['new_hook'])) $action['action'] = "new_hook";
    if(isset($_POST['new_variable'])) $action['action'] = "new_variable";
    if(isset($_POST['new_package'])) $action['action'] = "new_package";
    if(isset($_POST['new_partition'])) $action['action'] = "new_partition";

    if(isset($_POST['edit_continue'])) $action['action'] = "newClassNameSelected";
    if(isset($_POST['edit_cancel'])) $action['action'] = "cancel";

    if(isset($_POST['faiGroupHandle_cancel'])) $action['action'] = "cancel";
    if(isset($_POST['CancelBranchName'])) $action['action'] = "cancel";
    if(isset($_POST['delete_branch_confirm'])) $action['action'] = "removeBranchConfirmed";
    if(isset($_GET['PerformBranch'])) $action['action'] = "PerformBranch";
    if(isset($_POST['UseBranchName'])) $action['action'] = "saveBranch";
    if(isset($_POST['faiGroupHandle_apply']))  $action['action'] = "editByGroup";
    if(isset($_GET['act']) && $_GET['act'] == "branch_branch")  $action['action'] = "createBranch";
    if(isset($_GET['act']) && $_GET['act'] == "freeze_branch")  $action['action'] = "createFreeze";
    if(isset($_GET['act']) && $_GET['act'] == "remove_branch")  $action['action'] = "removeBranch";

    foreach($_POST as $name => $value){
      if(preg_match("/^edit_([0-9]*)_([a-z]*)_(x|y)/i", $name)){
        $id = preg_replace("/^edit_([0-9]*)_([a-z]*)_(x|y)/i","\\1", $name);
        $tab = preg_replace("/^edit_([0-9]*)_([a-z]*)_(x|y)/i","\\2", $name);
        $headpage = $this->getHeadpage();
        if(isset($headpage->entries[$id]['GROUPS'][$tab])){
          $data =$headpage->entries[$id]['GROUPS'][$tab];
          $type = $this->get_type($data);
          management::editEntry('editEntry',array($data['dn']),array(),$type[0],$type[2],$type[1]); 
        } 
        break;
      }
    }
    return($action);
  }


  function renderList()
  {
    $filter = $this->getFilter();
    $headpage = $this->getHeadpage();
    $filter->setComboBoxOptions("RELEASE",$this->getReleaseList());

    if(isset($_POST['RELEASE'])){
      $this->fai_release = get_post('RELEASE');
    }
    $headpage->setBase($this->fai_release);
    $headpage->update();
    $smarty = get_smarty();
    $smarty->assign("fai_release", $this->fai_release);
    $smarty->assign("fai_base", $this->fai_base);
    $r = $this->config->search("faiManagement", "POSTREMOVE",array('menu','tabs'));
    $c = $this->config->search("faiManagement", "POSTCREATE",array('menu','tabs'));
    $smarty->assign("allow_create", $c);
    $smarty->assign("allow_remove", $r);
    $display = $headpage->render();
    return($this->getHeader().$display);
  }

  function getReleaseList($base = "", $prefix ="")
  {
    $list = array();
    if(empty($base)){
      $base = $this->fai_base;
      $list[$base] = "/";
    }

    $ldap = $this->config->get_ldap_link();
    $ldap->ls("(objectClass=FAIbranch)",$base,array("ou","FAIstate"));
    $cfg_rel = $this->config->search("faiManagement","DEFAULTFAIRELEASE",array("menu"));

    while($release = $ldap->fetch()){
      $list[$release['dn']] = $prefix.$release['ou'][0];

      // Preset to prefered releaes if necessary  
      if(empty($this->fai_release) && $cfg_rel == $release['dn']){
        $this->fai_release = $release['dn'];
      }

      $list = array_merge($list,$this->getReleaseList($release['dn'],$prefix."&nbsp; "));
    }
    return($list);
  }


  static function filterProperties($row, $classes)
  {
    $objects = array(
        "FAIpartitionTable"  => array("IMG"=> "plugins/fai/images/fai_partitionTable.png",
          "NAME"=>_("Partition table"),"KZL"=> "PT", "VAR"=>"ShowPartitions"),
        "FAIpackageList"     => array("IMG"=> "plugins/fai/images/fai_packages.png",
          "NAME"=>_("Package list") ,  "KZL"=> "PL", "VAR"=>"ShowPackages"),
        "FAIscript"          => array("IMG"=> "plugins/fai/images/fai_script.png",
          "NAME"=>_("Scripts") ,       "KZL"=> "S",  "VAR"=>"ShowScripts"),
        "FAIvariable"        => array("IMG"=> "plugins/fai/images/fai_variable.png",
          "NAME"=>_("Variables") ,     "KZL"=> "V",  "VAR"=>"ShowVariables"),
        "FAIhook"            => array("IMG"=> "plugins/fai/images/fai_hook.png",
          "NAME"=>_("Hooks"),          "KZL"=> "H",  "VAR"=>"ShowHooks"),
        "FAIprofile"         => array("IMG"=> "plugins/fai/images/fai_profile.png",
          "NAME"=>_("Profile") ,       "KZL"=> "P",  "VAR"=>"ShowProfiles"),
        "FAItemplate"        => array("IMG"=> "plugins/fai/images/fai_template.png",
          "NAME"=>_("Templates") ,     "KZL"=> "T",  "VAR"=>"ShowTemplates"),
        "opsi_netboot"       => array("IMG"=> "plugins/opsi/images/netboot_package.png",
          "NAME"=>_("OPSI netboot product") ,     "KZL"=> "ON",  "VAR"=>"ShowOpsiNetboot"),
        "opsi_local"         => array("IMG"=> "plugins/opsi/images/local_package.png",
          "NAME"=>_("OPSI localboot product")   ,     "KZL"=> "OL",  "VAR"=>"ShowOpsiLocal"));

    $icon_list = "";
    foreach($objects as $type => $type_data){
      if(in_array($type, $classes)){
        $icon_list .= "<input type='image' src='".$type_data['IMG']."' title='".$type_data['NAME']."'
          alt='".$type_data['KZL']."' class='center' name='edit_".$row."_".$type."'>\n";
      }else{
        $icon_list .= "<img src='images/empty.png' alt=' ' class='center'>\n";
      }
    }

    return $icon_list;
  }




  function removeBranch()
  {
    /* Check if we have a post remove method configured
     *  else skip this operation. (Skip:Button in the ui should be disabled in this case too)
     */
    if("" != $this->config->search("faiManagement", "POSTREMOVE",array('menu','tabs'))){
      /* Load permissions for selected 'dn' and check if
         we're allowed to remove this 'dn' */
      if(preg_match("/d/",$this->ui->get_permissions($this->acl_base,"fai/faiManagement"))){
        $smarty=get_smarty();
        $smarty->assign("release_hidden",base64_encode($this->fai_release));
        $smarty->assign("info", msgPool::deleteInfo(LDAP::fix($this->fai_release),_("FAI branch/freeze")));
        return($smarty->fetch(get_template_path('remove_branch.tpl',TRUE)));
      } else {
        msg_dialog::display(_("Permission error"), _("You have no permission to delete this release!"), ERROR_DIALOG);
      }
    }
  }


  function removeBranchConfirmed()
  {
    /* Check if we have a post remove method configured
     *  else skip this operation. (Skip:Button in the ui should be disabled in this case too)
     */
    if("" != $this->config->search("faiManagement", "POSTREMOVE",array('menu','tabs'))){

      if(!isset($_POST['release_hidden']) || base64_decode($_POST['release_hidden']) != $this->fai_release){
        msg_dialog::display(_("Warning"),_("Release remove aborted because the release name check failed!"));
      }else{

        $bb =  $this->fai_release;
        $ldap = $this->config->get_ldap_link();

        $br = $this->getBranches();

        if(isset($br[$bb]) && preg_match("/d/",$this->ui->get_permissions($this->acl_base,"fai/faiManagement"))){
          $name = $br[$bb];

          $ldap->cd($bb);
          $ldap->recursive_remove();
          $ldap->cd(preg_replace('/,'.preg_quote(get_ou('faiBaseRDN'), '/').'/i', ','.get_ou('applicationRDN'), $bb));
          $ldap->recursive_remove();
          $ldap->cd(preg_replace('/,'.preg_quote(get_ou('faiBaseRDN'), '/').'/i', ','.get_ou('mimetypeRDN'), $bb));
          $ldap->recursive_remove();

          /* Search for all groups with configured application menus.
             - First search all groups, to ensure that we only remove entries form whithin groups.
             - The search für menu configuration for the specified release and collect all those dns.
             - Remove entries
           */
          $release_ou = preg_replace("/".preg_quote(get_ou("faiBaseRDN"), '/').".*$/i","",$bb);
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


  function createBranch()
  { 
    $smarty = get_smarty();
    $this->dispNewBranch=true;
    $this->dispNewFreeze=false;
    $smarty->assign("iframe",false);
    if(isset($_POST['BranchName'])){
      $smarty->assign("BranchName", $_POST['BranchName']);
    }else{
      $smarty->assign("BranchName","");
    }
    return($smarty->fetch(get_template_path('faiNewBranch.tpl', TRUE, dirname(__FILE__))));
  }

  function createFreeze()
  { 
    $smarty = get_smarty();
    $this->dispNewFreeze=true;
    $this->dispNewBranch=false;
    $smarty->assign("iframe",false);
    if(isset($_POST['BranchName'])){
      $smarty->assign("BranchName", $_POST['BranchName']);
    }else{
      $smarty->assign("BranchName","");
    }
    return($smarty->fetch(get_template_path('faiNewBranch.tpl', TRUE, dirname(__FILE__))));
  }



  function PerformBranch()
  {
    if(!preg_match("/c/",$this->ui->get_permissions($this->acl_base,"fai/faiManagement"))){
      msg_dialog::display(_("Permission error"), msgPool::permCreate(_("Branch")), ERROR_DIALOG);
    }else{

      /* In order to see error messages we have to reset the error handler.
         Due to the exit();
       */
      restore_error_handler();

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
      $appsrc = preg_replace("/".preg_quote(get_ou('faiBaseRDN'), '/')."/i",get_ou('applicationRDN'),$baseToUse);
      $appdst = preg_replace("/".preg_quote(get_ou('faiBaseRDN'), '/')."/i",get_ou('applicationRDN'),"ou=".$name.",".$baseToUse) ;

      $mimesrc = preg_replace("/".preg_quote(get_ou('faiBaseRDN'), '/')."/i",get_ou('mimetypeRDN'),$baseToUse);
      $mimedst = preg_replace("/".preg_quote(get_ou('faiBaseRDN'), '/')."/i",get_ou('mimetypeRDN'),"ou=".$name.",".$baseToUse) ;

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




  function saveBranch()
  {
    if($this->dispNewBranch){
      $type = "branch";
    }else{
      $type = "freeze";
    }

    /* Check branch name */
    $name = $_POST['BranchName'];
    $is_ok = true;
    $smarty = get_smarty();
    $smarty->assign("BranchName",$name);
    $base= $this->fai_base;

    /* Check used characters */
    if(!preg_match("/^[0-9a-z\.]*$/",$name)){
      msg_dialog::display(_("Error"), msgPool::invalid(_("Name"),$name,"/[0-9a-z\.]/"), ERROR_DIALOG);
      $is_ok = false;
    }

    // Check if this name is already in use 
    if(!$this->CheckNewBranchName($_POST['BranchName'],$this->fai_release)){
      msg_dialog::display(_("Error"), msgPool::duplicated(_("Name")), ERROR_DIALOG);
      $is_ok = false;
    }

    // Handle errors
    if(!$is_ok && $this->dispNewFreeze){
      return($this->createFreeze());
    }elseif(!$is_ok && $this->dispNewBranch){
      return($this->createBranch());
    }

    // Now create new release

    if(session::is_set('LASTPOST')){
      $LASTPOST = session::get('LASTPOST');
    }else{
      $LASTPOST = array();
    }
    $LASTPOST['base'] = $base;
    $LASTPOST['type'] = $type;
    $LASTPOST['BranchName'] = $name;
    session::set('LASTPOST',$LASTPOST);
    $smarty->assign("iframe", true);
    $smarty->assign("plugID", $_GET['plug']);
    $display  = $smarty->fetch(get_template_path('faiNewBranch.tpl', TRUE, dirname(__FILE__)));
    return($display);
 
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


  function get_type($array)
  {
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
      $res  = FAI::get_all_objects_for_given_base($base,"(objectClass=".$oc.")",TRUE);
    }
    $delete = array();
    $used   = array();
    foreach($res as $object){
      $used[$object['cn'][0]]= $object['cn'][0];
    }
    return($used);
  }


} 
// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
