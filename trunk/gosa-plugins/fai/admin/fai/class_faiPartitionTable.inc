<?php

class faiPartitionTable extends plugin
{
  /* attribute list for save action */
  var $ignore_account   = TRUE;
  var $attributes       = array("cn","description");
  var $objectclasses    = array("top","FAIclass","FAIpartitionTable");

  var $subAttributes      = array("cn","description");
  var $subPartAttributes  = array("cn","FAIpartitionNr","FAIpartitionSize","FAImountPoint","FAIfsType","FAIpartitionType","FAImountOptions","FAIfsOptions","FAIpartitionFlags","description");

  var $sub64coded = array();
  var $subBinary = array();

  /* Specific attributes */
  var $cn               = "";       // The class name for this object
  var $description      = "";       // The description for this set of partitions
  var $disks            = array();  // All defined Disks 
  var $is_dialog        = false;    // specifies which buttons will be shown to save or abort

  var $FAIstate         = "";
  var $ui;

  var $view_logged      = FALSE;

  function faiPartitionTable (&$config, $dn= NULL)
  {
    /* Load Attributes */
    plugin::plugin ($config, $dn);

    /* If "dn==new" we try to create a new entry
     * Else we must read all objects from ldap which belong to this entry.
     */
    $this->ui = get_userinfo();
    if($dn != "new"){
      $this->dn =$dn;

      /* Read all leaf objects of this object (For FAIscript this would be FAIscriptEntry)
       */
      $res = FAI::get_leaf_objects($this->dn, $this->cn, "FAIpartitionDisk", get_ou('faiPartitionRDN'));
      foreach($res as $obj){
        $objects = array();
        $objects['description']  = "";
        $objects['status']      = "edited";
        $objects['dn']          = $obj['dn'];
        $objects                = $this->get_object_attributes($objects,$this->subAttributes);
        $this->disks[$objects['cn']] = $objects;
        $this->disks[$objects['cn']]['partitions'] = array();
      }

      /* read all partitions for each disk 
       */
      foreach($this->disks as $name => $disk){

        $res = FAI::get_leaf_objects($disk['dn'], $name, "FAIpartitionEntry", "cn=".$this->cn.",".get_ou('faiPartitionRDN'));
        foreach($res as $obj){
          $objects = array();
          $objects['status']      = "edited";
          $objects['dn']          = $obj['dn'];
          $objects                = $this->get_object_attributes($objects,$this->subPartAttributes);
          unset($objects['dn']);;
          $this->disks[$name]['partitions'][$objects['FAIpartitionNr']] = $objects;
        }
      }
    }

    $this->is_new = FALSE;
    if($this->dn == "new"){
      $this->is_new =TRUE;
    }
    ksort($this->disks);
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
 
    /* Add Disk to this Partitionset
     * This code adds a new HDD to the disks 
     * A new Dialog will be opened 
     */
    if(isset($_POST['AddDisk']) && !preg_match("/freeze/i",$this->FAIstate)){
      $usedDiskNames =array();
      foreach($this->disks as $key=>$disk){
        $usedDiskNames[]= $key;
      }
      $this->dialog = new faiPartitionTableEntry($this->config,$this->dn,$usedDiskNames); 
      $this->dialog->set_acl_base($this->acl_base_for_current_object($this->dn));
      $this->dialog->set_acl_category("fai");
      $this->dialog->FAIstate = $this->FAIstate;
      $this->is_dialog = true;
    }

    /* Edit disk.
     * Open dialog which allows us to edit the selected entry 
     */    

    if($this->dn != "new"){
      set_object_info($this->dn);
    }

    if((isset($_POST['EditDisk']))&&(isset($_POST['disks']))){
      $usedDiskNames =array();
      $Udisk = $_POST['disks'][0];
      if(isset($this->disks[$Udisk])){

        foreach($this->disks  as $key=>$disk){
          if($key != $Udisk){
            $usedDiskNames[]= $key;
          }
        }

        /* Set object info string, which will be displayed in plugin info line */ 
        if(isset($this->disks[$Udisk]['dn'])){
          set_object_info($this->disks[$Udisk]['dn']);
          $dn = $this->disks[$Udisk]['dn'];
        }else{
          set_object_info("");
          $dn = "new";
        }

        $this->dialog = new faiPartitionTableEntry($this->config,$this->dn,$usedDiskNames,$this->disks[$Udisk]); 
        $this->dialog->set_acl_base($this->acl_base_for_current_object($dn));
        $this->dialog->set_acl_category("fai");
        $this->dialog->FAIstate = $this->FAIstate;

        $this->is_dialog = true;
      }
    }

    /* Edit aborted, close dialog, without saving anything
     */
    if(isset($_POST['CancelDisk'])){
      unset($this->dialog);
      $this->dialog = FALSE;
      $this->is_dialog=false;
    }

    /* Dialog saved
     * Save given data from Dialog, if no error is occurred
     */
    if(isset($_POST['SaveDisk'])){

      if (!preg_match("/freeze/i", $this->FAIstate)){
        $this->dialog->save_object();
        if(count($this->dialog->check())){
          foreach($this->dialog->check() as $msg){
            msg_dialog::display(_("Error"), $msg, ERROR_DIALOG);
          }
        }else{
          $disk = $this->dialog->save();
          if(isset($disk['rename'])){
            if($this->disks[$disk['rename']['from']]['status']=="edited"){
              $this->disks[$disk['rename']['from']]['status']="delete";
            }else{
              unset($this->disks[$disk['rename']['from']]);
            }

            foreach($disk['partitions'] as $key => $val){
              if($disk['partitions'][$key]['status']!="delete"){
                $disk['partitions'][$key]['status']= "new";
              }
            }

            $disk['status']="new";
            $disk['cn']= $disk['rename']['to'];
          }

          $this->disks[$disk['cn']]=$disk; 
          unset($this->dialog);
          $this->dialog = FALSE;
          $this->is_dialog=false;
          ksort($this->disks);
        }
      }else{
        $this->dialog = FALSE;
        $this->is_dialog=false;
      }
    }

    /* Delete selected disk drive from list
     * Assign delete status for all its partitions      
     */
    if((isset($_POST['DelDisk']))&&(!empty($_POST['disks']))){
      if (!preg_match("/freeze/i", $this->FAIstate)){
        foreach($_POST['disks'] as $disk) {

          if(isset($this->disks[$disk])){
            if($this->disks[$disk]['status']=="edited"){
              $this->disks[$disk."-delete"]=$this->disks[$disk];
              unset($this->disks[$disk]);
              $disk = $disk."-delete";        
              $this->disks[$disk]['status']="delete";
              foreach($this->disks[$disk]['partitions'] as $name => $value ){
                if($value['status']=="edited"){
                  $this->disks[$disk]['partitions'][$name]['status']="delete"; 
                }else{
                  unset($this->disks[$disk]['partitions'][$name]);
                }
              }
            }else{
              unset($this->disks[$disk]);
            }
          }
        }
      }
    }

    /* Display dialog if one is defined
     */
    if(is_object($this->dialog)){
      $this->dialog->save_object();
      return($this->dialog->execute());
    }

    /* Assign all attributes to smarty engine
     */
    foreach($this->attributes as $attrs){
      $smarty->assign($attrs,$this->$attrs);
      if($this->$attrs){
        $smarty->assign($attrs."CHK"," ");
      }else{
        $smarty->assign($attrs."CHK"," disabled ");
      }
    }
   
    $dn = $this->acl_base_for_current_object($this->dn);
    $smarty->assign("sub_object_is_addable",
        preg_match("/c/",$this->ui->get_permissions($dn,"fai/faiPartitionTableEntry")) &&
        !preg_match("/freeze/",$this->FAIstate));
    $smarty->assign("sub_object_is_removeable",
        preg_match("/d/",$this->ui->get_permissions($dn,"fai/faiPartitionTableEntry")) &&
        !preg_match("/freeze/",$this->FAIstate));

    $tmp = $this->plInfo();
    foreach($tmp['plProvidedAcls'] as $name => $translated){
      $smarty->assign($name."ACL",$this->getacl($name,preg_match("/freeze/i",$this->FAIstate)));
    }
    $disks = $this->getDisks();
    $smarty->assign("disks"   ,$disks);
    $display.= $smarty->fetch(get_template_path('faiPartitionTable.tpl', TRUE));
    return($display);
  }

  function getDisks(){
    /* Return all available disks for this partition table
     * Return in listBox friendly array
     */
    $a_return = array();
    foreach($this->disks as $key => $disk){

      $dn = "new";
      if(isset($obj['dn'])){
        $dn = $obj['dn'];
      }
      $dn = $this->acl_base_for_current_object($dn);
      $acl = $this->ui->get_permissions($dn,"fai/faiPartitionTableEntry");
      if(preg_match("/(r|w)/",$acl)) {

        if($disk['status'] != "delete"){
          $cnt=0;
          foreach($disk['partitions'] as $val){
            if($val['status']!="delete"){
              $cnt ++;
            }
          }
          if(!empty($disk['description'])){
            if($cnt == 1){
              $a_return[$key]=  $disk['cn']." [".$disk['description']."], ".sprintf(_("%s partition"), $cnt);
            }else{
              $a_return[$key]=  $disk['cn']." [".$disk['description']."], ".sprintf(_("%s partition(s)"), $cnt);
            }
          }else{
            if($cnt == 1){
              $a_return[$key]=  $disk['cn'].", ".sprintf(_("%s partition"), $cnt);
            }else{
              $a_return[$key]=  $disk['cn'].", ".sprintf(_("%s partition(s)"), $cnt);
            }
          }
        }
      }
    }
    return($a_return);
  }


  /* Delete me, and all my subtrees
   */
  function remove_from_parent()
  {
    $ldap = $this->config->get_ldap_link();
    $ldap->cd ($this->dn);

    $release = $this->parent->parent->fai_release;
    $use_dn = preg_replace("/".preg_quote(FAI::get_release_dn($this->dn), '/')."/i", $release, $this->dn);

    FAI::prepare_to_save_FAI_object($use_dn,array(),true);
    new log("remove","fai/".get_class($this),$use_dn,$this->attributes);   
    foreach($this->disks as $disk){
      $disk_dn = "cn=".$disk['cn'].",".$this->dn;
      $use_dn = preg_replace("/".preg_quote(FAI::get_release_dn($this->dn), '/')."/i",$release, $disk_dn);
      FAI::prepare_to_save_FAI_object($use_dn,array(),true);
      foreach($disk['partitions'] as $key => $partition){    
        $partition_dn= "FAIpartitionNr=".$partition['FAIpartitionNr'].",".$disk_dn;      
        $use_dn = preg_replace("/".preg_quote(FAI::get_release_dn($this->dn), '/')."/i", $release, $partition_dn);
        FAI::prepare_to_save_FAI_object($use_dn,array(),true);
      }
    }
  }


  /* Save data to object 
   */
  function save_object()
  {
    if (preg_match("/freeze/", $this->FAIstate)) return;
    plugin::save_object();
    foreach($this->attributes as $attrs){
      if(isset($_POST[$attrs])){
        $this->$attrs = $_POST[$attrs];
      }
    }
  }


  /* Check supplied data */
  function check()
  {
    /* Call common method to give check the hook */
    $message= plugin::check();

    /* Ensure that we do not overwrite an allready existing entry 
     */
    if($this->is_new){
      $release = $this->parent->parent->fai_release;
      $new_dn= 'cn='.$this->cn.",".get_ou('faiPartitionRDN').get_ou('faiBaseRDN').$release;
      $res = faiManagement::check_class_name("FAIpartitionTable",$this->cn,$new_dn);
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
    /* Save current settings.
     * 1 : We must save the partition table, with its description and cn 
     * 2 : Append Disk with cn and  description.
     * 3 : Save partitions for each disk
     */  

    $ldap = $this->config->get_ldap_link();

    FAI::prepare_to_save_FAI_object($this->dn,$this->attrs);

    if($this->initially_was_account){
      new log("modify","fai/".get_class($this),$this->dn,$this->attributes);
    }else{
      new log("create","fai/".get_class($this),$this->dn,$this->attributes);
    }
 
    /* Sort entries, because we must delete entries with status="delete" first */
    $order = array();
    foreach($this->disks as $key => $disk){
      if($disk['status'] == "delete"){
        $order[$key] = $disk;
      }
    }
    foreach($this->disks as $key => $disk){
      if($disk['status'] != "delete"){
        $order[$key] = $disk;
      }
    }

    /* Append all disks to ldap */
    foreach($order as $cn=>$disk){
      $disk_dn                    = "cn=".$disk['cn'].",".$this->dn;
      $disk_attrs['cn']           =  $disk['cn'];
      $disk_attrs['description']  =  $disk['description']; 

      if(empty($disk_attrs['description']) && $disk['status'] == "edited"){
        $disk_attrs['description'] = array();
      }

      $disk_attrs['objectClass']  =  array("top","FAIclass","FAIpartitionDisk");

      if($disk['status']=="new"){
        $ldap->cat($disk_dn,array("objectClass"));
        if($ldap->count()){
          $disk['status']="edited";
        }
      }

      /* Tag object */
      $ui= get_userinfo();
      $this->tag_attrs($disk_attrs, $disk_dn, $ui->gosaUnitTag);

      if($disk['status'] == "delete"){
        FAI::prepare_to_save_FAI_object($disk_dn,array(),true);
        $this->handle_post_events("remove");
      }elseif($disk['status'] == "edited"){
        FAI::prepare_to_save_FAI_object($disk_dn,$disk_attrs);
        $this->handle_post_events("modify");
      }elseif($disk['status']=="new"){
        FAI::prepare_to_save_FAI_object($disk_dn,$disk_attrs);
        $this->handle_post_events("add");
      }

      if($disk['status']!="delete")
      /* Add all partitions */
      foreach($disk['partitions'] as $key => $partition){
        $partition_attrs = array();

        foreach($partition as $key => $value){
          if(!empty($value)){
            $partition_attrs[$key]=$value;        
          }else{
            unset($partition_attrs[$key]);        
          }
        }

        $partition_dn= "FAIpartitionNr=".$partition_attrs['FAIpartitionNr'].",".$disk_dn;      
        $partition_attrs['objectClass']= array("top","FAIclass","FAIpartitionEntry");
        $partition_attrs['cn']= $partition_attrs['FAIpartitionNr'];
        
        unset($partition_attrs['status']);
        unset($partition_attrs['old_cn']);

        if($partition['status']=="new"){
          $ldap->cat($partition_dn,array("objectClass"));
          if($ldap->count()){
            $partition['status']="edited";
          }
        }

        if((!isset($partition['FAImountPoint']))||(empty($partition['FAImountPoint']))){
          $partition_attrs['FAImountPoint']="swap";
        }

        /* Tag object */
       $ui= get_userinfo();
        $this->tag_attrs($partition_attrs, $partition_dn, $ui->gosaUnitTag);

        if($partition['status'] == "delete"){
          FAI::prepare_to_save_FAI_object($partition_dn,array(),true);
          $this->handle_post_events("remove");
        }elseif($partition['status'] == "edited"){
          FAI::prepare_to_save_FAI_object($partition_dn,$partition_attrs);
          $this->handle_post_events("modify");
        }elseif($partition['status']=="new"){
          FAI::prepare_to_save_FAI_object($partition_dn,$partition_attrs);
          $this->handle_post_events("add");
        }
      }
    }
    $this->handle_post_events("add");
  }


  function PrepareForCopyPaste($source)
  {
    plugin::PrepareForCopyPaste($source);

    /* Read all leaf objects of this object (For FAIscript this would be FAIscriptEntry)
     */
    $res = FAI::get_leaf_objects($source['dn'], $source['cn'][0], "FAIpartitionDisk", get_ou('faiPartitionRDN'));
    foreach($res as $obj){
      $objects = array();
      $objects['description']  = "";
      $objects['status']      = "edited";
      $objects['dn']          = $obj['dn'];
      $objects                = $this->get_object_attributes($objects,$this->subAttributes);
      $this->disks[$objects['cn']] = $objects;
      $this->disks[$objects['cn']]['partitions'] = array();
    }

    /* read all partitions for each disk
     */
    foreach($this->disks as $name => $disk){
      $res = FAI::get_leaf_objects($disk['dn'], $name, "FAIpartitionEntry", "cn=".$source['cn'][0].",".get_ou('faiPartitionRDN'));
      foreach($res as $obj){
        $objects = array();
        $objects['status']      = "edited";
        $objects['dn']          = $obj['dn'];
        $objects                = $this->get_object_attributes($objects,$this->subPartAttributes);
        unset($objects['dn']);;
        $this->disks[$name]['partitions'][$objects['FAIpartitionNr']] = $objects;
      }
    }
    ksort($this->disks);
  }


  /* Return plugin informations for acl handling */ 
  static function plInfo()
  {
    return (array( 
          "plShortName" => _("Partition table"),
          "plDescription" => _("FAI partition table"),
          "plSelfModify"  => FALSE,
          "plDepends"     => array(),
          "plPriority"    => 26,
          "plSection"     => array("administration"),
          "plCategory"    => array("fai"),
          "plProvidedAcls" => array(
            "cn"                => _("Name")."&nbsp;("._("Read only").")",
            "description"       => _("Description"))
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

  /* Reload some attributes */
  function get_object_attributes($object,$attributes)
  {
    $ldap = $this->config->get_ldap_link();
    $ldap->cd($this->config->current['BASE']);
    $ldap->cat($object['dn'],$attributes);
    $tmp  = $ldap->fetch();

    foreach($attributes as $attrs){
      if(isset($tmp[$attrs][0])){
        $var = $tmp[$attrs][0];

        /* Check if we must decode some attributes */
        if(in_array_ics($attrs,$this->sub64coded)){
          $var = base64_decode($var);
        }

        /*  check if this is a binary entry */
        if(in_array_ics($attrs,$this->subBinary)){
          $var = $ldap->get_attribute($object['dn'], $attrs,$r_array=0);
        }

        /* Fix slashes */
        $var = addslashes($var);
        $object[$attrs] = $var;
      }
    }
    return($object);
  }

}

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
