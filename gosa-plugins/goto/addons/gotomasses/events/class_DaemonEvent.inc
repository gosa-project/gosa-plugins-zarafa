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
  protected $s_List_Image = "";     // The deamon command name.

  protected $target_add_list_used = FALSE; // Indicates that the target add list was used.
  protected $time_select_used     = FALSE; // Indicates that we have used the timestamp select boxes.
  protected $target_list_used     = FALSE; // Target list was diaplayed?
  protected $target_divlist       = NULL; // The divlist used by the target add dialog

  protected $config;          // GOsa configuration file 
  protected $data;            // The event data 

  protected $s_New_Action = "";       // The deamon command name.
  protected $s_Edit_Action= "";       // The deamon command name.
  protected $a_targets    = array();  // The list of assigned Targets 
  protected $timestamp    = 0;        // Event execution time; 
  protected $id           = -1;       // The Table ID
  protected $status       = "unknown";// The current event status
  protected $is_new       = TRUE;     // Is TRUE if this is a new event
  protected $_target_list = array();  // Object Cache of those objects displayed in the target add dialog

  protected $workstation_list = array();
  protected $server_list      = array();

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
    if($this->target_add_list_used){
      return("");
    }
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
 
    if($this->target_divlist == NULL){ 
      $this->target_divlist = new EventTargetAddList($this->config,$this);
    }
    $this->target_divlist->execute();

    $smarty = get_smarty();
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
          if(in_array("gosaGroupOfNames",$this->target_divlist->_target_list[$id]['objectClass'])){
            foreach($this->target_divlist->_target_list[$id]['member'] as $mem_dn){
              if(isset($this->target_divlist->workstation_list[$mem_dn])){
                $this->a_targets[] = $this->target_divlist->workstation_list[$mem_dn]['macAddress'][0];  
              }
              if(isset($this->target_divlist->server_list[$mem_dn])){
                $this->a_targets[] = $this->target_divlist->server_list[$mem_dn]['macAddress'][0];  
              }
            } 
          }else{
            if(isset($this->target_divlist->_target_list[$id]['macAddress'][0])){
              $this->a_targets[] = $this->target_divlist->_target_list[$id]['macAddress'][0];
            }
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
    $data['CLASS_NAME']         = get_class($this);
    foreach(array("s_Menu_Name","s_Event_Name","s_New_Action","s_Edit_Action") as $attr){
      $data[$attr]  = $this->$attr;
    }
    $data['MenuImage']    = "<img src='".$this->s_Menu_Image."' alt='".$this->s_Menu_Name."' border='0' class='center'>";
    $data['ListImage']    = "<img src='".$this->s_List_Image."' title='".$this->s_Event_Name."' 
                                alt='".$this->s_Event_Name."' border='0' class='center'>";
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
        $evt  = $tmp->get_event_info();
    
        $list['BY_CLASS'][$name]                      = $evt;
        $list['BY_EDIT_ACTION'][$evt['s_Edit_Action']]= $evt;
        $list['BY_NEW_ACTION'] [$evt['s_New_Action']] = $evt;
      }
    }
    return($list);
  }


  /*! \brief  Returns TRUE if this event is new. (Not edited)
      @return Boolean TRUE if new, else FALSE.
   */
  public function is_new()
  {
    return($this->is_new);
  }


  /*! \brief  Returns the event tag e.g. 'job_trigger_action_wake' 
      @param  Returns the event e.g. 'job_trigger_action_wake'
   */
  public function get_header_tag()
  {
    return($this->s_New_Action);
  }


  /*! brief  Returns an array containig all attributes \
             That should be written.
      @return Array e.g. 'status' => 'bla blub'  
   */ 
  public function save()
  {
    return(array());
  }


  /*! \brief  Returns the event targets
      @return Array  All selected targets.
   */ 
  public function get_targets()
  {
    return($this->a_targets);
  }


  /*! \brief  Returns the event timestamp in GOsa daemon format. 
      @return Returns the event timestamp (20081231120000)
   */
  public function get_timestamp()
  {
    return($this->_timestamp_to_event($this->timestamp));
  }


  /*! \brief  Returns a set of elements selected in a MultiSelectWindow
      @return Array[integer]=integer
   */
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
