<?php

class opsiware extends plugin
{
  private $opsi;
  private $SW_info;
  private $HW_info;
  private $hostId;

  public function __construct($config,$hostId)
  {
    $this->config = $config;
    $this->opsi   = new opsi($config);
    $this->hostId = preg_replace("/^opsi:=([^,]*),.*$/","\\1",$hostId);
  }

  public function execute()
  {
    print_a($this->opsi->get_client_hardware($this->hostId));
      
  }

}
// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
