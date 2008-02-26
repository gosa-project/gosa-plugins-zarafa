<?php

class DaemonEvent_wakeup extends DaemonEvent
{
  public function __construct($config,$data = array())
  {
    DaemonEvent::__construct($config,$data);
    $this->s_Menu_Name  = _("Wake up"); 
    $this->s_Event_Name = _("Wake up a system"); 
    $this->s_New_Action = "job_trigger_action_wake";
    $this->s_Edit_Action= "trigger_action_wake";
    $this->s_Menu_Image = "images/status_running.png";
    $this->s_List_Image = "images/status_running.png";
  }

  public function execute()
  {
    DaemonEvent::execute();

    $display = $this->get_header();

    /* Check if target add dialog is open */
    if($this->is_target_list_open()){
      $display .= $this->get_target_add_list();
    }else{
      $smarty = get_smarty();
      $smarty->assign("target_list" , $this->get_target_list());
      $smarty->assign("timestamp"   , $this->get_time_select());
      $display.= $smarty->fetch(get_template_path('DaemonEvent_wakeup.tpl', TRUE, dirname(__FILE__)));
    }   
 
    $display.= $this->get_footer();
    return($display);
  }

  public function save_object()
  {
    DaemonEvent::save_object();
  }
} 
// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
