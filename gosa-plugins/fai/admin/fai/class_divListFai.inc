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

  var $ShowOpsiLocal;
  var $ShowOpsiNetboot;

  function divListFai (&$config,$parent)
  {
    MultiSelectWindow::MultiSelectWindow($config, "Fai", "fai");
    
    $this->parent       = $parent;
    $this->ui           = get_userinfo();

    /* Set list strings */
    $this->SetTitle(  _("List of classes"));
    $this->SetSummary(_("List of deployment classes and products"));

    /* Result page will look like a headpage */
    $this->SetHeadpageMode();

    $this->EnableAplhabet   (true);
  
    /* Disable buttonsm */
    $this->EnableCloseButton(false);
    $this->EnableSaveButton (false);

    /* Dynamic action col, depending on snapshot icons */
    $action_col_size = 130;

    /* Toggle all selected / deselected */
    $chk = "<input type='checkbox' id='select_all' name='select_all' title='"._("Select all")."'
               onClick='toggle_all_(\"^item_selected_[0-9]*$\",\"select_all\");' >";

    /* set Page header */
    $this->AddHeader(array("string"=> $chk,          "attach"=>"style='width:20px;'"));
    $this->AddHeader(array("string" => _("Class name"),  "attach" => "style=''"));
    $this->AddHeader(array("string" => _("Class type"),         "attach" => "style='width:200px;'"));
    $this->AddHeader(array("string" => _("Actions"),            "attach" => "style='width:".$action_col_size."px;border-right:0px;text-align:right;'"));

    $this->AddCheckBox("ShowProfiles"   , sprintf(_("Display objects of type '%s'."),
          _("Profile"))  ,_("Show profiles")     ,true);
    $this->AddCheckBox("ShowTemplates"  , sprintf(_("Display objects of type '%s'."),
          _("Variable")) ,_("Show templates")    ,true);
    $this->AddCheckBox("ShowScripts"    , sprintf(_("Display objects of type '%s'."),
          _("Script"))   ,_("Show scripts")      ,true);
    $this->AddCheckBox("ShowHooks"      , sprintf(_("Display objects of type '%s'."),
          _("Hook"))     ,_("Show hooks")        ,true);
    $this->AddCheckBox("ShowVariables"  , sprintf(_("Display objects of type '%s'."),
          _("Variable")) ,_("Show variables")    ,true);
    $this->AddCheckBox("ShowPackages"   , sprintf(_("Display objects of type '%s'."),
          _("Package"))  ,_("Show packages")     ,true);
    $this->AddCheckBox("ShowPartitions" , sprintf(_("Display objects of type '%s'."),
          _("Partition")),_("Show partitions")   ,true);

    /* Add opsi checkboxes if opsi is enabled 
     */
    if($this->parent->opsi instanceof opsi && $this->parent->opsi->enabled()){
      $this->AddCheckBox("ShowOpsiNetboot" , sprintf(_("Display objects of type '%s'."),
        _("OPSI netboot product")),_("Show netboot products")   ,true);
      $this->AddCheckBox("ShowOpsiLocal" , sprintf(_("Display objects of type '%s'."),
        _("OPSI local product")),_("Show local products")   ,true);
    }

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
      $smarty->assign("branchimage","plugins/fai/images/branch.png");
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

    /* Add default header */
    $listhead = MultiSelectWindow::get_default_header(false);

    /* Add additional seperator */
    if($add_sep){
      $listhead .=" <img class='center' src='images/lists/seperator.png' align='middle' alt='' height='16' width='1'>&nbsp;";
    }

    $s  = ".|"._("Actions")."|\n";
    $s .= "..|<img src='images/lists/new.png' alt='' border='0' class='center'>".
      "&nbsp;"._("Create")."|\n";

    $acl = $ui->get_permissions($this->parent->acl_base,"fai/faiProfile");
    if($this->parent->lock_type == "freeze" && !$this->parent->allow_freeze_object_attach){
      $s.= "...|<input class='center' type='image' src='plugins/fai/images/freeze.png' alt=''>".
        "&nbsp;"._("Freezed")."|\n";
    }else{
      if(preg_match("/c/",$acl)){
        $s.= "...|<input class='center' type='image' src='plugins/fai/images/fai_new_profile.png' alt=''>".
          "&nbsp;"._("Profile")."|Create_profile|\n";
      }

      /* Append create icons for each fai component */
      $arr = array(
          array("plugins/fai/images/fai_new_partitionTable.png" , _("Partition table")  ,
            "Create_partition" , _("PT") , "faiPartitionTable"),
          array("plugins/fai/images/fai_new_script.png"         , _("Scripts")          ,
            "Create_script"    , _("S")  , "faiScript"),
          array("plugins/fai/images/fai_new_hook.png"           , _("Hooks")            ,
            "Create_hook"      , _("H")  , "faiHook"),
          array("plugins/fai/images/fai_new_variable.png"       , _("Variables")        ,
            "Create_variable"  , _("V")  , "faiVariable"),
          array("plugins/fai/images/fai_new_template.png"       , _("Templates ")       ,
            "Create_template"  , _("I")  , "faiTemplate"),
          array("plugins/fai/images/fai_new_packages.png"       , _("Package list")     ,
            "Create_package"   , _("PK") , "faiPackage"));

      foreach($arr as $ar){
        $acl = $ui->get_permissions($this->parent->acl_base,"fai/".$ar[4]);
        if(preg_match("/c/",$acl)){
          $s.= "...|<input class='center' type='image' src='".$ar[0]."' alt=''>".
            "&nbsp;".$ar[1]."|".$ar[2]."|\n";
        }
      }
    }

    /* Multiple options */
    $s.= "..|<img src='images/lists/trash.png' alt='' border='0' class='center'>".
      "&nbsp;"._("Remove")."|"."remove_multiple|\n";


    /* Add Copy & Paste header */
    $s .= $this->parent->get_copypaste_header($this->selectedBase,$this->module,TRUE,FALSE);

#   /* Add snapshot icons, if we are allowed to write and create the complete fai module ($ui->has_complete_category_acls) */ 
#   $s .= $this->parent->get_snapshot_header($this->selectedBase,$this->module);

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

    $edit_link = "<a href=\"?plug=".$_GET['plug']."&amp;id=%s&amp;act=edit\">%s</a>";
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
          "NAME"=>_("OPSI local product")   ,     "KZL"=> "OL",  "VAR"=>"ShowOpsiLocal"));


    /* Dynamic action col, depending on snapshot icons */
    $action_col_size = 130;

    /********************
      Attach objects
     ********************/

    /* Initialize summary attributes 
     */
    $cnts = array();
    foreach($objects as $key => $data){
      $cnts[$key] = 0;
    }

    /* Walk through entries an build up a printable row for each object 
     */
    foreach($list as $key => $value){

      /* Build up edit icons - depending on read acls for the current type 
       */
      $name = "";
      $name = "";
      $icon_list = "";
      $opsi_only = TRUE;
      foreach($objects as $type => $type_data){
        if(isset($value[$type])){
          $name = $value[$type]['cn'];
          $icon_list .= "<input type='image' src='".$type_data['IMG']."' title='".$type_data['NAME']."' 
            alt='".$type_data['KZL']."' class='center' name='edit_".$key."_".$type."'>\n";
          $cnts[$type] ++;
          if(!in_array($type,array("opsi_local","opsi_netboot"))){
            $opsi_only = FALSE;
          }
        }else{
          $icon_list .= "<img src='images/empty.png' alt=' ' class='center'>\n";
        }
      }

      /* Create checkbox
       */
      $checkbox = "&nbsp;";
      if(!$opsi_only){
        $checkbox = "<input type='checkbox' id='item_selected_".$key."' name='item_selected_".$key."'>";
      }
  
      /* Create actions string
       */
      $actions = "";
      $actions.= "<input type='image' name='group_edit_".$key."' class='center'
        src='images/lists/edit.png'>";
      if(!$opsi_only){
        $actions.= "<input type='image' name='group_remove_".$key."' class='center'
          src='images/lists/trash.png'>";
      }else{
        $actions.= "<img class='center' src='images/empty.png' alt=' '>";
      }

      /* Create list 
       */
      $field1 = array(
          "string" => $checkbox ,
          "attach" => "style='width:20px;'");
      $field2 = array(
          "string" => sprintf($edit_link,$key,$name),
          "attach" => "style=''");
      $field3 = array(
          "string" => $icon_list ,
          "attach" => "style='width:200px;'");
      $field4 = array(
          "string" => $actions,
          "attach" => "style='width:".$action_col_size."px;border-right:  0px;text-align:right;'");

      $this->AddElement(array($field1,$field2,$field3,$field4));
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
     $opsi_netboot   = sprintf(_("Number of listed '%s'"),_("netboot products"));
     $opsi_local     = sprintf(_("Number of listed '%s'"),_("local products"));

     $str = "<img class=\"center\" src=\"".$objects['FAIprofile']['IMG']."\"
       title=\"".$profile_str."\" alt=\"".$profile_str."\">&nbsp;".$cnts['FAIprofile']."&nbsp;&nbsp;&nbsp;&nbsp;";
     $str.= "<img class=\"center\" src=\"".$objects['FAIpartitionTable']['IMG']."\"
       title=\"".$partition_str."\" alt=\"".$partition_str."\">&nbsp;".$cnts['FAIpartitionTable']."&nbsp;&nbsp;&nbsp;&nbsp;";
     $str.= "<img class=\"center\" src=\"".$objects['FAIscript']['IMG']."\"
       title=\"".$script_str."\" alt=\"".$script_str."\">&nbsp;".$cnts['FAIscript']."&nbsp;&nbsp;&nbsp;&nbsp;";
     $str.= "<img class=\"center\" src=\"".$objects['FAIhook']['IMG']."\"
       title=\"".$hook_str."\" alt=\"".$hook_str."\">&nbsp;".$cnts['FAIhook']."&nbsp;&nbsp;&nbsp;&nbsp;";
     $str.= "<img class=\"center\" src=\"".$objects['FAIvariable']['IMG']."\"
       title=\"".$variable_str."\" alt=\"".$variable_str."\">&nbsp;".$cnts['FAIvariable']."&nbsp;&nbsp;&nbsp;&nbsp;";
     $str.= "<img class=\"center\" src=\"".$objects['FAItemplate']['IMG']."\"
       title=\"".$template_str."\" alt=\"".$template_str."\">&nbsp;".$cnts['FAItemplate']."&nbsp;&nbsp;&nbsp;&nbsp;";
     $str.= "<img class=\"center\" src=\"".$objects['FAIpackageList']['IMG']."\"
       title=\"".$package_str."\" alt=\"".$package_str."\">&nbsp;".$cnts['FAIpackageList']."&nbsp;&nbsp;&nbsp;&nbsp;";
     $str.= "<img class=\"center\" src='images/lists/folder.png'
       title=\"".$dep_str."\" alt=\"".$dep_str."\">&nbsp;".$num_deps."&nbsp;&nbsp;&nbsp;&nbsp;";

    if($this->parent->opsi instanceof opsi && $this->parent->opsi->enabled()){
      $str.= "<img class=\"center\" src=\"".$objects['opsi_netboot']['IMG']."\"
        title=\"".$opsi_netboot."\" alt=\"".$opsi_netboot."\">&nbsp;".$cnts['opsi_netboot']."&nbsp;&nbsp;&nbsp;&nbsp;";
      $str.= "<img class=\"center\" src=\"".$objects['opsi_local']['IMG']."\"
        title=\"".$opsi_local."\" alt=\"".$opsi_local."\">&nbsp;".$cnts['opsi_local']."&nbsp;&nbsp;&nbsp;&nbsp;";
    }
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
