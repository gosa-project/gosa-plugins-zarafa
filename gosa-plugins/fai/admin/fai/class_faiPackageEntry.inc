<?php

class faiPackageEntry extends plugin
{
  /* attribute list for save action */
  var $ignore_account= TRUE;
  var $attributes   = array();
  var $objectclasses= array();
  var $pkgs         = array();  

  var $package_sections   = array();
  var $searchStrPKG       = "*";
  var $selected           = array();

  /* Array with available class names */
  var $FAIAllclasses        = array();
  var $FAIdebianRelease ="";

  function faiPackageEntry (&$config,$release, $used_packages=array())
  {
    plugin::plugin ($config, NULL);

    $this->FAIdebianRelease = $release;
    $this->selected         = $used_packages;
    $this->initializeSections();

    /* Get old search filter, if there is one*/ 
    $this->searchStrPKG     = _("Please enter your search string here");
    if(session::is_set("searchStrPKG")){
      $this->searchStrPKG = session::get("searchStrPKG");
    }    
  }


  /*! \brief  Load all release seactions from GOsa support daemon
   */  
  function initializeSections()
  {
    $this->package_sections = array();
    $q = new gosaSupportDaemon();
    $ret = $q->FAI_get_packages($this->FAIdebianRelease,array("distinct section"));
    if($q->is_error()){
      msg_dialog::display(_("Service infrastructure"),msgPool::siError($q->get_error()),ERROR_DIALOG);
    }else{
      foreach($ret as $entry){
        $this->package_sections[array_pop($entry)] = TRUE; 
      }
    }
  }


  /*! \brief  Create HTML for this plugin.
      @return String The html code for this plugin.
   */
  function execute()
  {
    /* Call parent execute */
    plugin::execute();

    /* Fill templating stuff */
    $smarty     = get_smarty();
    $display = "";


    /* Create section checkboxes 
     */ 
    $Section_boxes = "<input type='hidden' value='chkchange' name='chkchange'><table summary=''><tr>";
    $i = 0;
    foreach($this->package_sections as $cat => $selected){
      if($i == 3){
        $Section_boxes .="</tr><tr>";
        $i = 0;
      }
      if($selected){
        $Section_boxes .="<td><input type='checkbox' value='1' name='CAT".$cat."' checked ".
          "onclick='document.mainform.submit()'>&nbsp;".$cat."&nbsp;</td>";
      }else{
        $Section_boxes .="<td><input type='checkbox' value='1' name='CAT".$cat."' ".
          "onclick='document.mainform.submit()'>&nbsp;".$cat."&nbsp;</td>";
      }
      $i ++ ;
    }
    $Section_boxes .= "</tr></table>";

    /* Create List && fill it with packages
     */
    $cnt = 0; 
    $this->reload();
    $divlist = new divlist("PackageEntry");
    $divlist->SetPluginMode();
    $divlist->SetSummary(_("This list displays all assigned package names for this repository settings."));
    $divlist->SetEntriesPerPage(0);
    #$divlist->SetHeight("650px;");
    $divlist->SetHeader(array(array("string" => "&nbsp;", "attach" => "style='text-align:center;width:20px;'"),
                              array("string" => "Name", "attach" => "style='width:180px;'"),
                              array("string" => "Version", "attach"=>"style='width:240px;'" ),
                              array("string" => "Description", "attach" => "style='border-right:0px;text-align:left;'")));

    foreach($this->pkgs as $pkg){
      if(in_array($pkg['PACKAGE'],$this->selected)){
        $check = " checked ";
      }else{
        $check = "";
      }
      $chkbox = "<input type='checkbox' value='1' name='use_".base64_encode($pkg['PACKAGE'])."' ".$check.">
        <input type='hidden' value='1' name='wasonpage_".base64_encode($pkg['PACKAGE'])."'>";
      $divlist->AddEntry(array(array("string"=>$chkbox, "attach" => "style='text-align:center;width:20px;'"),
            array("string"=>$pkg['PACKAGE'], "attach" => "style='width:180px;'"),
            array("string"=>"<div style='width:238px; overflow:hidden;'>".htmlentities($pkg['VERSION']),   "attach" => "style='width:240px'"),
            array("string"=>htmlentities($pkg['DESCRIPTION']), "attach" => "style='border-right:0px;text-align:left;'")));
    }

    $smarty->assign("Section_boxes"          , $Section_boxes);
    $smarty->assign("searchStrPKG"  , $this->searchStrPKG);
    $smarty->assign("faihead"       , "");
    $smarty->assign("failist"       , $divlist->DrawList());
    $smarty->assign("infoimage"     , get_template_path('images/info.png'));
    $smarty->assign("launchimage"   , get_template_path('images/launch.png'));
    $smarty->assign("alphabet"      , generate_alphabet());
    $smarty->assign("apply"         , apply_filter(TRUE));
    $smarty->assign("search_image"  , get_template_path('images/search.png'));

    $display.= $smarty->fetch(get_template_path('faiPackageEntry.tpl', TRUE));
    return($display);
  }


  /*! \brief  Save HTML POST data to object 
   */
  function save_object()
  {
    /* Keep sections selected */
    foreach($_POST as $name => $value){
      if(preg_match("/^wasonpage_/i",$name)){
        $name = preg_replace("/^wasonpage_/","",$name);
        $n =base64_decode($name);
        if(isset($_POST['use_'.$name])){
          $this->selected[$n] = $n;
        }else{
          if(isset($this->selected[$n])){
            unset($this->selected[$n]);
          }
        }

      }
    }

    /* Get Posted search filter 
     */
    if(isset($_POST['searchStrPKG'])){
      $this->searchStrPKG = get_post("searchStrPKG");
    }
    if(isset($_GET['search'])){
      $this->searchStrPKG= $_GET['search']."*";
    }
    if(empty($this->searchStrPKG)){
      $this->searchStrPKG = "*";
    }
    session::set('searchStrPKG',$this->searchStrPKG);
  }


  /*! \brief  Reload list of displayed packages 
   */
  function reload()
  {
    $this->pkgs = array(); 
    $q          = new gosaSupportDaemon();
    $filter     = preg_replace("/\*/","%",array($this->searchStrPKG));
    $attrs      = array("distribution", "package","version", "section", "description", "timestamp");
    $ret        = $q->FAI_get_packages($this->FAIdebianRelease,$attrs,$filter);
    if($q->is_error()){
      msg_dialog::display(_("Service infrastructure"),msgPool::siError($q->get_error()),ERROR_DIALOG);
    }else{
      foreach($ret as $attr){
        $this->pkgs[$attr['PACKAGE']] = $attr;
      }
    }
  }


  /*! \brief  Return all selected packages 
      @return Array  All selected package names
   */  
  function save()
  {
    return($this->selected);
  }
}
// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
