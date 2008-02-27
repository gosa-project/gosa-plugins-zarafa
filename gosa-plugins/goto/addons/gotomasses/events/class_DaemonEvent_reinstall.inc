<?php

class DaemonEvent_reinstall extends DaemonEvent
{
  var $progress = -1;  
  var $status   = "none";  

  public function __construct($config,$data = array())
  {
    DaemonEvent::__construct($config,$data);
    $this->s_Menu_Name  = _("Reinstall");
    $this->s_Event_Name = _("Reinstall");
    $this->s_Schedule_Action = "job_trigger_action_reinstall";
    $this->s_Trigger_Action= "trigger_action_reinstall";
    $this->s_Menu_Image = "images/fai_small.png";
    $this->s_List_Image = "images/fai_small.png";
  
    if(!$this->is_new()){
      if(isset($data['PROGRESS'])){
        $this->progress = $data['PROGRESS'];
      }
      if(isset($data['STATUS'])){
        $this->progress = $data['STATUS'];
      }
    }
  }

  public function execute()
  {
    DaemonEvent::execute();

    $display = $this->get_header();

    $tmp = $this->data;

    /* Check if target add dialog is open */
    if($this->is_target_list_open() && $this->is_new){
      return($this->get_target_add_list());
    }

    $smarty = get_smarty();
    $smarty->assign("status"        , $this->status);
    $smarty->assign("progress"        , $this->progress);
    $smarty->assign("data"        , $this->data);
    $smarty->assign("target_list" , $this->get_target_list());
    $smarty->assign("is_new"      , $this->is_new);
    $smarty->assign("timestamp"   , $this->get_time_select());
    $display.= $smarty->fetch(get_template_path('DaemonEvent_reinstall.tpl', TRUE, dirname(__FILE__)));
    $display.= $this->get_footer();
    return($display);
  }

  public function save_object()
  {
    $ret = DaemonEvent::save_object();
    foreach(array("progress","status") as $attr){
      if(isset($_POST[$attr])){
        $this->$attr = get_post($attr);
      }
    }
  } 

  public function save()
  {
    $ret = DaemonEvent::save();
    foreach(array("progress","status") as $attr){
      $ret[$attr] = $this->$attr;
    }
    return($ret);
  } 
} 
// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
