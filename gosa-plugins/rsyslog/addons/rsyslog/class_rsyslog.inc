<?php

class rsyslog extends plugin
{
  var $plHeadline= "rSyslog";
  var $plDescription= "View system logs";

  var $servers = array();
  var $selected_server = "";
  var $selected_host = "";

  function rsyslog (&$config, $dn= NULL)
  {
    $this->config= &$config;
    $this->ui = get_userinfo();

    // Get list of rsyslog servers 
    $ldap = $this->config->get_ldap_link();
    $ldap->cd($this->config->current['BASE']);
    $ldap->search("objectClass=rSyslogServer",array("cn","rSyslogUser","rSyslogDatabase","rSyslogPassword"));
    while($attrs = $ldap->fetch()){
      if(empty($this->selected_server)) $this->selected_server = $attrs['cn'][0];
      $s = array('cn' => $attrs['cn'][0], 'dn' => $attrs['dn'], 'Password' => '', 'status' => 'ok');
      $s['User'] = $attrs['rSyslogUser'][0]; 
      $s['Database'] = $attrs['rSyslogDatabase'][0]; 
      $s['Hosts'] = array();
      $s['ACL'] = "";
      if(isset($attrs['rSyslogPassword'])) $s['Password'] = $attrs['rSyslogPassword'][0];
      $this->servers[$attrs['cn'][0]] = $s;
    }

    // Check for installed mysql extension, if missing abort
    $this->mysql_extension_installed = is_callable("mysql_connect");
    if(!$this->mysql_extension_installed) return;

    // Test server connetivity 
    $ui = get_userinfo();
    foreach($this->servers as $name => $server)  
    {
      $link = @mysql_pconnect($server['cn'], $server['User'], $server['Password']);
      if ($link === FALSE){
        new log("debug","gosa_logging","dbconnect",array(),@mysql_error());
        $this->servers[$name]['status'] = "connectfailed";
        $this->servers[$name]['error']  = @mysql_error();
        continue;
      }elseif (! @mysql_select_db($server['Database'])){
        new log("debug","gosa_logging","selectdb",array(),@mysql_error());
        $this->servers[$name]['status'] = "dbmissing";
        $this->servers[$name]['error']  = @mysql_error();
        continue;
      }else{
        $this->servers[$name]['status'] = "ok";
        $this->servers[$name]['error']  = "";

        // Detect ACLs
        $this->servers[$name]['ACL'] =  $ui->get_permissions($server['dn'], 'server/rsyslog','viewEntries');

        // Get list of Hosts using this log server..
        if(preg_match('/r/', $this->servers[$name]['ACL'])){
          $query = "SELECT distinct(FromHost) FROM SystemEvents;";
          $res = @mysql_query($query, $link);
          if($res){
            while($attrs = @mysql_fetch_assoc($res)){
              $this->servers[$name]['Hosts'][$attrs['FromHost']] = $attrs['FromHost'];  
            }
          }else{
            $this->servers[$name]['status'] = "query_failed";
            $this->servers[$name]['error']  = @mysql_error();
          }
        }
      }
    }
  }

  function execute()
  {
    $smarty = get_smarty();
    $ui = get_userinfo();
    
    if(!isset($this->servers[$this->selected_server]) || !count($this->servers[$this->selected_server]['Hosts'])){
      $hosts = array('-' => _("-"));
    }else{
      $hosts = array('all' => _("All"));
      $hosts = array_merge($hosts,$this->servers[$this->selected_server]['Hosts']);
    }

    $smarty->assign("servers",$this->servers);
    $smarty->assign("hosts",$hosts);
    $smarty->assign("selected_server",$this->selected_server);
    $smarty->assign("selected_host",$this->selected_host);
    $smarty->assign("result", $this->get_result());
    return($smarty->fetch(get_template_path("rSyslog.tpl", TRUE)));
  }

  function get_result()
  {
    $result = array();
    $result['entries'] = array();
    $result['status']  = "ok";
    $result['count']   = 0;
    $result['error']   = "";

    // Check whether the selected server exists.
    if(!isset($this->servers[$this->selected_server])){
      $this->servers[$name]['status'] = sprintf(_("The selected server '%s' does not exists!"),$this->selected_server);
      $this->servers[$name]['error']  = $this->servers[$this->selected_server]['error'];
      return($result);
    }

    // Check database connectivity    
    $name = $this->selected_server;
    $server = $this->servers[$name];
    $link = @mysql_pconnect($server['cn'], $server['User'], $server['Password']);
    if ($link === FALSE){
      new log("debug","gosa_logging","dbconnect",array(),@mysql_error());
      $this->servers[$name]['status'] = "connectfailed";
      $this->servers[$name]['error']  = @mysql_error();
    }elseif (! @mysql_select_db($server['Database'])){
      new log("debug","gosa_logging","selectdb",array(),@mysql_error());
      $this->servers[$name]['status'] = "dbmissing";
      $this->servers[$name]['error']  = @mysql_error();
    }else{
      $this->servers[$name]['status'] = "ok";
      $this->servers[$name]['error']  = "";
    }

    // Get log entries matching the selected filter
    if(preg_match('/r/', $this->servers[$name]['ACL'])){

      $host = "%%";
      if($this->selected_host != "all") $host = $this->selected_host;

    
      $filter = "FROM SystemEvents WHERE FromHost like '{$host}' LIMIT 20;";
      
      // Detect amount of matching entries 
      $query = "SELECT count(ID) as `amount` ".$filter;
      $res = @mysql_query($query, $link);
      if($res && $attrs = @mysql_fetch_assoc($res)){ 
        $result['count'] = $attrs['amount'];
      }else{
        $this->servers[$name]['status'] = "query_failed";
        $this->servers[$name]['error']  = @mysql_error();
      }

      if($result['count']){

        $query = "SELECT * ".$filter;
        $res = @mysql_query($query, $link);
        if($res){
          while($attrs = @mysql_fetch_assoc($res)){
            $result['entries'][] = $attrs;
          }
          $this->servers[$name]['status'] = "ok";
          $this->servers[$name]['error']  = "";
        }else{
          $this->servers[$name]['status'] = "query_failed";
          $this->servers[$name]['error']  = @mysql_error();
        }
      }
    }
    $result['status'] =$this->servers[$name]['status'];
    $result['error'] =$this->servers[$name]['error'];
    return($result);
  }


  function save_object()
  {
    foreach(array("selected_host","selected_server") as $attr){
      if(isset($_POST[$attr])) $this->$attr = get_post($attr);
    }
  }
}
// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
