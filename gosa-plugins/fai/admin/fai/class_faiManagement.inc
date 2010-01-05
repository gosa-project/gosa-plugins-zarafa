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
    parent::__construct($config, $ui, "roles", $headpage);
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


} 
// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
