<?php

class goto_log_view extends plugin
{
  
  var $mac;
  var $logs;
  var $event;
  var $parent;
  var $config;
  
  var $o_queue;  

  var $selected_date;
  var $selected_file = 0;

  var $attributes = array("selected_file","selected_date");
 
  function __construct($config,$event,&$parent)
  {
    $this->config = $config;
    $this->event = $event;
    $this->parent = $parent;
    
    /* Try to fetch logs for the given event (mac)
     */
    $this->o_queue = new gosaSupportDaemon();

    /* Fetch logs for this event (mac)
     */      
    if(isset($this->event['MACADDRESS'])){
      $this->mac = $this->event['MACADDRESS'];
    }
    $res = $this->o_queue->get_log_info_for_mac($this->mac);
    if($this->o_queue->is_error()){
      msg_dialog::display(_("Error"), $this->o_queue->get_error(), ERROR_DIALOG);
    }
    if(!isset($res[$this->mac]) || !is_array($res[$this->mac])){
      $this->logs = array();
    }else{
      $this->selected_date = key($res[$this->mac]);
      $this->logs = $res;
    }
  }

  function execute()
  {
    $smarty = get_smarty();
    $smarty->assign("logs",$this->logs);   
    $smarty->assign("logs_available", isset($this->logs[$this->mac]));
    $smarty->assign("mac",$this->mac);
    $smarty->assign("selected_file",$this->selected_file);
    $smarty->assign("selected_date",$this->selected_date);
    $smarty->assign("log_file", $this->get_log($this->mac,$this->selected_date,$this->selected_file));        
    return($smarty->fetch(get_template_path('log_view.tpl', TRUE)));
  }


  function get_log($mac,$date,$file)
  {
    $this->mac = "00:01:6c:9d:b9:fa";
    $res = $this->o_queue->get_log_file($mac,$date,$file);
    if($this->o_queue->is_error()){
      msg_dialog::display(_("Error"), $this->o_queue->get_error(), ERROR_DIALOG);
    }
    $res = nl2br(htmlentities($res));
    return($res);
  }

  
  function save_object()
  {
    plugin::save_object();
    foreach($this->attributes as $attr){
      if(isset($_POST[$attr])){
        $this->$attr = get_post($attr);
      }
    }
  }

}




// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
