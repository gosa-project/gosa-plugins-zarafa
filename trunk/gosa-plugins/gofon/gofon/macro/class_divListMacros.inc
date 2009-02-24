<?php

class divListMacro extends MultiSelectWindow
{

  /* Current base */
  var $selectedBase       = "";
  var $departments        = array();

  /* Regex */
  var $Regex              = "*";

  /* checkboxes */
  var $ShowSendBocklists      ;
  var $ShowReceiveMacros  ;

  /* Subsearch checkbox */
  var $SubSearch;

  var $parent             ;
  var $ui                 ;

  function divListMacro (&$config,$parent)
  {
    MultiSelectWindow::MultiSelectWindow($config,"Macros", "gofonmacro");
    
    $this->parent       = $parent;
    $this->ui           = get_userinfo();

    /* Set list strings */
    $this->SetTitle(_("List of macros"));
    $this->SetSummary(_("List of macros"));

    /* Result page will look like a headpage */
    $this->SetHeadpageMode();

    $this->EnableAplhabet   (true);
  
    /* Disable buttonsm */
    $this->EnableCloseButton(false);
    $this->EnableSaveButton (false);

    /* Dynamic action col, depending on snapshot icons */
    $action_col_size = 100;
    if($this->parent->snapshotEnabled()){
      $action_col_size += 20;
    }

    /* Toggle all selected / deselected */
    $chk = "<input type='checkbox' id='select_all' name='select_all' title='"._("Select all")."'
               onClick='toggle_all_(\"^item_selected_[0-9]*$\",\"select_all\");' >";

    /* set Page header */
    $this->AddHeader(array("string"=> $chk,          "attach"=>"style='width:20px;'"));
    $this->AddHeader(array("string" => "&nbsp;", "attach" => "style='text-align:center;width:20px;'"));
    $this->AddHeader(array("string" => _("Name")." / "._("Department"), "attach" => "style=''"));
    $this->AddHeader(array("string" => _("Visible"), "attach" => "style='width:50px;'"));
    $this->AddHeader(array("string" => _("Actions"), "attach" => "style='width:".$action_col_size."px;border-right:0px;text-align:right;'" ));

    /* Add Checkboxes / SubSearch checkbox */
    $this->AddCheckBox("SubSearch",  msgPool::selectToView("","subsearch"), msgPool::selectToView("","subsearch_small"), false);

    /*                  Name                 ,Text                              ,Default  , Connect with alphabet  */
    $this->AddRegex   ("Regex",     _("Regular expression for matching macro names"),"*" , true);
  }

  function GenHeader()
  {
    /* Get all departments within this subtree */
    $base = $this->config->current['BASE'];
    $options  = $this->create_department_list($this->module);

    $ui =get_userinfo();
    $acl    = $ui->get_permissions($this->selectedBase,"gofonmacro/macro");
    $acl_all= $ui->has_complete_category_acls($this->selectedBase,"gofonmacro");

    /* Add default header */
    $listhead = MultiSelectWindow::get_default_header();
    $listhead .= _("Base")."&nbsp;<select name='CurrentMainBase' onChange='mainform.submit()' class='center'>$options</select>".
      " <input class='center' type='image' src='images/lists/submit.png' align='middle' 
      title='"._("Submit department")."' name='submit_department' alt='"._("Submit")."'>&nbsp;";
    
    /* Create Layers menu */
    $s  = ".|"._("Actions")."|\n";

    /* Append create options */
    if(preg_match("/c/",$acl)){
      $s .= "..|<img src='images/lists/new.png' alt='' border='0' class='center'>".
        "&nbsp;"._("Create")."|\n";
      $s.= "...|<input class='center' type='image' src='plugins/gofon/images/list_new_macro.png' alt=''>".
        "&nbsp;"._("Macro")."|goFonMacro_new|\n";
      $s.= "..|---|\n";
    }

    /* Multiple options */
    $s.= "..|<img src='images/lists/trash.png' alt='' border='0' class='center'>".
      "&nbsp;"._("Remove")."|"."remove_multiple|\n";


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
  
    $empty      = "<img class='center' src='images/nothing.png' style='width:16px;height:16px;' alt=''>";
    $macroimg   = "<img class='center' src='plugins/gofon/images/list_macro.png' alt='"._("Macro")."' title='%s'>";
    $visible    = "<img class='center' src='images/true.png' alt='"._("yes")."' title='"._("visible")."'>";
    $invisible  = "<img class='center' src='images/false.png' alt='"._("no")."'title='"._("invisible")."'>";
    $editlink   = "<a href='?plug=".$_GET['plug']."&amp;id=%s&amp;act=edit_entry'>%s</a>";

    /* Dynamic action col, depending on snapshot icons */
    $action_col_size = 100;
    if($this->parent->snapshotEnabled()){
      $action_col_size += 20;
    }

    $ui = get_userinfo();

    foreach($list as $key => $val){
       
      $acl    = $ui->get_permissions($val['dn'],"gofonmacro/macro");
      $acl_all= $ui->has_complete_category_acls($val['dn'],"gofonmacro");

      $action = "";
      if(!preg_match("/r/",$acl)){
        continue;
      }

      /* Add copy & cut functionality */
      $action.= $this->parent->get_copypaste_action($val['dn'],"gofonmacro","macro");
  
      $action.= "<input class='center' type='image' src='images/lists/edit.png' alt='"._("edit")."'     
        name='goFonMacro_edit_%KEY%' title='"._("Edit macro")."'>";

      /* Add snapshot icon */
      $action.= $this->parent->get_snapshot_action($val['dn'],$this->module);      

      if(preg_match("/d/",$acl)){
        $action.= "<input class='center' type='image' src='images/lists/trash.png' alt='"._("delete")."'   
          name='goFonMacro_del_%KEY%' title='"._("Delete macro")."'>";
      }

      if(isset($val['goFonMacroVisible']['0'])&&($val['goFonMacroVisible']['0'] == "1")){
        $pic1 = $visible;
      }else{
        $pic1 = $invisible;
      }

      $display= $val["displayName"][0]."&nbsp;(".$val["cn"][0].")";
      if(isset($val['description'][0])){

        $desc = $val['description'][0];
        if(strlen($desc) > 55){
          $desc = substr($desc,0,52)."...";
        }

        $display .= "&nbsp;[".$desc."]";
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

      /* Create each field */
      $field0 = array("string" => "<input type='checkbox' id='item_selected_".$key."' name='item_selected_".$key."'>" ,
                      "attach" => "style='width:20px;'");
      $field1 = array("string" => sprintf($macroimg,$val['dn']), "attach" => "style='text-align:center;width:20px;'");
      $field2 = array("string" => sprintf($editlink,$key,$display), "attach" => "style='' title='".preg_replace('/ /', '&nbsp;', LDAP::fix($val['dn']))."'");
      $field3 = array("string" => $pic1, "attach" => "style='width:50px;'");
      $field4 = array("string" => preg_replace("/%KEY%/", "$key", $action), "attach" => "style='width:".$action_col_size."px;border-right:0px;text-align:right;'");

      $this->AddElement(array($field0,$field1,$field2,$field3,$field4));
    }

    /* Create summary string for list footer */
    $num_deps=0;
    if(!$this->SubSearch){
      $num_deps = count($this->Added_Departments);
    }
    $num_objs = count($list);

    $num_obj_str = sprintf(_("Number of listed %s"),_("macros"));
    $num_dep_str = sprintf(_("Number of listed %s"),_("departments"));

    $str = "<img class='center' src='plugins/gofon/images/select_macro.png'
              title='".$num_obj_str."' alt='".$num_obj_str."'>&nbsp;".$num_objs."&nbsp;&nbsp;&nbsp;&nbsp;";
    $str.= "<img class='center' src='images/lists/folder.png'
              title='".$num_dep_str."' alt='".$num_dep_str."'>&nbsp;".$num_deps."&nbsp;&nbsp;&nbsp;&nbsp;";

    $this->set_List_Bottom_Info($str);
  }

  function Save()
  {
    MultiSelectWindow :: Save();  
  }

  function save_object()
  {
    /* Save automatic created POSTs like regex, checkboxes */
    MultiSelectWindow :: save_object();   
  }
}
// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
