<?php

class divListBlocklist extends MultiSelectWindow
{

  /* Current base */
  var $selectedBase       = "";
  var $departments        = array();

  /* Regex */
  var $Regex              = "*";

  /* checkboxes */
  var $ShowSendBocklists      ;
  var $ShowReceiveBlocklists  ;

  /* Subsearch checkbox */
  var $SubSearch;

  var $parent             ;
  var $ui                 ;

  function divListBlocklist (&$config, &$parent)
  {
    MultiSelectWindow::MultiSelectWindow($config, "Blocklists", "gofaxlist");
    
    $this->parent       = $parent;
    $this->ui           = get_userinfo();

    /* Set list strings */
    $this->SetTitle(_("List of blocklists"));
    $this->SetSummary(_("List of blocklists"));

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
    $this->AddHeader(array("string" => _("Actions"), "attach" => "style='width:".$action_col_size."px;border-right:0px;text-align:right;'" ));

    /* Add Checkboxes / SubSearch checkbox */
    $this->AddCheckBox("ShowSendBocklists"     , msgPool::selectToView(_("Send blocklist")),     _("Show send blocklists"),true);
    $this->AddCheckBox("ShowReceiveBlocklists" , msgPool::selectToView(_("Receive blocklist")),  _("Show receive blocklists"),true);
  
    $this->AddCheckBox(SEPERATOR);
    $this->AddCheckBox("SubSearch",  msgPool::selectToView("","subsearch"), msgPool::selectToView("","subsearch_small"), false);

    /* Name,Text,Default,Connect with alphabet */
    $this->AddRegex   ("Regex",     _("Regular expression for matching list names"),"*" , true);
  }


  function GenHeader()
  {
    /* Prepare departments,
       which are shown in the listbox on top of the listbox
     */
    $options= "";

    /* Get all departments within this subtree */
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
    $tdeps= $ui->get_module_departments("gofaxlist");
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
  
    /* Get acls */
    $ui       = get_userinfo();
    $acl      = $ui->get_permissions($this->selectedBase,"gofaxlist/blocklist");
    $acl_all  = $ui->has_complete_category_acls($this->selectedBase,"gofaxlist");

    /* Add default header */
    $listhead = MultiSelectWindow::get_default_header();

    /* Append department selector */
    $listhead .= "&nbsp;"._("Base")."&nbsp;<select name='CurrentMainBase' onChange='mainform.submit()' class='center'>$options</select>".
      " <input class='center' type='image' src='images/lists/submit.png' align='middle' 
          title='"._("Submit department")."' name='submit_department' alt='"._("Submit")."'>&nbsp;";

    /* Create Layers menu */
    $s  = ".|"._("Actions")."|\n";

    /* Append create options */
    if(preg_match("/c/",$acl)) {
      $s .= "..|<img src='images/lists/new.png' alt='' border='0' class='center'>".
        "&nbsp;"._("Create")."|\n";
      $s.= "...|<input class='center' type='image' src='images/list_new_blocklist.png' alt=''>".
        "&nbsp;"._("Blocklist")."|goFaxBlocklist_new|\n";
      $s.= "..|---|\n";
    }

    /* Multiple options */
    $s.= "..|<img src='images/lists/trash.png' alt='' border='0' class='center'>".
      "&nbsp;"._("Remove")."|"."remove_multiple|\n";

    if(is_object($this->parent->CopyPasteHandler) && preg_match("/r/",$acl_all)){
      $s.= "..|---|\n";
      $s.= "..|<img src='images/lists/copy.png' alt='' border='0' class='center'>".
        "&nbsp;"._("Copy")."|"."multiple_copy_systems|\n";
      $s.= "..|<img src='images/lists/cut.png' alt='' border='0' class='center'>".
        "&nbsp;"._("Cut")."|"."multiple_cut_systems|\n";
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

  function execute()
  {
    $this->ClearElementsList();
    $this->GenHeader();
  }

  function setEntries($list)
  {
    // User and Template  Images
    $blockimg = "<img class='center' src='images/list_blocklist.png' alt='User' title='%s'>";
    $editlink = "<a href='?plug=".$_GET['plug']."&amp;id=%s&amp;act=edit_entry'>%s</a>";

    /* Dynamic action col, depending on snapshot icons */
    $action_col_size = 100;
    if($this->parent->snapshotEnabled()){
      $action_col_size += 20;
    }

    /* Append to list */
    $ui = get_userinfo();
    foreach($list as $key => $val){

      $acl = $ui->get_permissions($val['dn'],"gofaxlist/blocklist");
      $acl_all  = $ui->has_complete_category_acls($val['dn'],"gofaxlist");

      if(!preg_match("/r/",$acl)){
        continue;
      }  

      $action ="";
      if($ui->is_cutable($val['dn'],"gofaxlist","blocklist") && $this->parent->CopyPasteHandler){
        $action .= "<input class='center' type='image'
          src='images/lists/cut.png' alt='"._("cut")."' name='cut_%KEY%' title='"._("Cut this entry")."'>&nbsp;";
      }else{
        $action.="<img src='images/empty.png' alt=' ' class='center'>&nbsp;";
      }

      if($ui->is_copyable($val['dn'],"gofaxlist","blocklist") && $this->parent->CopyPasteHandler){
        $action.= "<input class='center' type='image'
          src='images/lists/copy.png' alt='"._("copy")."' name='copy_%KEY%' title='"._("Copy this entry")."'>&nbsp;";
      }else{
        $action.="<img src='images/empty.png' alt=' ' class='center'>&nbsp;";
      }

      /* Edit link ; requires read access */
      $action.= "<input class='center' type='image' src='images/lists/edit.png' alt='"._("edit")."'     name='goFaxBlocklist_edit_%KEY%' title='"._("Edit user")."'>";

      /* Add snapshot icon */
      $action.= $this->parent->get_snapshot_action($val['dn'],$this->module);
 
      /* Delete entry ; requires remove access */
      if(preg_match("/d/",$acl)){
        $action.= "<input class='center' type='image' src='images/lists/trash.png' alt='"._("delete")."' name='goFaxBlocklist_del_%KEY%' title='"._("Delete user")."'>";
      }
      
      // Generate Array to Add
      if(isset($val["description"][0])){
        $display= $val["cn"][0]." [".$val["description"][0]."]";
      }else{
        $display= $val["cn"][0];
      }

      /* Cutted objects should be displayed in light grey */
      if(isset($this->parent->CopyPasteHandler->queue) && is_array($this->parent->CopyPasteHandler->queue)){
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
      $field1 = array("string" => sprintf($blockimg,$val['dn']), "attach" => "style='text-align:center;width:20px;'");
      $field2 = array("string" => sprintf($editlink,$key,$display), "attach" => "style='' title='".preg_replace('/ /', '&nbsp;', @LDAP::fix($val['dn']))."'");
      $field3 = array("string" => preg_replace("/%KEY%/", "$key", $action), "attach" => "style='width:".$action_col_size."px;border-right:0px;text-align:right;'");
      $this->AddElement( array($field0,$field1,$field2,$field3));
    }

    /* Create summary string for list footer */
    $num_deps=0;
    if(!$this->SubSearch){
      $num_deps = count($this->Added_Departments);
    }
    $num_objs = count($list);

    $num_obj_str = sprintf(_("Number of listed '%s'"),_("blocklists"));
    $num_dep_str = sprintf(_("Number of listed '%s'"),_("departments"));

    $str = "<img class='center' src='images/list_blocklist.png'
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
