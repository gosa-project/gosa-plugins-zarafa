<?php
class DaemonEvent_notify extends DaemonEvent
{
  var $visible_for = USER_EVENT;
  public function __construct($config,$data = array())
  {
    DaemonEvent::__construct($config,$data);
    $this->s_Menu_Name  = _("Send message");
    $this->s_Event_Name = _("Send message");
    $this->s_Schedule_Action = "job_trigger_action_notify";
    $this->s_Trigger_Action= "job_trigger_action_notify";
    $this->s_Queued_Action= "trigger_action_notify";
    $this->s_Menu_Image = "images/mailto.png";
    $this->s_List_Image = "images/mailto.png";

    $this->a_targets = array("GOsa");
  }

  public function execute()
  {
    DaemonEvent::execute();
    $display = $this->get_header();
    $tmp = $this->data;
    $smarty = get_smarty();
    $smarty->assign("subject" ,"subject");
    $smarty->assign("message" ,"message");
    $smarty->assign("data"        , $this->data);
    $smarty->assign("data"        , $this->data);
    $smarty->assign("is_new"      , $this->is_new);
    $smarty->assign("timestamp"   , $this->get_time_select());
    $display.= $smarty->fetch(get_template_path('DaemonEvent_notify.tpl', TRUE, dirname(__FILE__)));
    $display.= $this->get_footer();
    return($display);
  }

  public function save_object()
  {
    DaemonEvent::save_object();
  }

  public function save()
  {
    $ret = DaemonEvent::save();
    $ret['user'] = array("test","asdf");
    print_a($ret);
    return($ret);
  }
} 
// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
