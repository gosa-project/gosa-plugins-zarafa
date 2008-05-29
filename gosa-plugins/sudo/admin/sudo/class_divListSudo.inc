<?php
/*
 * This code is part of GOsa (http://www.gosa-project.org)
 * Copyright (C) 2003-2008 GONICUS GmbH
 *
 * ID: $$Id: class_divListSudSudo.inc 9934 2008-03-18 20:26:47Z cajus $$
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


/*! \brief  The sudo management class.
            This class lists all configured sudo roles.
 */
class divListSudo extends MultiSelectWindow
{

  /* Current base */
  private $departments        = array();

  /* Regex */
  public $Regex              = "*";
  public $UserRegex          = "*";

  /* Subsearch checkbox */
  public $SubSearch          = FALSE;
  protected $parent             ;
  public $ui                 ;


  /*! \brief  Initialize this class 
      @param  Object $config  The GOsa configuration object.
      @param  Object $parent  The parent class.
   */
  function divListSudo (&$config, &$parent)
  {
    MultiSelectWindow::MultiSelectWindow($config, "sudo", "sudo");
    
    $this->parent       = &$parent;
    $this->ui           = get_userinfo();

    /* Set list strings */
    $this->SetTitle(  _("List of sudo roles"));
    $this->SetSummary(_("List of sudo roles"));
    $this->EnableAplhabet(true);

    /* Result page will look like a headpage */
    $this->SetHeadpageMode();
  
    /* Disable buttonsm */
    $this->EnableCloseButton(false);
    $this->EnableSaveButton (false);

    /* Dynamic action col, depending on snapshot icons */
    $action_col_size = 54;
    if($this->parent->snapshotEnabled()){
      $action_col_size += 32;
    }
    if($this->parent->CopyPasteHandler){
      $action_col_size += 20;
    }

    /* Toggle all selected / deselected */
    $chk = "<input type='checkbox' id='select_all' name='select_all' title='"._("Select all")."'
               onClick='toggle_all_(\"^item_selected_[0-9]*$\",\"select_all\");' >";

    /* set Page header */
    $this->AddHeader(array("string"=> $chk,          "attach"=>"style='width:20px;'"));
    $this->AddHeader(array("string"=>"&nbsp;","attach"=>"style='width:20px;'"));
    $this->AddHeader(array("string"=>_("Name")."&nbsp;/&nbsp;"._("Department")));
    $this->AddHeader(array("string"=>_("Actions"),"attach"=>"style='width:".$action_col_size."px;border-right:0px;'"));

    /* Add SubSearch checkbox */    
    $this->AddCheckBox("SubSearch",  msgPool::selectToView("","subsearch"),msgPool::selectToView("","subsearch_small"), false);
    $this->AddRegex   ("Regex"    , _("Regular expression for matching role names")       , 
      "*" , true);
    $this->AddRegex   ("UserRegex", _("Regular expression for matching role member names"),
      "*" , FALSE,"images/lists/search-user.png");
  }


  /*! \brief  Generate the list header.
   */
  function GenHeader()
  {
    /* Prepare departments,
       which are shown in the listbox on top of the listbox
     */
    $base  = sudo::get_sudoers_ou($this->config);
    $acl    = $this->ui->get_permissions($base,"sudo/sudo"); 
    $acl_all= $this->ui->get_category_permissions($base,"sudo"); 
 
    /* Add default header */
    $listhead = MultiSelectWindow::get_default_header(false);
    
    /* Create Layers menu */
    $s  = ".|"._("Actions")."|\n";
    $s .= "..|<img src='images/lists/new.png' alt='' border='0' class='center'>".
      "&nbsp;"._("Create")."|\n";

    /* Append create options */
    if(preg_match("/c/",$acl)) {
      $s.= "...|<input class='center' type='image' src='images/list_new_sudo.png' alt=''>&nbsp;"._("Role")."|new_role|\n";
      $s.= "...|<input class='center' type='image' src='images/list_new_sudo.png' alt=''>&nbsp;"._("Default")."|new_default|\n";
    }

    /* Append multiple remove */
    if(preg_match("/r/",$acl)){
      $s.= "..|---|\n";
      $s.= "..|<img src='images/lists/trash.png' alt='' border='0' class='center'>".
        "&nbsp;"._("Remove")."|"."del_role|\n";
    }

    if(is_object($this->parent->CopyPasteHandler) && preg_match("/r/",$acl_all)){
      $s.= "..|---|\n";
      $s.= "..|<img src='images/lists/copy.png' alt='' border='0' class='center'>".
        "&nbsp;"._("Copy")."|"."multiple_copy_systems|\n";
    }

    /* Copy & paste icons */
    if(is_object($this->parent->CopyPasteHandler) && $this->parent->CopyPasteHandler->entries_queued()){
      $img = "<img border='0' class='center' src='images/lists/paste.png' alt=''>";
      $s.="..|".$img."&nbsp;"._("Paste")."|editPaste|\n";
    }else{
      $img = "<img border='0' class='center' src='images/lists/paste-grey.png' alt=''>";
      $s.="..|".$img."&nbsp;"._("Paste")."\n";
    }

    /* Add snapshot icons */
    $s .= $this->parent->get_snapshot_header($this->selectedBase,$this->module);   
 
    $this->SetDropDownHeaderMenu($s);
    $this->SetListHeader($listhead);
  }


  /*! \brief  Prepare the plguin to list a set of objects 
   */
  function execute()
  {
    $this->ClearElementsList();
    $this->GenHeader();
  }


  /*! \brief  Create a list of all objects that should be displayed 
      @param  Array $list   The list of sudo roles, passed from the sudoManagement class.
   */
  function setEntries($list)
  {
    /* Prepare links */
    $editlink = "<a href='?plug=".$_GET['plug']."&amp;id=%s&amp;act=edit_entry'>%s</a>";
    $userimg  = "<img class='center' src='images/select_sudo.png' alt='Sudo' title='%s'>";
    $ui       = get_userinfo();

    $action_col_size = 54;
    if($this->parent->snapshotEnabled()){
      $action_col_size += 32;
    }
    if($this->parent->CopyPasteHandler){
      $action_col_size += 20;
    }

    // Test Every Entry and generate divlist Array
    foreach($list as $key => $val){
    
      /* Create action icons - copy & paste icons */
      $acl    = $ui->get_permissions($val['dn'],"sudo/sudo");
      $acl_all= $this->ui->get_category_permissions($val['dn'],"sudo"); 

      /* Add edit icon */
      $actions= "";
      if($ui->is_copyable($val['dn'],"sudo","sudo") && $this->parent->CopyPasteHandler){
        $actions.= "<input class='center' type='image'
          src='images/lists/copy.png' alt='"._("copy")."' name='copy_%KEY%' title='"._("Copy this entry")."'>&nbsp;";
      }else{
        $actions.="<img src='images/empty.png' alt=' ' class='center'>&nbsp;";
      }

      $actions.= "<input class='center' type='image'
        src='images/lists/edit.png' alt='".msgPool::editButton()."' name='sudo_edit_%KEY%' title='"._("Edit this entry")."'>";

      /* Add snapshot icon */
      $actions.= $this->parent->get_snapshot_action($val['dn'],$this->module);

      if(preg_match("/d/",$acl)){
        $actions.= "<input class='center' type='image'
          src='images/lists/trash.png' alt='".msgPool::delButton()."' name='sudo_del_%KEY%' title='"._("Delete this entry")."'>";
      }

      $title = "title='".preg_replace('/ /', '&nbsp;', @LDAP::fix($val['dn']))."'";

      if(!isset($val['description'][0])){
        $desc = "";
      }else{
        $desc = " - [ ".$val['description'][0]." ]";
      }

      $display = $val['cn'][0].$desc;
     
      /* Highlight defaults entry */ 
      if(preg_match("/^defaults$/i",$val['cn'][0])){
        $display = "<b>$display</b>";
      }

      /* Cutted objects should be displayed in light grey */
      if($this->parent->CopyPasteHandler){
        foreach($this->parent->CopyPasteHandler->queue as $queue_key => $queue_data){
          if($queue_data['dn'] == $val['dn']) {
            $display = "<font color='#999999'>".$display."</font>";
            break;
          }
        }
      }

      $field1 = array("string" => "<input type='checkbox' id='item_selected_".$key."' name='item_selected_".$key."'>" ,
          "attach" => "style='width:20px;'");
      $field2 = array("string" => sprintf($userimg,$val['dn']), 
          "attach" => "style='text-align:center;width:20px;'");
      $field3 = array("string" => sprintf($editlink,$key,$display), 
          "attach" => "style='' ".$title);
      $field4 = array("string" => preg_replace("/%KEY%/", $key, $actions), 
          "attach" => "style='width:".$action_col_size."px;border-right:0px;text-align:right;'");

      $this->AddElement(array($field1,$field2,$field3,$field4));
    }

    /* Create summary string for list footer */
    $num     = count($list);
    $num_str  = _("Number of listed roles");
    $str = "<img class='center' src='images/select_sudo.png'
              title='".$num_str."' alt='".$num_str."'>&nbsp;".$num."&nbsp;";
    $this->set_List_Bottom_Info($str);
  }

  
  /*! \brief  Save this plugin. In this case nothing will be saved
    */
  function Save()
  {
    MultiSelectWindow :: Save();  
  }

  /*! \brief  Save all relevant HTML inputs for this plugin.
    */
  function save_object()
  {
    /* Save automatic created POSTs like regex, checkboxes */
    MultiSelectWindow::save_object();   
  }
}
// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
