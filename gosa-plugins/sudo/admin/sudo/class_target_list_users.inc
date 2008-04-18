<?php
/*
 * This code is part of GOsa (http://www.gosa-project.org)
 * Copyright (C) 2003-2008 GONICUS GmbH
 *
 * ID: $$Id: class_target_list.inc 9850 2008-03-14 14:15:12Z hickert $$
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

class target_list_users extends MultiSelectWindow
{
  public $config;
  private $list         =array();
  private $Targets_used =array();

  /* Current base */
  private $departments        = array();

  /* Regex */
  protected $Regex              = "*";

  /* CheckBoxes, to change default values modify $this->AddCheckBox */
  protected $user       ;
  protected $group      ;

  /* Subsearch checkbox */
  protected $SubSearch          ;
  protected $parent             ;
  protected $ui                 ;

  function target_list_users(&$config,$Targets_used)
  {
    MultiSelectWindow::MultiSelectWindow($config, "Targetselection", array("users"));

    $this->Targets_used  = $Targets_used;

    $this->SetInformation(  _("Select the target objects"));
    $this->SetTitle(        _("Available targets"));
    $this->SetSummary(      _("Available targets"));
    $this->SetHeadpageMode(FALSE);

    /* set Page header */
    $chk = "<input type='checkbox' id='select_all' name='select_all'
      onClick='toggle_all_(\"^item_selected_[0-9]*$\",\"select_all\");' >";
    $this->AddHeader(array("string"=> $chk,          "attach"=>"style='width:20px;'"));

    $this->AddHeader(array("string" => "&nbsp;", "attach" => "style='text-align:center;width:20px;'"));
    $this->AddHeader(array("string" => _("Object name"), "attach" => "style=''"));

    $this->AddCheckBox("user",    _("Select to see users") ,_("Show users"), true);
    $this->AddCheckBox("group",   _("Select to see groups"),_("Show groups"), true);

    /* Add SubSearch checkbox */
    $this->AddCheckBox(SEPERATOR);

    /* Add SubSearch checkbox */
    $this->AddCheckBox("SubSearch",  msgPool::selectToView("","subsearch"), msgPool::selectToView("","subsearch_small"), false);

    /* Name,Text,Default  , Connect with alphabet  */
    $this->AddRegex   ("Regex",     _("Regular expression for matching group names"), "*" , true);
    $this->EnableAplhabet(TRUE);
  }


  function GenHeader()
  {
    $options= "";

    /* Get all departments within this subtree */
    $ui= get_userinfo();
    $first = "";
    $found = FALSE;
    $base = $this->config->current['BASE'];

    /* Add base */
    $tmp = array();
    $tmp[] = array("dn"=>$this->config->current['BASE']);
    $tmp=  array_merge($tmp,get_list("(&(|(ou=*)(description=*))(objectClass=gosaDepartment))", $this->module, $base,
          array("ou", "description"), GL_SIZELIMIT | GL_SUBSEARCH));

    $deps = array();
    foreach($tmp as $tm){
      $deps[$tm['dn']] = $tm['dn'];
    }

    /* Load possible departments */
    $ui= get_userinfo();
    $tdeps= $ui->get_module_departments("users");
    $ids = $this->config->idepartments;
    $first = "";
    $found = FALSE;
    foreach($ids as $dep => $name){
      if(isset($deps[$dep]) && in_array_ics($dep, $tdeps)){

        /* Keep first base dn in mind, we could need this
         *  info if no valid base was found
         */
        if(empty($first)) {
          $first = $dep['dn'];
        }

        $value = $ids[$dep];
        if ($this->selectedBase == $dep){
          $found = TRUE;
          $options.= "<option selected='selected' value='".$dep."'>$value</option>";
        } else {
          $options.= "<option value='".$dep."'>$value</option>";
        }
      }
    }

    /* The currently used base is not visible with your acl setup.
     * Set base to first useable base.
     */
    if(!$found){
      $this->selectedBase = $first;
    }

    /* Get copy & paste icon */
    $acls  = $ui->get_permissions($this->selectedBase,"user/user");
    $acl_all= $ui->has_complete_category_acls($this->selectedBase,"users");

    /* Add default header */
    $listhead = MultiSelectWindow::get_default_header();

    /* Add department selector */
    $listhead .= _("Base")."&nbsp;<select name='CurrentMainBase' onChange='mainform.submit()' class='center'>$options</select>".
      " <input class='center' type='image' src='images/lists/submit.png' align='middle'
      title='"._("Submit department")."' name='submit_department' alt='"._("Submit")."'>&nbsp;";
    $listhead .="</div>";

    $this->SetListHeader($listhead);
  }


  function execute()
  {
    $this->ClearElementsList();
    $this->GenHeader();
    $this->reload();
    $this->SetEntries($this->list);
    return($this->Draw());
  }


  function SetEntries($list)
  {
    /* Add Copy & Paste buttons if copy&paste is enabled
     */
    // Defining Links
    $linkopen = "<a href='?plug=".$_GET['plug']."&amp;act=dep_open&amp;dep_id=%s'>%s</a>";
    $editlink = "<a href='?plug=".$_GET['plug']."&amp;id=%s&amp;act=edit_entry'>%s</a>";

    $ui = get_userinfo();

    // Assigning users
    foreach($list as $key => $val){

      if(in_array($val['cn'][0],$this->Targets_used)) continue;

      $title = "title='".preg_replace('/ /', '&nbsp;', @LDAP::fix($val['dn']))."'";
      if(!isset($val['description'][0])){
        $desc = "";
      }else{
        $desc = " - [ ".$val['description'][0]." ]";
      }

      $img1 = "<img src='images/select_user.png'  alt='"._("User")."'  ".$title.">";
      $img2 = "<img src='plugins/groups/images/groups.png' alt='"._("Group")."' ".$title.">";

      if(in_array("posixGroup",$val['objectClass'])){
        $img = $img2;
      }else{
        $img = $img1;
      }

      /* Create each field */
      $field0 = array("string" => "<input type='checkbox' id='item_selected_".$key."' name='item_selected_".$key."'>" ,
          "attach" => "style='width:20px;'");
      $field1 = array("string" => $img, 
          "attach" => "style='text-   align:center;width: 20px;'");
      $field2 = array("string" => $val['cn'][0].$desc, 
          "attach" => "style='border-right:0px;' ".$title);

      $this->AddElement(array($field0,$field1,$field2));
    }
  }


  function save()
  { 
    $ret = array();
    $items = $this->list_get_selected_items();
    foreach($items as $item){
      $ret[] = $this->list[$item];
    }
    return($ret);
  }


  function save_object()
  {
    MultiSelectWindow::save_object();
  }


  function reload()
  {
    /* Set base for all searches && initialise some vars */
    $this->list= array();
    $base     = $this->selectedBase;
    $Regex    = $this->Regex;

    /* Create filter */

    if($this->SubSearch){
      $res = array();
      if($this->user){
        $filter   = "(&(objectClass=person)(objectClass=gosaAccount))";
        $filter= "(&(|(cn=".$Regex.")(uid=".$Regex."))(|".$filter."))";
        $res= array_merge($res,get_sub_list($filter, array("users"), array(get_people_ou()),$base, 
              array("cn","uid","objectClass","description"), GL_SIZELIMIT | GL_SUBSEARCH));
      }
      if($this->group){
        $filter   = "(objectClass=posixGroup)";
        $filter= "(&(|(cn=".$Regex.")(uid=".$Regex."))(|".$filter."))";
        $res= array_merge($res,get_sub_list($filter, array("groups"), array(get_groups_ou()),$base, 
              array("cn","uid","objectClass","description"), GL_SIZELIMIT | GL_SUBSEARCH));
      }
    }else{
      $res = array();
      if($this->user){
        $filter   = "(&(objectClass=person)(objectClass=gosaAccount))";
        $filter= "(&(|(cn=".$Regex.")(uid=".$Regex."))(|".$filter."))";
        $res= array_merge($res,get_list($filter, array("users"), get_people_ou().$base, 
              array("cn","uid","objectClass","description"), GL_SIZELIMIT ));
      }
      if($this->group){
        $filter   = "(objectClass=posixGroup)";
        $filter= "(&(|(cn=".$Regex.")(uid=".$Regex."))(|".$filter."))";
        $res= array_merge($res,get_list($filter, array("groups"), get_groups_ou().$base, 
              array("cn","uid","objectClass","description"), GL_SIZELIMIT ));
      }
    }

    $this->list= $res;
    ksort ($this->list);
    reset ($this->list);
    $tmp=array();
    foreach($this->list as $tkey => $val ){
      $tmp[strtolower($val['cn'][0]).$val['cn'][0]]=$val;
    }
    ksort($tmp);
    $this->list=array();
    foreach($tmp as $val){
      $this->list[]=$val;
    }
    reset ($this->list);
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
}

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
