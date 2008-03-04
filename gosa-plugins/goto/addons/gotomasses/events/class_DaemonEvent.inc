<?php
/*
 * This code is part of GOsa (http://www.gosa-project.org)
 * Copyright (C) 2003-2008 GONICUS GmbH
 *
 * ID: $$Id$$
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

define("SCHEDULED_EVENT",1);
define("TRIGGERED_EVENT",2);

define("HIDDEN_EVENT",0);
define("SYSTEM_EVENT",1);
define("USER_EVENT"  ,2);


/*! \brief    This is the event base class 
  \author   Fabian Hickert <hickert@gonicus.de>
  \version  1.00
  \date     26.02.2008

  This is the base class for all new daemon events.
  It implements most of the required functionality.
 */
class DaemonEvent 
{
  /* Menu Strings */
  protected $s_Menu_Name  = "s_Menu_Name not set";  // Diplayed in the ActionsMenu->CreateNew
  protected $s_Event_Name = "s_Event_Name not set"; // Will be displayed in the Management list.
  protected $s_Menu_Image = "images/empty.png";     // Image displayed in Actions->New
  protected $s_List_Image = "";                     // Image displayed in event listing

  public $config;          // GOsa configuration object 
  protected $data;            // The event data, when edited

  protected $a_targets    = array();  // The list of assigned Targets (When newly created)
  protected $s_Schedule_Action = "";       // The deamon command name when newly created. (e.g. job_trigger_action_halt)
  protected $s_Trigger_Action= "";       // The deamon command name when edited. (e.g. trigger_action_halt)
  protected $s_Queued_Action= "";       // The deamon command name when edited. (e.g. trigger_action_halt)
  protected $timestamp    = 0;        // Event execution time; 
  protected $id           = -1;       // The Table ID
  protected $status       = "unknown";// The current event status
  protected $is_new       = TRUE;     // Is TRUE if this is a new event

  protected $mode         = SCHEDULED_EVENT; // Default action is sheduled.

  /* Sub dialog hanlding */
  protected $target_divlist       = NULL;     // The divlist used by the target add dialog
  protected $target_add_list_used = FALSE;    // Indicates that the target add list was used.
  protected $time_select_used     = FALSE;    // Indicates that we have used the timestamp select boxes.
  protected $target_list_used     = FALSE;    // Target list was diaplayed?
  protected $_target_list         = array();  // Object Cache of those objects displayed in the target add dialog
  protected $workstation_list     = array();  // Used as cache in the target list.
  protected $server_list          = array();  // Used as cache in the target list.

  protected $visible_for          = HIDDEN_EVENT;
 
  function set_type($type)
  {
    $this->mode = $type;
  }


  function get_type()
  {
    return($this->mode);
  }

  /*! \brief  Class contructor.
    @param  Array   GOsa configuration object.
    @param  Array   Event data, only given when edited.
   */
  public function __construct($config,$data = array())
  {
    $this->data   = $data;
    $this->config = $config;
    timezone::get_default_timezone();
    $this->timestamp = time();

    /* Load values from given data */
    if(count($data)){
      $this->is_new = FALSE;

      $attrs = array("id" => "ID");
      foreach($attrs as $to => $from){
        $this->$to = $data[$from];
      }
      if(isset($data['TIMESTAMP'])){
        $this->timestamp = $this->_event_to_timestamp($data['TIMESTAMP']);
      }
    }
  }


  /*! \brief  Create the HTML output for the plugin. 
    @return String  The generated HTML output.
   */
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

  /*! \brief  Returns the plugin header, displayed in the template.
    @return String  HTML header part.
   */
  public function get_header()
  {
    if($this->target_add_list_used){
      return("");
    }
    $str = "<h2>"._("Daemon event")."&nbsp;-&nbsp;".$this->s_Event_Name."</h2>";
    return($str);
  }


  /*! \brief  Returns the plugin footer (save cancel), displayed in the template.
    @return String  HTML footer part.
   */
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


  /*! \brief  Returns HTML representation of a timestamp using <select> boxes. 
    @return Returns HTML content.
   */
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


  /*! \brief  HTML representation of all currently assigned targets using (divSelectBox).
    @return String Returns a listbox with all used targets.
   */
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


  /*! \brief  Returns HTML content, displaying a dialog which allows to add new targets.
    @return String HTML content. (EventTargetAddList)
   */
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


  /*! \brief  Handles all posted HTML data, including target add,remove...
   */
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


  /*! \brief  Converts a daemon timestamp into an unix timestamp. \
    e.g.  20080101125959 -> 1199188799 
    @param  A daemon timestamp  YYYYddmmHHiiss
    @return Integer  A unix timestamp.
   */
  public function _event_to_timestamp($str)
  {
    return(strtotime($str));
  }


  /*! \brief  Converts a unix timestamp in to a gosa-si timestamp. \
    e.g.  1199188799 -> 20080101125959
    @param  A unix timestamp (e.g. 1199188799)
    @return Integer  A daemon timestamp (e.g. 20080101125959).
   */
  public function _timestamp_to_event($stamp)
  {
    return(date("YmdHis",$stamp));
  }


  /*! \brief  Returns event information, like menu strings, images ... 
    @return   Array Event informations.
   */
  public function get_event_info()
  {
    $data =array();
    $data['CLASS_NAME']         = get_class($this);
    foreach(array("s_Menu_Name","s_Event_Name","s_Queued_Action","s_Schedule_Action","s_Trigger_Action") as $attr){
      $data[$attr]  = $this->$attr;
    }
    $data['MenuImage']    = "<img src='".$this->s_Menu_Image."' alt='".$this->s_Menu_Name."' border='0' class='center'>";
    $data['ListImage']    = "<img src='".$this->s_List_Image."' title='".$this->s_Event_Name."' 
      alt='".$this->s_Event_Name."' border='0' class='center'>";
    return($data);
  }


  /*! \brief  Check if we have opened the target add dialog. 
    @return   Boolean TRUE if we have opened the target add dialog else FALSE.
   */
  protected function is_target_list_open()
  {
    return($this->target_add_list_used);
  }


  /*! \brief  Returns a complete list of all available events.
    @return   Array   Containing $this->get_event_info() for all available events.
   */
  static function get_event_types($type)
  {
    global $class_mapping,$config;
    $list = array();
    $list['BY_CLASS'] = array();
    $list['BY_TRIGGERED_ACTION'] = array();
    $list['BY_SCHEDULED_ACTION'] = array();
    $list['BY_QUEUED_ACTION'] = array();

    foreach($class_mapping as $name => $path){
      if(preg_match("/^DaemonEvent_/",$name)){
        $tmp  = new $name($config);

        if($tmp->visible_for & $type){
          $evt  = $tmp->get_event_info();

          $list['BY_CLASS'][$name]                      = $evt;
          $list['BY_TRIGGERED_ACTION'][$evt['s_Trigger_Action']]  = $evt;
          $list['BY_SCHEDULED_ACTION'][$evt['s_Schedule_Action']] = $evt;
          $list['BY_QUEUED_ACTION']   [$evt['s_Queued_Action']]   = $evt;
        }
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


  /*! \brief  Returns the event tag to schedule a new action 
    @param    Returns the event e.g. 'job_trigger_action_wake'
   */
  public function get_schedule_action()
  {
    return($this->s_Schedule_Action);
  }

  //
  /*! \brief  Returns the event tag to schedule a new action 
    @param    Returns the event e.g. 'trigger_action_wake'
   */
  public function get_trigger_action()
  {
    return($this->s_Trigger_Action);
  }

  /*! brief  Returns an array containig all attributes \
    That should be written.
    @return Array e.g. 'status' => 'bla blub'  
   */ 
  public function save()
  {
    return(array("timestamp" => $this->_timestamp_to_event($this->timestamp)));
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


  /*! \brief  Returns the event ID
    @return Returns the event ID
   */
  public function get_id()
  {
    if($this->is_new){
      return(-1);
    }else{
      return($this->data['ID']);
    }
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


  /*! \brief Add a target MAC address 
      @param Array A List of all target that should be added.
   */
  public function set_timestamp($stamp) 
  {
    $this->timestamp = $stamp;
  }


  /*! \brief Add a target MAC address 
      @param Array A List of all target that should be added.
   */
  public function add_targets($targets) 
  {
    foreach($targets as $target){
      $this->a_targets[] = $target;
    }
  }
}

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
