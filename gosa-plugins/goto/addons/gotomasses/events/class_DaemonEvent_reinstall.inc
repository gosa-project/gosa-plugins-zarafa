<?php

class DaemonEvent_reinstall extends DaemonEvent
{
  var $progress = 0;  
  var $status   = "";  

  public function __construct($config,$data = array())
  {
    DaemonEvent::__construct($config,$data);
    $this->s_Menu_Name  = _("Reinstall");
    $this->s_Event_Name = _("Reinstall");
    $this->s_Schedule_Action = "job_trigger_action_reinstall";
    $this->s_Trigger_Action= "job_trigger_action_reinstall";
    $this->s_Queued_Action= "trigger_action_reinstall";
    $this->s_Menu_Image = "images/fai_small.png";
    $this->s_List_Image = "images/fai_small.png";
  
    if(!$this->is_new()){
      if(isset($data['PROGRESS'])){
        $this->progress = $data['PROGRESS'];
      }
      if(isset($data['STATUS'])){
        $this->status = $data['STATUS'];
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
    $smarty->assign("status"      , $this->status);
    $smarty->assign("progress"    , $this->progress);
    $smarty->assign("data"        , $this->data);
    $smarty->assign("target_list" , $this->get_target_list());
    $smarty->assign("is_new"      , $this->is_new);
    $smarty->assign("timestamp"   , $this->get_time_select());
    $display.= $smarty->fetch(get_template_path('DaemonEvent_reinstall.tpl', TRUE, dirname(__FILE__)));
    $display.= $this->get_footer();
    return($display);
  }

  public function save()
  {
    $ret = DaemonEvent::save();
    return($ret);
  } 
} 
// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
