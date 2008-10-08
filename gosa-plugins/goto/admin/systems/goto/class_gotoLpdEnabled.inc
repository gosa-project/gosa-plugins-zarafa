<?php

class gotoLpdEnable extends plugin
{
	public $parent;

  protected $DevID = 0;
  private $data = array();
  private $default_entry = array();

  protected $s_Type = "USB";
  protected $s_Device = "/dev/usb/lp";
  protected $i_Port = 9100;   
  protected $s_Options = "";
  protected $s_WriteOnly = "Y";

  protected $s_Speed = 9600;
  protected $s_FlowControl = "S";
  protected $s_Parity = "E";
  protected $i_Bit = 5;

  /* Option predefinition */
  private $a_Types = array();
  private $a_Boolean = array();
  private $a_FlowControls = array(); //  
  private $a_Parities = array();    //  
  private $a_Speeds = array();      // 
  private $a_Bits = array();        //
  private $a_Devices = array();

  public    $attributes = array("s_Type","s_Device","i_Port","s_Options","s_WriteOnly",
      "s_Speed","s_FlowControl","s_Parity","i_Bit");

	public function __construct($config,$data,&$parent)
	{
		plugin::plugin($config,NULL);
		$this->parent = $parent;
    $this->DevID = 0;

    /****
      Predefine values 
     ****/ 
    $this->a_Types = array(
        "USB" => _("USB"),
        "Parallel" => _("Parallel port"),
        "Serial" => _("Serial"));
    $this->a_Boolean = array(
        "Y" => _("Yes"),
        "N" => _("No"));
    $this->a_FlowControl = array(
        "S" => _("Software"),
        "H" => _("Hardware"));
    $this->a_Parities = array(
        "E"  => _("Even"),
        "O"  => _("Odd"),
        "N"  => _("None"));
    $this->a_Speeds = array(
        "4800"  => "4.800&nbsp;"._("bit/s"),
        "9600"  => "9.600&nbsp;"._("bit/s"),
        "19200" => "19.200&nbsp;"._("bit/s"),
        "38400" => "38.400&nbsp;"._("bit/s"),
        "57600" => "57.600&nbsp;"._("bit/s"),
        "115200"=> "115.200&nbsp;"._("bit/s"),
        "230400"=> "230.400&nbsp;"._("bit/s"),
        "460800"=> "460.800&nbsp;"._("bit/s"));
    $this->a_Bits = array(
        5,6,7,8);
    $this->a_Devices = array(
        "USB" => "/dev/usb/lp",
        "Parallel" => "/dev/lp",
        "Serial" => "/dev/ttyS");

    /* Create default entry 
     */
    foreach($this->attributes as $attr){
      $this->default_entry[$attr] = $this->$attr;
    }

    /* Load current settings 
     */
    if(count($data) == 0){
      $this->is_account = FALSE;
    }else{
      $this->is_account = TRUE;
      foreach($data as $dat){
        list($s_Type,$s_Device,$i_Port,$s_Speed,$s_FlowControl,$s_Parity,$i_Bit,$s_WriteOnly,$s_Options) = split("\:",$dat);
        $entry = array();
        foreach($this->attributes as $attr){
          $entry[$attr] = $$attr;
        }
        $this->data[] = $entry;
      }

      /* Set first entry values 
       */
      foreach($this->attributes as $attr){
        $this->$attr = $this->data[$this->DevID][$attr];
      }
    }
    $this->initially_was_account = $this->is_account;
  }

	public function execute()
	{
    $display = "";
    if(isset($_POST['modify_state'])){
      $this->is_account = !$this->is_account;
      if(!$this->is_account){
        $this->data = array();
      }else{
        $entry = $this->default_entry;
        $entry['i_Port'] = $entry['i_Port'] + count($this->data);
        $entry['s_Device'] = $entry['s_Device'].count($this->data);
        $this->data = array(0 => $entry);
        $this->DevID = 0; 
        foreach($this->attributes as $attr){
          $this->$attr = $this->data[$this->DevID][$attr];
        }
      }
    }

    if($this->is_account){
        $display .= $this->show_disable_header(msgPool::removeFeaturesButton(_("Printer")),"",FALSE);
    }else{
        $display .= $this->show_enable_header(msgPool::addFeaturesButton(_("Printer")),"",FALSE);
    }
		$smarty = get_smarty();
    foreach($this->attributes as $attr){
      $smarty->assign($attr,$this->$attr);
    }
    foreach(array("DevID","a_Types","a_Boolean","a_FlowControl","a_Parities","a_Speeds","a_Bits") as $attr){
      $smarty->assign($attr,$this->$attr);
    }
    ksort($this->data);
    $smarty->assign("data_cnt",count($this->data));
    $smarty->assign("data",$this->data);
    $smarty->assign("a_DevIDs",array_keys($this->data));
    $smarty->assign("s_WriteOnly", $this->s_WriteOnly == "Y");
    $smarty->assign("is_account",$this->is_account);
		return($display.$smarty->fetch(get_template_path("gotoLpdEnable.tpl",TRUE,dirname(__FILE__))));
	}


  public function save_object()
  {
    if(!$this->is_account) return;
    $propose_device = "";
    if(isset($_POST['s_Device']) && empty($_POST['s_Device']) && 
        isset($_POST['s_Type']) && $_POST['s_Type'] != $this->s_Type){
      $propose_device = $this->a_Devices[$_POST['s_Type']].$this->DevID;
    }

    plugin::save_object();

    if(isset($_POST['gotoLpdEnable_entry_posted']) && isset($_POST['s_WriteOnly'])){
      $this->s_WriteOnly = "Y";
    }else{
      $this->s_WriteOnly = "N";
    }

    if(!empty($propose_device)){
      $this->s_Device = $propose_device;
    }

    foreach($this->attributes as $attr){
      $this->data[$this->DevID][$attr] = $this->$attr;
    } 

    /* Device ID has changed */
    if(isset($_POST['DevID']) && $_POST['DevID'] != $this->DevID){
      $this->DevID = $_POST['DevID'];
      foreach($this->attributes as $attr){
        $this->$attr = $this->data[$this->DevID][$attr];
      }
    }

    if(isset($_POST['del_printer'])){
      unset($this->data[$this->DevID]);
      $this->data = array_values($this->data);
      if(count($this->data) == 0){
        $this->is_account = FALSE;
      }
      $this->DevID = key($this->data);
    }
    if(isset($_POST['add_printer'])){
      if(count($this->data) < 3){
        $entry = $this->default_entry;
        $entry['i_Port'] = $entry['i_Port'] + count($this->data);
        $entry['s_Device'] = $entry['s_Device'].count($this->data);
        $this->DevID = count($this->data);
        $this->data[] = $entry; 
        foreach($this->attributes as $attr){
          $this->$attr = $this->data[$this->DevID][$attr];
        }
      }
    }
  }

  public function save()
  {
    $ret = array();
    $attrs = array("s_Type","s_Device","i_Port","s_Speed","s_FlowControl","s_Parity","i_Bit","s_WriteOnly","s_Options"); 
    foreach($this->data as $entry){
      $str = "";
      foreach($attrs as $attr){
        $str .= $entry[$attr].":";
      }
      $ret[] = preg_replace("/:$/","",$str);
    }
    return($ret);
  }


  /* Allow every action, check are made in class service not here.
   */
  public function acl_is_writeable($attr,$skip_write = FALSE)
  { 
    return(TRUE);
  }
  public function acl_is_removeable($skip_write = FALSE)
  { 
    return(TRUE);
  }
  public function acl_is_createable($skip_write = FALSE)
  { 
    return(TRUE);
  }
}

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
