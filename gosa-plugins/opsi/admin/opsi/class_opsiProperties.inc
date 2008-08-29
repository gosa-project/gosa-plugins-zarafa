<?php

class opsiProductConfig extends plugin
{
  private $product;
  private $cfg;
  private $hostId;

  public function __construct($config,$product,$cfg = array() ,$hostId = "")
  {
    $this->config  = $config;
    $this->product = $product;
    $this->cfg     = $cfg;
    $this->hostId  = $hostId;
  }

  public function execute()
  {
    $smarty = get_smarty();
    $smarty->assign("cfg",$this->cfg);
    return($smarty->fetch(get_template_path("properties.tpl",TRUE,dirname(__FILE__))));
  }

  public function save_object()
  {
    foreach($this->cfg as $name => $value){
      if(isset($_POST['value_'.$name])){
        $this->cfg[$name] = validate(get_post('value_'.$name));
      }
    }
  }

  public function get_cfg() 
  {
    return($this->cfg);
  }

  public function get_product()
  {
    return($this->product);
  }

  public function get_hostId()
  {
    return($this->hostId);
  }
}





// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
