<?php

class goKrbServer extends goService{
	
  var $cli_summary      = "This pluign is used within the ServerService Pluign \nand indicates that this server supports NTP service.";
  var $cli_description  = "Some longer text\nfor help";
  var $cli_parameters   = array("eins" => "Eins ist toll", "zwei" => "Zwei ist noch besser");

  /* This plugin only writes its objectClass */
  var $objectclasses    = array("goKrbServer");
  var $attributes       = array("goKrbRealm");
  var $StatusFlag       = "goKrbServerStatus";
 
  /* This class can't be assigned twice so it conflicts with itsself */
  var $conflicts        = array("goKrbServer");

  var $DisplayName      = "";
  var $dn               = NULL;
  var $goKrbServerStatus= "";
  var $cn               = "";
  var $goKrbRealm       = "";
  var $view_logged      = FALSE;

  var $policies         = array();
 
  function goKrbServer(&$config,$dn)
  {
    goService::goService($config,$dn);
    $this->DisplayName = _("Kerberos service");

    /* Get configured policies 
     */ 
    $o = new gosaSupportDaemon();
    $tmp = $o->krb5_list_policies("00:01:6c:9d:aa:16");
    if($o->is_error()){
      msg_dialog::display(_("Service infrastructure"),msgPool::siError($o_queue->get_error()),ERROR_DIALOG);
    }else{
      $this->policies = array();
      foreach($tmp as $policy){
        $this->policies[] = array("NAME" => $policy,"STATUS" => "LOADED");
      }
    }
  }


  function execute()
  { 
    $smarty = get_smarty(); 

    if($this->is_account && !$this->view_logged){
      $this->view_logged = TRUE;
      new log("view","server/".get_class($this),$this->dn);
    }

    $tmp = $this->plinfo();
    foreach($tmp['plProvidedAcls'] as $name => $translation){
      $smarty->assign($name."ACL",$this->getacl($name));
    }

    foreach($this->attributes as $attr){
      $smarty->assign($attr,$this->$attr);
    }

    $divlist = new divSelectBox("Kerberos Policies");
    $action  = "<input type='image' src='images/lists/edit.png' class='center' name='policy_edit_%id'>";
    $action .= "<input type='image' src='images/lists/trash.png' class='center' name='policy_del_%id'>";
    foreach($this->policies as $key => $policy){
      $actions = preg_replace("/%id/",$key,$action);
      $field1 = array("string" => $policy['NAME']);
      $field3 = array("string" => $actions, 
                      "attach"=>"style='width:40px; text-align:right; border:0px;'");

      $divlist->AddEntry(array($field1,$field3));
    }

    $smarty->assign("divlist",$divlist->DrawList());
    return($smarty->fetch(get_template_path("goKrbServer.tpl",TRUE,dirname(__FILE__))));
  }


  function getListEntry()
  {
    $fields               = goService::getListEntry();
    $fields['Message']    = _("Kerberos service (kadmin access informations)");
    $fields['AllowEdit']  = true;
    return($fields);
  }

  function check()
  { 
    $message = plugin::check();
    if (empty($this->goKrbRealm)){
      $message[]= msgPool::required(_("Realm"));
    }
    return($message);
  }
  

  function save_object()
  {
    if(isset($_POST['goKrbServerPosted'])){
      plugin::save_object();
    }

    /* */
    if(isset($_POST['policy_add'])){
      echo "add";
      // $this->dialog = new krb5_policy($this->config,array());
    }
    foreach($_POST as $name => $value){
      if(preg_match("/^policy_del/",$name)){
        $id = preg_replace("/^policy_del_([0-9]*)_.*/i","\\1",$name);
        if(isset($this->policies[$id])){
          unset($this->policies[$id]);
        }
        break;
      }
      if(preg_match("/^policy_edit/",$name)){
        $id = preg_replace("/^policy_edit_([0-9]*)_.*/i","\\1",$name);
        if(isset($this->policies[$id])){
          echo "edit $id";
        // $this->dialog = new krb5_policy($this->config,$this->policies[$id]);
        }
        break;
      }
    }
  } 

    /* Return plugin informations for acl handling */
  static function plInfo()
  {
    return (array(
          "plShortName"   => _("Kerberos"),
          "plDescription" => _("Kerberos access information")." ("._("Services").")",
          "plSelfModify"  => FALSE,
          "plDepends"     => array(),
          "plPriority"    => 95,
          "plSection"     => array("administration"),
          "plCategory"    => array("server"),

          "plProvidedAcls"=> array(
            "goKrbPolicy"   => _("Policies"),
            "goKrbRealm"    => _("Realm")) 
          ));
  }
}
// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
