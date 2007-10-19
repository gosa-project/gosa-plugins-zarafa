<?php

class multi_plug
{
  /* Tab handler for each given dn entry */
	public $a_handles = array();

  /* Dummy handler which collects the data */
  private $o_tab     = NULL;  

  public $dn    = array();
  public $config = NULL;
  private $s_class  = "";
  public $current = "";

  public function __construct($config,$class,$tab,$dns)
  {
    $this->dn = $dns;
    $this->config = $config;
    $this->s_class = $class;
    $this->o_tab = new $class($config,$tab,"new");

    if($this->o_tab->multiple_support_available()){
      $this->o_tab->enable_multiple_support();
    
      foreach($dns as $dn){
        $this->a_handles[] = new $class($config,$tab,$dn);
      }
    }
  }

  public function execute()
  {
    return($this->o_tab->execute());
  }

  public function entries_locked()
  {
    return(FALSE);
  }

  public function display_lock_message()
  {
    return("sdf");
  }

  public function lock_entries($uid)
  {
    return(TRUE);
  }

  public function multiple_available()
  {
    return($this->o_tab->multiple_support_available());
  }

  public function set_active_tab($str)
  {
    $this->current = $str;
  }

  public function get_object_info()
  {
    return("Oi -----");
  }

  public function save_object()
  {
    $this->o_tab->save_object(); 
  }

  public function check()
  {
    $this->populate_values();
    $messages = $this->o_tab->check();
    foreach($this->a_handles as $key => $obj){
      $msgs = $obj->check();
      foreach($msgs as $msg){
        $messages[] = $msg;
      }
    }
    return($messages);
  }

  public function password_change_needed()
  {
    return(FALSE);
  }

  public function populate_values()
  {

    if($this->multiple_available() && is_array($this->a_handles)){
      foreach($this->o_tab->by_object as $name => $obj){
        $values = $this->o_tab->by_object[$name]->get_multi_edit_values();
        foreach($values as $a_name => $a_value){
          foreach($this->a_handles as $i_id => $o_handle){
            $o_handle->by_object[$name]->$a_name = $a_value;
          }
        }
      }
    }
  }

  public function save()
  {
    if($this->multiple_available() && is_array($this->a_handles)){
      $this->populate_values();
      foreach($this->a_handles as $i_id => $o_handle){
        $o_handle->save();
      }
    }
  }
}

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
