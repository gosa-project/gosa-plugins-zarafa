<?php


class krb5_policy extends plugin
{
  var $name   = "";
  var $data   = array();
  var $is_new = TRUE;
  var $init_name ="";
  var $parent ;
  var $config ;
  var $attributes = array("MASK","POLICY_REFCNT","PW_HISTORY_NUM","PW_MAX_LIFE",
      "PW_MIN_CLASSES","PW_MIN_LENGTH","PW_MIN_LIFE");

	public function __construct($config,$entry,$parent)
	{
    $this->config = $config;
    $this->parent = $parent;
    if(count($entry)){
      $this->is_new =FALSE;
      $this->name = $entry['NAME'];
      $this->data = $entry['DATA'];
    }
    $this->init_name = $this->name;
	}

  public function execute()
  {
    $display = plugin::execute();
    $smarty = get_smarty();    

    $smarty->assign("name",$this->name);
    foreach($this->attributes as $attr){
      $smarty->assign($attr,$this->data[$attr]);
    }
    return($smarty->fetch(get_template_path("krb5_policy.tpl",TRUE,dirname(__FILE__))));
  }


  public function save_object()
  {
    if(isset($_POST['Policy_Posted'])){
      if(isset($_POST['name'])){
        $this->name = get_post("name");
      }
      foreach($this->attributes as $attr){
        if(isset($_POST[$attr])){
          $this->data[$attr] = get_post($attr);
        }
      }
    }
  }
   
  
  public function check()
  {
    $messages = array();
    $names = $this->parent->getPolicyNames();
    if($this->name != $this->init_name && in_array($this->name,$names)){
      $messages[] = msgPool::duplicated(_("Name"));
    }
    return($messages);
  }
   

  public function save()
  {
    $ret = array();
    $ret['NAME']        = $this->name;
    $ret['DATA']        = $this->data;
    return($ret);
  }
}

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
