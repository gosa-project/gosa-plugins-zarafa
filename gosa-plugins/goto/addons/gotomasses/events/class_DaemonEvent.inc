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

  protected $s_Menu_Name  = ""; // Diplayed in the ActionsMenu->CreateNew
  protected $s_Event_Name = ""; // Will be displayed in the Management list.
  protected $s_action     = ""; // The deamon command name.
  protected $s_Menu_Image = "images/empty.png"; // The deamon command name.

  protected $config;          // GOsa configuration file 
  protected $data;            // The event data 
  protected $timestamp = 0;   // Event execution time; 

  protected $time_select_used = FALSE; // Indicates that we have used the timestamp select boxes.
  protected $target_list_used = FALSE; // Indicates that the target list was used.

  protected $target_divlist = FALSE; // The divlist used by the target add dialog

  public function __construct($config,$data = array())
  {
    $this->data   = $data;
    $this->config = $config;
    timezone::get_default_timezone();
    $this->timestamp = time();
  }

  public function execute()
  {
    $this->time_select_used = FALSE;

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
    if($this->target_list_used){
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
    $smarty = get_smarty();
 
    if(!$this->target_divlist){ 
      $this->target_divlist = new MultiSelectWindow($this->config,"EventTargets","gotomasses");
      $this->target_divlist->SetSummary(_("Targets"));
      $this->target_divlist->SetInformation(_("This dialog shows all available targets for your event, check the targets you want to add and use the 'Use' button to accept."));
      $this->target_divlist->EnableCloseButton(FALSE); 
      $this->target_divlist->EnableSaveButton(FALSE); 
    }
    
    $smarty->assign("divlist",$this->target_divlist->Draw());

    return($smarty->fetch(get_template_path('target_list.tpl', TRUE, dirname(__FILE__))));
  }
 
  public function save_object()
  {
    if(isset($_POST['open_target_list'])){
      $this->target_list_used =TRUE;
    }
    if($this->target_divlist){
      $this->target_divlist->save_object();
    }
    if($this->target_list_used){
      if(isset($_POST['abort_target_dialog'])){
        $this->target_list_used =FALSE;
        $this->target_divlist = FALSE;
      }
      if(isset($_POST['save_target_dialog'])){
        $this->target_list_used =FALSE;
        echo "Targets auslesen";
        $this->target_divlist = FALSE;
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
    $data['EventName']   = $this->s_Event_Name;
    $data['Action']       = $this->s_action;
    $data['MenuImage']    = "<img src='".$this->s_Menu_Image."' alt='".$this->s_Menu_Name."' border='0' class='center'>";
    return($data);
  }


  protected function is_target_list_open()
  {
    return($this->target_list_used);
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
}

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
