<?php

class DaemonEvent_update extends DaemonEvent
{
  public function __construct($config,$data = array())
  {
    DaemonEvent::__construct($config,$data);
    $this->s_Menu_Name  = _("Software update"); 
    $this->s_Event_Name = _("software update"); 
    $this->s_action     = "trigger_action_update";
    $this->s_Menu_Image = "images/time.png";
  }

} 
// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
