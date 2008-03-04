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
  var $current        = FALSE;
  var $dialog         = FALSE;
  var $ids_to_remove  = array();
  var $divlist        = NULL;

  var $events         = array();
  var $event_tags     = array();

  var $sort_by  = "Schedule";
  var $sort_dir = "down";
  var $entries  = array();
  var $range    = 25;
  var $start    = 0;

  function gotomasses(&$config, $dn= NULL)
  {
    /* Include config object */
    $this->config= &$config;
    $this->o_queue = new gosaSupportDaemon(TRUE,10);
    $this->events  = DaemonEvent::get_event_types( USER_EVENT | SYSTEM_EVENT);

    /* Get tags that will be used in queue searches */
    foreach($this->events['BY_CLASS'] as $evt){
      if(isset($evt['s_Queued_Action'])){
        $this->event_tags[] = $evt['s_Queued_Action'];
      }
    }
  }


  function execute()
  {
    $smarty = get_smarty();
 
    /************
     * Handle posts 
     ************/
    
    $s_entry = $s_action = "";
    $arr = array( 

        "/^pause_/"           => "pause",
        "/^resume_/"          => "resume",
        "/^execute_process_/" => "execute_process",
        "/^abort_process_/"   => "abort_process",

        "/^prio_up_/"     => "prio_up",
        "/^prio_down_/"   => "prio_down",

        "/^edit_task_/"             =>  "edit",
        "/^remove_task_/"           =>  "remove",
        "/^new_task_/"              =>  "new_task");;


    foreach($arr as $regex => $action){
      foreach($_POST as $name => $value){
        if(preg_match($regex,$name)){
          $s_action = $action;
          $s_entry  = preg_replace($regex,"",$name);
          $s_entry  = preg_replace("/_(x|y)$/","",$s_entry);
        }
      }
    }

    /* Menu actions */
    if(isset($_POST['menu_action']) && !empty($_POST['menu_action'])){
      $s_action = $_POST['menu_action'];
    }
    
    /* Edit posted from list link */
    if(isset($_GET['act']) && $_GET['act'] == "edit" && isset($_GET['id']) && isset($this->tasks[$_GET['id']])){
      $s_action = "edit";
      $s_entry = $_GET['id'];
    }


    /************
     * Handle Priority modifications  
     ************/

    if(preg_match("/^prio_/",$s_action)){
      switch($s_action){
        case 'prio_down'  : $this->update_priority($s_entry,"down");break;
        case 'prio_up'    : $this->update_priority($s_entry,"up");break;
      }
    }

    /************
     * Handle pause/resume/execute modifications  
     ************/

    if(preg_match("/^resume/",$s_action) || 
       preg_match("/^pause/",$s_action) || 
       preg_match("/^abort_process/",$s_action) || 
       preg_match("/^execute_process/",$s_action)){

      switch($s_action){
        case 'resume'         : $this->resume_queue_entries   (array($s_entry));break; 
        case 'pause'          : $this->pause_queue_entries    (array($s_entry));break; 
        case 'execute_process': $this->execute_queue_entries  (array($s_entry));break; 
        case 'abort_process'  : $this->abort_queue_entries    (array($s_entry));break; 
        case 'resume_all'         : $this->resume_queue_entries   ($this->list_get_selected_items());break; 
        case 'pause_all'          : $this->pause_queue_entries    ($this->list_get_selected_items());break; 
        case 'execute_process_all': $this->execute_queue_entries  ($this->list_get_selected_items());break; 
        case 'abort_process_all'  : $this->abort_queue_entries    ($this->list_get_selected_items());break; 

        default : trigger_error("Undefined action setting used (".$s_action.").");
      }
      if($this->o_queue->is_error()){
        msg_dialog::display(_("Error"), $this->o_queue->get_error(), ERROR_DIALOG);
      }
    }

    /************
     * ADD 
     ************/
  
    if(preg_match("/^add_event_/",$s_action)){
      $type = preg_replace("/^add_event_/","",$s_action);
      if(isset($this->events['BY_CLASS'][$type])){
        $e_data = $this->events['BY_CLASS'][$type];
        $this->dialog = new $e_data['CLASS_NAME']($this->config);
      }
    }

    /************
     * EDIT
     ************/

    if($s_action == "edit"){  
      $id =  $s_entry;
      $type = FALSE;
      if(isset($this->entries[$id])){
        $event = $this->entries[$s_entry];
      
        
        if($event['STATUS'] == "waiting" && isset($this->events['BY_QUEUED_ACTION'][$event['HEADERTAG']])){
          $type = $this->events['BY_QUEUED_ACTION'][$event['HEADERTAG']];
          $this->dialog = new $type['CLASS_NAME']($this->config,$event);
        }
      }
    }

    /************
     * REMOVE 
     ************/

    /* Remove multiple */
    if($s_action == "remove_multiple" || $s_action == "remove"){

      if(!$this->acl_is_removeable()){
        msg_dialog::display(_("Permission error"), _("You have no permission to delete this entry!"), ERROR_DIALOG);
      }else{

        if($s_action == "remove"){
          $ids = array($s_entry);
        }else{
          $ids = $this->list_get_selected_items();
        }

        if(count($ids)){
          $ret = $this->o_queue->ids_exist($ids);
          $ret = $this->o_queue->get_entries_by_id($ret);

          $tmp = "";
          foreach($ret as $task){

            /* Only remove WAITING or ERROR entries */
            if(in_array($task['STATUS'],array("waiting","error"))){
              $this->ids_to_remove[] = $task['ID'];
              if(isset($this->events['BY_QUEUED_ACTION'][$task['HEADERTAG']])){
                $evt = $this->events['BY_QUEUED_ACTION'][$task['HEADERTAG']];
                $tmp.= "\n".$task['ID']." - ".$evt['s_Menu_Name']."&nbsp;".$task['MACADDRESS'];
              }else{
                $tmp.= "\n".$task['ID']." - ".$task['HEADERTAG']."&nbsp;".$task['MACADDRESS'];
              }
            }
          }
          $smarty->assign("multiple", TRUE); 
          $smarty->assign("info",sprintf(_("You are about to remove the following actions from the GOsa support Daemon: %s"),"<pre>".$tmp."</pre>"));
          $this->current = $s_entry;
          return($smarty->fetch(get_template_path('remove.tpl', TRUE)));
        }
      }
    }

    /* Remove specified tasks */
    if(count($this->ids_to_remove) && isset($_POST['delete_multiple_confirm'])){
      $this->o_queue->remove_entries($this->ids_to_remove);
      $this->save();
    }

    /* Remove aborted */
    if(isset($_POST['delete_cancel'])){
      $this->ids_to_remove = array();;
    }


    /************
     * EDIT 
     ************/

    /* Close dialog */
    if(isset($_POST['save_event_dialog'])){
      if(is_object($this->dialog)){
        $this->dialog->save_object();
        if(!$this->o_queue->append($this->dialog)){
          msg_dialog::display(_("Daemon"),sprintf(_("Something went wrong while talking to the daemon: %s."),
                $this->o_queue->get_error()),ERROR_DIALOG);
        }else{
          $this->dialog = FALSE; 
          $this->current = -1;
        } 
      }
    }


    /* Close dialog */
    if(isset($_POST['abort_event_dialog'])){
      $this->dialog = FALSE;
      $this->current = -1;
    }

    /* Display dialogs if currently opened */
    if(is_object($this->dialog)){
      $this->dialog->save_object();
      return($this->dialog->execute());
    }

    /************
     * Handle Divlist 
     ************/

    $divlist = new MultiSelectWindow($this->config,"gotoMasses",array("gotomasses"));
    $divlist->SetInformation(_("This menu allows you to remove and change the properties of GOsa tasks."));
    $divlist->SetSummary(_("List of queued jobs"));
    $divlist->EnableCloseButton(FALSE);
    $divlist->EnableSaveButton(FALSE);
    $divlist->SetHeadpageMode();
    $s = ".|"._("Actions")."|\n";
    $s.= "..|<img src='images/list_new.png' alt='' border='0' class='center'>&nbsp;"._("Create")."\n";
    foreach($this->events['BY_CLASS'] as $name =>  $event){
      $s.= "...|".$event['MenuImage']."&nbsp;".$event['s_Menu_Name']."|add_event_".$name."\n";
    }
    if($this->acl_is_removeable()){
      $s.= "..|---|\n";
      $s.= "..|<img src='images/edittrash.png' alt='' border='0' class='center'>&nbsp;"._("Remove")."|remove_multiple\n";
    }
    if(preg_match("/w/",$this->getacl(""))){
      $s.= "..|---|\n";
      $s.= "..|<img src='images/status_start.png' alt='' border='0' class='center'>&nbsp;"._("Resume all")."|resume_all\n";
      $s.= "..|<img src='images/status_pause.png' alt='' border='0' class='center'>&nbsp;"._("Pause all")."|pause_all\n";
      $s.= "..|<img src='images/small_error.png'  alt='' border='0' class='center'>&nbsp;"._("Abort all")."|abort_process_all\n";
      $s.= "..|<img src='images/rocket.png'       alt='' border='0' class='center'>&nbsp;"._("Execute all")."|execute_process_all\n";
    }

    $divlist->SetDropDownHeaderMenu($s);

    if($this->sort_dir == "up"){
      $sort_img = "<img src='images/sort_up.png' alt='/\' border=0>";
    }else{
      $sort_img = "<img src='images/sort_down.png' alt='\/' border=0>";
    }

    if($this->sort_by == "TargetName"){ $sort_img_1 = $sort_img; } else { $sort_img_1 = "" ;}
    if($this->sort_by == "TaskID"){ $sort_img_2 = $sort_img; } else { $sort_img_2 = "" ;}
    if($this->sort_by == "Schedule"){ $sort_img_3 = $sort_img; } else { $sort_img_3 = "" ;}
    if($this->sort_by == "Action"){ $sort_img_4 = $sort_img; } else { $sort_img_4 = "" ;}

    /* Create divlist */
    $divlist->SetListHeader("<input type='image' src='images/list_reload.png' title='"._("Reload")."'>");

    $plug  = $_GET['plug'];
    $chk = "<input type='checkbox' id='select_all' name='select_all'
               onClick='toggle_all_(\"^item_selected_[0-9]*$\",\"select_all\");' >";

    /* set Page header */
    $divlist->AddHeader(array("string"=> $chk,          "attach"=>"style='width:20px;'"));
    $divlist->AddHeader(array("string"=>"<a href='?plug=".$plug."&amp;sort=TargetName'>"._("Target").$sort_img_1."</a>"));
    $divlist->AddHeader(array("string"=>"<a href='?plug=".$plug."&amp;sort=TaskID'>"._("Task").$sort_img_2."</a>",
                                      "attach"=>"style='width:120px;'"));
    $divlist->AddHeader(array("string"=>"<a href='?plug=".$plug."&amp;sort=Schedule'>"._("Schedule").$sort_img_3."</a>",
                                      "attach"=>"style='width:100px;'"));
    $divlist->AddHeader(array("string"=>"<a href='?plug=".$plug."&amp;sort=Action'>"._("Status").$sort_img_4."</a>",
                                      "attach"=>"style='width:80px;'"));
    $divlist->AddHeader(array("string"=>_("Action"),
                                      "attach"=>"style='border-right:0px;width:120px;'"));


    /* Reload the list of entries */
    $this->reload();

    foreach($this->entries as $key => $task){

      $prio_actions="";
      $action = "";

      /* If WAITING add priority action
       */  
      if(in_array($task['STATUS'],array("waiting"))){
        $prio_actions.= "<input class='center' type='image' src='images/prio_increase.png' 
          title='"._("Move up in execution queue")."' name='prio_up_".$key."'>&nbsp;";
        $prio_actions.= "<input class='center' type='image' src='images/prio_decrease.png' 
          title='"._("Move down in execution queue")."' name='prio_down_".$key."'>&nbsp;";
      }
    
      /* If WAITING add pause action
       */  
      if(in_array($task['STATUS'],array("waiting"))){
        $prio_actions.= "<input class='center' type='image' src='images/status_pause.png' 
          title='"._("Pause job")."' name='pause_".$key."'>&nbsp;";
      }

      /* If PAUSED add resume action
       */  
      if(in_array($task['STATUS'],array("paused"))){
        $prio_actions.= "<input class='center' type='image' src='images/status_start.png' 
          title='"._("Resume job")."' name='resume_".$key."'>&nbsp;";
      }

      /* If PROCESSING add abort action
       */  
      if(in_array($task['STATUS'],array("processing"))){
        $prio_actions.= "<input class='center' type='image' src='images/small_error.png' 
          title='"._("Abort execution")."' name='abort_process_".$key."'>";
      }

      /* If PAUSED or WAITING add execution action
       */  
      if(in_array($task['STATUS'],array("paused","waiting"))){
        $prio_actions.= "<input class='center' type='image' src='images/rocket.png' 
          title='"._("Force execution now!")."' name='execute_process_".$key."'>&nbsp;";
      }

      /* If PAUSED or WAITING add edit action
       */  
      if(in_array($task['STATUS'],array("waiting"))){
        $action.= "<input type='image' src='images/edit.png' name='edit_task_".$key."' 
          class='center' alt='"._("Edit")."'>";
      }

      /* If WAITING or ERROR add remove action
       */  
      if( $this->acl_is_removeable() && in_array($task['STATUS'],array("waiting","error"))){
        $action.= "<input type='image' src='images/edittrash.png' name='remove_task_".$key."' 
          class='center' alt='"._("Remove")."'>";
      }

      $color = "";
      $display = $task['MACADDRESS'];
      $display2= $task['HEADERTAG'];
     
      /* Check if this event exists as Daemon class 
       * In this case, display a more accurate entry.
       */ 
      if(isset($this->events['BY_QUEUED_ACTION'][$task['HEADERTAG']]['s_Menu_Name'])){
        $event_type = $this->events['BY_QUEUED_ACTION'][$task['HEADERTAG']];
        $display2= $event_type['s_Menu_Name'];

        if(strlen($display2) > 20){
          $display2 = substr($display2,0,18)."...";
        }

        if(isset($event_type['ListImage']) && !empty($event_type['ListImage'])){
          $display2 = $event_type['ListImage']."&nbsp;".$display2;
        }
      } 

      $status = $task['STATUS'];
  
      if($status == "waiting"){
        $status = "<img class='center' src='images/clock.png' alt=''>&nbsp;"._("Waiting");
      }
      if($status == "error"){
        $status = "<img class='center' src='images/false.png' alt=''>&nbsp;"._("Error");
      }

      /* Special handling for all entries that have 
          STATUS == "processing" && PROGRESS == NUMERIC
       */
      if($status == "processing" && isset($task['PROGRESS'])){
        $percent = $task['PROGRESS'];
        $status = "<img src='progress.php?x=80&amp;y=13&amp;p=".$percent."' alt='".$percent."&nbsp;%'>";
      }


      /* Create each field */
      $field0 = array("string" => "<input type='checkbox' id='item_selected_".$task['ID']."' name='item_selected_".$key."'>" ,
                      "attach" => "style='width:20px;".$color."'");
      $field1 = array("string" => $display,
                      "attach" => "style='".$color."'");
      $field1a= array("string" => $display2,
                      "attach" => "style='".$color.";width:120px;'");
      $field2 = array("string" => date("d.m.Y H:i:s",strtotime($task['TIMESTAMP'])),"attach" => "style='".$color.";width:100px;'");
      $field3 = array("string" => $status,"attach" => "style='".$color.";width:80px;'");
      $field4 = array("string" => $prio_actions.$action,"attach" => "style='".$color.";text-align:right;width:120px;border-right:0px;'");
      $divlist->AddElement(array($field0,$field1,$field1a,$field2,$field3,$field4));
    }

    $smarty = get_smarty();
    $smarty->assign("events",$this->events);
    $smarty->assign("start",$this->start);
    $smarty->assign("start_real", ($this->start + 1));
    $smarty->assign("ranges", array("10" => "10",
                                    "20" => "20",
                                    "25" => "25",
                                    "50" => "50",
                                    "100"=> "100",
                                    "200"=> "200",
                                    "9999" => "*"));

    $count = $this->o_queue->number_of_queued_entries();
    $smarty->assign("range_selector", range_selector($count, $this->start, $this->range,"range"));
    $smarty->assign("range",$this->range);
    $smarty->assign("div",$divlist->Draw());
    return($smarty->fetch (get_template_path('gotomasses.tpl', TRUE, dirname(__FILE__))));
  }


  /*! \brief  Move an entry up or down in the queue, by updating its execution timestamp  
      @param  $id     Integer  The ID of the entry which should be updated.
      @param  $type   String   "up" / "down"
      @return boolean TRUE in case of success else FALSE
  */
  public function update_priority($id,$type = "up")
  {
    if($type == "up"){
      $tmp = $this->o_queue->get_queued_entries($this->event_tags,-1,-1,"timestamp DESC");
    }else{
      $tmp = $this->o_queue->get_queued_entries($this->event_tags,-1,-1,"timestamp ASC");
    }
    $last = array();
    foreach($tmp as $entry){
      if($entry['ID'] == $id){
        if(count($last)){
          $time = strtotime($last['TIMESTAMP']);
          if($type == "up"){
            $time ++;
          }else{
            $time --;
          }
          $time_str = date("YmdHis",$time); 
          return($this->o_queue->update_entries(array($id),array("timestamp" => $time_str)));
        }else{
          return(FALSE);
        }
      }
      $last = $entry;
    }
    return(FALSE);
  }


  /*! \brief  Resumes to status 'waiting'.
   *  @return Boolean TRUE in case of success, else FALSE. 
   */
  private function resume_queue_entries($ids)
  {
    if(!count($ids)){
      return;
    }

    /* Entries are resumed by setting the status to 
     *  'waiting'
     */
    $data = array("status"    => "waiting");
  
    /* Check if given ids are valid and check if the status
     *  allows resuming.
     */
    $update_ids = array();
    foreach($this->o_queue->get_entries_by_id($ids) as $entry){
      if(isset($entry['STATUS']) && preg_match("/paused/",$entry['STATUS'])){
        $update_ids[] = $entry['ID'];
      }
    }

    /* Tell the daemon that we have entries to update.
     */
    if(count($update_ids)){
      if(!$this->o_queue->update_entries($update_ids,$data)){
        msg_dialog::display(_("Error"), sprintf(_("Could not update queue entry: %s"),$id) , ERROR_DIALOG);
        return(FALSE);
      }
    }
    return(TRUE);
  }


  /*! \brief  Force queue job to be done as far as possible.
   *  @return Boolean TRUE in case of success, else FALSE.
   */
  private function execute_queue_entries($ids)
  {
    if(!count($ids)){
      return;
    }

    /* Execution is forced by updating the status to 
     *  waiting and setting the timestamp to current time.
     */
    $data = array(  "timestamp" => date("YmdHis",time()), 
                    "status"    => "waiting");

    /* Only allow execution of paused or waiting entries 
     */
    $update_ids = array();
    foreach($this->o_queue->get_entries_by_id($ids) as $entry){
      if(in_array($entry['STATUS'],array("paused","waiting"))){
        $update_ids[] = $entry['ID'];
      }
    }

    /* Tell the daemon that we want to update some entries
     */
    if(count($update_ids)){
      if(!$this->o_queue->update_entries($update_ids,$data)){
        msg_dialog::display(_("Error"), sprintf(_("Could not update queue entries.")) , ERROR_DIALOG);
        return(FALSE);
      }
    }
    return(TRUE);
  }


  /*! \brief  Force queue job to be done as far as possible.
   *  @return Boolean TRUE in case of success, else FALSE.
   */
  private function abort_queue_entries($ids)
  {
    if(!count($ids)){
      return;
    }

    /* Entries are paused by setting the status to
     *  something different from 'waiting'.
     * We simply use 'paused'.
     */
    $data = array("status"    => "paused");

    /* Detect if the ids we got are valid and
     *  check if the status allows pausing.
     */
    $update_ids = array();
    foreach($this->o_queue->get_entries_by_id($ids) as $entry){
      if(isset($entry['STATUS']) && preg_match("/processing/",$entry['STATUS'])){
        if(isset($entry['MACADDRESS'])){
          $update_ids[] = $entry['MACADDRESS'];
        }else{
          trigger_error("No mac address found in event.");
        }
      }
    }

    if(class_available("DaemonEvent_faireboot")){
      $tmp = new DaemonEvent_faireboot($this->config);
      $tmp->add_targets($update_ids);
      if(!$this->o_queue->append($tmp)){
        msg_dialog::display(_("Error"), sprintf(_("Could not update queue entry: %s"),$id) , ERROR_DIALOG);
        return(FALSE);
      }
    }else{
      msg_dialog::display(_("Error"),
          sprintf(_("The Job could not be aborted, the '%s' event class was not found."),
            "DaemonEvent_faireboot") , ERROR_DIALOG);
    }
  }


  /*! \brief Pauses the specified queue entry from execution.
   *  @return Boolean TRUE in case of success, else FALSE. 
   */
  private function pause_queue_entries($ids)
  {
    if(!count($ids)){
      return;
    }

    /* Entries are paused by setting the status to 
     *  something different from 'waiting'.
     * We simply use 'paused'.
     */   
    $data = array("status"    => "paused");

    /* Detect if the ids we got are valid and
     *  check if the status allows pausing.
     */ 
    $update_ids = array();
    foreach($this->o_queue->get_entries_by_id($ids) as $entry){
      if(isset($entry['STATUS']) && preg_match("/waiting/",$entry['STATUS'])){
        $update_ids[] = $entry['ID'];
      }
    }

    /* Tell the daemon that we want to update some entries
     */
    if(count($update_ids)){
      if(!$this->o_queue->update_entries($update_ids,$data)){
        msg_dialog::display(_("Error"), sprintf(_("Could not update queue entry: %s"),$id) , ERROR_DIALOG);
        return(FALSE);
      }
    }
    return(TRUE);
  }


  /*! \brief  Request list of queued jobs.
   *  @return Returns an array of all queued jobs.
   */
  function reload()
  {

    /* Sort map   html-post-name => daemon-col-name
     */
    $map = array(
        "QueuePosition" => "id",
        "Action"        => "status",
        "TaskID"        => "headertag",
        "TargetName"    => "macaddress",
        "Schedule"      => "timestamp");

    /* Create sort header 
     */
    if(!isset($map[$this->sort_by])){
      $sort = "id DESC";
    }else{
      $sort   = $map[$this->sort_by]; 
      if($this->sort_dir == "up"){
        $sort.= " ASC";
      }else{
        $sort.= " DESC";
      }
    }
     
    /* Get entries. */ 
    $start  = $this->start; 
    $stop   = $this->range;
    $entries = $this->o_queue->get_queued_entries($this->event_tags,$start,$stop,$sort);
    if ($this->o_queue->is_error()){
      msg_dialog::display(_("Error"), sprintf(_("Cannot load queue entries: %s"), "<br><br>".$this->o_queue->get_error()), ERROR_DIALOG);
    }

    /* Assign entries by id.
     */
    $this->entries = array();
    foreach($entries as $entry){
      $this->entries[$entry['ID']]= $entry;
    }
  }


  /*! \brief  Handle post jobs, like sorting.
   */
  function save_object()
  {
    /* Check for sorting changes 
     */
    $sort_vals = array("Action","QueuePosition","TargetName","Schedule","TaskID");
    if(isset($_GET['sort']) && in_array($_GET['sort'],$sort_vals)){
      $sort = $_GET['sort'];
      if($this->sort_by == $sort){
        if($this->sort_dir == "up"){
          $this->sort_dir = "down";
        }else{
          $this->sort_dir = "up";
        }
      }
      $this->sort_by = $sort;
    }

    /* Range selection used? */
    if(isset($_POST['range']) && is_numeric($_POST['range'])){
      $this->range = $_POST['range'];
    }
    
    /* Page changed. */
    if(isset($_GET['start'])){
      $start = $_GET['start'];
      if(is_numeric($start) || $start == 0){
        $this->start = $start;
      }
    }

    /* Check start stop and reset if necessary */
    $count = $this->o_queue->number_of_queued_entries();
    if($this->start >= $count){
      $this->start = $count -1;
    }
    if($this->start < 0){
      $this->start = 0;
    }
  }


  function save()
  {
    // We do not save anything here.
  }


  /*! \brief  Return a list of all selected items.
    @return Array   Returns an array containing all selected item ids.
   */
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


  static function plInfo()
  {
    return (array(
          "plShortName"   => _("System mass deployment"),
          "plDescription" => _("Provide a mechanism to automatically activate a set of systems"),
          "plSelfModify"  => FALSE,
          "plDepends"     => array(),
          "plPriority"    => 0,
          "plSection"     => array("addon"),
          "plCategory"    => array("gotomasses" => array("objectClass" => "none", "description" => _("System mass deployment"))),
          "plProvidedAcls" => array("Comment"   => _("Description")) 
          ));
  }
}
// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
