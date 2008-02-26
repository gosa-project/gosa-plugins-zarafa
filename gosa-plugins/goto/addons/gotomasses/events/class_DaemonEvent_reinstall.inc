<?php

class DaemonEvent_reinstall extends DaemonEvent
{
  public function __construct($config,$data = array())
  {
    DaemonEvent::__construct($config,$data);
    $this->s_Menu_Name  = _("Reinstall"); 
    $this->s_Event_Name = _("Reinstall");
    $this->s_New_Action     = "trigger_action_reinstall"; 
    $this->s_Menu_Image = "images/fai_small.png";
  }

} 
// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
