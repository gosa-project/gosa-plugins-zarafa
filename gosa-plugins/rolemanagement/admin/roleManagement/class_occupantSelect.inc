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


class occupantSelect  extends plugin {
  function __construct($config, $dn, $parent){
    $this->config = $config;
    $this->dn = $dn;
    $this->parent = $parent;
    $this->skipEntries = $parent->getOccupants();

    /* Get global filter config */
    if (!session::is_set("roleAddOccupantFilter")){
      $ui= get_userinfo();
      $base= get_base_from_people($ui->dn);
      $roleAddOccupantFilter= array(  
          "dselect"       => $base,
          "subtrees"       => FALSE,
          "regex"           => "*");
      session::set("roleAddOccupantFilter", $roleAddOccupantFilter);
    }

  }
  

  function execute()
  {
    /* Save data */
    $roleAddOccupantFilter= session::get("roleAddOccupantFilter");
    if(isset($_POST['occupantSearched'])){

      $roleAddOccupantFilter['subtrees'] = false;
      if(isset($_POST['subtrees'])){
        $roleAddOccupantFilter['subtrees'] = "checked";
      }
  
      foreach( array("dselect", "regex") as $type){
        if (isset($_POST[$type])){
          $roleAddOccupantFilter[$type]= $_POST[$type];
        }
      }
    }
    if (isset($_GET['search'])){
      $s= mb_substr($_GET['search'], 0, 1, "UTF8")."*";
      if ($s == "**"){
        $s= "*";
      }
      $roleAddOccupantFilter['regex']= $s;
    }
    session::set("roleAddOccupantFilter", $roleAddOccupantFilter);

    $smarty = get_smarty();
    $smarty->assign("search_image", get_template_path('images/lists/search.png'));
    $smarty->assign("launchimage", get_template_path('images/lists/action.png'));
    $smarty->assign("tree_image", get_template_path('images/lists/search-subtree.png'));
    $smarty->assign("deplist", $this->config->idepartments);
    $smarty->assign("alphabet", generate_alphabet());
    $smarty->assign("hint", print_sizelimit_warning());
    $smarty->assign("apply", apply_filter());
    $smarty->assign("objects", $this->getObjects());
    foreach( array("dselect", "regex", "subtrees") as $type){
      $smarty->assign("$type", $roleAddOccupantFilter[$type]);
    }
    return($smarty->fetch(get_template_path("occupantSelect.tpl",TRUE, dirname(__FILE__))));
  }

  
  function getObjects()
  {

    // Initialize variables 
    $ldap = $this->config->get_ldap_link();
    $roleAddOccupantFilter= session::get("roleAddOccupantFilter");
    $regex  = $roleAddOccupantFilter['regex'];

    // Detect the base to be use for ldap search
    if($roleAddOccupantFilter['subtrees']){
      $base =  $roleAddOccupantFilter['dselect'];
    }else{
      $base =  get_people_ou().$roleAddOccupantFilter['dselect'];
    }

    // Set search flags flat/subsearch 
    $Get_list_flags = 0;
    if($roleAddOccupantFilter['subtrees'] == "checked"){
      $Get_list_flags |= GL_SUBSEARCH;
    }

    // Search for matching user objects.
    $filter = "(&(objectClass=gosaAccount)(!(uid=*$))(|(uid=$regex)(cn=$regex)))";
    $res    = get_list($filter, "users/user"  , $base,
        array("description", "objectClass", "sn", "givenName", "uid","cn"),$Get_list_flags);

    // Collect data.
    $this->objects = array(); 
    foreach($res as $attrs){

      // Skip entries which are already part of our role 
      if(in_array($attrs['dn'], $this->skipEntries)) continue;
 
      $name = $attrs['cn'][0];
      $dn = $attrs['dn'];
      if (isset($attrs["description"][0])){
        $name .= " [".$attrs["description"][0]."]";
      }
  
      $entry = array();
      $entry['attrs'] = $attrs;
      $entry['name'] = $name;
      $entry['dn'] = $dn;
      $this->objects[] = $entry; 
    }
    uksort ($this->objects, 'strnatcasecmp');
    reset ($this->objects);

    // Create HTML content 
    $temp = "";
    foreach ($this->objects as $key => $value){
      $temp.= "
        <option 
            title='".addslashes($value['dn'])."' 
            value='$key' class='select' 
            style=\"background-image:url('plugins/generic/images/head.png');
      \">".$value['name']."</option>\n";
    }
    return ($temp);
  }
 

  function save()
  {
    $ret = array();
    if(isset($_POST['objects'])){
      $objects = $_POST['objects'];
      foreach($objects as $id){
        $ret[] = $this->objects[$id];
      }
    }
    return($ret); 
  } 
}

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
