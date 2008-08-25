<?php

class opsiware extends plugin
{
  private $opsi;
  private $SW_info;
  private $HW_info;
  private $hostId;
  private $type = 0;
  private $init_failed = FALSE;

  public function __construct($config,$hostId)
  {
    $this->config = $config;
    $this->opsi   = new opsi($config);
    $this->hostId = preg_replace("/^opsi:=([^,]*),.*$/","\\1",$hostId);
    $this->init();
  }

  private function init()
  {
    $err = FALSE;
    if(!$err){
      $this->HW_info = $this->opsi->get_client_hardware($this->hostId);
      $err |= $this->opsi->is_error(); 
    }
    if(!$err){
      $this->SW_info = $this->opsi->get_client_software($this->hostId);
      $err |= $this->opsi->is_error(); 
    }
    $this->init_failed = $err;
  }

  public function execute()
  {
    if(isset($_POST['reinit']) && $this->init_failed){
      $this->init();
    }
    if(isset($_GET['type'])) $this->type = $_GET['type'];

    $smarty = get_smarty();
    $smarty->assign("init_failed",$this->init_failed);
    if($this->init_failed){
      $smarty->assign("type", $this->type);
      $smarty->assign("message",$this->opsi->get_error());
      return($smarty->fetch(get_template_path("opsiware.tpl",TRUE,dirname(__FILE__))));
    }  

    $smarty->assign("type", $this->type);

    if($this->type == 1){
      $smarty->assign("info", $this->HW_info);
    }else{
      $smarty->assign("info", $this->SW_info);
    }
    return($smarty->fetch(get_template_path("opsiware.tpl",TRUE,dirname(__FILE__))));
  }
}
// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
