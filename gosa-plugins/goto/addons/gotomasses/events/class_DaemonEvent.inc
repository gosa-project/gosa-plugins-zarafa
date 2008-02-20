<?php


/*

trigger_action_wake
ping
set_activated_for_installation
new_key_for_client
detect_hardware
trigger_action_localboot
trigger_action_reboot
trigger_action_halt
trigger_action_update
trigger_action_reinstall
trigger_action_memcheck
trigger_action_sysinfo
trigger_action_instant_update
trigger_action_rescan
*/


class DaemonEvent 
{
  /* Menu Strings */
  protected $s_Menu_Name  = "s_Menu_Name not set";  // Diplayed in the ActionsMenu->CreateNew
  protected $s_Event_Name = "s_Event_Name not set"; // Will be displayed in the Management list.
  protected $s_Menu_Image = "images/empty.png";     // The deamon command name.

  protected $target_add_list_used = FALSE; // Indicates that the target add list was used.
  protected $time_select_used     = FALSE; // Indicates that we have used the timestamp select boxes.
  protected $target_list_used     = FALSE; // Target list was diaplayed?
  protected $target_divlist       = NULL; // The divlist used by the target add dialog

  protected $config;          // GOsa configuration file 
  protected $data;            // The event data 

  protected $s_action     = "";       // The deamon command name.
  protected $a_targets    = array();  // The list of assigned Targets 
  protected $timestamp    = 0;        // Event execution time; 
  protected $id           = -1;       // The Table ID
  protected $status       = "unknown";// The current event status
  protected $is_new       = TRUE;     // Is TRUE if this is a new event
  protected $_target_list = array();  // Object Cache of those objects displayed in the target add dialog

  public function __construct($config,$data = array())
  {
    $this->data   = $data;
    $this->config = $config;
    timezone::get_default_timezone();
    $this->timestamp = time();

    /* Load values from given data */
    if(count($data)){
      $this->is_new = FALSE;
    }
  }

  public function execute()
  {
    $this->time_select_used = FALSE;
    $this->target_list_used = FALSE;

    $str = "<h2>"._("This event has no template.")."</h2>";
    $str.= "<p class='seperator'></p>";
    $str.= "<div style='text-align:right;width:100%;padding:3px;'>
              <input type='submit' name='abort_event_dialog' value='"._("Cancel")."'>
            </div>";
    return($str);
  }

 
  public function get_header()
  {
    $str = "<h2>"._("Daemon event")."&nbsp;-&nbsp;".$this->s_Event_Name."</h2>";
    return($str);
  }


  public function get_footer()
  {
    if($this->target_add_list_used){
      return("");
    }
    $str = "<p class='seperator'></p>";
    $str.= "<div style='text-align:right;width:100%;padding:5px;'>
              <input type='submit' name='save_event_dialog' value='"._("Save")."'>&nbsp;
              <input type='submit' name='abort_event_dialog' value='"._("Cancel")."'>
            </div>";
    return($str);
  }
 

  public function get_time_select()
  {
    $this->time_select_used = TRUE;

    $smarty = get_smarty();

    $year   = date("Y",$this->timestamp);
    $month  = date("m",$this->timestamp);
    $day    = date("d",$this->timestamp);

    $hour   = date("H",$this->timestamp);
    $minute = date("i",$this->timestamp);
    $second = date("s",$this->timestamp);

    $years = array();
    for($i = date("Y",time()); $i <= 2037 ;$i ++){
      $years[$i] = $i;
    }
    $months = array();
    for($i = 1; $i <= 12; $i ++){
      $e = str_pad($i,2,"0",STR_PAD_LEFT);
      $months[$e] = $e;
    }
    $days = array();
    for($i = 1; $i <= cal_days_in_month(CAL_GREGORIAN,$month,$year); $i ++){
      $e = str_pad($i,2,"0",STR_PAD_LEFT);
      $days[$e] = $e;
    }
    $hours = array();
    for($i = 0; $i < 24; $i ++){
      $e = str_pad($i,2,"0",STR_PAD_LEFT);
      $hours[$e] = $e;
    }
    $minutes = array();
    for($i = 0; $i < 60; $i ++){
      $e = str_pad($i,2,"0",STR_PAD_LEFT);
      $minutes[$e] = $e;
    }
    $seconds = array();
    for($i = 0; $i < 60; $i ++){
      $e = str_pad($i,2,"0",STR_PAD_LEFT);
      $seconds[$e] = $e;
    }

    $smarty->assign("years", $years);
    $smarty->assign("months", $months);
    $smarty->assign("days", $days);
    $smarty->assign("hours", $hours);
    $smarty->assign("minutes", $minutes);
    $smarty->assign("seconds", $seconds);
    $smarty->assign("time_year",$year);
    $smarty->assign("time_month",$month);
    $smarty->assign("time_day",$day);
    $smarty->assign("time_hour",$hour);
    $smarty->assign("time_minute",$minute);
    $smarty->assign("time_second",$second);
    return($smarty->fetch(get_template_path('timestamp_select.tpl', TRUE, dirname(__FILE__))));
  } 


  public function get_target_list()
  {
    $this->target_list_used = TRUE;
    $divlist = new divSelectBox("EventTargets");
    foreach($this->a_targets as $key => $target){
      $divlist->AddEntry(array(
            array("string"  => $target),
            array("string"  => "<input type='image' src='images/edittrash.png' title='"._("Remove")."' name='del_target_".$key."'>",
              "attach"  => "style='width:20px; border-right:0px;'")
            ));
    }
    $list_footer = "<input type='submit' name='open_target_list' value='"._("Add target")."'>";
    return($divlist->DrawList().$list_footer);
  }

  public function get_target_add_list()
  {
    $this->target_add_list_used = TRUE;
    $smarty = get_smarty();
 
    if($this->target_divlist == NULL){ 
      $this->target_divlist = new MultiSelectWindow($this->config,"EventAddTargedtList","gotomasses");
      $this->target_divlist->SetSummary(_("Targets"));
      $this->target_divlist->SetInformation(_("This dialog shows all available targets for your event, check the targets you want to add and use the 'Use' button to accept."));
      $this->target_divlist->EnableCloseButton(FALSE); 
      $this->target_divlist->EnableSaveButton(FALSE);


      /* Toggle all selected / deselected */
      $chk = "<input type='checkbox' id='select_all' name='select_all'
        onClick='toggle_all_(\"^item_selected_[0-9]*$\",\"select_all\");' >";

      $this->target_divlist->EnableAplhabet(TRUE);
  
      /* set Page header */
      $this->target_divlist->AddHeader(array("string"=> $chk,          "attach"=>"style='width:20px;'"));
      $this->target_divlist->AddHeader(array("string"=>"&nbsp;","attach"=>"style='width:20px;'"));
      $this->target_divlist->AddHeader(array("string"=>_("System / Department")));

      //$name,$string,$value,$conn,$image="images/search.png") 
      $this->target_divlist->AddRegex("regex" ,"regex"  ,"*"                , TRUE);
      $this->target_divlist->AddRegex("ipfrom","ipfrom" ,"0.0.0.0"          , FALSE);
      $this->target_divlist->AddRegex("ipto"  ,"ipto"   ,"255.255.255.255"  , FALSE);

      $this->target_divlist->display_server       = TRUE;
      $this->target_divlist->display_workstation  = TRUE;
      $this->target_divlist->display_ogroup       = TRUE;
      $this->target_divlist->filter_iprange       = FALSE;
      $this->target_divlist->AddCheckBox("display_server","1"     ,_("Display server"),TRUE);
      $this->target_divlist->AddCheckBox("display_workstation","1",_("Display workstation"),TRUE);
      $this->target_divlist->AddCheckBox("display_ogroup","1"     ,_("Display object groups"),TRUE);
      $this->target_divlist->AddCheckBox("filter_iprange","1"     ,_("Filter by IP range"),FALSE);

    }
    $this->target_divlist->ClearElementsList();

    $modules = array("server","workstation");

    /* Add base */
    $tmp = array();
    $base = $this->config->current['BASE'];
    $tmp[] = array("dn"=>$this->config->current['BASE']);
    $tmp=  array_merge($tmp,get_list("(&(|(ou=*)(description=*))(objectClass=gosaDepartment))", $modules, $base,
                    array("ou", "description"), GL_SIZELIMIT | GL_SUBSEARCH));

    $deps = array();
    foreach($tmp as $tm){
      $deps[$tm['dn']] = $tm['dn'];
    }

    $department = $departments = array();
    $ui= get_userinfo();
    $d = $ui->get_module_departments($modules);
    foreach($d as $department){
      $departments[$department] = $department;
    }
  
     /* Load possible departments */
    $ids = $this->config->idepartments;
    $first = "";
    $found = FALSE;
    $options = array();
    foreach($ids as $dep => $name){
      if(isset($deps[$dep]) && in_array_ics($dep, $departments)){

        /* Keep first base dn in mind, we could need this
         *  info if no valid base was found
         */
        if(empty($first)) {
          $first = $dep['dn'];
        }

        $value = $ids[$dep];
        if ($this->target_divlist->selectedBase == $dep){
          $found = TRUE;
          $options.= "<option selected='selected' value='".$dep."'>$value</option>";
        } else {
          $options.= "<option value='".$dep."'>$value</option>";
        }
      }
    }

    $listhead = $this->target_divlist->get_default_header();

    /* Add base selection */
    $listhead .= _("Base")."&nbsp; <select name='CurrentMainBase' onChange='mainform.submit()' class='center'>$options</select>".
      " <input class='center' type='image' src='images/list_submit.png' align='middle'
      title='"._("Submit department")."' name='submit_department' alt='". _("Submit")."'>&nbsp;";

    $this->target_divlist->SetListHeader($listhead);
    $this->target_divlist->AddDepartments($this->target_divlist->selectedBase,2,1);

    $_target_list = array();
    if($this->target_divlist->display_server){
      $_target_list = array_merge($_target_list,get_list("(&(cn=".$this->target_divlist->regex.")(objectClass=goServer))",
            "server",get_ou("serverou").$this->target_divlist->selectedBase,
            array("cn","objectClass","description","ipHostNumber","macAddress"),GL_NONE));
    }
    if($this->target_divlist->display_workstation){
      $_target_list = array_merge($_target_list,get_list("(&(cn=".$this->target_divlist->regex.")(objectClass=gotoWorkstation))",
            "workstation",get_ou("workstationou").$this->target_divlist->selectedBase,
            array("cn","objectClass","description","ipHostNumber","macAddress"),GL_NONE));
    }
    if($this->target_divlist->display_ogroup){
      $_target_list = array_merge($_target_list,get_list("(&(cn=".$this->target_divlist->regex.")(objectClass=gosaGroupOfNames))",
            "ogroups",get_ou("ogroupou").$this->target_divlist->selectedBase,
            array("cn","objectClass","description"),GL_NONE));
    }
    $this->_target_list = $_target_list;

    $tmp = array();
    foreach($this->_target_list as $key => $object){
      $tmp[$key] = $object['cn'][0];
    }
    natcasesort($tmp);

    foreach($tmp as $key => $dummy){

      $obj = $this->_target_list[$key];
      $name = $obj['cn'][0];
      if(isset($obj['description'])){
        $name .= "&nbsp;[".$obj['description'][0]."]";
      }
      if(isset($obj['macAddress'])){
        $name .= "&nbsp;- ".$obj['macAddress'][0]."";
      }
      if(isset($obj['ipHostNumber'])){
        $name .= "&nbsp;- ".$obj['ipHostNumber'][0]."";
      }

      $img ="";
      if(in_array("goServer",$obj['objectClass'])){
        $img = '<img class="center" src="images/select_server.png" alt="S" title="'._("Server").'">'; 

        if($this->target_divlist->filter_iprange){
          if(!isset($obj['ipHostNumber']) || !tests::is_in_ip_range($this->target_divlist->ipfrom,$this->target_divlist->ipto,$obj['ipHostNumber'][0])) {
            continue;
          }
        }        

      }elseif(in_array("gotoWorkstation",$obj['objectClass'])){
        $img = '<img class="center" src="images/select_workstation.png" alt="W" title="'._("Workstation").'">'; 
        if($this->target_divlist->filter_iprange){
          if(!isset($obj['ipHostNumber']) || !tests::is_in_ip_range($this->target_divlist->ipfrom,$this->target_divlist->ipto,              $obj['ipHostNumber'][0])) {
            continue;
          }
        }

      }elseif(in_array("gosaGroupOfNames",$obj['objectClass'])){
        $img = '<img class="center" src="images/select_ogroup.png" alt="O" title="'._("Object group").'">'; 
      }

      $field1 = array("string" => "<input type='checkbox' id='item_selected_".$key."' name='item_selected_".$key."'>");
      $field2 = array("string" => $img);
      $field3 = array("string" => $name , "attach" => "title='".$obj['dn']."'");
      $this->target_divlist->AddElement(array($field1,$field2,$field3));
    }

    $smarty->assign("divlist",$this->target_divlist->Draw());

    return($smarty->fetch(get_template_path('target_list.tpl', TRUE, dirname(__FILE__))));
  }
 
  public function save_object()
  {
    if(isset($_POST['open_target_list'])){
      $this->target_add_list_used =TRUE;
    }
    if($this->target_divlist != NULL){
      $this->target_divlist->save_object();
    }
    if($this->target_add_list_used){
      if(isset($_POST['abort_target_dialog'])){
        $this->target_add_list_used =FALSE;
        $this->target_divlist = NULL;
      }
      if(isset($_POST['save_target_dialog'])){
        $this->target_add_list_used =FALSE;
        foreach($this->list_get_selected_items() as $id){
          if(isset($this->_target_list[$id]['macAddress'][0])){
            $this->a_targets[] = $this->_target_list[$id]['macAddress'][0];
          }else{
            $this->a_targets[] = $this->_target_list[$id]['cn'][0];
          }
        }
        $this->target_divlist = NULL;
      }
    }

    if($this->time_select_used){
      $time_stamp_values_found = TRUE;
      foreach(array("time_year","time_month","time_day","time_hour","time_minute","time_second") as $attr){
        $time_stamp_values_found &= isset($_POST[$attr]);
      }
      if($time_stamp_values_found){
        $this->timestamp = mktime(
            $_POST['time_hour'],
            $_POST['time_minute'],        
            $_POST['time_second'],        
            $_POST['time_month'],        
            $_POST['time_day'],        
            $_POST['time_year']);
      }
    }

    if($this->target_list_used){
      foreach($_POST as $name => $value){
        if(preg_match("/^del_target_/",$name)){
          $id = preg_replace("/^del_target_([0-9]*)_.*/","\\1",$name);
          if(isset($this->a_targets[$id])){
            unset($this->a_targets[$id]);
          }
          break; 
        }
      }
    }
  }
 
  public function _event_to_timestamp($str)
  {
    return(strtotime($str));
  }

  public function _timestamp_to_event($stamp)
  {
    return(date("YmdHis",$stamp));
  }


  public function get_event_info()
  {
    $data =array();
    $data['NAME']         = get_class($this);
    $data['MenuName']     = $this->s_Menu_Name;
    $data['EventName']    = $this->s_Event_Name;
    $data['Action']       = $this->s_action;
    $data['MenuImage']    = "<img src='".$this->s_Menu_Image."' alt='".$this->s_Menu_Name."' border='0' class='center'>";
    return($data);
  }


  protected function is_target_list_open()
  {
    return($this->target_add_list_used);
  }

  
  static function get_event_types()
  {
    global $class_mapping,$config;
    $list = array();
    foreach($class_mapping as $name => $path){
      if(preg_match("/^DaemonEvent_/",$name)){
        $tmp  = new $name($config);
        $list[$name] = $tmp->get_event_info();
      }
    }
    return($list);
  }


  protected  function list_get_selected_items()
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
