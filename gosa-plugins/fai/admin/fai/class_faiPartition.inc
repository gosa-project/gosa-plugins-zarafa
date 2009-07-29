<?php

class faiPartition extends plugin
{
  var $attributes = array("cn","description","FAIfsCreateOptions","FAIfsOptions","FAIfsTuneOptions",
      "FAIfsType","FAImountOptions","FAImountPoint","FAIpartitionNr","FAIpartitionFlags","FAIdiskType",
      "FAIpartitionSize","FAIpartitionType","FAIstate", "bootable","resize", "preserve", "preserveType",
      "encrypted");

  var $cn  = "";
  var $description  = "";
  var $FAIfsCreateOptions = "";
  var $FAIfsOptions = "";
  var $FAIfsTuneOptions = "";
  var $FAIfsType = "";
  var $FAImountOptions = "";
  var $FAImountPoint = "";
  var $FAIpartitionNr = "";
  var $FAIpartitionSize = "";
  var $FAIpartitionType = "";
  var $FAIstate = "";
  var $FAIpartitionFlags = "";
  
  var $FAIdiskType = "disk";

  var $FAIpartitionSizeType = "fixed";
  var $sizeStart = 0;
  var $sizeStop  = 0;
  var $sizeStart_Type = "MB";
  var $sizeStop_Type  = "MB";
   
  var $bootable = false; 
  var $resize = false;
  var $preserve = false;
  var $preserveType = "always";
  var $encrypted = false;

  function __construct($config, $object, $type)
  {
    foreach($this->attributes as $attr){
      if(isset($object[$attr])){
        $this->$attr = $object[$attr];
      }
    }

    $this->FAIdiskType = $type;

    /* Prepare size attribute 
     * This attribute may represent a range, a fixed value 
     *  or a percentage.
     * fixed is just a number   * 500MB
     * range                    * 500MB-1TB
     * remaining                * -
     */
    
    // Fixed
    if(preg_match("/^[0-9]*(KB|MB|GB|TB|PB)$/",$this->FAIpartitionSize)){
      $this->sizeStart = preg_replace("/^([0-9]*)(KB|MB|GB|TB|PB)$/","\\1",$this->FAIpartitionSize);
      $this->sizeStart_Type = preg_replace("/^([0-9]*)(KB|MB|GB|TB|PB)$/","\\2",$this->FAIpartitionSize);
      $this->FAIpartitionSizeType = "fixed";
    }else
    
    // Dynamic range
    if(preg_match("/^[0-9]*(KB|MB|GB|TB|PB)-[0-9]*(KB|MB|GB|TB|PB)$/",$this->FAIpartitionSize)){
      $this->sizeStart = preg_replace("/^([0-9]*)(KB|MB|GB|TB|PB).*$/","\\1",$this->FAIpartitionSize);
      $this->sizeStart_Type = preg_replace("/^([0-9]*)(KB|MB|GB|TB|PB).*$/","\\2",$this->FAIpartitionSize);
      $this->sizeStop = preg_replace("/^[^\-]*\-([0-9]*)(KB|MB|GB|TB|PB)$/","\\1",$this->FAIpartitionSize);
      $this->sizeStop_Type = preg_replace("/^[^\-]*\-([0-9]*)(KB|MB|GB|TB|PB)$/","\\2",$this->FAIpartitionSize);
      $this->FAIpartitionSizeType = "dynamic";
    }else

    // Dynamic range
    if(preg_match("/^\-$/",$this->FAIpartitionSize)){
      $this->FAIpartitionSizeType = "remaining";
    }
  }

  function execute()
  {
    plugin::execute();
    $smarty = get_smarty();

    // Create a list of selectable partition types
    if($this->FAIdiskType == "disk"){
      $types  = array(
          "primary" => _("Primary"),
          "secondary" => _("Secondary"));
    }elseif($this->FAIdiskType == "raid"){
      $types  = array(
          "raid0" => _("Raid 0"),
          "raid1" => _("Raid 1"),
          "raid5" => _("Raid 5"),
          "raid6" => _("Raid 6"));
    }else{
      $types = "";
    }

     
    // Create a list of all size options
    $partitionSizeTypes  = array(
        "fixed"     => _("Fixed"),
        "dynamic"     => _("Dynamic"),
        "remaining" => _("Use remaining space")
        );

    // Create a list of all size options
    $sizeTypes  = array(
        "KB"      => _("KB"),
        "MB"      => _("MB"),
        "GB"      => _("GB"),
        "TB"      => _("TB"),
        "PB"      => _("PB"),
        "%"      => _("Percent")
        );

    // Preserve types 
    $preserveTypes = array(
        "always" => _("Always"),
        "reinstall" => _("Reinstall"));
  
    $FAIfsTypes = array(
        "swap" => _("Swap"),
        "ext2" => _("Ext 2"),
        "ext3" => _("Ext 3"),
        "ext4" => _("Ext 4"),
        "reiserfs" => _("Reiser fs"),
        "xfs" => _("Xfs"),
        "btrfs" => _("Btrfs"));
 
    $smarty->assign("partitionTypes", $types);
    $smarty->assign("partitionSizeTypes", $partitionSizeTypes);
    $smarty->assign("FAIpartitionSizeType", $this->FAIpartitionSizeType);
    $smarty->assign("sizeTypes", $sizeTypes);

    $smarty->assign("sizeStart_Type", $this->sizeStart_Type);
    $smarty->assign("sizeStop_Type", $this->sizeStop_Type);
    $smarty->assign("sizeStart", $this->sizeStart);
    $smarty->assign("sizeStop", $this->sizeStop);

    $smarty->assign("preserveTypes", $preserveTypes);
    $smarty->assign("preserveType", $this->preserveType);

    $smarty->assign("FAIfsTypes", $FAIfsTypes);

    foreach($this->attributes as $attr){
      $smarty->assign($attr,$this->$attr);
    }
    return($smarty->fetch(get_template_path("faiPartition.tpl", TRUE, dirname(__FILE__))));
  }

  function save_object()
  {
    if(isset($_POST['faiPartition'])){
      foreach($this->attributes as $attr){
        if(isset($_POST[$attr])){
          $this->$attr = get_post($attr);
        }
      }
      foreach(array("FAIpartitionSizeType","sizeStart","sizeStop","sizeStart_Type","sizeStop_Type") as $attr){
        if(isset($_POST[$attr])){
          $this->$attr = get_post($attr);
        }
      }
      foreach(array("bootable","preserve","resize","encrypted") as $attr){
        if(isset($_POST[$attr])){
          $this->$attr = TRUE;
        }else{
          $this->$attr = FALSE;
        }
      }
    } 
  }

  function save()
  {
    $ret = array();
    foreach($this->attributes as $attr){
      $ret[$attr] = $this->$attr;
    }

    // Save partition size 
    switch($this->FAIpartitionSizeType){
      case 'fixed' : 
        $ret['FAIpartitionSize'] = $this->sizeStart.$this->sizeStart_Type;break; 
      case 'dynamic' : 
        $ret['FAIpartitionSize'] = $this->sizeStart.$this->sizeStart_Type."-".  
          $this->sizeStop.$this->sizeStop_Type;break; 
      case 'remaining' : 
        $ret['FAIpartitionSize'] = "-";break; 
      default: trigger_error("Unknown partition size!");
    }

    return($ret);
  }
}
// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
