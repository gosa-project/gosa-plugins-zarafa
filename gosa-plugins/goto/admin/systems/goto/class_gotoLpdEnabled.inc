<?php

class gotoLpdEnable extends plugin
{
	public $parent;

  protected $device   = "";
  protected $port     = "";
  protected $options  = "";
  protected $writeonly= FALSE;

  public    $attributes = array("device","port","options","writeonly");

	public function __construct($config,$data,&$parent)
	{
		plugin::plugin($config,NULL);
    
		$this->parent = $parent;
	}

	public function execute()
	{
		$smarty = get_smarty();

    $smarty->assign("devices",array(
          "USB"       => _("USB"),
          "PARALLEL"  => _("Parallel"),
          "SERIAL"    => _("Serial")));

    foreach($this->attributes as $attr){
      $smarty->assign($attr,$this->$attr);
    }
    foreach(array("devices","ports") as $attr){
      $smarty->assign($attr,array());
    }

		return($smarty->fetch(get_template_path("gotoLpdEnable.tpl",TRUE,dirname(__FILE__))));
	}

  public function save_object()
  {
    plugin::save_object();
  }


  /* Allow every action, check are made in class service not here.
   */
  public function acl_is_writeable($attr,$skip_write = FALSE)
  { 
    return(TRUE);
  }
}



// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
