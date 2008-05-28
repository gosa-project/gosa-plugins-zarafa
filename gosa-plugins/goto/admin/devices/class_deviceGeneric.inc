<?php

class deviceGeneric extends plugin
{
  public $dn            = "";
  public $cn            = "";
  public $orig_cn       = "";
  public $orig_base     = "";
  public $orig_dn       = "";
  public $description   = "";
  public $vendor        = "";
  public $dev_id        = "";
  public $serial        = "";
  public $base          = "";	
  public $types;
  public $type;

  public $posts           = array("description","dev_id","serial","vendor", "type");
  public $attributes      = array("cn");
  public $objectclasses   = array("top","gotoDevice");

  public $CopyPasteVars   = array("orig_cn","description","vendor","dev_id","serial","base");

  public function deviceGeneric(&$config,$dn = NULL)
  {
    plugin::plugin($config,$dn);

    $this->is_account = TRUE;

    $this->types= array("camera" => _("Digital camera"),
                        "disc" => _("USB stick"),
                        "cdrom" => _("CD/DVD drive"),);
    asort($this->types);

    /* Set class values */
    if(isset($this->attrs['gotoHotplugDevice'][0])){
      $tmp = preg_split("/\|/",$this->attrs['gotoHotplugDevice'][0]);
      $this->cn         = $this->attrs['cn'][0];
      $this->description= $tmp[0];
      $this->dev_id     = $tmp[1];
      $this->serial     = $tmp[2];
      $this->vendor     = $tmp[3];
      if (isset($tmp[4])){
        $this->type= $tmp[4];
      }
    }     

    $this->orig_cn = $this->cn;

    /* Set Base */
    if ($this->dn == "new"){
      if(session::is_set('CurrentMainBase')){
        $this->base = session::get('CurrentMainBase');
      }else{
        $ui= get_userinfo();
        $this->base= dn2base($ui->dn);
      }
    } else {
      $this->base =preg_replace ("/^[^,]+,".get_ou('deviceou')."/","",$this->dn);
    }
    $this->orig_base = $this->base;
    $this->orig_dn   = $this->dn;
  }


  public function execute()
  {
    $smarty = get_smarty();
    $smarty->assign("base",$this->base);
    $smarty->assign("bases",$this->get_allowed_bases());
    foreach($this->attributes as $attr){
      $smarty->assign($attr,$this->$attr);
    }
    foreach($this->posts as $attr){
      $smarty->assign($attr,$this->$attr);
    }

    $smarty->assign("type",$this->type);
    $smarty->assign ("types", $this->types);

    return($smarty->fetch(get_template_path("deviceGeneric.tpl",TRUE,dirname(__FILE__))));
  }

  
  public function check()
  {
    $message = plugin::check();

    if(empty($this->cn)||(preg_match("/[^a-z0-9]/i",$this->cn))){
      $message[]= msgPool::invalid(_("Name"),$this->cn,"/^[a-z0-9]*$/i");
    }
    if(preg_match("/[^a-z0-9!\"?.,;:-_\(\) ]/i",$this->description)){
      $message[]= msgPool::invalid(_("Description"),$this->cn,"/^[a-z0-9!\"?.,;:-_\(\) ]*$/i");
    }

    /* Skip serial check if vendor and product id are given */
    if(preg_match("/^\s+$/i",$this->dev_id)){
      $message[]= msgPool::invalid(_("iSerial"),"","01234");
    }
    if(empty($this->serial) || !$this->is_2byteHex($this->serial)){
      $message[]= msgPool::invalid(_("Serial"),"","","0x1234");
    }
    if(empty($this->vendor) || !$this->is_2byteHex($this->vendor)){
      $message[]= msgPool::invalid(_("Vendor ID"),"","","0x1234");
    }
  
    /* Check if entry already exists */ 
    if($this->cn != $this->orig_cn || $this->dn == "new"){
      $ldap = $this->config->get_ldap_link();
      $ldap->search("(&(objectClass=gotoDevice)(cn=".$this->cn."*))",array("cn"));
      if($ldap->count()){
        $message[]= msgPool::duplicated(_("Name"));
      }
    }

    /* Check if we are allowed to create or move this object
     */
    if($this->orig_dn == "new" && !$this->acl_is_createable($this->base)){
      $message[] = msgPool::permCreate();
    }elseif($this->orig_dn != "new" && $this->base != $this->orig_base && !$this->acl_is_moveable($this->base)){
      $message[] = msgPool::permMove();
    }

    return($message);
  }


  public function save_object()
  {
    if(isset($_POST['deviceGeneric_posted'])){
      plugin::save_object();
  
      if(isset($_POST['base'])){
        $tmp = $this->get_allowed_bases();
        if(isset($tmp[get_post("base")])){
          $this->base = get_post("base");
        }
      }
    
      foreach($this->posts as $post){
        if(isset($_POST[$post])){
          $this->$post = get_post($post);
        }
      }
    }
  }

  
  public function remove_from_parent()
  {
    plugin::remove_from_parent();
    $ldap = $this->config->get_ldap_link();
    $ldap->cd($this->config->current['BASE']);

    $ldap->search("(&(objectClass=gotoEnvironment)(gotoHotplugDeviceDN=".$this->dn."))",array("cn","gotoHotplugDeviceDN"));
    $skip = FALSE;
    $obj = array();
    while($attrs = $ldap->fetch()){
      $obj[$ldap->getDN()]= $attrs['cn'][0];
      $skip =TRUE;
    }
    if($skip){
      msg_dialog::display(_("Error"), msgPool::stillInUse(_("Device"), $obj), INFO_DIALOG);
    }else{
      $ldap->rmdir_recursive($this->dn);
    }
  }


  public function save()
  {
    plugin::save();

    $this->attrs['gotoHotplugDevice'] = "";
    foreach($this->posts as $post){
      $this->attrs['gotoHotplugDevice'] .= $this->$post."|"; 
    }
    $this->attrs['gotoHotplugDevice'] = preg_replace("/\|$/","",$this->attrs['gotoHotplugDevice']);

    $ldap = $this->config->get_ldap_link();
    $ldap->cd($this->config->current['BASE']);
    $ldap->cat($this->dn);
    if($ldap->count()){
      $ldap->cd($this->dn);
      $ldap->modify($this->attrs);
    }else{
      $ldap->create_missing_trees(preg_replace("/^[^,]+,/","",$this->dn));
      $ldap->cd($this->dn);
      $ldap->add($this->attrs);
    }
    if (!$ldap->success()){
      msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $this->dn, 0, get_class()));
    }
  }


  /* check if given str in like this 0xffff*/
  function is_2byteHex($str)
  {
    return !strlen($str) || preg_match("/^(0x|x|)[a-f0-9][a-f0-9][a-f0-9][a-f0-9]/i",$str);
  }


  function PrepareForCopyPaste($source)
  {
    plugin::PrepareForCopyPaste($source);
    $source_o = new deviceGeneric($this->config,$source['dn']);
    foreach($this->CopyPasteVars as $post){
      $this->$post = $source_o->$post;
    }
  }


  /* Return a dialog with all fields that must be changed,
     if we want to copy this entry */
  function getCopyDialog()
  {
    $str = "";
    $smarty = get_smarty();
    $smarty->assign("cn",         $this->cn);
    $str = $smarty->fetch(get_template_path("paste_deviceGeneric.tpl",TRUE,dirname(__FILE__)));

    $ret = array();
    $ret['string'] = $str;
    $ret['status'] = "";
    return($ret);
  }


  /* Save all */
  function saveCopyDialog()
  {
    $attrs = array("cn");
    foreach($attrs as $attr){
      if(isset($_POST[$attr])){
        $this->$attr = $_POST[$attr];
      }
    }
  }



  /* Return plugin informations for acl handling  */
  public static function plInfo()
  {
    return (array(
          "plShortName"   => _("Generic"),
          "plDescription" => _("Device generic"),
          "plSelfModify"  => FALSE,
          "plDepends"     => array(),
          "plPriority"    => 0,
          "plSection"     => array("administration"),
          "plCategory"    => array("devices" => array("description"  => _("Devices"),
                                                        "objectClass"  => "gotoHotplugDevice")),
          "plProvidedAcls"=> array(
            "cn"                          => _("Name"))
          ));

  }
	
}
// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
