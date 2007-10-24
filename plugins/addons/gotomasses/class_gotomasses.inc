<?php

class gotomasses extends plugin
{
  /* Definitions */
  var $plHeadline     = "System deployment";
  var $plDescription  = "This does something";

  /* attribute list for save action */
  var $attributes= array();
  var $objectclasses= array();

  /* Queue tasks */
  var $current =false;
  var $dialog = FALSE;
  var $ids_to_remove = array();
  var $divlist = NULL;

  function gotomasses(&$config, $dn= NULL)
  {
    /* Include config object */
    $this->config= &$config;
    $this->divlist = new divListMasses($this->config,$this);
    $this->o_queue = new hostActionQueue(&$config);
  }


  function execute()
  {
    $smarty = get_smarty();
    /************
     * Handle posts 
     ************/

    $s_entry = $s_action = "";
    $arr = array( "/^edit_task_/"=>"edit","/^remove_task_/"=>"remove",
                  "/^new_task_/"=>"new_task","/^remove_multiple_task_/" => "remove_multiple");
    foreach($arr as $regex => $action){
      foreach($_POST as $name => $value){
        if(preg_match($regex,$name)){
          $s_action = $action;
          $s_entry  = preg_replace($regex,"",$name);
          $s_entry  = preg_replace("/_(x|y)$/","",$s_entry);
        }
      }
    }

    /* Edit posted from list link */
    if(isset($_GET['act']) && $_GET['act'] == "edit" && isset($_GET['id']) && isset($this->tasks[$_GET['id']])){
      $s_action = "edit";
      $s_entry = $_GET['id'];
    }

    /************
     * REMOVE 
     ************/

    /* Remove multiple */
    if($s_action == "remove_multiple"){
      if(!$this->acl_is_removeable()){
        print_red(_("You are not allowed to remove a task."));
      }else{
        $this->ids_to_remove = $this->list_get_selected_items();
        $tmp = "";
        foreach($this->ids_to_remove as $key => $id){
          if(isset($this->tasks[$id])){
            $task = $this->tasks[$id];
            $tmp.= "\n".$this->target_to_string($task);
          }else{
            unset($this->ids_to_remove[$key]);
          }
        }
        $smarty->assign("multiple", TRUE); 
        $smarty->assign("info",sprintf(_("Your are about to delete the following tasks: %s"),"<pre>".$tmp."</pre>"));
        $this->current = $s_entry;
        return($smarty->fetch(get_template_path('remove.tpl', TRUE)));
      }
    }

    /* Remove specified tasks */
    if(count($this->ids_to_remove) && isset($_POST['delete_multiple_confirm'])){
      foreach($this->ids_to_remove as $id){
        $this->o_queue->remove($id);    
      }
      $this->save();
    }

    /* Remove aborted */
    if(isset($_POST['delete_cancel'])){
      $this->ids_to_remove = array();;
    }


    /************
     * EDIT 
     ************/

    /* Edit selected entry */
    if($s_action == "edit" && isset($this->tasks[$s_entry])){
      $entry = $this->tasks[$s_entry];
      $this->dialog = new goto_task($this->config,$this,$entry);
      $this->current = $s_entry;
    }

    /* New entry */
    if($s_action== "new_task" && $this->acl_is_createable()){
      $this->dialog = new goto_task($this->config,$this);
      $this->current = -1;
    }
  
    /* Close dialog */
    if(isset($_POST['close_goto_task'])){
      $this->dialog = FALSE;
      $this->current = -1;
    }

    /* Close dialog */
    if((isset($_POST['save_goto_task']) || isset($_POST['apply_goto_task'])) && is_object($this->dialog) ){
      $this->dialog->save_object();
      $msgs = $this->dialog->check();
      if(count($msgs)){
        foreach($msgs as $msg){
          print_red($msg);  
        }
      }else{
        if(isset($this->tasks[$this->current]) && $this->current != -1){
          $this->tasks[$this->current] = $this->dialog->save();
        }else{
          $tmp = $this->dialog->save();

          $targets =$tmp['Target'];
          foreach($targets as $target){
            $tmp['Target'] = array($target);
            if(!$this->o_queue->add($tmp)){
              print_red($this->o_queue->get_error());
            }
          }
        }
        if(!isset($_POST['apply_goto_task'])){
          $this->dialog = FALSE;
          $this->current = -1;
        }
        $this->save();
      }
    }

    /* Display dialogs if currently opened */
    if(is_object($this->dialog)){
      $this->dialog->save_object();
      return($this->dialog->execute());
    }


    /************
     * Handle Divlist 
     ************/

    $this->divlist->execute();
    $this->divlist->SetEntries($this->get_queue_entries()); 
    return($this->divlist->Draw());
  }

    
  function get_queue_entries()
  {
   if(!$this->o_queue->load()){
     print_red("ERROR:".$this->o_queue->get_error());
     return(FALSE);
   }
    $ret = array();
    while($entry = $this->o_queue->fetch()){
      $ret[] = $entry;
    }
    return($ret);
  }
 









 
  function target_to_string($data)
  {
    $ret = "";
    if($data['Action'] == "initial_install"){
      foreach($data['Initial_Target'] as $target){
        $ret .= $target['MAC'].", ";
      } 
    }else{
      foreach($data['Target'] as $target){
        $ret .= $target.", ";
      } 
    }
    return(preg_replace("/, $/","",$ret));
  }

  
  function time_to_string($data)
  {
    return($data['Minute']." ".$data['Hour']." ".$data['Day']." ".$data['Month']." ".$data['Weekday']);
  }

  
  function action_to_string($data)
  {
    $tmp = $this->get_actions();
    if(isset($tmp[$data['Action']])){
      return($tmp[$data['Action']]);
    }else{
      return(_("Unknown"));
    }
  }

  
  function save_object()
  {
    $this->divlist->save_object();
  }


  /* Return list of object groups */
  function get_object_groups()
  {
    $ret = array();
    $ldap = $this->config->get_ldap_link();
    $ldap->cd($this->config->current['BASE']);
    $ldap->search("(&(objectClass=gosaGroupOfNames)(cn=*))",array("cn"));
    while($attrs = $ldap->fetch()){
      $ret[$attrs['cn'][0]] = $attrs['cn'][0];
    }
    return($ret); 
  }

  
  function save()
  {
  }


  function list_get_selected_items()
  {
    $ids = array();
    foreach($_POST as $name => $value){
      if(preg_match("/^item_selected_[0-9]*$/",$name)){
        $id   = preg_replace("/^item_selected_/","",$name);
        $ids[$id] = $id;
      }
    }
    return($ids);
  }


  function get_actions()
  {
    /* Prepare list of available actions */
    $actions = array(       "reboot"          => _("Reboot"),
                            "localboot"       => _("Localboot"),
                            "halt"            => _("Halt system"),
                            "initial_install" => _("Initial installation"),
                            "update"          => _("Update"),
                            "reinstall"       => _("(Re)Install"),
                            "rescan"          => _("Rescan"),
                            "wake"            => _("Wake"),
                            "memcheck"        => _("Memory check"));
    return($actions);
  }


  function plInfo()
  {
    return (array(
        "plShortName"   => _("System mass deployment"),
        "plDescription" => _("Provide a mechanism to automatically activate a set of systems"),
        "plSelfModify"  => FALSE,
        "plDepends"     => array(),
        "plPriority"    => 0,
        "plSection"     => array("addon"),
        "plCategory"    => array("gotomasses" => array("objectClass" => "none", "description" => _("System mass deployment"))),
        "plProvidedAcls" => array("Comment"   => _("Description"), 
                                  "Action"    => _("Action"),
                                  "Day"       => _("Day"),
                                  "Minute"    => _("Minute"),
                                  "Hour"      => _("Hour"),
                                  "Month"     => _("Month"),
                                  "Weekday"   => _("Week day"),
                                  "Target"    => _("Target"))
        ));
  }
}
// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
