<?php


class DaemonEvent_update extends DaemonEvent
{
  public function __construct($config,$data = array())
  {
    DaemonEvent::__construct($config,$data);
    $this->s_Menu_Name  = _("Software update");
    $this->s_Event_Name = _("software update");
    $this->s_Schedule_Action = "job_trigger_action_update";
    $this->s_Trigger_Action= "gosa_trigger_action_update";
    $this->s_Menu_Image = "images/time.png";
    $this->s_List_Image = "images/time.png";
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
    $smarty->assign("data"        , $this->data);
    $smarty->assign("target_list" , $this->get_target_list());
    $smarty->assign("is_new"      , $this->is_new);
    $smarty->assign("timestamp"   , $this->get_time_select());
    $display.= $smarty->fetch(get_template_path('DaemonEvent_update.tpl', TRUE, dirname(__FILE__)));
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
