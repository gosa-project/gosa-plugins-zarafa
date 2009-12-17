<?php

class rsyslog extends plugin
{
  var $plHeadline= "rSyslog";
  var $plDescription= "View system logs";

  var $servers = array();

  function rsyslog (&$config, $dn= NULL)
  {
    $this->config= &$config;
    $this->ui = get_userinfo();

    $ldap = $this->config->get_ldap_link();
    $ldap->cd($this->config->current['BASE']);
    $ldap->search("objectClass=rSyslogServer",array("rSyslogUser","rSyslogDatabase","rSyslogPassword"));
    while($attrs = $ldap->fetch()){
      $this->servers[] = $attrs;
    }
  }

  function execute()
  {
    $smarty = get_smarty();
    $ui = get_userinfo();
    foreach($this->servers as $server){
      echo "<b>".$server['dn']."</b><br>";
      echo $ui->get_permissions($server['dn'], 'server/rsyslog','viewEntries')."<br>";
    }

    return($smarty->fetch(get_template_path("rSyslog.tpl", TRUE)));
  }
}
// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
