<?php

class krb_host_keys extends plugin 
{

  public  $config;
  private $object;
  private $o_queue;
  private $host_key_realm = "disabled";
  private $host_key_realms = array();
  private $kerberos_support= FALSE;

  public function __construct($config,$object)
  {
    plugin::plugin($config, NULL);
    $this->object = $object;
    $this->o_queue = new gosaSupportDaemon();
    $this->host_key_realms['disabled'] = _("Disabled");

    /* Collect other realms here later */
    $this->kerberos_support = TRUE;
  }

  public function execute()
  {
    /* Skip if there is no kerberos support available 
     */
    if(!$this->kerberos_support) return("");
  
    $smarty = get_smarty(); 
    $smarty->assign("host_key_realms",$this->host_key_realms);
    $smarty->assign("host_key_realm",$this->host_key_realm);
    return($smarty->fetch (get_template_path('krb_host_keys.tpl', TRUE,dirname(__FILE__))));
  }


  private function generate_key()
  {
    echo "NIY";
  }


  public function save_object()
  {
    /* Skip if there is no kerberos support available 
     */
    if(!$this->kerberos_support) return;
    
    if(isset($_POST['host_key_realm']) && isset($this->host_key_realms[$_POST['host_key_realm']])){
      $this->host_key_realm = get_post('host_key_realm');
    }
    if(isset($_POST['host_key_generate'])){
      $this->generate_key();
    }
  } 
}











// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
