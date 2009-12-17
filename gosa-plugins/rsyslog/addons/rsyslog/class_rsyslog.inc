<?php

class rsyslog extends plugin
{
  var $plHeadline= "rSyslog";
  var $plDescription= "View system logs";

  function gosa_logview (&$config, $dn= NULL)
  {
    $this->config= &$config;
    $this->ui = get_userinfo();
  }

  function execute()
  {
    $smarty = get_smarty();

    return($smarty->fetch(get_template_path("rSyslog.tpl", TRUE)));
  }
}
// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
