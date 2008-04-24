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
  var $id               = -1;
 
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
        $this->policies[] = array("NAME" => $policy,"STATUS" => "LOADED","DATA" => array());
      }
    }
  }


  function execute()
  { 
    $smarty = get_smarty(); 

    if(!$this->view_logged){
      $this->view_logged = TRUE;
      new log("view","server/".get_class($this),$this->dn);
    }

    /*******  
      Display sub dialogs
     *******/

    /*  CANCEL Policy dialog 
     */
    if(isset($_POST['cancel_policy'])){
      $this->dialog = NULL;
    }
    
    /*  SAVE Policy dialog 
     */
    if($this->dialog instanceof krb5_policy && isset($_POST['save_policy'])){
      $this->dialog->save_object();
      $msgs = $this->dialog->check();
      if(count($msgs)){
        msg_dialog::displayChecks($msgs);
      }else{
        $this->AddPolicy($this->id,$this->dialog->save());
        $this->dialog = NULL;
      }
    }

    /* DISPLAY policy dialog
     */
    if($this->dialog instanceof krb5_policy){
      $this->dialog->save_object();
      return($this->dialog->execute());
    } 

    /*******
      Create HTML output for this plugin
     *******/  

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
      if($policy['STATUS'] == "REMOVED") continue;
  
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
  


  function AddPolicy($id,$policy)
  {
    if($id != -1 && $this->policies[$id]['NAME'] == $policy['NAME']){

      /* Policy was edited 
       */
      if($this->policies[$id]['STATUS'] == "LOADED"){
        $policy['STATUS'] = "EDITED";
      }else{
        $policy['STATUS'] = $this->policies[$id]['STATUS'];
      }
      $this->policies[$this->id] = $policy;

    }elseif($id != -1 && $this->policies[$id]['NAME'] != $policy['NAME']){

      /* Policy was renamed, remove old and add new policy  
       */
      $this->RemovePolicy($id);
      $this->AddPolicy(-1,$policy);
    }else{

      /* Policy was added 
       */
      $name = $policy['NAME'];
    
      /* Check if there is already a policy with this name 
          which was removed before
       */
      foreach($this->policies as $pid => $entry){
        if($entry['NAME'] == $name && $entry['STATUS'] == "REMOVED"){
          $id = $pid;
          break;
        }
      }
       
      /* Update existing policy or create new one
       */ 
      if(isset($this->policies[$id])){
        $policy['STATUS'] = "EDITED";
        $this->policies[$id] = $policy;
      }else{
        $policy['STATUS'] = "ADDED";
        $this->policies[] = $policy;
      }
    }
  }


  function getPolicyNames()
  {
    $ret = array();
    foreach($this->policies as $policy){
      if($policy['STATUS'] == "REMOVED") continue;
      $ret[] = $policy['NAME'];
    }
    return($ret);
  }

  
  function RemovePolicy($id)
  {
    if($this->policies[$id]['STATUS'] == "ADDED"){
      unset($this->policies[$id]);
    }else{
      $this->policies[$id]['STATUS'] = "REMOVED";
    }
  }  
  

  function save_object()
  {
    if(isset($_POST['goKrbServerPosted'])){
      plugin::save_object();

      /* Add new Policy requested 
       */
      if(isset($_POST['policy_add'])){
        $this->dialog = new krb5_policy($this->config,array(),&$this);
        $this->id = -1;
      }

      /* Walk through POSTs an check for image-button posts 
       */
      foreach($_POST as $name => $value){

        /* Remove policy requested 
         */
        if(preg_match("/^policy_del/",$name)){
          $id = preg_replace("/^policy_del_([0-9]*)_.*/i","\\1",$name);
          if(isset($this->policies[$id])){
            $this->RemovePolicy($id);
          }
          break;
        }

        /* Edit Policy requested 
         */
        if(preg_match("/^policy_edit/",$name)){
          $id = preg_replace("/^policy_edit_([0-9]*)_.*/i","\\1",$name);
          if(isset($this->policies[$id])){

            /* Load policy information, if not done before 
             */ 
            if($this->policies[$id]['STATUS'] == "LOADED" && empty($this->policies[$id]['DATA'])){
              $o = new gosaSupportDaemon();
              $this->policies[$id]['DATA'] = $o->krb5_get_policy("00:01:6c:9d:aa:16",$this->policies[$id]['NAME']);
            }

            /* Open dialog */
            $this->id = $id;
            $this->dialog = new krb5_policy($this->config,$this->policies[$id],&$this);
          }
          break;
        }
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
