<?php

class workgeneric extends plugin
{
  /* Generic terminal attributes */
  var $gotoMode= "locked";
  var $gotoSyslogServer= "";
  var $gotoSyslogServers= array();
  var $gotoNtpServer= array();
  var $gotoNtpServers= array();
  var $gotoSndModule= "";
  var $gotoFloppyEnable= "";
  var $gotoCdromEnable= "";
  var $description= "";
  var $ghCpuType= "-";
  var $ghMemSize= "-";
  var $ghUsbSupport= "-";
  var $ghNetNic= array();
  var $ghIdeDev= array();
  var $ghScsiDev= array();
  var $ghGfxAdapter= "-";
  var $ghSoundAdapter= "-";
  var $gotoLastUser= "-";
  var $FAIscript= "";
  var $view_logged = FALSE;

  /* Needed values and lists */
  var $base= "";
  var $cn= "";
  var $l= "";
  var $orig_dn= "";

  /* Plugin side filled */
  var $modes= array();

  var $netConfigDNS;

  var $inheritTimeServer = true;

  /* attribute list for save action */
  var $ignore_account= TRUE;
  var $attributes= array("gotoMode", "gotoSyslogServer", "gotoNtpServer",
      "gotoFloppyEnable", "gotoCdromEnable", "cn", "gotoSndModule",
      "ghCpuType", "ghMemSize", "ghUsbSupport", "description",
      "ghGfxAdapter", "ghSoundAdapter", "gotoLastUser", "l","FAIscript");
  var $objectclasses= array("top", "gotoWorkstation", "GOhard");

  var $validActions   = array("reboot" => "", "localboot" => "", "halt" => "", "update" => "", "reinstall" => "",
                            "rescan" => "", "wake" => "", "memcheck" => "", "sysinfo" => "");
  
  var $fai_activated = FALSE;

  var $member_of_ogroup = FALSE;

  var $currently_installing = FALSE;
  var $currently_installing_warned = FALSE;

  function workgeneric (&$config, $dn= NULL, $parent= NULL)
  {
    $tmp= $config->search("faiManagement", "CLASS",array('menu','tabs'));
    if(!empty($tmp)){
      $this->fai_activated = TRUE;
    }

    plugin::plugin ($config, $dn, $parent);

    if(!isset($this->parent->by_object['ogroup'])){
      $ldap = $this->config->get_ldap_link();
      $ldap->cd ($this->config->current['BASE']);
      $ldap->search("(&(objectClass=gotoWorkstationTemplate)(member=".LDAP::prepare4filter($this->dn)."))",array("cn"));
      $this->member_of_ogroup = $ldap->count() >= 1;
    }

    $this->netConfigDNS = new termDNS($this->config,$this,$this->objectclasses);

    /* Check if this host is currently in installation process*/
    $o = new gosaSupportDaemon();
    $e_types = DaemonEvent::get_event_types(USER_EVENT | SYSTEM_EVENT | HIDDEN_EVENT);
    $evts = $o->get_entries_by_mac(array($this->netConfigDNS->macAddress));
    foreach($evts as $evt){
      if(isset($e_types['QUEUED'][$evt['HEADERTAG']]) && $evt['status'] == "processing" &&
          $e_types['QUEUED'][$evt['HEADERTAG']] == "DaemonEvent_reinstall"){
        $this->currently_installing =TRUE;
      }
    }

    /* Read arrays */
    foreach (array("ghNetNic", "ghIdeDev", "ghScsiDev") as $val){
      if (!isset($this->attrs[$val])){
        continue;
      }
      for ($i= 0; $i<$this->attrs[$val]['count']; $i++){
        array_push($this->$val, $this->attrs[$val][$i]);
      }
    }

    /* Create used ntp server array */
    $this->gotoNtpServer= array();
    if(isset($this->attrs['gotoNtpServer'])){
      $this->inheritTimeServer = false;
      unset($this->attrs['gotoNtpServer']['count']);
      foreach($this->attrs['gotoNtpServer'] as $server){
        $this->gotoNtpServer[$server] = $server;
      }
    }

    /* Set inherit checkbox state */
    if((in_array("default",$this->gotoNtpServer)) || (count($this->gotoNtpServer) == 0)){
      $this->inheritTimeServer = true;
      $this->gotoNtpServer=array();
    }

    /* You can't inherit the NTP service, if we are not member in an object group */
    if(!$this->member_of_ogroup){
      $this->inheritTimeServer = FALSE;
    }

    /* Create available ntp options */
    $tmp = $this->config->data['SERVERS']['NTP'];
    $this->gotoNtpServers = array();
    foreach($tmp as $key => $server){
      if($server == "default") continue;
      $this->gotoNtpServers[$server] = $server;
    }

    $this->modes["active"]= _("Activated");
    $this->modes["locked"]= _("Locked");

    /* Set base */
    if ($this->dn == "new"){
      $ui= get_userinfo();
      $this->base= dn2base($ui->dn);
    } else {
      $this->base= preg_replace ("/^[^,]+,[^,]+,[^,]+,/", "", $this->dn);
    }

    /* Create an array of all Syslog servers */
    $tmp = $this->config->data['SERVERS']['SYSLOG'];
    foreach($tmp as $server){
      $visible = $server;
      if($server == "default" && $this->member_of_ogroup) {
        $visible = "["._("inherited")."]";
      }
      $this->gotoSyslogServers[$server] = $visible;
    }

    /* Save 'dn' for later referal */
    $this->orig_dn= $this->dn;
  }


  function set_acl_base($base)
  {
    plugin::set_acl_base($base);
    $this->netConfigDNS->set_acl_base($base);
  }

  function set_acl_category($cat)
  {
    plugin::set_acl_category($cat);
    $this->netConfigDNS->set_acl_category($cat);
  }

  function execute()
  {
    /* Call parent execute */
    plugin::execute();

    if($this->is_account && !$this->view_logged){
      $this->view_logged = TRUE;
      new log("view","workstation/".get_class($this),$this->dn);
    }

    /* Do we need to flip is_account state? */
    if(isset($_POST['modify_state'])){
      if($this->is_account && $this->acl_is_removeable()){
        $this->is_account= FALSE;
      }elseif(!$this->is_account && $this->acl_is_createable()){
        $this->is_account= TRUE;
      }
    }

    if ((isset($_POST['action'])) && ($this->acl_is_writeable("FAIstate")) && isset($this->validActions[$_POST['saction']]) ){
      $action= $_POST['saction'];

      /* Check if we have an DaemonEvent for this action */ 
      if(class_available("DaemonEvent")){
        $events = DaemonEvent::get_event_types(SYSTEM_EVENT | HIDDEN_EVENT);
        if(isset($events['TRIGGERED']["DaemonEvent_".$action])){
          $evt = $events['TRIGGERED']["DaemonEvent_".$action];
          $tmp = new $evt['CLASS_NAME']($this->config);
          $tmp->add_targets(array($this->netConfigDNS->macAddress));
          $tmp->set_type(TRIGGERED_EVENT);
          $o_queue = new gosaSupportDaemon();
          if(!$o_queue->append($tmp)){
            msg_dialog::display(_("Daemon"),sprintf(_("Something went wrong while talking to the daemon: %s."),
                  $o_queue->get_error()),ERROR_DIALOG);
          }
        }
      } else {
        msg_dialog::display(_("Event error"),
                    sprintf(_("Event '%s' is not available!"),$action),ERROR_DIALOG);
      }


    }

    /* Do we represent a valid terminal? */
    if (!$this->is_account && $this->parent === NULL){
      $display= "<img alt=\"\" src=\"images/stop.png\" align=middle>&nbsp;<b>".
        _("This 'dn' has no workstation features.")."</b>";
      return($display);
    }

    /* Base select dialog */
    $once = true;
    foreach($_POST as $name => $value){
      if(preg_match("/^chooseBase/",$name) && $once && $this->acl_is_writeable("base")){
        $once = false;
        $this->dialog = new baseSelectDialog($this->config,$this,$this->get_allowed_bases());
        $this->dialog->setCurrentBase($this->base);
      }
    }

    /* Dialog handling */
    if(is_object($this->dialog)){
      /* Must be called before save_object */
      $this->dialog->save_object();

      if($this->dialog->isClosed()){
        $this->dialog = false;
      }elseif($this->dialog->isSelected()){

        /* A new base was selected, check if it is a valid one */
        $tmp = $this->get_allowed_bases();
        if(isset($tmp[$this->dialog->isSelected()])){
          $this->base = $this->dialog->isSelected();
        }

        $this->dialog= false;
      }else{
        return($this->dialog->execute());
      }
    }

    /* Add new ntp Server to our list */ 
    if((isset($_POST['addNtpServer'])) && (isset($_POST['gotoNtpServers'])) && $this->acl_is_writeable("gotoNtpServer")){
      $this->gotoNtpServer[$_POST['gotoNtpServers']] = $_POST['gotoNtpServers'];
    }

    /* Delete selected NtpServer for list of used servers  */
    if((isset($_POST['delNtpServer'])) && (isset($_POST['gotoNtpServerSelected'])) && $this->acl_is_writeable("gotoNtpServer")){
      foreach($_POST['gotoNtpServerSelected'] as $name){
        unset($this->gotoNtpServer[$name]);
      }
    }

    /* Fill templating stuff */
    $smarty= get_smarty();

    /* Set acls */
    $tmp = $this->plInfo();
    foreach($tmp['plProvidedAcls'] as $name => $translation){
      $smarty->assign($name."ACL",$this->getacl($name));
    }

    $smarty->assign("cn", $this->cn);
    $smarty->assign("description", $this->description);
    $smarty->assign("l", $this->l);
    $smarty->assign("bases", $this->get_allowed_bases());
    $smarty->assign("staticAddress", "");

    $tmp = array();
    foreach($this->gotoNtpServers as $server){
      if(!in_array($server,$this->gotoNtpServer)){
        $tmp[$server] = $server;
      }
    }
    $smarty->assign("gotoNtpServers",$tmp);
        
    /* Check if workstation is online */
    if (gosaSupportDaemon::ping($this->netConfigDNS->macAddress)){
      $smarty->assign("actions", array("halt" => _("Switch off"), "reboot" => _("Reboot"),
                                       "update" => _("System update"),
                                       "reinstall" => _("Reinstall"),
                                       "rescan" => _("Rescan hardware"),
                                       "memcheck" => _("Memory test"),
                                       "localboot" => _("Force localboot"),
                                       "sysinfo"  => _("System analysis")));
    } else {
      $smarty->assign("actions", array("wake" => _("Wake up"),
                                       "reinstall" => _("Reinstall"),
                                       "update" => _("System update"),
                                       "memcheck" => _("Memory test"),
                                       "localboot" => _("Force localboot"),
                                       "sysinfo"  => _("System analysis")));
    }
    /* Arrays */
    $smarty->assign("modes", $this->modes);
    $smarty->assign("nfsservers", $this->config->data['SERVERS']['NFS']);
    $smarty->assign("syslogservers", $this->gotoSyslogServers);
    $smarty->assign("fai_activated",$this->fai_activated);

    $ntpser = array();
    foreach($this->gotoNtpServers as $server){
      if(!in_array($server,$this->gotoNtpServer)){
        $ntpser[$server] = $server;
      }
    }
    $smarty->assign("gotoNtpServers", $ntpser);

    /* Variables */
    foreach(array("base", "gotoMode", "gotoSyslogServer", "gotoNtpServer") as $val){
      $smarty->assign($val."_select", $this->$val);
    }

    /* tell smarty the inherit checkbox state */
    $smarty->assign("inheritTimeServer",$this->inheritTimeServer);
    $smarty->assign("member_of_ogroup",$this->member_of_ogroup);

    $str = $this->netConfigDNS->execute();
    if(is_object($this->netConfigDNS->dialog)){
      return($str);
    }
    $smarty->assign("netconfig", $str);

    /* Show main page */
    $smarty->assign("currently_installing", $this->currently_installing);
    return($smarty->fetch (get_template_path('workstation.tpl', TRUE, dirname(__FILE__))));
  }

  function remove_from_parent()
  {
    if($this->acl_is_removeable()){

      $this->netConfigDNS->remove_from_parent();
      $ldap= $this->config->get_ldap_link();
      $ldap->rmdir($this->dn);
      new log("remove","workstation/".get_class($this),$this->dn,array_keys($this->attrs),$ldap->get_error());
      if (!$ldap->success()){
        msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $this->dn, LDAP_DEL, get_class()));
      }

      /* Optionally execute a command after we're done */
      $this->handle_post_events("remove", array("macAddress" => $this->netConfigDNS->macAddress,"ipHostNumber" => $this->netConfigDNS->ipHostNumber));

      /* Delete references to object groups */
      $ldap->cd ($this->config->current['BASE']);
      $ldap->search ("(&(objectClass=gosaGroupOfNames)(member=".LDAP::prepare4filter($this->dn)."))", array("cn"));
      while ($ldap->fetch()){
        $og= new ogroup($this->config, $ldap->getDN());
        unset($og->member[$this->dn]);
        $og->save ();
      }
    }

    /* Clean queue form entries with this mac 
     */
    if(class_available("gosaSupportDaemon") && tests::is_mac($this->netConfigDNS->orig_macAddress)){
      $q = new gosaSupportDaemon();
      $q->clean_queue_from_mac($this->netConfigDNS->orig_macAddress);
    }

    if(isset($_POST["inheritAll"])){
      $this->set_everything_to_inherited();
    }
  }


  /* Save data to object */
  function save_object()
  {

    /* Create a base backup and reset the
       base directly after calling plugin::save_object();
       Base will be set seperatly a few lines below */
    $base_tmp = $this->base;
    plugin::save_object();
    $this->base = $base_tmp;

    /* Save base, since this is no LDAP attribute */
    $tmp = $this->get_allowed_bases();
    if(isset($_POST['base'])){
      if(isset($tmp[$_POST['base']])){
        $this->base= $_POST['base'];
      }
    }

    $this->netConfigDNS->save_object();

    /* Set inherit mode */
    if((isset($_POST['workgeneric_posted'])) && ($this->acl_is_writeable("gotoNtpServer"))){
      if(isset($_POST["inheritTimeServer"]) && $this->member_of_ogroup){
        $this->inheritTimeServer = true;
      }else{
        $this->inheritTimeServer = false;
      }
    }
    
    if(isset($_POST["inheritAll"])){
      $this->set_everything_to_inherited();
    }
  }


  /* Check supplied data */
  function check()
  {
    /* Call common method to give check the hook */
    $message= plugin::check();
  
    /* Skip IP & Mac checks if this is a template */
    if($this->cn != "wdefault"){
      $message= array_merge($message, $this->netConfigDNS->check());
    }

    $this->dn= "cn=".$this->cn.",".get_ou('workstationou').$this->base;

    if ($this->cn == ""){
      $message[]= _("The required field 'Workstation name' is not set.");
    }

    /* Check if given name is a valid host/dns name */
    if(!tests::is_dns_name($this->cn)){
      $message[] = _("Please specify a valid name for this object.");
    }

    if ($this->orig_dn != $this->dn){
      $ldap= $this->config->get_ldap_link();
      $ldap->cd ($this->base);

      if($this->cn == "wdefault"){
        $ldap->cat($this->dn);
      }else{
        $ldap->search ("(&(cn=".$this->cn.")(objectClass=gotoWorkstation))", array("cn"));
      }
      if ($ldap->count() != 0){
        while ($attrs= $ldap->fetch()){
          if (preg_match("/cn=dhcp,/",$attrs['dn']) || preg_match ("/,".get_ou('incomingou')."/", $ldap->getDN())){
            continue;
          } else {
            if ($attrs['dn'] != $this->orig_dn){
              $message[]= sprintf (_("There is already an entry '%s' in the base choosen by you"), $this->cn);
              break;
            }
          }
        }
      }
    }

    /* Check for valid ntpServer selection */
    if((!$this->inheritTimeServer) && (!count($this->gotoNtpServer))){
      $message[]= _("There must be at least one NTP server selected, or the inherit mode activated.");
    }

    /* Warn the user, that this host is currently installing */
    if($this->currently_installing && !$this->currently_installing_warned){
      $this->currently_installing_warned = TRUE;
      $message[] = _("This host is currently installing, if you really want to save it, save again.");
    }

    return ($message);
  }


  /* Save to LDAP */
  function save()
  {
    /* Detect mode changes */
    $activate= (isset($this->saved_attributes['gotoMode']) &&
        $this->gotoMode != $this->saved_attributes['gotoMode'] &&
        $this->gotoMode == "active" &&
        tests::is_ip($this->netConfigDNS->ipHostNumber));
    plugin::save();

    /* Strip out 'default' values */
    foreach (array("gotoSyslogServer") as $val){

      if (isset($this->attrs[$val]) && $this->attrs[$val] == "default"){
        $this->attrs[$val]= array();
      }
    }

    /* Add missing arrays */
    foreach (array("ghScsiDev", "ghIdeDev", "ghNetNic") as $val){
      if (isset ($this->$val) && count ($this->$val) != 0){
        $this->attrs["$val"]= $this->$val;
      }
    }

    /* Remove all empty values */
    if ($this->orig_dn == 'new'){
      $attrs= array();
      foreach ($this->attrs as $key => $val){
        if (is_array($val) && count($val) == 0){
          continue;
        }
        $attrs[$key]= $val;
      }
      $this->attrs= $attrs;
    }

    /* Update ntp server settings */
    if($this->inheritTimeServer){
      if($this->is_new){
        if(isset($this->attrs['gotoNtpServer'])){
          unset($this->attrs['gotoNtpServer']);
        }
      }else{
        $this->attrs['gotoNtpServer'] = array();
      }
    }else{
      /* Set ntpServers */
      $this->attrs['gotoNtpServer'] = array();
      foreach($this->gotoNtpServer as $server){
        $this->attrs['gotoNtpServer'][] = $server;
      }
    }

    /* Write back to ldap */
    $ldap= $this->config->get_ldap_link();
    if ($this->orig_dn == 'new'){
      $ldap->cd($this->config->current['BASE']);
      $ldap->create_missing_trees(preg_replace('/^[^,]+,/', '', $this->dn));
      $ldap->cd($this->dn);
      $ldap->add($this->attrs);
      new log("create","workstation/".get_class($this),$this->dn,array_keys($this->attrs),$ldap->get_error());
      if (!$ldap->success()){
        msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $this->dn, 0, get_class()));
      }
      $this->handle_post_events("add",array("macAddress" => $this->netConfigDNS->macAddress,"ipHostNumber" => $this->netConfigDNS->ipHostNumber));
    } else {
      if ($this->orig_dn != $this->dn){
        $this->move($this->orig_dn, $this->dn);
      }
      $ldap->cd($this->dn);
      $this->cleanup();
      $ldap->modify ($this->attrs); 
      if (!$ldap->success()){
        msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $this->dn, LDAP_MOD, get_class()));
      }
      new log("modify","workstation/".get_class($this),$this->dn,array_keys($this->attrs),$ldap->get_error());

      $this->handle_post_events("modify",array("macAddress" => $this->netConfigDNS->macAddress,"ipHostNumber" => $this->netConfigDNS->ipHostNumber));
    }

    /* cn=default and macAddress=- indicates that this is a template */
    if($this->cn == "wdefault"){
      $this->netConfigDNS->macAddress = "-";
    }

    $this->netConfigDNS->cn = $this->cn;
    $this->netConfigDNS->save();

    if ($activate && class_available("DaemonEvent")){

      /* Send installation activation
       */
      $events = DaemonEvent::get_event_types(SYSTEM_EVENT | HIDDEN_EVENT);
      $o_queue = new gosaSupportDaemon();
      if(isset($events['TRIGGERED']['DaemonEvent_installation_activation'])){
        $evt = $events['TRIGGERED']['DaemonEvent_installation_activation'];
        $tmp = new $evt['CLASS_NAME']($this->config);
        $tmp->set_type(TRIGGERED_EVENT);
        $tmp->add_targets(array($this->netConfigDNS->macAddress));
        if(!$o_queue->append($tmp)){
          msg_dialog::display(_("Daemon"),sprintf(_("Something went wrong while talking to the daemon: %s."),
                $o_queue->get_error()),ERROR_DIALOG);
        }
      }
    }
  }


  /* Display generic part for server copy & paste */
  function getCopyDialog()
  {
    $vars = array("cn");
    $smarty = get_smarty();
    $smarty->assign("cn" ,$this->cn);
    $smarty->assign("object","workstation");
    $str = $smarty->fetch(get_template_path("paste_generic.tpl",TRUE));
    $ret = array();
    $ret['string'] = $str;
    $ret['status'] = "";
    return($ret);
  }


  function saveCopyDialog()
  {
    if(isset($_POST['cn'])){
      $this->cn = $_POST['cn'];
    }
  }


  function PrepareForCopyPaste($source)
  {
    plugin::PrepareForCopyPaste($source);
    if(isset($source['macAddress'][0])){
      $this->netConfigDNS->macAddress = $source['macAddress'][0];
    }
    if(isset($source['ipHostNumber'][0])){
      $this->netConfigDNS->ipHostNumber = $source['ipHostNumber'][0];
    }

    /* Create used ntp server array */
    $this->gotoNtpServer= array();
    if(isset($source['gotoNtpServer'])){
      $this->inheritTimeServer = false;
      unset($source['gotoNtpServer']['count']);
      foreach($source['gotoNtpServer'] as $server){
        $this->gotoNtpServer[$server] = $server;
      }
    }

    /* Set inherit checkbox state */
    if((in_array("default",$this->gotoNtpServer)) || (count($this->gotoNtpServer)==0)){
      $this->inheritTimeServer = true;
      $this->gotoNtpServer=array();
    }
  }


  /* Return plugin informations for acl handling 
      #FIXME FAIscript seams to ununsed within this class... */ 
  static function plInfo()
  {
    return (array(  
          "plShortName"   => _("Generic"),
          "plDescription" => _("Workstation generic"),
          "plSelfModify"  => FALSE,
          "plDepends"     => array(),
          "plPriority"    => 0,
          "plSection"     => array("administration"),
          "plCategory"    => array("workstation" => array("description"  => _("Workstation"),
                                                          "objectClass"  => "gotoWorkstation")),
          "plProvidedAcls"=> array(
            "cn"                  => _("Workstation name"),
            "description"         => _("Description") ,
            "l"                   => _("Location") ,
            "base"                => _("Base") ,
            "gotoMode"            => _("Goto mode"), 
            "gotoSyslogServer"    => _("Syslog server"), 
            "gotoNtpServer"       => _("Ntp server"), 
            "gotoRootPasswd"      => _("Root password"),
            "FAIstate"            => _("Action flag"))
          ));
  }

  function set_everything_to_inherited()
  {
    $this->gotoSyslogServer  = "default";
    $this->inheritTimeServer = TRUE;

    /* Set workstation service attributes to inherited */
    if($this->member_of_ogroup && isset($this->parent->by_object['workservice'])){
      foreach(array("gotoXKbLayout","gotoXKbModel","gotoXKbVariant",
            "gotoXResolution","gotoXColordepth","gotoXMouseType","gotoXMouseport") as $name){
        $this->parent->by_object['workservice']->$name = "default"; 
      }
    }

    /* Set workstation startup attributes to inherited */
    if($this->member_of_ogroup && isset($this->parent->by_object['workstartup'])){
      $this->parent->by_object['workstartup']->gotoBootKernel = "default-inherited";
      $this->parent->by_object['workstartup']->gotoLdapServer = "default-inherited";
      $this->parent->by_object['workstartup']->FAIdebianMirror= "inherited";
    }
  }
}
  // vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
