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

  var $fai_release = ""; // The currently selected release while in release management mode!

  function __construct($config,$ui)
  {
    $this->config = $config;
    $this->ui = $ui;

    $this->storagePoints = array(
        get_ou('faiPartitionRDN'),
        get_ou('faiPackageRDN'),
        get_ou('faiScriptRDN'),
        get_ou('faiVariableRDN'),
        get_ou('faiHookRDN'),
        get_ou('faiProfileRDN'),get_ou('faiTemplateRDN'));
 
    // Build filter
#    if (session::global_is_set(get_class($this)."_filter")){
 #     $filter= session::global_get(get_class($this)."_filter");
  #  } else {
      $filter = new filter(get_template_path("fai-filter.xml", true));
      $filter->setObjectStorage($this->storagePoints);
   # }
    $this->setFilter($filter);

    // Build headpage
    $headpage = new listing(get_template_path("fai-list.xml", true));
    $headpage->setFilter($filter);
    $headpage->registerElementFilter("filterProperties", "faiManagement::filterProperties");

    // Add copy&paste and snapshot handler.
    if ($this->config->boolValueIsTrue("main", "copyPaste")){
      $this->cpHandler = new CopyPasteHandler($this->config);
    }
    if($this->config->get_cfg_value("enableSnapshots") == "true"){
      $this->snapHandler = new SnapshotHandler($this->config);
    }
    $this->registerAction("editByGroup","editByGroup");
    parent::__construct($config, $ui, "roles", $headpage);
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


  function editByGroup()
  {
    if($this->dialogObject instanceOf faiGroupHandle && $this->dialogObject->get_mode() == "edit"){
      $this->dialogObject->save_object();
      $entry = $this->dialogObject->get_selected();
      $this->closeDialogs();
      $data = array_pop($entry);
      $type = $this->get_type($data);
      management::editEntry('editEntry',array($data['dn']),array(),$type[0],$type[2],$type[1]);
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

  
  function detectPostActions()
  {
    $action = management::detectPostActions();
    if(isset($_POST['faiGroupHandle_cancel'])) $action['action'] = "cancel";
    if(isset($_POST['faiGroupHandle_apply']))  $action['action'] = "editByGroup";
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
    $display = $headpage->render();
    return($this->getHeader().$display);
  }

  function getReleaseList($base = "", $prefix ="")
  {
    $list = array();
    if(empty($base)){
      $base = get_ou('faiBaseRDN').$this->config->current['BASE'];
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
} 
// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
