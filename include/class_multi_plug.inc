<?php

class multi_plug
{
  /* Tab handler for each given dn entry */
	public $a_handles = array();

  /* Dummy handler which collects the data */
  private $o_tab     = NULL;  

  private $a_dns    = array();
  private $o_config = NULL;
  private $s_class  = "";

  public function __construct($config,$class,$tab,$dns)
  {
    $this->a_dns = $dns;
    $this->o_config = $config;
    $this->s_class = $class;
    $this->o_tab = new $class($config,$tab,"new");

    if($this->o_tab->multiple_support_available()){
      $this->o_tab->enable_multiple_support();
    
      foreach($dns as $dn){
        $this->a_handles[] = new $class($config,$tab,$dn);
      }
    }
  }

  public function entries_locked()
  {
    return(FALSE);
  }

  public function display_lock_message()
  {
    return("");
  }

  public function lock_entries($uid)
  {
    return(TRUE);
  }

  public function multiple_available()
  {
    return(TRUE);
  }

  public function set_active_tab($str)
  {
  }

  public function get_object_info()
  {
    return("Oi");
  }
}

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
