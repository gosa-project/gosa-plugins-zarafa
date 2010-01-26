<?php
/*
   This code is part of GOsa (https://gosa.gonicus.de)
   Copyright (C) 2004-2005 Cajus Pollmeier

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

class sambaAccount extends plugin
{
  /* Definitions */
  var $plHeadline= "Samba";
  var $plDescription= "This does something";
  var $view_logged = FALSE;
  var $password_expires= 0;

  /* Switch for Samba version */
  var $uidNumber= 65535;
  var $gidNumber= 65535;

  /* Samba attributes */
  var $SID= "";
  var $ridBase= 0;
  var $sambaSID= "";
  var $sambaPwdLastSet= "0";
  var $sambaLogonTime= "0";
  var $sambaLogoffTime= "2147483647";
  var $sambaKickoffTime= "2147483647";
  var $sambaPwdCanChange= "";
  var $sambaPwdMustChange= "0";
  var $sambaAcctFlags= "[UX        ]";
  var $sambaHomePath= "";
  var $sambaHomeDrive= "";
  var $sambaLogonScript= "";
  var $sambaProfilePath= "";
  var $sambaPrimaryGroupSID= "";
  var $sambaDomainName= "";
  var $sambaUserWorkstations= "";
  var $sambaBadPasswordCount= "";
  var $sambaBadPasswordTime= "";
  var $sambaPasswordHistory= "";
  var $sambaLogonHours= "";
  var $orig_sambaDomainName= "";
  var $sambaMungedDial= "";
  var $mungedObject;

  /* Helper */
  var $cache = array();
  var $show_ws_dialog= FALSE;
  var $logon_time_set= 0;
  var $logoff_time_set= 0;
  var $kickoff_time_set= 0;

  /* attribute list for save action */
  var $ctxattributes= array();
  var $attributes= array();
  var $objectclasses= array();
  
  var $uid= "";
  var $CopyPasteVars = array("kickoff_time_set","logoff_time_set","logon_time_set","mungedObject","orig_sambaDomainName");

  var $multiple_support = TRUE;

  /* Only used  for multiple edit */
  var $temporary_disable = FALSE;
  var $no_password_required = FALSE;
  var $no_expiry = FALSE;
  var $multiple_sambaUserWorkstations = array();

  function sambaAccount (&$config, $dn= NULL)
  {
    /* Load attributes depending on the samba version */
    $this->attributes= array ("sambaSID", "sambaPwdLastSet", "sambaLogonTime",
        "sambaLogoffTime", "sambaKickoffTime", "sambaPwdCanChange",
        "sambaPwdMustChange", "sambaAcctFlags", "uid", "sambaMungedDial",
        "sambaHomePath", "sambaHomeDrive", "sambaLogonScript",
        "sambaProfilePath", "sambaPrimaryGroupSID", "sambaDomainName",
        "sambaUserWorkstations", "sambaPasswordHistory",
        "sambaLogonHours", "sambaBadPasswordTime",
        "sambaBadPasswordCount");
    $this->objectclasses= array ("sambaSamAccount");
    $this->mungedObject= new sambaMungedDial;
    $this->ctxattributes= $this->mungedObject->ctxattributes;

    plugin::plugin ($config, $dn);

    /* Setting uid to default */
    if(isset($this->attrs['uid'][0])){
      $this->uid = $this->attrs['uid'][0];
    }

    /* Get samba Domain in case of samba 3 */
    if ($this->sambaSID != ""){
      $this->SID= preg_replace ("/-[^-]+$/", "", $this->sambaSID);
      $ldap= $this->config->get_ldap_link();
      $ldap->cd($this->config->current['BASE']);
      $ldap->search ("(&(objectClass=sambaDomain)(sambaSID=$this->SID))",array("sambaAlgorithmicRidBase","sambaDomainName"));
      if ($ldap->count() != 0){
        $attrs= $ldap->fetch();
        if(isset($attrs['sambaAlgorithmicRidBase'])){
          $this->ridBase= $attrs['sambaAlgorithmicRidBase'][0];
        } else {
          $this->ridBase= $this->config->get_cfg_value("sambaRidBase");
        }
        if ($this->sambaDomainName == ""){
          $this->sambaDomainName= $attrs['sambaDomainName'][0];
        }
      } else {
        if ($this->sambaDomainName == ""){
          $this->sambaDomainName= "DEFAULT";
        }
        $this->ridBase= $this->config->get_cfg_value("sambaRidBase");
        $this->SID= $this->config->get_cfg_value("sambaSid");
      }

      /* Save in order to compare later on */
      $this->orig_sambaDomainName= $this->sambaDomainName;
    }

    /* Fill mungedDial field */
    if (isset($this->attrs['sambaMungedDial'])){
      $this->mungedObject->load($this->sambaMungedDial);
    }

    /* Password expiery */
    if(isset($this->attrs['sambaPwdMustChange']) &&
        $this->attrs['sambaPwdMustChange'][0] != 0){
      $this->password_expires= 1;
    }

    if(isset($this->attrs['sambaLogonTime']) && ! (
        $this->attrs['sambaLogonTime'][0] == 0 ||
        $this->attrs['sambaLogonTime'][0] == 2147483647
      )){
      $this->logon_time_set= 1;
    }
    if(isset($this->attrs['sambaLogoffTime']) && ! (
        $this->attrs['sambaLogoffTime'][0] == 0 ||
        $this->attrs['sambaLogoffTime'][0] == 2147483647
      )){
      $this->logoff_time_set= 1;
    }
    
    /* Account expiery */
    if(isset($this->attrs['sambaKickoffTime']) && ! (
        $this->attrs['sambaKickoffTime'][0] == 0 ||
        $this->attrs['sambaKickoffTime'][0] == 2147483647
      )){
      $this->kickoff_time_set= 1;
    }

    /* Get global filter config */
    if (!session::is_set("sambafilter")){
      $ui= get_userinfo();
      $base= get_base_from_people($ui->dn);
      $sambafilter= array( "depselect" => $base, "regex" => "*");
      session::set("sambafilter", $sambafilter);
    }

    /* Save initial account state */
    $this->initially_was_account= $this->is_account;

    /* Convert kickoff */
    #TODO: use date format
    $this->sambaKickoffTime= $this->sambaKickoffTime == 0?"":date('d.m.Y', $this->sambaKickoffTime);
    $this->sambaPwdMustChange= $this->sambaPwdMustChange == 2147483647?"":date('d.m.Y', $this->sambaPwdMustChange);
  }

  function execute()
  {
    /* Call parent execute */
    plugin::execute();

    /* Log view */
    if($this->is_account && !$this->view_logged){
      $this->view_logged = TRUE;
      new log("view","users/".get_class($this),$this->dn);
    }

    /* Do we need to flip is_account state? */
    if (isset($_POST['modify_state'])){
      $this->is_account= !$this->is_account;
    }
    /* Do we represent a valid account? */
    if (!$this->is_account && $this->parent === NULL){
      $display= "<img alt=\"\"src=\"images/small-error.png\" align=\"middle\">&nbsp;<b>".
        msgPool::noValidExtension(_("Samba"))."</b>";
      $display.= back_to_main();
      return ($display);
    }

    $display ="";
    if(!$this->multiple_support_active){

      /* Show tab dialog headers */
      $display= "";
      if ($this->parent !== NULL){
        if ($this->is_account){
          $display= $this->show_disable_header(msgPool::removeFeaturesButton(_("Samba")),
              msgPool::featuresEnabled(_("Samba")));
        } else {
          $obj= $this->parent->by_object['posixAccount'];

          /* Samba3 dependency on posix accounts are enabled
             in the moment, because I need to rely on unique
             uidNumbers. There'll be a better solution later
             on. */
          if ($obj->is_account){
            $display= $this->show_enable_header(msgPool::addFeaturesButton(_("Samba")),
                msgPool::featuresDisabled(_("Samba")));
          } else {
            $display= $this->show_enable_header(msgPool::addFeaturesButton(_("Samba")),
                msgPool::featuresDisabled(_("Samba"), _("POSIX")), TRUE);
          }
          return ($display);
        }
      }
    }

    $SkipWrite = (!isset($this->parent) || !$this->parent) && !session::is_set('edit');

    /* Open Samaba Logong hours dialog */
    if(isset($_POST['SetSambaLogonHours']) && $this->acl_is_readable("sambaLogonHours")){
      $this->dialog = new sambaLogonHours($this->config,$this->dn,$this->sambaLogonHours);
    }

    /* Cancel dialog */
    if(isset($_POST['cancel_logonHours'])){
      $this->dialog = FALSE;
    }

    /* Save selected logon hours */
    if(isset($_POST['save_logonHours'])){
      $this->dialog->save_object();
      if($this->acl_is_writeable("sambaLogonHours")){
        $this->sambaLogonHours = $this->dialog->save();
      }
      $this->dialog = FALSE;
    }

    /* Display dialog */
    if((isset($this->dialog)) && (is_object($this->dialog))){
      $this->dialog->save_object();
      return($this->dialog->execute());
    }

    /* Prepare templating */
    $smarty= get_smarty();
    $smarty->assign("usePrototype", "true");

    $tmp = $this->plInfo();
    foreach($tmp['plProvidedAcls'] as $var => $rest){
      $smarty->assign($var."ACL",$this->getacl($var,$SkipWrite));
    }

    if(!session::is_set('edit') && !isset($this->parent)){
      $smarty->assign("sambaLogonHoursACL","");
    }

    if ($this->sambaLogonTime=="2147483647" || $this->sambaLogonTime=="0"){
      $sambaLogonTime_date= getdate();
    } else {
      $sambaLogonTime_date= getdate($this->sambaLogonTime);
    }
    
    if ($this->sambaLogoffTime=="2147483647" || $this->sambaLogoffTime=="0"){
      $sambaLogoffTime_date= getdate();
    } else {
      $sambaLogoffTime_date= getdate($this->sambaLogoffTime);
    }
    
    /* Remove user workstations? */
    if (isset($_POST["delete_ws"]) && isset($_POST['workstation_list'])){

      if($this->acl_is_writeable("sambaUserWorkstations",$SkipWrite)){

        if($this->multiple_support_active){
          foreach($_POST['workstation_list'] as $name){
            if(isset($this->multiple_sambaUserWorkstations[trim($name)])){
              unset($this->multiple_sambaUserWorkstations[trim($name)]);
            }
          } 
        }else{
          $tmp= $this->sambaUserWorkstations;
          foreach($_POST['workstation_list'] as $name){
            $tmp= preg_replace("/$name/", '', $tmp);
            $this->is_modified= TRUE;
          }
          $tmp= preg_replace('/,+/', ',', $tmp);
          $this->sambaUserWorkstations= trim($tmp, ',');
        }
      }
    }

    /* Add user workstation? */
    if (isset($_POST["add_ws"])){
      if($this->acl_is_writeable("sambaUserWorkstations",$SkipWrite)){
        $this->show_ws_dialog= TRUE;
        $this->dialog= TRUE;
      }
    }

    /* Add user workstation finished? */
    if (isset($_POST["add_ws_cancel"])){
      $this->show_ws_dialog= FALSE;
      $this->dialog= FALSE;
    }

    /* Add user workstation? */
    if (isset($_POST["add_ws_finish"])){

      if (isset($_POST['wslist'])){
        if($this->multiple_support_active){
          foreach($_POST['wslist'] as $ws){
            $this->multiple_sambaUserWorkstations[trim($we)] = array("Name" => trim($ws), "UsedByAllUsers" => TRUE);
          }
        }else{
          $tmp= $this->sambaUserWorkstations;
          foreach($_POST['wslist'] as $ws){
            $tmp.= ",$ws";
          }
          $tmp= preg_replace('/,+/', ',', $tmp);
          $this->sambaUserWorkstations= trim($tmp, ',');
        }
        $this->is_modified= TRUE;

        $this->show_ws_dialog= FALSE;
        $this->dialog= FALSE;
      } else {
        msg_dialog::display(_("Error"), _("Please select an entry!"), ERROR_DIALOG);
      }
    }

    /* Show ws dialog */
    if ($this->show_ws_dialog){

      /* Save data */
      $sambafilter= session::get("sambafilter");
      foreach( array("depselect", "regex") as $type){
        if (isset($_POST[$type])){
          $sambafilter[$type]= $_POST[$type];
        }
      }
      if (isset($_GET['search'])){
        $s= mb_substr($_GET['search'], 0, 1, "UTF8")."*";
        if ($s == "**"){
          $s= "*";
        }
        $sambafilter['regex']= $s;
      }
      session::set("sambafilter", $sambafilter);

      /* Get workstation list */
      $exclude= "";

      if($this->multiple_support_active){
        foreach($this->multiple_sambaUserWorkstations as $ws){
          if($ws['UsedByAllUsers']){
            $exclude.= "(cn=".$ws['Name']."$)";
          }
        }
      }else{
        foreach(explode(',', $this->sambaUserWorkstations) as $ws){
          $exclude.= "(cn=$ws$)";
        }
      }
      if ($exclude != ""){
        $exclude= "(!(|$exclude))";
      }
      $regex= $sambafilter['regex'];
      $filter= "(&(objectClass=sambaSAMAccount)$exclude(uid=*$)(|(uid=$regex)(cn=$regex)))";
      $res= get_list($filter, "winworkstation", $sambafilter['depselect'], array("uid"), GL_SUBSEARCH | GL_SIZELIMIT);
        
      $wslist= array();
      foreach ($res as $attrs){
        $wslist[]= preg_replace('/\$/', '', $attrs['uid'][0]);
      }
      asort($wslist);

      $smarty->assign("search_image", get_template_path('images/lists/search.png'));
      $smarty->assign("launchimage", get_template_path('images/lists/action.png'));
      $smarty->assign("tree_image", get_template_path('images/lists/search-subtree.png'));
      $smarty->assign("deplist", $this->config->idepartments);
      $smarty->assign("alphabet", generate_alphabet());
      foreach( array("depselect", "regex") as $type){
        $smarty->assign("$type", $sambafilter[$type]);
      }
      $smarty->assign("hint", print_sizelimit_warning());
      $smarty->assign("wslist", $wslist);
      $smarty->assign("apply", apply_filter());
      $display= $smarty->fetch (get_template_path('samba3_workstations.tpl', TRUE,
                                dirname(__FILE__)));
      return ($display);
    }

    /* Fill boxes */
    $domains= array();
    foreach($this->config->data['SERVERS']['SAMBA'] as $name => $content){
      $domains[]= $name;
    }
    $smarty->assign("domains", $domains);
    $letters= array("");
    for ($i= 68; $i<91; $i++){
      $letters[]= chr($i).":";
    }
    $smarty->assign("drives", $letters);

    /* Fill terminal server settings */
    foreach ($this->ctxattributes as $attr){
      /* Fill common attributes */
      if (isset($this->mungedObject->ctx[$attr])){
        $smarty->assign("$attr", $this->mungedObject->ctx[$attr]);
        // Set field  to blank if value is 0
        if(in_array($attr, array("CtxMaxConnectionTime", "CtxMaxDisconnectionTime", "CtxMaxIdleTime"))) {
          if($this->mungedObject->ctx[$attr] == 0) {
            $smarty->assign("$attr", "");
          }
        }
      } else {
        $smarty->assign("$attr", "");
      }
    }

    /* Assign enum values for preset items */
    $shadowModeVals= array( "0" => _("disabled"),
        "1" => _("input on, notify on"),
        "2" => _("input on, notify off"),
        "3" => _("input off, notify on"),
        "4" => _("input off, nofify off"));

    $brokenConnModeVals= array( 	"0" => _("disconnect"),
        "1" => _("reset"));

    $reConnModeVals= array( "0" => _("from any client"),
        "1" => _("from previous client only"));

    /* Fill preset items */
    $smarty->assign("shadow", $shadowModeVals);
    $smarty->assign("brokenconn", $brokenConnModeVals);
    $smarty->assign("reconn", $reConnModeVals);

    /* Fill preset items with values */
    $smarty->assign("shadowmode", $this->mungedObject->getShadow());
    $smarty->assign("brokenconnmode", $this->mungedObject->getBrokenConn());
    $smarty->assign("reconnmode", $this->mungedObject->getReConn());

    if(session::get('js')){
      /* Set form elements to disabled/enable state */
      $smarty->assign("tsloginstate", $this->mungedObject->getTsLogin()?"":"disabled");

      $smarty->assign("inheritstate", "");
      if($this->acl_is_writeable("AllowLoginOnTerminalServer",$SkipWrite)){
        $smarty->assign("inheritstate", $this->mungedObject->getInheritMode()?"disabled":"");
      }
    }else{
      $smarty->assign("tsloginstate", "");
      $smarty->assign("inheritstate", "");
    }      

    /* Set checkboxes to checked or unchecked state */
    $smarty->assign("tslogin", $this->mungedObject->getTsLogin()?"checked":"");
    $smarty->assign("inherit", $this->mungedObject->getInheritMode()?"checked":"");
    $smarty->assign("connectclientdrives",
                    $this->mungedObject->getConnectClientDrives()?"checked":"");
    $smarty->assign("connectclientprinters",
                    $this->mungedObject->getConnectClientPrinters()?"checked":"");
    $smarty->assign("defaultprinter",
                    $this->mungedObject->getDefaultPrinter()?"checked":"");
    $smarty->assign("CtxMaxConnectionTimeF",
                    $this->mungedObject->getCtxMaxConnectionTimeF()?"checked":"");
    $smarty->assign("CtxMaxDisconnectionTimeF",
                    $this->mungedObject->getCtxMaxDisconnectionTimeF()?"checked":"");
    $smarty->assign("CtxMaxIdleTimeF",
                    $this->mungedObject->getCtxMaxIdleTimeF()?"checked":"");

    
    /* Fill sambaUserWorkstations */
    $ws= explode(",", $this->sambaUserWorkstations);
    sort($ws);
    
    /* Tidy checks for empty option, and smarty will produce one if array[0]="" */
    if(($ws[0]=="")&&(count($ws)==1)) $ws=array();

    if($this->multiple_support_active){
      $smarty->assign("multiple_workstations",$this->multiple_sambaUserWorkstations);
    }  

    $smarty->assign("workstations", $ws);
    

    /* Variables */
    foreach($this->attributes as $val){
      $smarty->assign("$val", $this->$val);
    }

    /* 'sambaAcctFlags' checkboxes */
    /* Check for 'lock-account'-flag: 'D' or 'L' */
    if (is_integer(strpos($this->sambaAcctFlags, "D")) ||
        is_integer(strpos($this->sambaAcctFlags, "L"))) {
        $smarty->assign("flagsD", "checked");
    } else {
        $smarty->assign("flagsD", "");
    }
    
    /* Check for no_password_required flag 'N' */
    if (is_integer(strpos($this->sambaAcctFlags, "N"))) {
        $smarty->assign("flagsN", "checked");
    } else {
        $smarty->assign("flagsN", "");
    }

    // check if password never expires
    if (is_integer(strpos($this->sambaAcctFlags, "X"))) {
        $smarty->assign("flagsX", "checked");
    } else {
        $smarty->assign("flagsX", "");
    }

    if ($this->sambaPwdCanChange=="1"){
      $smarty->assign("flagsP", "checked");
    } else {
      $smarty->assign("flagsP", "");
    }

    if ($this->password_expires=="1"){
      $smarty->assign("flagsC", "checked");
    } else {
      $smarty->assign("flagsC", "");
    }
    if ($this->logon_time_set=="1"){
      $smarty->assign("flagsT", "checked");
    } else {
      $smarty->assign("flagsT", "");
    }
    if ($this->logoff_time_set=="1"){
      $smarty->assign("flagsO", "checked");
    } else {
      $smarty->assign("flagsO", "");
    }
    if ($this->kickoff_time_set=="1"){
      $smarty->assign("flagsK", "checked");
    } else {
      $smarty->assign("flagsK", "");
    }
   

    /* In case of javascript, disable some fields on demand */
    foreach($this->mungedObject->getOnDemandFlags() as $key => $value) {
      $smarty->assign("$key", "$value");
    }


    foreach($this->attributes as $attr){
      if(in_array($attr,$this->multi_boxes)){
        $smarty->assign("use_".$attr,TRUE);
      }else{
        $smarty->assign("use_".$attr,FALSE);
      }
    }
    foreach(array("allow_pwchange","tslogin","CtxWFHomeDir","CtxWFHomeDirDrive","CtxWFProfilePath",
          "inherit","CtxWorkDirectory","CtxInitialProgram","CtxMaxConnectionTimeF","CtxMaxConnectionTime","CtxMaxDisconnectionTimeF",
          "CtxMaxDisconnectionTime","CtxMaxIdleTimeF","CtxMaxIdleTime","connectclientdrives",
          "onnectclientprinters","defaultprinter","shadow","brokenconn",
          "reconn","allow_pwchange","connectclientprinters","no_expiry","no_password_required","temporary_disable", 
          "password_expires","logon_time_set","logoff_time_set","kickoff_time_set","SetSambaLogonHours",
          "workstation_list") as $attr){
      if(in_array($attr,$this->multi_boxes)){
        $smarty->assign("use_".$attr,TRUE);
      }else{
        $smarty->assign("use_".$attr,FALSE);
      }
    }

    if($this->multiple_support_active){
      $smarty->assign("tsloginstate","");
    }

    /* Create additional info for sambaKickOffTime and sambaPwdMustChange. 
       e.g. Display effective kickoff time. Domain policy + user settings. 
     */
    $additional_info_PwdMustChange = "";

    /* Calculate effective max Password Age 
        This can only be calculated if sambaPwdLastSet ist set. 
     */
    if(isset($this->attrs['sambaPwdLastSet'][0])){
      $last = $this->attrs['sambaPwdLastSet'][0];

      $sid = $this->get_domain_info();
      if(isset($sid['sambaMaxPwdAge'][0])){
        $d = ($last + $sid['sambaMaxPwdAge'][0]) - time();

        /* A negative value means the password is outdated 
         */
        if($d < 0){
          $additional_info_PwdMustChange = sprintf(_("The password is outdated since %s, by domain policy."),
              date("d.m.Y H:i:s",$last + $sid['sambaMaxPwdAge'][0]));
        }else{
          if($this->password_expires && ($last + $sid['sambaMaxPwdAge'][0]) > $this->sambaPwdMustChange){
            $additional_info_PwdMustChange = sprintf(_("The password is valid till %s, by user policy."),
                date("d.m.Y H:i:s",  $this->sambaPwdMustChange));
          }else{
             $additional_info_PwdMustChange = sprintf(_("The password is valid till %s, by domain policy."),
                date("d.m.Y H:i:s",  ($last + $sid['sambaMaxPwdAge'][0])));
          }
        }
      }
    }
    $smarty->assign("additional_info_PwdMustChange",$additional_info_PwdMustChange);
    $smarty->assign("no_expiry",$this->no_expiry);

    /* Show main page */
    $smarty->assign("multiple_support",$this->multiple_support_active);
    $display.= $smarty->fetch (get_template_path('samba3.tpl', TRUE, dirname(__FILE__)));

    return ($display);
  }


  /*! \brief  Returns the samba Domain object, selected in the samba tab.   
   */
  function get_domain_info()
  {
    /* Only search once, return last result if available
     */
    if(!isset($this->cache['DOMAIN'][$this->sambaDomainName])){
      $this->cache['DOMAIN'][$this->sambaDomainName] = array();
      if(!empty($this->sambaDomainName) && isset($this->config->data['SERVERS']['SAMBA'][$this->sambaDomainName])){
        $cfg = $this->config->data['SERVERS']['SAMBA'][$this->sambaDomainName];
        $ldap = $this->config->get_ldap_link();
        $ldap->cd($this->config->current['BASE']);
        $ldap->search("(&(objectClass=sambaDomain)(sambaSID=".$cfg['SID']."))",array("*"));
        if($ldap->count()){
          $this->cache['DOMAIN'][$this->sambaDomainName] = $ldap->fetch();
        }
      }
    }
    return($this->cache['DOMAIN'][$this->sambaDomainName]);
  }



  function get_samba_information()
  {

    /* Defaults 
     */
    $sambaMinPwdLength = "unset";
    $sambaPwdHistoryLength = "unset";
    $sambaLogonToChgPwd = "unset";
    $sambaMaxPwdAge = "unset";
    $sambaMinPwdAge = "unset";
    $sambaLockoutDuration = "unset";
    $sambaLockoutThreshold = "unset";
    $sambaForceLogoff = "unset";
    $sambaRefuseMachinePwdChange = "unset";
    $sambaPwdLastSet = "unset";
    $sambaLogonTime = "unset";
    $sambaLogoffTime = "unset";

    $sambaKickoffTime = "unset"; 
    $sambaPwdCanChange = "unset";
    $sambaPwdMustChange = "unset";
    $sambaBadPasswordCount = "unset";
    $sambaBadPasswordTime = "unset";

    /* Domain attributes 
     */
    $domain_attributes = array("sambaMinPwdLength","sambaPwdHistoryLength","sambaMaxPwdAge",
        "sambaMinPwdAge","sambaLockoutDuration","sambaRefuseMachinePwdChange",
        "sambaLogonToChgPwd","sambaLockoutThreshold","sambaForceLogoff");

    /* User attributes 
     */
    $user_attributes = array("sambaBadPasswordTime","sambaPwdLastSet","sambaLogonTime","sambaLogoffTime",
        "sambaKickoffTime","sambaPwdCanChange","sambaPwdMustChange","sambaBadPasswordCount", "sambaSID");

    /* Get samba SID object and parse settings.
     */  
    $ldap = $this->config->get_ldap_link();
    $ldap->cd($this->config->current['BASE']);
    if(!empty($this->sambaDomainName) && isset($this->config->data['SERVERS']['SAMBA'][$this->sambaDomainName])){
      $attrs = $this->get_domain_info();
      foreach($domain_attributes as $attr){
        if(isset($attrs[$attr])){
          $$attr = $attrs[$attr][0];
        }
      }
    }
  
    /* Get user infos
     */
    foreach($user_attributes as $attr){
      if(isset($this->attrs[$attr])){
        $$attr = $this->attrs[$attr][0];
      }
    }
    if($this->password_expires){
      $sambaPwdMustChange = $this->sambaPwdMustChange;
    } else {
      if (is_numeric($sambaPwdMustChange)) {
        $sambaPwdMustChange= date('d.m.Y', $sambaPwdMustChange);
      }
    }
    if($this->kickoff_time_set){
      $sambaKickoffTime = $this->sambaKickoffTime;
    } else {
      if (is_numeric($sambaKickoffTime)) {
        $sambaKickoffTime= date('d.m.Y', $sambaKickoffTime);
      }
    }
    $sambaPwdCanChange = $this->sambaPwdCanChange;


    /* DOMAIN Attributes 
     */

    /* sambaMinPwdLength: Password length has a default of 5 
     */
    if($sambaMinPwdLength == "unset" || $sambaMinPwdLength == 5){
      $sambaMinPwdLength  = "5 <i>("._("default").")</i>";
    }

    /* sambaPwdHistoryLength: Length of Password History Entries (default: 0 => off)
     */
    if($sambaPwdHistoryLength == "unset" || $sambaPwdHistoryLength == 0){
      $sambaPwdHistoryLength = _("Off")." <i>("._("default").")</i>";
    }

    /* sambaLogonToChgPwd: Force Users to logon for password change (default: 0 => off, 2 => on) 
     */
    if($sambaLogonToChgPwd == "unset" || $sambaLogonToChgPwd == 0){
      $sambaLogonToChgPwd = _("Off")." <i>("._("default").")</i>";
    }else{
      $sambaLogonToChgPwd = _("On");
    }
    
    /* sambaMaxPwdAge: Maximum password age, in seconds (default: -1 => never expire passwords)'
     */
    if($sambaMaxPwdAge == "unset" || $sambaMaxPwdAge == "-1"){
      $sambaMaxPwdAge = _("disabled")." <i>("._("default").")</i>";
    }else{
      $sambaMaxPwdAge .= " "._("seconds"); 
    }

    /* sambaMinPwdAge: Minimum password age, in seconds (default: 0 => allow immediate password change
     */
    if($sambaMinPwdAge == "unset" || $sambaMinPwdAge == 0){
      $sambaMinPwdAge = _("disabled")." <i>("._("default").")</i>";
    }else{
      $sambaMinPwdAge .= " "._("seconds"); 
    }

    /* sambaLockoutDuration: Lockout duration in minutes (default: 30, -1 => forever)
     */
    if($sambaLockoutDuration == "unset" || $sambaLockoutDuration == 30){
      $sambaLockoutDuration = "30 "._("minutes")." <i>("._("default").")</i>";
    }elseif($sambaLockoutDuration == -1){
      $sambaLockoutDuration = _("forever");
    }else{
      $sambaLockoutDuration .= " "._("minutes");
    }

    /* sambaLockoutThreshold: Lockout users after bad logon attempts (default: 0 => off
     */
    if($sambaLockoutThreshold == "unset" || $sambaLockoutThreshold == 0){
      $sambaLockoutThreshold = _("disabled")." <i>("._("default").")</i>";
    }

    /* sambaForceLogoff: Disconnect Users outside logon hours (default: -1 => off, 0 => on 
     */
    if($sambaForceLogoff == "unset" || $sambaForceLogoff == -1){
      $sambaForceLogoff = _("off")." <i>("._("default").")</i>";
    }else{
      $sambaForceLogoff = _("on");
    }

    /* sambaRefuseMachinePwdChange: Allow Machine Password changes (default: 0 => off
     */
    if($sambaRefuseMachinePwdChange == "none" || $sambaRefuseMachinePwdChange == 0){
      $sambaRefuseMachinePwdChange = _("off")." <i>("._("default").")</i>";
    }else{
      $sambaRefuseMachinePwdChange = _("on");
    }
   
    /* USER Attributes 
     */
    /* sambaBadPasswordTime: Time of the last bad password attempt
     */
    if($sambaBadPasswordTime == "unset" || empty($sambaBadPasswordTime)){
      $sambaBadPasswordTime = "<i>("._("unset").")</i>";
    }else{
      $sambaBadPasswordTime = date("d.m.Y H:i:s",$sambaBadPasswordTime);
    }

    /* sambaBadPasswordCount: Bad password attempt count 
     */
    if($sambaBadPasswordCount == "unset" || empty($sambaBadPasswordCount)){
      $sambaBadPasswordCount = "<i>("._("unset").")</i>";
    }else{
      $sambaBadPasswordCount = date("d.m.Y H:i:s",$sambaBadPasswordCount);
    }

    /* sambaPwdLastSet: Timestamp of the last password update
     */
    if($sambaPwdLastSet == "unset" || empty($sambaPwdLastSet)){
      $sambaPwdLastSet = "<i>("._("unset").")</i>";
    }else{
      $sambaPwdLastSet = date("d.m.Y H:i:s",$sambaPwdLastSet);
    }

    /* sambaLogonTime: Timestamp of last logon
     */
    if($sambaLogonTime == "unset" || empty($sambaLogonTime)){
      $sambaLogonTime = "<i>("._("unset").")</i>";
    }else{
      $sambaLogonTime = date("d.m.Y H:i:s",$sambaLogonTime);
    }

    /* sambaLogoffTime: Timestamp of last logoff
     */
    if($sambaLogoffTime == "unset" || empty($sambaLogoffTime)){
      $sambaLogoffTime = "<i>("._("unset").")</i>";
    }else{
      $sambaLogoffTime = date("d.m.Y H:i:s",$sambaLogoffTime);
    }
   
    /* sambaKickoffTime: Timestamp of when the user will be logged off automatically
     */
    if($sambaKickoffTime == "unset" || empty($sambaKickoffTime)){
      $sambaKickoffTime = "<i>("._("unset").")</i>";
    }

    /* sambaPwdMustChange: Timestamp of when the password will expire
     */
    if($sambaPwdMustChange == "unset" || empty($sambaPwdMustChange)){
      $sambaPwdMustChange = "<i>("._("unset").")</i>";
    }

    /* sambaPwdCanChange: Timestamp of when the user is allowed to update the password
     */
    if($sambaPwdCanChange == "unset" || empty($sambaPwdCanChange)){
      $sambaPwdCanChange = "<i>("._("unset").")</i>";
    }elseif($sambaPwdCanChange != "unset" && time() > $sambaPwdCanChange){
      $sambaPwdCanChange = _("immediately") ;
    }else{
      $days     = floor((($sambaPwdCanChange - time()) / 60 / 60 / 24)) ;
      $hours    = floor((($sambaPwdCanChange - time()) / 60 / 60) % 24) ;
      $minutes  = floor((($sambaPwdCanChange - time()) / 60 ) % 60) ;
    
      $sambaPwdCanChange = " ".$days." "._("days");
      $sambaPwdCanChange.= " ".$hours." "._("hours");
      $sambaPwdCanChange.= " ".$minutes." "._("minutes");
    }

    $str =
      "\n<div style='height:200px; overflow: auto;'>".
      "\n<table style='width:100%;'>".
      "\n<tr><td><b>"._("Domain attributes")."</b></td></tr>". 
      "\n<tr><td>"._("Min password length").":           </td><td>".$sambaMinPwdLength."</td></tr>". 
      "\n<tr><td>"._("Min password length").":           </td><td>".$sambaMinPwdLength."</td></tr>". 
      "\n<tr><td>"._("Password history").":              </td><td>".$sambaPwdHistoryLength."</td></tr>".
      "\n<tr><td>"._("Force password change").":         </td><td>".$sambaLogonToChgPwd."</td></tr>".
      "\n<tr><td>"._("Maximum password age").":          </td><td>".$sambaMaxPwdAge."</td></tr>".
      "\n<tr><td>"._("Minimum password age").":          </td><td>".$sambaMinPwdAge."</td></tr>".
      "\n<tr><td>"._("Lockout duration").":              </td><td>".$sambaLockoutDuration."</td></tr>".
      "\n<tr><td>"._("Bad lockout attempt").":           </td><td>".$sambaLockoutThreshold."</td></tr>".
      "\n<tr><td>"._("Disconnect time").":               </td><td>".$sambaForceLogoff."</td></tr>".
      "\n<tr><td>"._("Refuse machine password change").":</td><td>".$sambaRefuseMachinePwdChange."</td></tr>".
      "\n<tr><td>&nbsp;</td></tr>". 
      "\n<tr><td><b>"._("User attributes")."</b></td></tr>". 
      "\n<tr><td>"._("SID").":                           </td><td>".$sambaSID."</td></tr>".
      "\n<tr><td>"._("Last failed login").":             </td><td>".$sambaBadPasswordTime."</td></tr>".
      "\n<tr><td>"._("Logon attempts").":                </td><td>".$sambaBadPasswordCount."</td></tr>".
      "\n<tr><td>"._("Last password update").":          </td><td>".$sambaPwdLastSet."</td></tr>".
      "\n<tr><td>"._("Last logon").":                    </td><td>".$sambaLogonTime."</td></tr>".
      "\n<tr><td>"._("Last logoff").":                   </td><td>".$sambaLogoffTime."</td></tr>".
      "\n<tr><td>"._("Automatic logoff").":              </td><td>".$sambaKickoffTime."</td></tr>";

      if($this->no_expiry){
        $str .= "\n<tr><td>"._("Password expires").":              </td><td>"._("No")."</td></tr>";
        $str .= "\n<tr><td colspan='2'><font color='gray'>".
          sprintf(_("The password would expire on %s, but the password expiry is disabled."),$sambaPwdMustChange).
          "</font></td></tr>";
      }else{
        $str .= "\n<tr><td>"._("Password expires").":              </td><td>".$sambaPwdMustChange."</td></tr>";
      }
    
      $str .= "\n<tr><td>"._("Password change available").":     </td><td>".$sambaPwdCanChange."</td></tr>".
      "\n</table>";
      "\n</div>";
    return($str);
  }


  function remove_from_parent()
  {
    /* Cancel if there's nothing to do here */
   if (!$this->initially_was_account){
     return;
   }
    
    /* include global link_info */
    $ldap= $this->config->get_ldap_link();

    plugin::remove_from_parent();

    /* Keep uid attribute for gosaAccount */
    unset($this->attrs['uid']);
    unset($this->attrs['uidNumber']);
    unset($this->attrs['gidNumber']);

    /* Remove objectClass for sambaIdmapEntry */
    $tmp= array();
    for ($i= 0; $i<count($this->attrs["objectClass"]); $i++){
      if ($this->attrs['objectClass'][$i] != 'sambaIdmapEntry'){
        $tmp[]= $this->attrs['objectClass'][$i];
      }
    }
    $this->attrs['objectClass']= $tmp;

    @DEBUG (DEBUG_LDAP, __LINE__, __FUNCTION__, __FILE__,
        $this->attributes, "Save");
    $ldap->cd($this->dn);
    $this->cleanup();
    $ldap->modify ($this->attrs); 

    new log("remove","users/".get_class($this),$this->dn,array_keys($this->attrs),$ldap->get_error());

    if (!$ldap->success()){
      msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $this->dn, LDAP_MOD, get_class()));
    }

    /* Optionally execute a command after we're done */
    $this->handle_post_events("remove", array("uid" => $this->uid));
  }


  /* Check for input problems */
  function check()
  {
    /* Call common method to give check the hook */
    $message= plugin::check();

    /* sambaHomePath requires sambaHomeDrive and vice versa */
    if(!empty($this->sambaHomePath) && empty($this->sambaHomeDrive)){
      $message[]= msgPool::required(_("Home drive"));
    }
    if(!empty($this->sambaHomeDrive) && empty($this->sambaHomePath)){
      $message[]= msgPool::required(_("Home path"));
    }

    /* Strings */
    foreach (array( "sambaHomePath" => _("Home directory"),
          "sambaProfilePath" => _("Profile path")) as $key => $val){
      if (!$this->mungedObject->is_samba_path($this->$key)){
        $message[]= msgPool::invalid($val);
      }
    }

    /* Numeric values */
    foreach (array(	"CtxMaxConnectionTime" => _("Connection"),
          "CtxMaxDisconnectionTime" => _("Disconnection"),
          "CtxMaxIdleTime" => _("IDLE")) as $key => $val){

      if (isset($this->mungedObject->ctx[$key]) && !tests::is_id($this->mungedObject->ctx[$key]) && $val != 0){
        $message[]= msgPool::invalid($val);
      }
    }

    /* Check dates */
    if (!tests::is_date($this->sambaKickoffTime)){
      $message[]= msgPool::invalid(_("Account expires after"), $this->sambaKickoffTime,"" ,"23.02.2009");
    }
    if (!tests::is_date($this->sambaPwdMustChange)){
      $message[]= msgPool::invalid(_("Password expires on"), $this->sambaPwdMustChange,"" ,"23.02.2009");
    }

    /* Too many workstations? Windows usrmgr only supports eight */
    if (substr_count($this->sambaUserWorkstations, ",") >= 8){
      $message[]= _("The windows usermanager allows eight clients at maximum!");
    }

    return ($message);
  }


  /* Save data to object */
  function save_object()
  {

    $SkipWrite = (!isset($this->parent) || !$this->parent) && !session::is_set('edit');

    /* We only care if we are on the sambaTab... */
    if (isset($_POST['sambaTab'])){
      plugin::save_object();

      if(isset($_POST['display_information'])){
        msg_dialog::display(_("Information"), 
          $this->get_samba_information(),
          INFO_DIALOG);
      }

      /* Take care about access options */
      if ($this->acl_is_writeable("sambaAcctFlagsL",$SkipWrite) || ($this->acl_is_writeable("sambaAcctFlagsN",$SkipWrite))){
        $attrname= "sambaPwdCanChange";
        if (isset($_POST["allow_pwchange"]) && $_POST["allow_pwchange"] == 1){
          $tmp= 1;
        } else {
          $tmp= 0;
        }
        if ($this->$attrname != $tmp){
          $this->is_modified= TRUE;
        }
        $this->sambaPwdCanChange= $tmp;
      }
      $tmp= "U";

      $this->no_expiry = FALSE;
      if (isset($_POST["no_expiry"])){
        if ($_POST["no_expiry"] == 1){
          $tmp.= "X";
          $this->no_expiry = TRUE;
        }
      }

      $this->no_password_required = FALSE;
      if (isset($_POST["no_password_required"])){
        if ($_POST["no_password_required"] == 1){
          $tmp.= "N";
          $this->no_password_required = TRUE;
        }
      }
      if (isset($_POST["password_expires"])){
        if ($_POST["password_expires"] == 1){
          $this->password_expires= 1;
        }
      } else {
        $this->password_expires= 0;
      }
      $this->temporary_disable = FALSE;
      if (isset($_POST["temporary_disable"])){
        if ($_POST["temporary_disable"] == 1){
          $this->temporary_disable = TRUE;
          if (is_integer(strpos($this->sambaAcctFlags, "L"))) {
            $tmp.= "L";
          } else {
            $tmp.= "D";
          }
        }
      }
      if (isset($_POST["logon_time_set"])){
        if ($_POST["logon_time_set"] == 1){
          $this->logon_time_set= 1;
        }
      } else {
        $this->logon_time_set= 0;
      }
      if (isset($_POST["logoff_time_set"])){
        if ($_POST["logoff_time_set"] == 1){
          $this->logoff_time_set= 1;
        }
      } else {
        $this->logoff_time_set= 0;
      }
      if (isset($_POST["kickoff_time_set"])){
        if ($_POST["kickoff_time_set"] == 1){
          $this->kickoff_time_set= 1;
        }
      } else {
        $this->kickoff_time_set= 0;
      }
      
      $fill= "";
      for ($i= strlen($tmp); $i<12; $i++){
        $fill.= " ";
      }

      $tmp= "[$tmp$fill]";

      /* Only save if acl's are set */
      if ($this->acl_is_writeable("sambaAcctFlagsL",$SkipWrite) || ($this->acl_is_writeable("sambaAcctFlagsN",$SkipWrite))){
        $attrname= "sambaAcctFlags";
        if ($this->$attrname != $tmp){
          $this->is_modified= TRUE;
        }
        $this->$attrname= $tmp;
      }

      /* Save sambaDomain attribute */
      if ($this->acl_is_writeable("sambaDomainName",$SkipWrite) && isset ($_POST['sambaDomainName'],$SkipWrite)){
        $this->sambaDomainName= validate($_POST['sambaDomainName']);
      }

      /* Save CTX values */
      /* Save obvious values */
      foreach($this->ctxattributes as $val){
        if (isset($_POST[$val]) && $this->acl_is_writeable("AllowLoginOnTerminalServer",$SkipWrite)){
          if (get_magic_quotes_gpc()) {
            $this->mungedObject->ctx[$val]= stripcslashes(validate($_POST[$val]));
          } else {
            $this->mungedObject->ctx[$val]= validate($_POST[$val]);
          }
        }
      }

      /* Save checkbox states. */
      $this->mungedObject->setTsLogin(!isset($_POST['tslogin'])
                      && $this->acl_is_writeable("AllowLoginOnTerminalServer",$SkipWrite));
      // Need to do some index checking to avoid messages like "index ... not found"
      if(isset($_POST['brokenconn'])) {
        $this->mungedObject->setBrokenConn($_POST['brokenconn'] == '1'
                      && $this->acl_is_writeable("AllowLoginOnTerminalServer",$SkipWrite));
      }
      if(isset($_POST['reconn'])) {
        $this->mungedObject->setReConn($_POST['reconn'] == '1'
                      && $this->acl_is_writeable("AllowLoginOnTerminalServer",$SkipWrite));
      }
      $this->mungedObject->setInheritMode(isset($_POST['inherit'])
                      && $this->acl_is_writeable("AllowLoginOnTerminalServer",$SkipWrite));
      $this->mungedObject->setCtxMaxConnectionTimeF(!isset($_POST['CtxMaxConnectionTimeF'])
                      && $this->acl_is_writeable("AllowLoginOnTerminalServer",$SkipWrite));
      $this->mungedObject->setCtxMaxDisconnectionTimeF(
                      !isset($_POST['CtxMaxDisconnectionTimeF']) 
                      && $this->acl_is_writeable("AllowLoginOnTerminalServer",$SkipWrite));
      $this->mungedObject->setCtxMaxIdleTimeF(!isset($_POST['CtxMaxIdleTimeF'])
                      && $this->acl_is_writeable("AllowLoginOnTerminalServer",$SkipWrite));
      $this->mungedObject->setConnectClientDrives(isset($_POST['connectclientdrives'])
                      && $this->acl_is_writeable("AllowLoginOnTerminalServer",$SkipWrite));
      $this->mungedObject->setConnectClientPrinters(isset($_POST['connectclientprinters'])  
                      && $this->acl_is_writeable("AllowLoginOnTerminalServer",$SkipWrite));
      $this->mungedObject->setDefaultPrinter(isset($_POST['defaultprinter'])
                      && $this->acl_is_writeable("AllowLoginOnTerminalServer",$SkipWrite));

      /* Save combo boxes. Takes two values */
      if(isset($_POST['reconn'])) {
        $this->mungedObject->setShadow(isset($_POST['shadow']) && $this->acl_is_writeable("AllowLoginOnTerminalServer",$SkipWrite),$_POST['shadow']);
      }

      /* Check for changes */
      if ($this->sambaMungedDial != $this->mungedObject->getMunged()){
        $this->is_modified= TRUE;
      }
      
    }
  }


  /* Save to LDAP */
  function save()
  {
    /* Load uid and gid of this 'dn' */
    $ldap= $this->config->get_ldap_link();
    $ldap->cat($this->dn, array('uidNumber', 'gidNumber'));
    $tmp= $ldap->fetch();
    $this->uidNumber= $tmp['uidNumber'][0];
    $this->gidNumber= $tmp['gidNumber'][0];

    plugin::save();

    /* Remove objectClass for sambaIdmapEntry */
    $tmp= array();
    for ($i= 0; $i<count($this->attrs["objectClass"]); $i++){
      if ($this->attrs['objectClass'][$i] != 'sambaIdmapEntry'){
        $tmp[]= $this->attrs['objectClass'][$i];
      }
    }
    $this->attrs['objectClass']= $tmp;

    /* Generate rid / primaryGroupId */
    if (!isset($this->config->data['SERVERS']['SAMBA'][$this->sambaDomainName]['SID'])){
      msg_dialog::display(_("Warning"), _("Undefined Samba SID detected. Please fix this problem manually!"), WARNING_DIALOG);
    } else {
      $this->SID= $this->config->data['SERVERS']['SAMBA'][$this->sambaDomainName]['SID'];
      $this->ridBase= $this->config->data['SERVERS']['SAMBA'][$this->sambaDomainName]['RIDBASE'];
    }

    /* Need to generate a new uniqe uid/gid combination? */
    if ($this->sambaSID == "" || $this->orig_sambaDomainName != $this->sambaDomainName){
      $uidNumber= $this->uidNumber;
      while(TRUE){
        $sid= $this->SID."-".($uidNumber*2 + $this->ridBase);
        $ldap->cd($this->config->current['BASE']);
        $ldap->search("(sambaSID=$sid)", array("sambaSID"));
        if ($ldap->count() == 0){
          break;
        }
        $uidNumber++;
      }
      $this->attrs['sambaSID']= $sid;

      /* Check for users primary group */
      $ldap->cd($this->config->current['BASE']);
      $ldap->search("(&(objectClass=posixGroup)(gidNumber=".$this->gidNumber."))", array("cn"));
      if ($ldap->count() != 1){
        msg_dialog::display(_("Warning"), _("Cannot convert primary group to samba group: group cannot be identified!"), WARNING_DIALOG);
      } else {
        $attrs= $ldap->fetch();
        $g= new group($this->config, $ldap->getDN());
        if ($g->sambaSID == ""){
          $g->sambaDomainName= $this->sambaDomainName;
          $g->smbgroup= TRUE;
          $g->save ();
        }
        $this->attrs['sambaPrimaryGroupSID']= $g->sambaSID;
      }
    }

    if ($this->sambaHomeDrive == ""){
      $this->attrs["sambaHomeDrive"]= array();
    }

    /* Generate munged dial value */
    $this->attrs["sambaMungedDial"]= $this->mungedObject->getMunged();

    /* User wants me to fake the idMappings? This is useful for
       making winbind resolve the user names in a reasonable amount
       of time in combination with larger databases. */
    if ($this->config->get_cfg_value("sambaidmapping") == "true"){
      $this->attrs['objectClass'][]= "sambaIdmapEntry";
    }


    /* Password expiery */
    if ($this->password_expires == "1"){
      #TODO: check for date format
      if ($this->attrs['sambaPwdMustChange'] == ""){
        $this->attrs['sambaPwdMustChange']= 0;
      } else {
        list($day, $month, $year)= explode('.', $this->sambaPwdMustChange);
        $this->attrs['sambaPwdMustChange']= mktime(0,0,0,$month, $day, $year);
      }
    } else {
      $this->attrs['sambaPwdMustChange']= array();
    }
    /* Make sure not to save zero in sambaPwdLastset */
    if ($this->sambaPwdLastSet != "0"){
      $this->attrs['sambaPwdLastSet']= $this->sambaPwdLastSet;
    } else {
      $this->attrs['sambaPwdLastSet']= array();
    }
    /* Account expiery */
    if ($this->logon_time_set == "1"){
      $this->attrs['sambaLogonTime']= $this->sambaLogonTime;
    } else {
      $this->attrs['sambaLogonTime']= array();
    }
    if ($this->logoff_time_set == "1"){
      $this->attrs['sambaLogoffTime']= $this->sambaLogoffTime;
    } else {
      $this->attrs['sambaLogoffTime']= array();
    }
    if ($this->kickoff_time_set == "1"){
      /* Adapt values to be timestamps */
      #TODO: check for date format
      if ($this->attrs['sambaKickoffTime'] == ""){
        $this->attrs['sambaKickoffTime']= 2147483647;
      } else {
        list($day, $month, $year)= explode('.', $this->sambaKickoffTime);
        $this->attrs['sambaKickoffTime']= mktime(0,0,0,$month, $day, $year);
      }
    } else {
      $this->attrs['sambaKickoffTime']= array();
    }

    /* Write back to ldap */
    $ldap->cd($this->dn);
    $this->cleanup();
    $ldap->modify ($this->attrs); 

    if($this->initially_was_account){
      new log("modify","users/".get_class($this),$this->dn,array_keys($this->attrs),$ldap->get_error());
    }else{
      new log("create","users/".get_class($this),$this->dn,array_keys($this->attrs),$ldap->get_error());
    }

    if (!$ldap->success()){
      msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $this->dn, 0, get_class()));
    }

    /* Optionally execute a command after we're done */
    if ($this->initially_was_account == $this->is_account){
      if ($this->is_modified){
        $this->handle_post_events("modify", array("uid" => $this->uid));
      }
    } else {
      $this->handle_post_events("add", array("uid" => $this->uid));
    }
  }


  /* Force password set, if this account doesn't have any samba passwords  */
  function password_change_needed()
  {
    if(!$this->initially_was_account && $this->is_account){
      $ldap = $this->config->get_ldap_link();
      $ldap->cat($this->dn,array("sambaLMPassword","sambaNTPassword"));
      $attrs = $ldap->fetch();
      if(!isset($attrs['sambaLMPassword']) || !isset($attrs['sambaNTPassword'])){
        return(TRUE);
      }
    }
    return(FALSE);
  }


  function adapt_from_template($dn, $skip= array())
  {
    plugin::adapt_from_template($dn, $skip);


    $this->sambaSID= "";
    $this->sambaPrimaryGroupSID= "";

    /* Fill mungedDial field */
    if (isset($this->attrs['sambaMungedDial']) && !in_array('sambaMungedDial', $skip)){
      $this->mungedObject->load($this->sambaMungedDial);
    }

    /* Adapt munged attributes */
    foreach($this->ctxattributes as $attr){
      if(isset($this->mungedObject->ctx[$attr]))
        $val = $this->mungedObject->ctx[$attr];

      foreach (array("sn", "givenName", "uid") as $repl){
        if (preg_match("/%$repl/i", $val)){
          $val= preg_replace ("/%$repl/i", $this->parent->$repl, $val);
        }
      }
      $this->mungedObject->ctx[$attr] = $val;
    }

    /* Password expiery */
    if(isset($this->attrs['sambaPwdMustChange']) &&
        $this->attrs['sambaPwdMustChange'][0] != 0 && !in_array('sambaPwdMustChange', $skip)){
      $this->password_expires= 1;
    }

    if(isset($this->attrs['sambaLogonTime']) && ! (
        $this->attrs['sambaLogonTime'][0] == 0 ||
        $this->attrs['sambaLogonTime'][0] == 2147483647
      ) && !in_array('sambaLogonTime', $skip)){
      $this->logon_time_set= 1;
    }
    if(isset($this->attrs['sambaLogoffTime']) && ! (
        $this->attrs['sambaLogoffTime'][0] == 0 ||
        $this->attrs['sambaLogoffTime'][0] == 2147483647
      ) && !in_array('sambaLogonTime', $skip)){
      $this->logoff_time_set= 1;
    }

    /* Account expiery */
    if(isset($this->attrs['sambaKickoffTime']) && ! (
        $this->attrs['sambaKickoffTime'][0] == 0 ||
        $this->attrs['sambaKickoffTime'][0] == 2147483647
      ) && !in_array('sambaKickoffTime', $skip)){
      $this->kickoff_time_set= 1;
    }

    /* Get global filter config */
    if (!session::is_set("sambafilter")){
      $ui= get_userinfo();
      $base= get_base_from_people($ui->dn);
      $sambafilter= array( "depselect" => $base, "regex" => "*");
      session::set("sambafilter", $sambafilter);
    }
  }

  
  static function plInfo()
  {
    return (array(
          "plShortName"     => _("Samba"),
          "plDescription"   => _("Samba settings"),
          "plSelfModify"    => TRUE,
          "plDepends"       => array("user"),
          "plPriority"      => 5,
          "plSection"     => array("personal" => _("My account")),
          "plCategory"    => array("users"),
          "plOptions"       => array(),

          "plProvidedAcls"  => array(

            "sambaHomePath"               => _("Generic home directory") ,
            "sambaHomeDrive"              => _("Generic samba home drive") ,
            "sambaDomainName"             => _("Domain") ,
            "sambaLogonScript"            => _("Generic script path") ,
            "sambaProfilePath"            => _("Generic profile path") ,
            "AllowLoginOnTerminalServer"  => _("Allow login on terminal server"),
            "InheritClientConfig"         => _("Inherit client config"),
            "sambaPwdCanChange"           => _("Allow user to change password") ,
            "sambaAcctFlagsN"             => _("Login from windows client requires no password"),
            "sambaAcctFlagsX"             => _("Password never expires"),
            "sambaAcctFlagsL"             => _("Lock samba account"),
            "sambaKickoffTime"            => _("Account expires") ,
            "sambaPwdMustChange"          => _("Password expires") ,
            "sambaLogonTime"              => _("Limit Logon Time") ,
            "sambaLogoffTime"             => _("Limit Logoff Time") ,
            "sambaLogonHours"             => _("Logon hours") ,
            "sambaUserWorkstations"       => _("Allow connection from"))
          ));
  }    

  function enable_multiple_support()
  {
    plugin::enable_multiple_support();
    $this->multiple_support_active = TRUE;
  } 

  function multiple_save_object()
  {
    if (isset($_POST['sambaTab'])){
      $this->save_object();
      plugin::multiple_save_object();
      foreach(array("allow_pwchange","tslogin","CtxWFHomeDir","CtxWFHomeDirDrive","CtxWFProfilePath",
            "inherit","CtxWorkDirectory","CtxInitialProgram","CtxMaxConnectionTimeF","CtxMaxConnectionTime","CtxMaxDisconnectionTimeF",
            "CtxMaxDisconnectionTime","CtxMaxIdleTimeF","CtxMaxIdleTime","connectclientdrives",
            "onnectclientprinters","defaultprinter","shadow","brokenconn",
            "reconn","allow_pwchange","connectclientprinters","no_expiry","no_password_required","temporary_disable",
            "password_expires","logon_time_set","logoff_time_set","kickoff_time_set","SetSambaLogonHours",
            "workstation_list") as $attr){
        if(isset($_POST["use_".$attr])){
          $this->multi_boxes[] = $attr;
        }
      }
    }
  }


  function multiple_check()
  {
    $message = plugin::multiple_check();

    /* Strings */
    foreach (array( "sambaHomePath" => _("Home directory"),
          "sambaProfilePath" => _("Profile path")) as $key => $val){
      if (in_array($key,$this->multi_boxes) && !$this->mungedObject->is_samba_path($this->$key)){
        $message[]= msgPool::invalid($val);
      }
    }

    /* Numeric values */
    foreach (array( "CtxMaxConnectionTime"    => _("Connection"),
                    "CtxMaxDisconnectionTime" => _("Disconnection"),
                    "CtxMaxIdleTime"          => _("IDLE")) as $key => $val){
      if (in_array($key,$this->multi_boxes) && 
          isset($this->mungedObject->ctx[$key]) && 
          !tests::is_id($this->mungedObject->ctx[$key]) && $val != 0){
        $message[]=msgPool::invalid($val);
      }
    }

    /* Too many workstations? Windows usrmgr only supports eight */
    if (substr_count($this->sambaUserWorkstations, ",") >= 8){
      $message[]= _("The windows user manager only allows eight clients. You've specified more than eight.");
    }
    return($message);
  }

  
  function get_multi_init_values()
  {
    $ret = plugin::get_multi_init_values();

    /* Parse given sambaUserWorkstations into array
     *  to allow "init_multiple_support()" to detect multiple used workstations.
     *  Those workstations will be displayed in light grey.
     */
    $tmp2 = array("count" => 0);
    $tmp = explode(",", $this->sambaUserWorkstations);
    foreach($tmp as $station){
      $station = trim($station);
      if(!empty($station)){
        $tmp2[] = $station;
        $tmp2['count'] ++;
      }
    } 
    $ret['sambaUserWorkstations'] = $tmp2;
    return($ret);
  }



  function init_multiple_support($attrs,$all)
  {
    plugin::init_multiple_support($attrs,$all);

    $this->multiple_sambaUserWorkstations = array();
    if(isset($all['sambaUserWorkstations'])){
      for($i = 0 ; $i < $all['sambaUserWorkstations']['count'] ; $i++){
        $station = trim($all['sambaUserWorkstations'][$i]);
        $this->multiple_sambaUserWorkstations[$station] = array("Name" => $station, "UsedByAllUsers" => FALSE);
      }
    }
    if(isset($attrs['sambaUserWorkstations'])){
      for($i = 0 ; $i < $attrs['sambaUserWorkstations']['count'] ; $i++){
        $station = trim($attrs['sambaUserWorkstations'][$i]);
        $this->multiple_sambaUserWorkstations[$station] = array("Name" => $station, "UsedByAllUsers" => TRUE);
      }
    }
  }

  function multiple_execute()
  {
    return($this->execute());
  } 

  function get_multi_edit_values()
  {
    $ret = plugin::get_multi_edit_values();

    /* Terminal Server  */
    if(in_array("tslogin",$this->multi_boxes)){
      $ret['tslogin'] = $this->mungedObject->getTsLogin();
    }
    if(in_array("CtxWFHomeDirDrive",$this->multi_boxes)){
      $ret['CtxWFHomeDirDrive'] = $this->mungedObject->ctx['CtxWFHomeDirDrive'];
    }
    if(in_array("CtxWFHomeDir",$this->multi_boxes)){
      $ret['CtxWFHomeDir'] = $this->mungedObject->ctx['CtxWFHomeDir'];
    }
    if(in_array("CtxWFProfilePath",$this->multi_boxes)){
      $ret['CtxWFProfilePath'] = $this->mungedObject->ctx['CtxWFProfilePath'];
    }

    if(in_array("inherit",$this->multi_boxes)){
      $ret['inherit'] = $this->mungedObject->getInheritMode();
    }       
    if(in_array("CtxInitialProgram",$this->multi_boxes)){
      $ret['CtxInitialProgram'] = $this->mungedObject->ctx['CtxInitialProgram'];
    } 
    if(in_array("CtxWorkDirectory",$this->multi_boxes)){
      $ret['CtxWorkDirectory'] = $this->mungedObject->ctx['CtxWorkDirectory'];
    } 

    /* Time Limits. Be careful here, there are some negations  */
    if(in_array("CtxMaxConnectionTimeF",$this->multi_boxes)){
      $ret["CtxMaxConnectionTimeF"]   =  !$this->mungedObject->getCtxMaxConnectionTimeF();
      if(!$ret["CtxMaxConnectionTimeF"]){
        $ret["CtxMaxConnectionTime"]   =  $this->mungedObject->ctx['CtxMaxConnectionTime'];
      }
    }
    if(in_array("CtxMaxDisconnectionTimeF",$this->multi_boxes)){
      $ret["CtxMaxDisconnectionTimeF"]=  !$this->mungedObject->getCtxMaxDisconnectionTimeF();
      if(!$ret["CtxMaxDisconnectionTimeF"]){
        $ret["CtxMaxDisconnectionTime"]=  $this->mungedObject->ctx['CtxMaxDisconnectionTime'];
      }
    }
    if(in_array("CtxMaxIdleTimeF",$this->multi_boxes)){
      $ret["CtxMaxIdleTimeF"]         =  !$this->mungedObject->getCtxMaxIdleTimeF();
      if(!$ret["CtxMaxIdleTimeF"]){
        $ret["CtxMaxIdleTime"]         =  $this->mungedObject->ctx['CtxMaxIdleTime'];
      }
    }

    /* Client Devices */
    if(in_array("connectclientdrives",$this->multi_boxes)){
      $ret["connectclientdrives"]     =  $this->mungedObject->getConnectClientDrives();
    }
    if(in_array("connectclientprinters",$this->multi_boxes)){
      $ret["connectclientprinters"]   =  $this->mungedObject->getConnectClientPrinters();
    }
    if(in_array("defaultprinter",$this->multi_boxes)){
      $ret["defaultprinter"]          =  $this->mungedObject->getDefaultPrinter();
    }

    /* Misc */
    if(in_array("shadow",$this->multi_boxes)){
      $ret["shadow"]    =$this->mungedObject->getShadow();
    }
    if(in_array("brokenconn",$this->multi_boxes)){
      $ret["brokenconn"]=$this->mungedObject->getBrokenConn();
    }
    if(in_array("reconn",$this->multi_boxes)){
      $ret["reconn"]    =$this->mungedObject->getReConn();
    }

    /* Flags */
    if(in_array("allow_pwchange",$this->multi_boxes)){
      $ret['sambaPwdCanChange'] = $this->sambaPwdCanChange;
    }
  
    if(in_array("password_expires",$this->multi_boxes)){
      $ret['password_expires']  = $this->password_expires;
      $ret['sambaPwdMustChange']= $this->sambaPwdMustChange;
    }
    if(in_array("logon_time_set",$this->multi_boxes)){
      $ret['logon_time_set'] = $this->logon_time_set;
      $ret['sambaLogonTime'] = $this->sambaLogonTime;
    }
    if(in_array("logoff_time_set",$this->multi_boxes)){
      $ret['logoff_time_set'] = $this->logoff_time_set;
      $ret['sambaLogoffTime'] = $this->sambaLogoffTime;
    }
    if(in_array("kickoff_time_set",$this->multi_boxes)){
      $ret['kickoff_time_set'] = $this->kickoff_time_set;
      $ret['sambaKickoffTime'] = $this->sambaKickoffTime;
    }

    if(in_array("no_password_required",$this->multi_boxes)){
      $ret['no_password_required'] = $this->no_password_required;
    }

    if(in_array("no_expiry",$this->multi_boxes)){
      $ret['no_expiry'] = $this->no_expiry;
    }

    if(in_array("temporary_disable",$this->multi_boxes)){
      $ret['temporary_disable'] = $this->temporary_disable;
    }
    
    if(in_array("SetSambaLogonHours",$this->multi_boxes)){
      $ret['sambaLogonHours'] = $this->sambaLogonHours;
    }

    if(in_array("workstation_list",$this->multi_boxes)){
      $ret['multiple_sambaUserWorkstations'] = $this->multiple_sambaUserWorkstations;
    }
    return($ret);
  }

  function set_multi_edit_values($values)
  {
    plugin::set_multi_edit_values($values);

    /* Prepare current workstation settings to be merged 
     *  with multiple edit settings.
     */
    if(isset($values['multiple_sambaUserWorkstations'])){
      $cur_ws = array();
      $m_ws = $values['multiple_sambaUserWorkstations'];

      /* Prepare current settings to be merged */
      if(isset($this->sambaUserWorkstations)){
        $ttmp = explode(",",$this->sambaUserWorkstations);
        foreach($ttmp as $station){
          $station = trim($station);
          if(!empty($station)){
            $cur_ws[$station] = array("Name" => $station, "UsedByAllUsers" => TRUE);
          }
        }
      }

      /* Unset removed workstations */
      foreach($cur_ws as $cur_name => $cur_station){
        if(!isset($m_ws[$cur_name])){
          unset($cur_ws[$cur_name]);
        }
      }

      /* Add all added workstations */
      foreach($m_ws as $name => $station){
        if($station['UsedByAllUsers']){
          $cur_ws[$name] = $station;
        }
      }

      $this->sambaUserWorkstations = "";
      foreach($cur_ws as $name => $ws){
        $this->sambaUserWorkstations .= $name.",";
      }
      $this->sambaUserWorkstations=preg_replace("/,$/","",$this->sambaUserWorkstations);
    }

    /* Enable disabled terminal login, this is inverted somehow */
    if(isset($values['tslogin']))   $this->mungedObject->setTsLogin(!$values['tslogin']);
  
    /* Imherit client configuration */
    if(isset($values['inherit']))   $this->mungedObject->setInheritMode($values['inherit']);
  
    /* Get all ctx values posted */
    $ctx = array("CtxWFHomeDirDrive","CtxWFHomeDir","CtxWFProfilePath","CtxInitialProgram","CtxWorkDirectory",
                 "CtxMaxConnectionTime","CtxMaxDisconnectionTime","CtxMaxIdleTime");
    foreach($ctx as $attr){
      if(isset($values[$attr])){
        $this->mungedObject->ctx[$attr] = $values[$attr] ;
      }
    }

    if(isset($values['CtxMaxConnectionTimeF']))   $this->mungedObject->setCtxMaxConnectionTimeF($values['CtxMaxConnectionTimeF']);
    if(isset($values['CtxMaxDisconnectionTimeF']))$this->mungedObject->setCtxMaxDisconnectionTimeF($values['CtxMaxDisconnectionTimeF']);
    if(isset($values['CtxMaxIdleTimeF']))         $this->mungedObject->setCtxMaxIdleTimeF($values['CtxMaxIdleTimeF']);

    if(isset($values['connectclientdrives']))   $this->mungedObject->setConnectClientDrives($values['connectclientdrives']);
    if(isset($values['connectclientprinters'])) $this->mungedObject->setConnectClientPrinters($values['connectclientprinters']);
    if(isset($values['defaultprinter']))        $this->mungedObject->setDefaultPrinter($values['defaultprinter']);

    if(isset($values['shadow']))        $this->mungedObject->setShadow($values['shadow'],$values['shadow']);
    if(isset($values['brokenconn']))    $this->mungedObject->setBrokenConn($values['brokenconn'],$values['brokenconn']);
    if(isset($values['reconn']))        $this->mungedObject->setReConn($values['reconn'],$values['reconn']);

  
    if(isset($values['sambaPwdCanChange']))  $this->sambaPwdCanChange  = $values['sambaPwdCanChange'];

    
    

    if(isset($values['password_expires'])){
      $this->password_expires = $values['password_expires'];
      $this->sambaPwdMustChange = $values['sambaPwdMustChange'];
    }
    if(isset($values['logon_time_set'])){
      $this->logon_time_set = $values['logon_time_set'];
      $this->sambaLogonTime = $values['sambaLogonTime'];
    }
    if(isset($values['logoff_time_set'])){
      $this->logoff_time_set = $values['logoff_time_set'];
      $this->sambaLogoffTime = $values['sambaLogoffTime'];
    }
    if(isset($values['kickoff_time_set'])){
      $this->kickoff_time_set = $values['kickoff_time_set'];
      $this->sambaKickoffTime = $values['sambaKickoffTime'];
    }

    if(isset($values['no_password_required'])){
      if($values['no_password_required']){
        if(!preg_match("/N/",$this->sambaAcctFlags)) {
          $this->sambaAcctFlags = preg_replace("/ /","N",$this->sambaAcctFlags,1);
        }
      }else{
        $this->sambaAcctFlags = preg_replace("/N/"," ",$this->sambaAcctFlags,1);
      }
    }      

    if(isset($values['no_expiry'])){
      if($values['no_expiry']){
        if(!preg_match("/N/",$this->sambaAcctFlags)) {
          $this->sambaAcctFlags = preg_replace("/ /","N",$this->sambaAcctFlags,1);
        }
      }else{
        $this->sambaAcctFlags = preg_replace("/N/"," ",$this->sambaAcctFlags,1);
      }
    }      

    if(isset($values['temporary_disable'])){
      if($values['temporary_disable']){
        if(preg_match("/L/",$this->sambaAcctFlags)) {
          // Keep L
        }else{
          $this->sambaAcctFlags = preg_replace("/ /","D",$this->sambaAcctFlags,1);
        }
      }else{
        $this->sambaAcctFlags = preg_replace("/D/"," ",$this->sambaAcctFlags,1);
      }
    }
  }


  function PrepareForCopyPaste($source)
  {
    plugin::PrepareForCopyPaste($source);

    /* Set a new SID */
    $this->sambaSID = "";
  }

}

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
