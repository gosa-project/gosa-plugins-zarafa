<?php

class divListFai extends MultiSelectWindow
{
  /* Regex */
  var $Regex              = "*";
  var $ShowProfiles;
  var $ShowTemplates;
  var $ShowScripts;
  var $ShowHooks;
  var $ShowVariables;
  var $ShowPackages;
  var $ShowPartitions;

  /* Subsearch checkbox */
  var $SubSearch;

  var $parent             ;
  var $ui                 ;

  function divListFai (&$config,$parent)
  {
    MultiSelectWindow::MultiSelectWindow($config, "Fai", "fai");
    
    $this->parent       = $parent;
    $this->ui           = get_userinfo();

    /* Set list strings */
    $this->SetTitle(  _("List of classes"));
    $this->SetSummary(_("List of deployment classes and packages"));

    /* Result page will look like a headpage */
    $this->SetHeadpageMode();

    $this->EnableAplhabet   (true);
  
    /* Disable buttonsm */
    $this->EnableCloseButton(false);
    $this->EnableSaveButton (false);

    /* Dynamic action col, depending on snapshot icons */
    $action_col_size = 120;

    /* Toggle all selected / deselected */
    $chk = "<input type='checkbox' id='select_all' name='select_all'
               onClick='toggle_all_(\"^item_selected_[0-9]*$\",\"select_all\");' >";

    /* set Page header */
    $this->AddHeader(array("string"=> $chk,          "attach"=>"style='width:20px;'"));
    $this->AddHeader(array("string" => "&nbsp;",                "attach" => "style='text-align:center;width:20px;'"));
    $this->AddHeader(array("string" => _("Class name"),  "attach" => "style=''"));
    $this->AddHeader(array("string" => _("Class type"),         "attach" => "style='width:200px;'"));
    $this->AddHeader(array("string" => _("Actions"),            "attach" => "style='width:".$action_col_size."px;border-right:0px;text-align:right;'"));

    $this->AddCheckBox("ShowProfiles"   , sprintf(_("Display objects of type '%s'."),_("Profile"))  ,_("Show profiles")     ,true);
    $this->AddCheckBox("ShowTemplates"  , sprintf(_("Display objects of type '%s'."),_("Variable")) ,_("Show templates")    ,true);
    $this->AddCheckBox("ShowScripts"    , sprintf(_("Display objects of type '%s'."),_("Script"))   ,_("Show scripts")      ,true);
    $this->AddCheckBox("ShowHooks"      , sprintf(_("Display objects of type '%s'."),_("Hook"))     ,_("Show hooks")        ,true);
    $this->AddCheckBox("ShowVariables"  , sprintf(_("Display objects of type '%s'."),_("Variable")) ,_("Show variables")    ,true);
    $this->AddCheckBox("ShowPackages"   , sprintf(_("Display objects of type '%s'."),_("Package"))  ,_("Show packages")     ,true);
    $this->AddCheckBox("ShowPartitions" , sprintf(_("Display objects of type '%s'."),_("Partition")),_("Show partitions")   ,true);

    /* Add SubSearch checkbox */
    //$this->AddCheckBox(SEPERATOR);
    //$this->AddCheckBox("SubSearch",  msgPool::selectToView("","subsearch"), msgPool::selectToView("","subsearch_small"), false);

    /*                  Name                 ,Text                              ,Default  , Connect with alphabet  */
    $this->AddRegex   ("Regex",     _("Display users matching"),"*" , true);
  }

  function AddUserBoxToFilter($position)
  {
    $str = "";
    if($position  == 2){

      /* Check if there are post commands available for fai management.
       * If not, grey out freeze/branch and release remove buttons.
       */
      $r = $this->config->search("faiManagement", "POSTREMOVE",array('menu','tabs'));
      $c = $this->config->search("faiManagement", "POSTCREATE",array('menu','tabs'));

      $smarty = get_smarty();
      $releases = $this->parent->getBranches();
      $smarty->assign("allow_create", $c);
      $smarty->assign("allow_remove", $r);
      $smarty->assign("fai_release" , $releases[$this->parent->fai_release]);
      $smarty->assign("fai_base"    , $releases[$this->parent->fai_base]);
      $smarty->assign("branchimage","images/branch.png");
      $smarty->assign("fai_releases", $releases);
      $plug_id = 0;       
      if(isset($_GET['plug'])){
        $plug_id = $_GET['plug'];
      }
      $smarty->assign("plug_id",$plug_id);
      $str = $smarty->fetch(get_template_path('branch_selector.tpl', TRUE));
    }
    return($str);
  }

  function GenHeader()
  {
    /* Get all departments within this subtree */
    $ui= get_userinfo();
    $first = "";
    $found = FALSE;
    $base = $this->config->current['BASE'];

    /* Add seperator */
    $add_sep = false;

    /* Get complete fai acls, to be able to check if we must show or hide the snapshot abilities */ 
    $acl_all = $ui->has_complete_category_acls($this->parent->fai_base,$this->module);

    /* Add default header */
    $listhead = MultiSelectWindow::get_default_header();

    /* Add additional seperator */
    if($add_sep){
      $listhead .=" <img class='center' src='images/lists/seperator.png' align='middle' alt='' height='16' width='1'>&nbsp;";
    }

    $s  = ".|"._("Actions")."|\n";
    $s .= "..|<img src='images/list_new.png' alt='' border='0' class='center'>".
      "&nbsp;"._("Create")."|\n";

    $acl = $ui->get_permissions($this->parent->fai_base,"fai/faiProfile");
    if($this->parent->lock_type == "freeze" && !$this->parent->allow_freeze_object_attach){
      $s.= "...|<input class='center' type='image' src='images/freeze.png' alt=''>".
        "&nbsp;"._("Freezed")."|\n";
    }else{
      if(preg_match("/c/",$acl)){
        $s.= "...|<input class='center' type='image' src='images/fai_new_profile.png' alt=''>".
          "&nbsp;"._("Profile")."|Create_profile|\n";
      }

      /* Append create icons for each fai component */
      $arr = array(
          array("images/fai_new_partitionTable.png" , _("Partition table")  ,"Create_partition" , _("PT") , "faiPartitionTable"),
          array("images/fai_new_script.png"         , _("Scripts")          ,"Create_script"    , _("S")  , "faiScript"),
          array("images/fai_new_hook.png"           , _("Hooks")            ,"Create_hook"      , _("H")  , "faiHook"),
          array("images/fai_new_variable.png"       , _("Variables")        ,"Create_variable"  , _("V")  , "faiVariable"),
        array("images/fai_new_template.png"       , _("Templates ")       ,"Create_template"  , _("I")  , "faiTemplate"),
        array("images/fai_new_packages.png"       , _("Package list")     ,"Create_package"   , _("PK") , "faiPackage"));

      foreach($arr as $ar){
        $acl = $ui->get_permissions($this->parent->fai_base,"fai/".$ar[4]);
        if(preg_match("/c/",$acl)){
          $s.= "...|<input class='center' type='image' src='".$ar[0]."' alt=''>".
            "&nbsp;".$ar[1]."|".$ar[2]."|\n";
        }
      }
    }

    /* Multiple options */
    $s.= "..|<img src='images/lists/trash.png' alt='' border='0' class='center'>".
      "&nbsp;"._("Remove")."|"."remove_multiple|\n";

    /* Add multiple copy & cut icons */
    if(is_object($this->parent->CopyPasteHandler)){
      $s.= "..|---|\n";
      $s.= "..|<img src='images/lists/copy.png' alt='' border='0' class='center'>".
        "&nbsp;"._("Copy")."|"."multiple_copy_systems|\n";
      $s.= "..|<img src='images/lists/cut.png' alt='' border='0' class='center'>".
        "&nbsp;"._("Cut")."|"."multiple_cut_systems|\n";
    }

    /* Add copy & paste icons, currently disabled, this ability is not implemeneted yet */
    if(!($this->parent->lock_type == "freeze" && !$this->parent->allow_freeze_object_attach) && 
      preg_match("/(c.*w|w.*c)/",$acl_all) &&  isset($this->parent->CopyPasteHandler) && $this->parent->CopyPasteHandler){
      $s .= "..|---|\n";
      if($this->parent->CopyPasteHandler->entries_queued()){
        $img = "<img border='0' class='center' src='images/lists/paste.png' alt=''>";
        $s.="..|".$img."&nbsp;"._("Paste")."|editPaste|\n";
      }else{
        $img = "<img border='0' class='center' src='images/lists/paste.png' alt=''>";
        $s.="..|".$img."&nbsp;"._("Paste")."\n";
      }
    }

#   /* Add snapshot icons, if we are allowed to write and create the complete fai module ($ui->has_complete_category_acls) */ 
#   if(preg_match("/(c.*w|w.*c)/",$acl_all)){ 
#     if($this->parent->snapshotEnabled()){
#       $s .= "..|---|\n";
#       $s .= $this->get_snapshot_header(TRUE);
#     }
#   }

    $this->SetDropDownHeaderMenu($s);
    $this->SetListHeader($listhead);
  }

  /* so some basic settings */
  function execute()
  {
    $this->ClearElementsList();
    $this->GenHeader();
    $this->AvailableBranches = $this->parent->getBranches();
  }

  function setEntries($list)
  {
    /********************
      Variable init
     ********************/

     $objects = array(
           "FAIpartitionTable"  => array("IMG"=> "images/fai_partitionTable.png", "NAME"=>_("Partition table"),"KZL"=> "PT", "VAR"=>"ShowPartitions"),
           "FAIpackageList"     => array("IMG"=> "images/fai_packages.png",       "NAME"=>_("Package list") ,  "KZL"=> "PL", "VAR"=>"ShowPackages"),
           "FAIscript"          => array("IMG"=> "images/fai_script.png",         "NAME"=>_("Scripts") ,       "KZL"=> "S",  "VAR"=>"ShowScripts"),
           "FAIvariable"        => array("IMG"=> "images/fai_variable.png",       "NAME"=>_("Variables") ,     "KZL"=> "V",  "VAR"=>"ShowVariables"),
           "FAIhook"            => array("IMG"=> "images/fai_hook.png",           "NAME"=>_("Hooks"),          "KZL"=> "H",  "VAR"=>"ShowHooks"),
           "FAIprofile"         => array("IMG"=> "images/fai_profile.png",        "NAME"=>_("Profile") ,       "KZL"=> "P",  "VAR"=>"ShowProfiles"),
           "FAItemplate"        => array("IMG"=> "images/fai_template.png",       "NAME"=>_("Templates") ,     "KZL"=> "T",  "VAR"=>"ShowTemplates"));


    /* Dynamic action col, depending on snapshot icons */
    $action_col_size = 120;
#   if($this->parent->snapshotEnabled()){
#     $action_col_size += 32;
#   }

    /********************
      Attach objects
     ********************/

    $cnts = array();
    foreach($objects as $key => $data){
      $cnts[$key] = 0;
    }

     foreach($list as $key => $value){
       $info     = "";
       $img      = "";
       $type     = $value['type'];
       $abort    =false;

       $cnts[$type] ++;

       if(isset($objects[$type])){
         $img   = "<img class='center' src='".$objects[$type]['IMG']."' title='".$objects[$type]['NAME']."' alt='".$objects[$type]['KZL']."'>";
         $info  = $objects[$type]['NAME'];
         $var   = $objects[$type]['VAR'];
       }else{
         $img   = "<img class='center' src='images/empty.png' alt=''>";
         $info  = "";
         $var   = "";
       }

       if((isset($value['description']))&&(!empty($value['description']))){
         $desc= " [".$value['description']."]";
       }else{
         $desc= "";
       }

       /* Add copy & cut icons */
       $ui = get_userinfo();
       $action ="";
       $acl_all = $ui->has_complete_category_acls($this->parent->fai_base,$this->module);
       if(preg_match("/(c.*w|w.*c)/",$acl_all) && $this->parent->CopyPasteHandler){
         $action .= "<input class='center' type='image'
           src='images/lists/cut.png' alt='"._("cut")."' name='cut_%KEY%' title='"._("Cut this entry")."'>&nbsp;";
         $action.= "<input class='center' type='image'
           src='images/lists/copy.png' alt='"._("copy")."' name='copy_%KEY%' title='"._("Copy this entry")."'>&nbsp;";
       }

       if(preg_match("/^freeze/", $value['FAIstate'])){
         $editlink ="<a href='?plug=".$_GET['plug']."&amp;edit_freeze_entry=%KEY%' title='%TITLE%'>%NAME%</a>";
         $action.=  "<input class='center' type='image' src='images/lists/edit.png'  alt='"._("edit")."'
           name='entry_freeze_edit_%KEY%' title='"._("Edit class")."'>";
         $action.=  "<img src='images/lists/locked.png' alt='F' class='center'>";
       }else{
         $editlink ="<a href='?plug=".$_GET['plug']."&amp;edit_entry=%KEY%' title='%TITLE%'>%NAME%</a>";
         $action.= "<input class='center' type='image' src='images/lists/edit.png'  alt='"._("edit")."'
           name='entry_edit_%KEY%' title='"._("Edit class")."'>";
         if(preg_match("/d/",$value['acl'])){
           $action.= "<input class='center' type='image' src='images/lists/trash.png' alt='"._("delete")."'
             name='entry_delete_%KEY%' title='"._("Delete class")."'>";
         }
       }

       /* Cutted objects should be displayed in light grey */
       $display = $value['cn'].$desc;
       if($this->parent->CopyPasteHandler){
         foreach($this->parent->CopyPasteHandler->queue as $queue_key => $queue_data){
           if($queue_data['dn'] == $value['dn']) {
             $display = "<font color='#999999'>".$display."</font>";
             break;
           }
         }
       }
       $edi = $editlink;
       $acti = $action;

       /* Create each field */
       $field0 = array("string" => "<input type='checkbox' id='item_selected_".$key."' name='item_selected_".$key."'>" ,
           "attach" => "style='width:20px;'");
       $field1 = array("string" => $img , "attach" => "style='text-align:center;width:20px;'");
       $field2 = array("string" => preg_replace(array("/%KEY%/","/%NAME%/","/%TITLE%/"),array($key,$display,preg_replace('/ /', '&nbsp;', @LDAP::fix($value['dn']))),$edi) , "attach" => "style=''");
       $field3 = array("string" => $info, "attach" => "style='width:200px;'");
       $field4 = array("string" => preg_replace("/%KEY%/",$key,$acti) , "attach" => "style='width:".$action_col_size."px;border-right:0px;text-align:right;'");
       $this->AddElement(array($field0,$field1,$field2,$field3,$field4));
     }

     /* Create summary string for list footer */
     $num_deps=0;
     if(!$this->SubSearch){
       $num_deps = count($this->Added_Departments);
     }

     $profile_str    = sprintf(_("Number of listed '%s'"),_("profiles"));
     $partition_str  = sprintf(_("Number of listed '%s'"),_("partitions"));
     $script_str     = sprintf(_("Number of listed '%s'"),_("scripts"));
     $hook_str       = sprintf(_("Number of listed '%s'"),_("hooks"));
     $variable_str   = sprintf(_("Number of listed '%s'"),_("variables"));
     $template_str   = sprintf(_("Number of listed '%s'"),_("templates"));
     $package_str    = sprintf(_("Number of listed '%s'"),_("packages"));   
     $dep_str        = sprintf(_("Number of listed '%s'"),_("departments"));

     $str = "<img class='center' src='".$objects['FAIprofile']['IMG']."'
       title='".$profile_str."' alt='".$profile_str."'>&nbsp;".$cnts['FAIprofile']."&nbsp;&nbsp;&nbsp;&nbsp;";
     $str.= "<img class='center' src='".$objects['FAIpartitionTable']['IMG']."'
       title='".$partition_str."' alt='".$partition_str."'>&nbsp;".$cnts['FAIpartitionTable']."&nbsp;&nbsp;&nbsp;&nbsp;";
     $str.= "<img class='center' src='".$objects['FAIscript']['IMG']."'
       title='".$script_str."' alt='".$script_str."'>&nbsp;".$cnts['FAIscript']."&nbsp;&nbsp;&nbsp;&nbsp;";
     $str.= "<img class='center' src='".$objects['FAIhook']['IMG']."'
       title='".$hook_str."' alt='".$hook_str."'>&nbsp;".$cnts['FAIhook']."&nbsp;&nbsp;&nbsp;&nbsp;";
     $str.= "<img class='center' src='".$objects['FAIvariable']['IMG']."'
       title='".$variable_str."' alt='".$variable_str."'>&nbsp;".$cnts['FAIvariable']."&nbsp;&nbsp;&nbsp;&nbsp;";
     $str.= "<img class='center' src='".$objects['FAItemplate']['IMG']."'
       title='".$template_str."' alt='".$template_str."'>&nbsp;".$cnts['FAItemplate']."&nbsp;&nbsp;&nbsp;&nbsp;";
     $str.= "<img class='center' src='".$objects['FAIpackageList']['IMG']."'
       title='".$package_str."' alt='".$package_str."'>&nbsp;".$cnts['FAIpackageList']."&nbsp;&nbsp;&nbsp;&nbsp;";
     $str.= "<img class='center' src='images/lists/folder.png'
       title='".$dep_str."' alt='".$dep_str."'>&nbsp;".$num_deps."&nbsp;&nbsp;&nbsp;&nbsp;";
     $this->set_List_Bottom_Info($str);
  }

  function Save()
  {
    MultiSelectWindow::Save();  
  }

  function save_object()
  {
    /* Save automatic created POSTs like regex, checkboxes */
    MultiSelectWindow::save_object(); 
  }
}
// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
