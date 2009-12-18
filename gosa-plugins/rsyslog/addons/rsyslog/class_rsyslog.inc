<?php

class rsyslog extends plugin
{
  var $plHeadline= "rSyslog";
  var $plDescription= "View system logs";

  var $servers = array();
  var $selected_server = "";
  var $selected_host = "";

  var $startTime = "";
  var $stopTime  = "";

  var $sort_value  = "FromHost";
  var $sort_type  = "DESC";
  var $limit  = 20;
  var $limits = array(20,50,100,200,500,1000,'-');
  var $page   = 0;

  function rsyslog (&$config, $dn= NULL)
  {
    $this->config= &$config;
    $this->ui = get_userinfo();

    $this->startTime = date("d.m.Y", (time() - 7*24*60*60));
    $this->stopTime  = date("d.m.Y", time());

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
    $smarty->assign("usePrototype", "true");
    $ui = get_userinfo();
    
    if(!isset($this->servers[$this->selected_server]) || !count($this->servers[$this->selected_server]['Hosts'])){
      $hosts = array('-' => _("-"));
    }else{
      $hosts = array('all' => _("All"));
      $hosts = array_merge($hosts,$this->servers[$this->selected_server]['Hosts']);
    }

    $result =$this->get_result();

    $smarty->assign("servers",$this->servers);
    $smarty->assign("hosts",$hosts);
    $smarty->assign("selected_server",$this->selected_server);
    $smarty->assign("selected_host",$this->selected_host);
    $smarty->assign("startTime",  $this->startTime);
    $smarty->assign("stopTime",    $this->stopTime);
    $smarty->assign("sort_type",    $this->sort_type);
    $smarty->assign("sort_value",   $this->sort_value);
    $smarty->assign("limits",   $this->limits);
    $smarty->assign("limit",    $this->limit);
    $smarty->assign("page",    $this->page);
    $smarty->assign("plug_id", $_GET['plug']);
    $smarty->assign("downimg", "<img src='images/lists/sort-down.png' class='center' alt='\\/' border='0'>");
    $smarty->assign("upimg", "<img src='images/lists/sort-up.png' class='center' alt='\\/' border='0'>");
    $smarty->assign("result", $result);
    if($this->limit != '-'){
      $smarty->assign("page_sel", range_selector($result['count'],$this->page,$this->limits[$this->limit]));
    }else{
      $smarty->assign("page_sel", "");
    }

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

      $filter = "FROM SystemEvents WHERE FromHost like '{$host}' ";
      $filter.= "AND DeviceReportedTime >= '".(date("Y.m.d 00:00:00", strtotime($this->startTime)))."' ";     
      $filter.= "AND DeviceReportedTime <= '".(date("Y.m.d 23:59:59", strtotime($this->stopTime)))."' ";     

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
        $filter.= "ORDER BY ".$this->sort_value." ".$this->sort_type." ";
        if($this->limit != "-"){
          $filter.= "LIMIT ".$this->page.", ".$this->limits[$this->limit];
        }else{
          $this->page = 0;
        }
        $query = "SELECT * ".$filter;
        $res = mysql_query($query, $link);
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
    foreach(array("selected_host","selected_server","stopTime","startTime") as $attr){
      if(isset($_POST[$attr])) $this->$attr = get_post($attr);
    }

    if(isset($_GET['start'])) $this->page = $_GET['start'];
    if(isset($_POST['limit']) && isset($this->limits[$_POST['limit']])){
      $this->limit = $_POST['limit'];
    }

    if(isset($_GET['sort_value']) && in_array($_GET['sort_value'], 
          array("DeviceReportedTime","FromHost","Facility","Priority","Message"))){
      $sort_value = $_GET['sort_value'];
      if($this->sort_value == $sort_value){
        if($this->sort_type == "ASC"){
          $this->sort_type="DESC"; 
        }else{
          $this->sort_type="ASC"; 
        }
      } 
      $this->sort_value=$sort_value; 
    } 
  }
}
// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
