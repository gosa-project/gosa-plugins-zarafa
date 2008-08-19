<?php

class opsi_generic extends plugin
{

  public function execute()
  {
    $smarty = get_smarty();
    return($smarty->fetch(get_template_path("generic.tpl",TRUE,dirname(__FILE__))));
  }
}


// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
