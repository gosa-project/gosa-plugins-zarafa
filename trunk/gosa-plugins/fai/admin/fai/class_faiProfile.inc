<?php

class faiProfile extends plugin
{
  /* attribute list for save action */
  var $ignore_account   = TRUE;

  /* Attributes for this Object */
  var $attributes       = array("cn","description","FAIclass");

  /* ObjectClasses for this Object*/
  var $objectclasses    = array("top","FAIclass","FAIprofile");

  /* Specific attributes */
  var $old_cn           = "";
  var $cn               = "";       // The class name for this object
  var $description      = "";       // The description for this set of partitions
  var $is_dialog        = false;    // specifies which buttons will be shown to save or abort
  var $FAIclass         = "";       // Classnames used for this profile
  var $FAIclasses       = array();  // Contains classname seperated in an array
  var $FAIAllclasses    = array();  // Contains all possible Classnames
  var $ui               ;
  var $FAIstate      = "";
  var $view_logged      = FALSE;

  function faiProfile(&$config, $dn= NULL)
  {
    /* Load Attributes */
    plugin::plugin ($config, $dn);
    
    $this->ui = get_userinfo();

    /* Parse ldap attribute to get all assigned classes */
    $tmp = split(" ",$this->FAIclass);
    $tmp2 = array();
    foreach($tmp as $class){
      if(!empty($class)){
        $class = trim($class);
        $tmp2[] = $class;
      }
    }

    /* Sort assigned classes */ 
    if(is_array($tmp2)){
      foreach($tmp2 as $class){
        $this->FAIclasses[]=$class;
      }
    }


    if($dn != "new"){
      $this->dn =$dn;
    }
    $this->old_cn   = $this->cn;
  }

  function get_all_fai_classes()
  {
    if (count($this->FAIAllclasses)) {
      return;
    }
    
    $ldap=$this->config->get_ldap_link();

    $categories = array("FAIscript","FAItemplate","FAIhook","FAIvariable","FAIpartitionTable","FAIpackageList");

    /* Build filter */
    $filter= "";
    foreach ($categories as $cat){
      $filter.= "(objectClass=$cat)";
    }
    
    $sort = array();
    /* search all FAI classes */
    $res = FAI::get_all_objects_for_given_base($this->parent->parent->fai_release, "(|$filter)", true);
    foreach($res as $attrs){
      /* Sort by categorie */
      foreach($categories as $cat){
        if(in_array($cat,$attrs['objectClass'])){

          /* Append entry */
          $this->FAIAllclasses[$attrs['cn'][0]][$cat]=$attrs;
  
          /* Create sort array, because the array above is a multidimensional array, and can't be sorted by php sorting functions*/
          $sort[strtolower($attrs['cn'][0])] = $attrs['cn'][0];
        }
      }
    } 

    /* Sort the sort array */
    //ksort($sort);

    /* Reorder the FAIclasses array */
    foreach($sort as $name){
      $tmp[$name] =$this->FAIAllclasses[$name];
    }

    /* Assign sorted classes */
    $this->FAIAllclasses = array();
    $this->FAIAllclasses = $tmp;
  }

  function acl_base_for_current_object($dn)
  {
    if($dn == "new" || $dn == ""){
      if($this->dn == "new"){
        $dn= $this->parent->parent->acl_base;
      }else{
        $dn = $this->dn;
      }
    }
    return($dn);
  }


  /* returns position in array */
  function getpos($atr,$attrs)
  {
    $i = 0;
    foreach($attrs as $value) {
      if($atr == $value) {
        return($i);
      }
      $i++;
    }
  }

  /* Transports the given Arraykey one position up*/
  function ArrayUp($atr,$attrs)
  {
    $index = $this->getpos($atr,$attrs);
    $res = $attrs;

    /* Don't do anything if there is just one element */
    if (count($attrs) == 1) {
      return($res);
    }

    for($i=0;$i<count($attrs);$i++) {
      if ($i==$index) {
        $left_index = $index - 1;
        $res[$left_index] = $attrs[$index];
        $res[$index] = $attrs[$left_index];
      }
    }

    return($res);
  }


  /* Transports the given Arraykey one position down*/
  function ArrayDown($atr,$attrs)
  {
    $index = $this->getpos($atr,$attrs);
    $res = $attrs;

    /* Don't do anything if there is just one element */
    if (count($attrs) == 1) {
      return($res);
    }

    for($i=0;$i<count($attrs);$i++) {
      if ($i==$index) {
        $left_index = ($index+1);
        $res[$left_index] = $attrs[$index];
        $res[$index] = $attrs[$left_index];
      }
    }

    return($res);
  }

  /* class one position up */
  function catUp($id)
  {
    /* Get all cats depinding on current dir */
    $cats = $this->FAIclasses;
    $this->FAIclasses =$this->ArrayUp($id,$cats);
  }

  /* Class one position down */
  function catDown($id)
  {
    /* Get all cats depinding on current dir */
    $cats = $this->FAIclasses;
    $this->FAIclasses =$this->ArrayDown($id,$cats);
  }

  function execute()
  {
    /* Call parent execute */
    plugin::execute();

    if($this->is_account && !$this->view_logged){
      $this->view_logged = TRUE;
      new log("view","fai/".get_class($this),$this->dn);
    }

    /* Fill templating stuff */
    $smarty= get_smarty();
    $display= "";

    $s_entry = "";
    $s_action = "";

    /* Remove class name From list */
    $sort_once = false;
    if(!preg_match("/freeze/",$this->FAIstate)){
      foreach($_POST as $name => $post){
        if(preg_match("/DEL_/i",$name) && $this->acl_is_writeable("FAIclass")){
          $s_action = "delete";
          $s_entry  = preg_replace("/DEL_/","",$name);
          $s_entry  = base64_decode(preg_replace("/_.*$/","",$s_entry));
        }elseif(preg_match("/Add_class/i",$name)&& $this->acl_is_writeable("FAIclass")){
          $s_action  = "add";
        }elseif(preg_match("/DelClass/i",$name) && $this->acl_is_writeable("FAIclass")){
          $s_action  = "delete";
          $s_entry = $_POST['FAIclass'];
        }elseif(preg_match("/AddClass/i",$name) && $this->acl_is_writeable("FAIclass")){
          $s_action  = "add";
        }

        /* Check if a list element should be pushed one position up */
        if((preg_match("/sortup_/",$name))&&(!$sort_once) && $this->acl_is_writeable("FAIclass")){
          $sort_once = true;
          $val = preg_replace("/sortup_/","",$name);
          $val = preg_replace("/_.*$/","",$val);
          $val = base64_decode($val);
          $this->catUp($val);
        }

        /* Check if a list element should be pushed one position down */
        if((preg_match("/sortdown_/",$name))&&(!$sort_once) && $this->acl_is_writeable("FAIclass")){
          $sort_once = true;
          $val = preg_replace("/sortdown_/","",$name);
          $val = preg_replace("/_.*$/","",$val);
          $val = base64_decode($val);
          $this->catDown($val);
        }

      }

      if($s_action == "delete" && $this->acl_is_writeable("FAIclass")){
        $newfaiclasses = array();
        foreach($this->FAIclasses as $key => $name) {
          if ($name != $s_entry) {
            array_push($newfaiclasses, $name);
          }
        }
        $this->FAIclasses = $newfaiclasses;
      }

      if($s_action == "add" && $this->acl_is_writeable("FAIclass")){
        $this->dialog = new faiProfileEntry($this->config,$this->dn,$this->FAIclasses,$this->parent->parent->fai_release);
        $this->is_dialog  =true;
      }

      /* Save Dialog */
      if(isset($_POST['SaveSubObject'])){
        $this->dialog->save_object();
        $msgs= $this->dialog->check();
        if(count($msgs)){
          msg_dialog::display(_("Error"), $msgs, ERROR_DIALOG);
        }else{
          $ret = $this->dialog->save();
          foreach($ret as $class){
            $this->FAIclasses[] =$class; 
          }
          $this->is_dialog=false;
          unset($this->dialog);
          $this->dialog=FALSE;
          //ksort($this->FAIclasses);
        }
      }
    }

    /* Cancel Dialog */
    if(isset($_POST['CancelSubObject'])){
      $this->is_dialog=false;
      unset($this->dialog);
      $this->dialog=FALSE;
    }

    if(is_object($this->dialog)){
      $this->dialog->save_object();
      return($this->dialog->execute());
    }

    $divlist  =new divSelectBox("Profile");
    $divlist->SetSummary(_("This list displays all assigned class names for this profile."));

    /* item images */
    $objTypes['FAIhook']            = "<img src='plugins/fai/images/fai_hook.png' title='"._("Hook bundle")."' alt=''>";
    $objTypes['FAItemplate']        = "<img src='plugins/fai/images/fai_template.png' title='"._("Template bundle")."' alt=''>";
    $objTypes['FAIscript']          = "<img src='plugins/fai/images/fai_script.png' title='"._("Script bundle")."' alt=''>";
    $objTypes['FAIvariable']        = "<img src='plugins/fai/images/fai_variable.png' title='"._("Variable bundle")."' alt=''>";
    $objTypes['FAIpackageList']        = "<img src='plugins/fai/images/fai_packages.png' title='"._("Package bundle")."' alt=''>";
    $objTypes['FAIpartitionTable']  = "<img src='plugins/fai/images/fai_partitionTable.png' title='"._("Partition table")."' alt=''>";

    /* Delete button */
    $actions = "<input type='image' src='images/lists/trash.png' title='"._("Remove class from profile")."' name='DEL_%KEY%'>"; 
    
    /* Up down buttons */
    $linkup = " &nbsp;<input type='image' name='sortup_%s'   alt='up'    title='"._("Up")."'   src='images/lists/sort-up.png' align='top' >";
    $linkdown = " &nbsp;<input type='image' name='sortdown_%s' alt='down'  title='"._("Down")."' src='images/lists/sort-down.png' >";
    /* Append fai classes to divlist */
    if($this->acl_is_readable("FAIclass")){
      $i = 1;
      $count = count($this->FAIclasses);

      $linkupdown = '';
      foreach($this->FAIclasses as $usedClass){
        if($i==1) {
          if ($count > 1) {
            $linkupdown = "&nbsp;&nbsp;&nbsp;&nbsp;" . $linkdown;
          }
        } elseif ($i<$count) {
          $linkupdown = $linkup . $linkdown;    
        }
        else {
          $linkupdown = $linkup;
        }

        $str = "&nbsp;";
        $act = "";

        $this->get_all_fai_classes();

        if(isset($this->FAIAllclasses[$usedClass])){
          foreach($this->FAIAllclasses[$usedClass] as $class => $obj){
            $str.= $objTypes[$class]; 
          }
        }

        $field1 = array("string"=> $usedClass,"attach"=>"");
        $field2 = array("string"=> $str,"attach"=>"");
        if(!preg_match("/freeze/", $this->FAIstate) && $this->acl_is_writeable("FAIclass")){
          $field3 = array("string"=> preg_replace("/%KEY%/",base64_encode($usedClass),$actions).
              preg_replace("/%s/",base64_encode($usedClass),$linkupdown),
              "attach"=>"style='border-right:none;'");
        }else{
          $field3 = array("string"=>"&nbsp;", "attach"=>"style='border-right:none;'");
        }
        $divlist->AddEntry(array($field1,$field2,$field3));
        $i++;
      }
    }

    $smarty->assign("freeze", preg_match("/freeze/i",$this->FAIstate));

    $smarty->assign("FAIclasses"  ,$this->FAIclasses);
    $smarty->assign("divlist"     ,$divlist->DrawList());

    /* Magic quotes GPC, escapes every ' " \, to solve some security risks
     * If we post the escaped strings they will be escaped again
     */
    foreach($this->attributes as $attrs){
      if(get_magic_quotes_gpc()){
        $smarty->assign($attrs,stripslashes($this->$attrs));
      }else{
        $smarty->assign($attrs,($this->$attrs));
      }
    }

    
    $dn = $this->acl_base_for_current_object($this->dn);
    $smarty->assign("sub_object_is_addable",
        preg_match("/c/",$this->ui->get_permissions($dn,"fai/faiScriptEntry")) &&
        !preg_match("/freeze/",$this->FAIstate));

    $tmp = $this->plInfo();
    foreach($tmp['plProvidedAcls'] as $name => $translated){
      $smarty->assign($name."ACL",$this->getacl($name, preg_match("/freeze/",$this->FAIstate)));
    }

    $display.= $smarty->fetch(get_template_path('faiProfile.tpl', TRUE));
    return($display);
  }

  function remove_from_parent()
  {
    $ldap = $this->config->get_ldap_link();
    $ldap->cd ($this->dn);
    $release = $this->parent->parent->fai_release;
    $use_dn = preg_replace("/".preg_quote(FAI::get_release_dn($this->dn), '/')."/i", $release, $this->dn);
    new log("remove","fai/".get_class($this),$use_dn,$this->attributes);
    FAI::prepare_to_save_FAI_object($use_dn,array(),true);
    $this->handle_post_events("remove");    
  }


  /* Save data to object 
   */
  function save_object()
  {
    if(!preg_match("/freeze/",$this->FAIstate)){
      plugin::save_object();
    }
  }


  /* Check supplied data */
  function check()
  {
    /* Call common method to give check the hook */
    $message= plugin::check();

    if(count($this->FAIclasses) == 0){
      $message[]=_("No class specified for this profile!");
    }

    if($this->cn == ""){
      $message[]= msgPool::required(_("Name"));
    }

    /* Ensure that we do not overwrite an allready existing entry 
     */
    if($this->dn == "new" || $this->cn != $this->old_cn){
      $release = $this->parent->parent->fai_release;
      $new_dn= 'cn='.$this->cn.",".get_ou('faiProfileRDN').get_ou('faiBaseRDN').$release;
      $res = faiManagement::check_class_name("FAIprofile",$this->cn,$new_dn);
      if(isset($res[$this->cn])){
        $message[] = msgPool::duplicated(_("Name"));
      }
    }

    if (preg_match("/^-/", $this->cn)) {
      $message[] = sprintf(_("The specified classname '%s' is invalid. Classnames must not start with a dash."), $this->cn);
    }

    if($this->cn != preg_replace("/ /","",trim($this->cn))){
      $message[] = msgPool::invalid(_("Name"),preg_replace("/ /","_",$this->cn),"/[^_]/");
    }

    return ($message);
  }


  /* Save to LDAP */
  function save()
  {
    plugin::save();

    $ldap = $this->config->get_ldap_link();

    $this->FAIclass = "";
    foreach($this->FAIclasses as $class){
      $this->FAIclass.=$class." ";
    }

    $this->attrs['FAIclass']=trim($this->FAIclass);

    /* Remove the old FAI profile, if the dn has changed.
     */
    if($this->cn != $this->old_cn && $this->old_cn != ""){

      $old_dn = preg_replace("/^cn=[^,]+,/","cn=".$this->old_cn.",",$this->dn);
      FAI::prepare_to_save_FAI_object($old_dn,array(),TRUE);
    }
    FAI::prepare_to_save_FAI_object($this->dn,$this->attrs);
   
    if($this->initially_was_account){
      new log("modify","fai/".get_class($this),$this->dn,$this->attributes);
    }else{
      new log("create","fai/".get_class($this),$this->dn,$this->attributes);
    }
  }


  function PrepareForCopyPaste($source)
  {
    plugin::PrepareForCopyPaste($source);

    /* Parse ldap attribute to get all assigned classes */
    $tmp = split(" ",$this->FAIclass);
    $tmp2 = array();
    foreach($tmp as $class){
      if(!empty($class)){
        $class = trim($class);
        $tmp2[] = $class;
      }
    }

    /* Sort assigned classes */
    if(is_array($tmp2)){
      foreach($tmp2 as $class){
        $this->FAIclasses[]=$class;
      }
    }
  }


  /* Return plugin informations for acl handling */ 
  static function plInfo()
  {
    return (array( 
          "plShortName" => _("Profile"),
          "plDescription" => _("FAI profile"),
          "plSelfModify"  => FALSE,
          "plDepends"     => array(),
          "plPriority"    => 30,
          "plSection"     => array("administration"),
          "plCategory"    => array("fai"),
          "plProvidedAcls" => array(
            "cn"                => _("Name"),
            "description"       => _("Description"),
            "FAIclass"          => _("FAI classes"))
          ));
  }


  /*! \brief  Used for copy & paste.
              Returns a HTML input mask, which allows to change the cn of this entry.
      @param  Array   Array containing current status && a HTML template.
   */
  function getCopyDialog()
  {
    $vars = array("cn");
    $smarty = get_smarty();
    $smarty->assign("cn", htmlentities($this->cn));
    $str = $smarty->fetch(get_template_path("paste_generic.tpl",TRUE));
    $ret = array();
    $ret['string'] = $str;
    $ret['status'] = "";
    return($ret);
  }


  /*! \brief  Used for copy & paste.
              Some entries must be renamed to avaoid duplicate entries.
   */
  function saveCopyDialog()
  {
    if(isset($_POST['cn'])){
      $this->cn = get_post('cn');
    }
  }
}

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
