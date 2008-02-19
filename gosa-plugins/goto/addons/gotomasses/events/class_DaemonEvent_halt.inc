<?php

class DaemonEvent_halt extends DaemonEvent
{
  public function __construct($config,$data = array())
  {
    DaemonEvent::__construct($config,$data);
    $this->s_Menu_Name  = _("Shutdown"); 
    $this->s_Event_Name = _("Shutdown"); 
    $this->s_action     = "trigger_action_halt";
    $this->s_Menu_Image = "images/status_stopped.png";
  }

} 
// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
