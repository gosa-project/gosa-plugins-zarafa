<?php

class rSyslogServer extends goService{
	
  /* This plugin only writes its objectClass */
  var $objectclasses    = array("gosaLogServer");
  var $attributes       = array("goLogDB","goLogDBUser","goLogDBPassword");
  var $StatusFlag       = "";

  /* This class can't be assigned twice so it conflicts with itsself */
  var $conflicts        = array("gosaLogServer");

  var $DisplayName      = "";
  var $dn               = NULL;
  var $acl;
  var $cn                 = "";
  var $goLogDB    = "Syslog";
  var $goLogDBUser        = "rsyslog";
  var $goLogDBPassword    = "";  
  var $view_logged  =FALSE;


  function rSyslogServer(&$config,$dn)
  {
    goService::goService($config,$dn);
    $this->DisplayName = _("rSyslog");
  }


  function execute()
  { 
    $smarty = get_smarty(); 

    if($this->is_account && !$this->view_logged){
      $this->view_logged = TRUE;
      new log("view","server/".get_class($this),$this->dn);
    }

    $tmp = $this->plInfo();
    foreach($tmp['plProvidedAcls'] as $name => $translation){
      $smarty->assign($name."ACL",$this->getacl($name));
    }

    foreach($this->attributes as $attr){
      $smarty->assign($attr,$this->$attr);
    }
    return($smarty->fetch(get_template_path("rSyslogServer.tpl",TRUE,dirname(__FILE__))));
  }


  function getListEntry()
  {
    $fields = goService::getListEntry();
    $fields['Message']      = _("rSyslog logging service");
    #$fields['AllowEdit']    = true;
    #$fields['AllowStart']   = FALSE;
    #$fields['AllowStop']    = FALSE;
    #$fields['AllowRestart'] = FALSE;
    return($fields);
  }


  function check()
  { 
    $message = plugin::check();
    if (empty($this->goLogDBUser)){
      $message[]= msgPool::required(_("User"));
    }
    if (empty($this->goLogDBPassword)){
      $message[]= msgPool::required(_("Password"));
    }
    if (empty($this->goLogDB)){
      $message[]= msgPool::required(_("Database"));
    }
    return($message);
  }


  function save_object()
  {
    if(isset($_POST['rSyslogServerPosted'])){
      plugin::save_object();
    }
  } 

  /* Return plugin informations for acl handling */
  static function plInfo()
  {
    return (array(
          "plShortName"   => _("rSyslog logging service"),
          "plDescription" => _("rSyslog logging service")." ("._("Services").")",
          "plSelfModify"  => FALSE,
          "plDepends"     => array(),
          "plPriority"    => 99,
          "plSection"     => array("administration"),
          "plCategory"    => array("server"),

          "plProvidedAcls"=> array(
            "goLogDB" => _("Database"),
            "goLogDBUser"     => _("User"),
            "goLogDBPassword" => _("Password"), 
            "viewEntries"     => _("Allow view of entries on this server")) 
          ));
  }
}
// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
