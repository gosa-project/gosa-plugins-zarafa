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
    }else{
      $this->data['MASK']           = 0;
      $this->data['POLICY_REFCNT']  = 0;
      $this->data['PW_HISTORY_NUM'] = 5;
      $this->data['PW_MAX_LIFE']    = 604800;
      $this->data['PW_MIN_CLASSES'] = 3;
      $this->data['PW_MIN_LENGTH']  = 6;
      $this->data['PW_MIN_LIFE']    = 36000;
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
    $message = array();
    $names = $this->parent->getPolicyNames();
    if($this->name != $this->init_name && in_array($this->name,$names)){
      $message[] = msgPool::duplicated(_("Name"));
    }
    if(empty($this->name)){
      $message[] = msgPool::required(_("Name"));
    }
  
    if(!preg_match("/^[a-z0-9\@\.\-_]*$/i",$this->name)){
      $message[] = msgPool::invalid(_("Name"),$this->name,"/[a-z0-9]/i");
    }

    /* Check password history */
    if(!is_numeric($this->data['PW_HISTORY_NUM'])){
      $message[] = msgPool::invalid(_("Password history"));
    }elseif($this->data['PW_HISTORY_NUM'] <= 0){
      $message[] = msgPool::toosmall(_("Password history"));
    }

    /* Check password minimum length */
    if(!is_numeric($this->data['PW_MIN_LENGTH'])){
      $message[] = msgPool::invalid(_("Password minimum length"));
    }elseif($this->data['PW_MIN_LENGTH'] <= 0){
      $message[] = msgPool::toosmall(_("Password minimum length"));
    }

    /* Check password different character classes */
    if(!is_numeric($this->data['PW_MIN_CLASSES'])){
      $message[] = msgPool::invalid(_("Password min characters"));
    }elseif($this->data['PW_MIN_CLASSES'] <= 0){
      $message[] = msgPool::toosmall(_("Password min characters"));
    }elseif($this->data['PW_MIN_CLASSES'] > $this->data['PW_MIN_LENGTH']){
      $message[] = sprintf(_("The value specified for '%s' must be smaller than the value specified for '%s'."),
        _("Password min characters"),_("Password minimum length"));
    }

    /* Check password min lifetime */
    if(!is_numeric($this->data['PW_MIN_LIFE'])){
      $message[] = msgPool::invalid(_("Password minimum lifetime"));
    }elseif($this->data['PW_MIN_LIFE'] <= 0){
      $message[] = msgPool::toosmall(_("Password minimum lifetime"));
    }

    /* Check password lifetime */
    if(!is_numeric($this->data['PW_MAX_LIFE'])){
      $message[] = msgPool::invalid(_("Password lifetime"));
    }elseif($this->data['PW_MAX_LIFE'] <= 0){
      $message[] = msgPool::toosmall(_("Password lifetime"));
    }elseif($this->data['PW_MAX_LIFE'] < $this->data['PW_MIN_LIFE']){
      $message[] = sprintf(_("The value specified for '%s' must be smaller than the value specified for '%s'."),
        _("Password minimum lifetime"),_("Password lifetime"));
    }

    return($message);
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
