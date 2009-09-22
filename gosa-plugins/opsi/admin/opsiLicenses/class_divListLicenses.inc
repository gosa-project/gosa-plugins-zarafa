<?php
/*
 * This code is part of GOsa (http://www.gosa-project.org)
 * Copyright (C) 2003-2008 GONICUS GmbH
 *
 * ID: $$Id: class_divListRole.inc 12852 2008-10-31 16:19:40Z cajus $$
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

class divListRole extends MultiSelectWindow
{

  // Current base 
  var $selectedBase       = "";
  var $departments        = array();

  // Regex 
  var $Regex              = "*";

  // Subsearch checkbox 
  var $SubSearch          ;
  var $parent             ;
  var $ui                 ;

  function divListRole (&$config,$parent)
  {
    MultiSelectWindow::MultiSelectWindow($config, "Licenses", "licenses");
    
    $this->parent       = $parent;
    $this->ui           = get_userinfo();

    // Dynamic action col, depending on snapshot icons 
    $action_col_size = 90;
    if($this->parent->snapshotEnabled()){
      $action_col_size += 38;
    }

    // Set list strings 
    $this->SetTitle(_("List of licenses"));
    $this->SetSummary(_("List of licenses"));
    $this->EnableAplhabet(true);

    // Result page will look like a headpage 
    $this->SetHeadpageMode();
  
    // Disable buttonsm 
    $this->EnableCloseButton(false);
    $this->EnableSaveButton (false);

    // Add checkbox: Toggle all selected / deselected 
    $chk = "<input type='checkbox' id='select_all' name='select_all' title='"._("Select all")."'
               onClick='toggle_all_(\"^item_selected_[0-9]*$\",\"select_all\");' >";

    // Set Page col headers 
    $this->AddHeader(array("string" => $chk,"attach"=>"style='width:20px;'"));
    $this->AddHeader(array("string" => "&nbsp;", "attach" => "style='text-align:center;width:20px;'"));
    $this->AddHeader(array("string" => _("Role")." / "._("Department"), "attach" => "style=''"));
    $this->AddHeader(array("string" => _("Actions"), 
          "attach" => "style='width:".$action_col_size."px;border-right:0px;text-align:right;'"));

    // Add SubSearch checkbox 
    //$this->AddCheckBox(SEPERATOR);
    $this->AddCheckBox("SubSearch",  msgPool::selectToView("","subsearch"), 
        msgPool::selectToView("","subsearch_small"), false);

    // Add regex user input field
    $this->AddRegex   ("Regex", _("Regular expression for matching license names"), "*" , true);
  }


  function GenHeader()
  {
    /* Get all departments within this subtree */
    $ui= get_userinfo();
    $first = "";
    $found = FALSE;
    $base = $this->config->current['BASE'];
    $options  = $this->create_department_list($this->module);

    /* Add a seperator after displaying c&p and snapshot icons ? */
    $add_sep = false;
  
    /* Get copy & paste icon */
    $acls  = $ui->get_permissions($this->selectedBase,"licenses/licenseGeneric");
    $acl_all= $ui->has_complete_category_acls($this->selectedBase,"licenses");

    /* Add default header */
    $listhead = MultiSelectWindow::get_default_header();
  
    /* Add department selector */
    $listhead .= _("Base")."&nbsp;<select name='CurrentMainBase' 
      onChange='mainform.submit()' class='center'>$options</select>".
      " <input class='center' type='image' src='images/lists/submit.png' align='middle' 
      title='"._("Submit department")."' name='submit_department' alt='"._("Submit")."'>&nbsp;";

    /* Create Layers menu */
    $s  = ".|"._("Actions")."|\n";

    /* Append create options */
    if(preg_match("/c/",$acls)) {
      $s .= "..|<img src='images/lists/new.png' alt='' border='0' class='center'>".
        "&nbsp;"._("Create")."|\n";
      $s.= "...|<input class='center' type='image' src='plugins/licensemanagement/images/new.png' alt=''>".
        "&nbsp;"._("Role")."|license_new|\n";
    }

    /* Multiple options */
    $s.= "..|---|\n";
    $s.= "..|<img src='images/lists/trash.png' alt='' border='0' class='center'>".
      "&nbsp;"._("Remove")."|"."remove_multiple_licenses|\n";

    /* Add Copy & Paste header */
    $s .= $this->parent->get_copypaste_header($this->selectedBase,$this->module);

    /* Add snapshot icons */
    $s .= $this->parent->get_snapshot_header($this->selectedBase,$this->module);

    $this->SetDropDownHeaderMenu($s);
    $this->SetListHeader($listhead);
  }


  function execute()
  {
    $this->ClearElementsList();
    $this->GenHeader();
  }

  function setEntries($list)
  {
    // Defining Links
    $editlink = "<a href='?plug=".$_GET['plug']."&amp;id=%s&amp;act=edit_entry'>%s</a>";

    // Dynamic action col, depending on snapshot icons 
    $action_col_size = 90;
    if($this->parent->snapshotEnabled()){
      $action_col_size += 38;
    }
  
    $ui = get_userinfo();

    // Assigning licenses
    foreach($list as $key => $val){
      
      // Get object permissions
      $acl    = $ui->get_permissions($val['dn'],"licenses/licenseGeneric");
      $acl_all= $ui->has_complete_category_acls($val['dn'],"licenses");


      // Add copy & cut functionality */
      $actions = $this->parent->get_copypaste_action($val['dn'],"ogroups","ogroup");
 
      // Create edit and remove icon buttons
      $actions.= "<input class='center' type='image'
        src='images/lists/edit.png' alt='"._("edit")."' 
        name='license_edit_%KEY%' title='"._("Edit this entry")."'>";

      $actions.= $this->parent->get_snapshot_action($val['dn'],$this->module);
      if(preg_match("/d/",$acl)){
        $actions.= "<input class='center' type='image'
          src='images/lists/trash.png' alt='"._("delete")."' 
          name='license_del_%KEY%' title='"._("Delete this entry")."'>";
      }else{
        $actions.= "<img class='center' src='images/empty.png' alt='&nbsp;'
          title='".msgPool::permDelete()."'>";
      }

      // Append license descriptio, if available
      $title = "title='".preg_replace('/ /', '&nbsp;', LDAP::fix($val['dn']))."'";
      if(!isset($val['description'][0])){
        $desc = "";
      }else{
        $desc = " - [ ".$val['description'][0]." ]";
      }

      // Cutted entry are displayed in a light grey as indication.
      $display = $val['cn'][0].$desc;
      if($this->parent->CopyPasteHandler){
        foreach($this->parent->CopyPasteHandler->queue as $queue_key => $queue_data){
          if($queue_data['dn'] == $val['dn']) {
            $display = "<font color='#999999'>".$display."</font>";
            break;
          }
        }
      }

      // Append the entry to the divlist
      $field0 = array("string" => "<input type='checkbox' id='item_selected_".$key."' 
          name='item_selected_".$key."'>" ,
          "attach" => "style='width:20px;'");
      $field1 = array("string" => "<img src='plugins/licensemanagement/images/license.png' 
          alt='"._("Role")."' ".$title.">", 
          "attach" => "style='text-align:center;width: 20px;'");
      $field2 = array("string" => sprintf($editlink,$key,$display), 
          "attach" => "style='' ".$title);
      $field4 = array("string" => preg_replace("/%KEY%/", $key, $actions), 
          "attach" => "style='width:".$action_col_size."px;border-right:0px;text-align:right;'");

      $this->AddElement(array($field0,$field1,$field2,$field4));
    }
   
 
    // Create the list summary 
    $num_deps=0;
    if(!$this->SubSearch){
      $num_deps = count($this->Added_Departments);
    }
    $num_objs = count($list);
    $num_obj_str = _("Number of listed licenses");
    $num_dep_str = _("Number of listed departments");
    $str = "<img class='center' src='plugins/licensemanagement/images/license.png'
              title='".$num_obj_str."' alt='".$num_obj_str."'>&nbsp;".$num_objs."&nbsp;&nbsp;&nbsp;&nbsp;";
    $str.= "<img class='center' src='images/lists/folder.png'
              title='".$num_dep_str."' alt='".$num_dep_str."'>&nbsp;".$num_deps."&nbsp;&nbsp;&nbsp;&nbsp;";
    $this->set_List_Bottom_Info($str);
  }
}
// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
