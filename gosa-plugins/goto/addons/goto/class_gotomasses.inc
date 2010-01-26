<?php
/*
 * This code is part of GOsa (http://www.gosa-project.org)
 * Copyright (C) 2003-2008 GONICUS GmbH
 *
 * ID: $$Id$$
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

class gotomasses extends plugin
{
  /* Definitions */
  var $plHeadline     = "Deployment status";
  var $plDescription  = "System deployment status";
  var $plIcon         = "plugins/goto/images/goto.png";

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
  var $sort_dir = "up";
  var $entries  = array();
  var $range    = 25;
  var $start    = 0;

  var $recently_removed = array();

  function gotomasses(&$config, $dn= NULL)
  {
    /* Include config object */
    $this->config= &$config;
    $this->o_queue = new gosaSupportDaemon(TRUE,5);
    $this->events  = DaemonEvent::get_event_types( SYSTEM_EVENT);

    /* Get tags that will be used in queue searches */
    $this->event_tags = array("none");
    foreach($this->events['SCHEDULED'] as $evt){
      $this->event_tags[] = $evt['s_Queued_Action'];
    }

    /* Load filter settings */
    if(!session::is_set("gotomasses_filter")){
      $gotomasses_filter = 
        array(
            "range" => $this->range,
            "sort_by" => $this->sort_by,
            "sort_dir" => $this->sort_dir);
      session::set("gotomasses_filter",$gotomasses_filter);
    }
    $gotomasses_filter = session::get("gotomasses_filter");
    foreach(array("range","sort_by","sort_dir") as $attr) {
      $this->$attr = $gotomasses_filter[$attr];
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
        "/^log_view_/"              =>  "logview",
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
     * Import CSV file  
     ************/
    
    if($s_action == "import_file" && $this->acl_is_writeable("")){
      $this->dialog = new goto_import_file($this->config,$this);
    }
  
    if(isset($_POST['import_abort'])){
      $this->dialog = FALSE;
    }


    /************
     * Handle Priority modifications  
     ************/

    if(preg_match("/^prio_/",$s_action) && $this->acl_is_writeable("")){
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

      if($this->acl_is_writeable("")){
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
      }
      if($this->o_queue->is_error()){
        msg_dialog::display(_("Error"), $this->o_queue->get_error(), ERROR_DIALOG);
      }
    }

    /************
     * ADD 
     ************/
  
    if(preg_match("/^add_event_/",$s_action) && $this->acl_is_writeable("")){
      $type = preg_replace("/^add_event_/","",$s_action);
      if(isset($this->events['BY_CLASS'][$type])){
        $e_data = $this->events['BY_CLASS'][$type];
        $this->dialog = new $e_data['CLASS_NAME']($this->config);
      }
    }

    /************
     * EDIT
     ************/

    if($s_action == "edit" && $this->acl_is_writeable("")){  
      $id =  $s_entry;
      $type = FALSE;
      if(isset($this->entries[$id])){
        $event = $this->entries[$s_entry];
        if($event['STATUS'] == "waiting" && isset($this->events['QUEUED'][$event['HEADERTAG']])){
          $evt_name = $this->events['QUEUED'][$event['HEADERTAG']];
          $type = $this->events['BY_CLASS'][$evt_name];
          $this->dialog = new $type['CLASS_NAME']($this->config,$event);
        }
      }
    }

    
    /************
     * LOG VIEW
     ************/

    if($s_action == "logview"  && $this->acl_is_readable("")){  
      $id =  $s_entry;
      $type = FALSE;
      if(isset($this->entries[$id])){
        $event = $this->entries[$s_entry];
        $this->dialog = new gotoLogView($this->config,"",$event,$this);
      }
    }


    /************
     * REMOVE 
     ************/

    /* Remove multiple */
    if($s_action == "remove_multiple" || $s_action == "remove"){

      if(!$this->acl_is_removeable()){
        msg_dialog::display(_("Permission"), msgPool::permDelete(), ERROR_DIALOG);
      }else{

        if($s_action == "remove"){
          $ids = array($s_entry);
        }else{
          $ids = $this->list_get_selected_items();
        }

        $this->ids_to_remove = array();

        if(count($ids)){
          $ret = $this->o_queue->ids_exist($ids);
          $ret = $this->o_queue->get_entries_by_id($ret);
          $tmp = "";

          $deleteable_jobs = array();      
          $not_deleteable_jobs = array();      
          foreach($ret as $task){

            /* Create a printable job name/description */
            if(isset($this->events['QUEUED'][$task['HEADERTAG']])){
              $evt_name = $this->events['QUEUED'][$task['HEADERTAG']];
              $evt = $this->events['BY_CLASS'][$evt_name];
              $j_name = $task['ID']." - ".$evt['s_Menu_Name']."&nbsp;".$task['MACADDRESS'];
            }else{
              $j_name = $task['ID']." - ".$task['HEADERTAG']."&nbsp;".$task['MACADDRESS'];
            }

            /* Only remove WAITING or ERROR entries */
            if(in_array($task['STATUS'],array("waiting","error","processed")) || 
                ($task['STATUS'] == "processing" && !preg_match("/install/",$task['HEADERTAG'])) ){
              $this->ids_to_remove[] = $task['ID'];
              $deleteable_jobs[] = $j_name;
            }else{
              $not_deleteable_jobs[] = $j_name;
            }
          }
          if(count($not_deleteable_jobs)){
            msg_dialog::display(_("Remove"),
                sprintf(_("The following jobs couldn't be deleted, they have to be aborted: %s"),
                  "<br>".msgPool::buildList($not_deleteable_jobs)),INFO_DIALOG);
          }

          if(count($this->ids_to_remove)){
            $smarty->assign("multiple", TRUE); 
            $smarty->assign("info",msgPool::deleteInfo($deleteable_jobs));
            $this->current = $s_entry;
            return($smarty->fetch(get_template_path('remove.tpl', TRUE)));
          }
        }
      }
    }

    /* Remove specified tasks */
    if(count($this->ids_to_remove) && isset($_POST['delete_multiple_confirm'])){

      /* Reboot hosts with not yet startet installations and timestamps in the past 
       */
      if($this->acl_is_removeable("")){
        timezone::get_default_timezone();
        foreach($this->ids_to_remove as $id){
          $entry = $this->o_queue->get_entries_by_id(array($id));
          if(isset($entry['ANSWER1'])){
            $entry = $entry['ANSWER1'];
            if( $entry['STATUS'] == "waiting" && 
                $entry['HEADERTAG'] == "trigger_action_reinstall"){
              $evt = new DaemonEvent_reinstall($this->config,$entry);
              if($evt->get_timestamp(FALSE)  < time()){
                $r_evt = new DaemonEvent_localboot($this->config);
                $r_evt->add_targets(array($entry['MACADDRESS']));
                $r_evt->set_type(TRIGGERED_EVENT);
                $this->o_queue->append($r_evt);
              }
            }
          }
        }

        $this->o_queue->remove_entries($this->ids_to_remove);
        $this->save();
      }
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
          msg_dialog::display(_("Service infrastructure"),msgPool::siError($this->o_queue->get_error()),ERROR_DIALOG);
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
      $display = $this->dialog->execute();

      if($this->dialog instanceOf goto_import_file && $this->dialog->import_successful){
        $this->dialog = FALSE;
      }else{
        return($display);
      }
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
    $s.= "..|<img src='images/lists/new.png' alt='' border='0' class='center'>&nbsp;"._("Create")."\n";

    if($this->acl_is_writeable("")){
      foreach($this->events['SCHEDULED'] as $name =>  $event){
        $s.= "...|".$event['MenuImage']."&nbsp;".$event['s_Menu_Name']."|add_event_".$name."\n";
      }
    }
    if($this->acl_is_removeable()){
      $s.= "..|---|\n";
      $s.= "..|<img src='images/lists/import.png' alt='' border='0' class='center'>&nbsp;"._("Import")."|import_file\n";
      $s.= "..|<img src='images/lists/trash.png' alt='' border='0' class='center'>&nbsp;"._("Remove")."|remove_multiple\n";
    }
    if(preg_match("/w/",$this->getacl(""))){
      $s.= "..|---|\n";
      $s.= "..|<img alt='"._("Resume")."' src='images/status_start.png' border='0' class='center'>&nbsp;"._("Resume")."|resume_all\n";
      $s.= "..|<img alt='"._("Pause")."' src='images/status_pause.png' border='0' class='center'>&nbsp;"._("Pause")."|pause_all\n";
      $s.= "..|<img alt='"._("Abort")."' src='images/small_error.png'  border='0' class='center'>&nbsp;"._("Abort")."|abort_process_all\n";
      $s.= "..|<img alt='"._("Execute")."' src='images/rocket.png'       border='0' class='center'>&nbsp;"._("Execute")."|execute_process_all\n";
    }

    $divlist->SetDropDownHeaderMenu($s);

    if($this->sort_dir == "up"){
      $sort_img = "<img src='images/lists/sort-up.png' alt='/\' border=0>";
    }else{
      $sort_img = "<img src='images/lists/sort-down.png' alt='\/' border=0>";
    }

    if($this->sort_by == "TargetName"){ $sort_img_1 = $sort_img; } else { $sort_img_1 = "" ;}
    if($this->sort_by == "TaskID"){ $sort_img_2 = $sort_img; } else { $sort_img_2 = "" ;}
    if($this->sort_by == "Schedule"){ $sort_img_3 = $sort_img; } else { $sort_img_3 = "" ;}
    if($this->sort_by == "Action"){ $sort_img_4 = $sort_img; } else { $sort_img_4 = "" ;}

    /* Create divlist */
    $divlist->SetListHeader("<input type='image' src='images/lists/reload.png' title='"._("Reload")."'>");

    $plug  = $_GET['plug'];
    $chk = "<input type='checkbox' id='select_all' name='select_all'
               onClick='toggle_all_(\"^item_selected_[0-9]*$\",\"select_all\");' >";

    /* set Page header */
    $divlist->AddHeader(array("string"=> $chk,          "attach"=>"style='width:20px;'"));
    $divlist->AddHeader(array("string"=>"<a href='?plug=".$plug."&amp;sort=TargetName'>"._("Target").$sort_img_1."</a>"));
    $divlist->AddHeader(array("string"=>"<a href='?plug=".$plug."&amp;sort=TaskID'>"._("Task").$sort_img_2."</a>",
                                      "attach"=>"style='width:120px;'"));
    $divlist->AddHeader(array("string"=>_("Period"),
                                      "attach"=>"style='width:60px;'"));
    $divlist->AddHeader(array("string"=>"<a href='?plug=".$plug."&amp;sort=Schedule'>"._("Schedule").$sort_img_3."</a>",
                                      "attach"=>"style='width:140px;'"));
    $divlist->AddHeader(array("string"=>"<a href='?plug=".$plug."&amp;sort=Action'>"._("Status").$sort_img_4."</a>",
                                      "attach"=>"style='width:80px;'"));
    $divlist->AddHeader(array("string"=>_("Action"),
                                      "attach"=>"style='border-right:0px;width:140px;'"));


    /* Reload the list of entries */
    $this->reload();

    foreach($this->entries as $key => $task){

      $prio_actions="";
      $action = "";


      /* If WAITING add priority action
       */  
      if(in_array($task['STATUS'],array("waiting")) && $this->acl_is_writeable("")){
        $prio_actions.= "<input class='center' type='image' src='plugins/goto/images/prio_increase.png' 
          title='"._("Move up")."' name='prio_up_".$key."'>&nbsp;";
        $prio_actions.= "<input class='center' type='image' src='plugins/goto/images/prio_decrease.png' 
          title='"._("Move down")."' name='prio_down_".$key."'>&nbsp;";
      }
    
      /* If WAITING add pause action
       */  
      if(in_array($task['STATUS'],array("waiting")) && $this->acl_is_writeable("")){
        $prio_actions.= "<input class='center' type='image' src='images/status_pause.png' 
          title='"._("Pause job")."' name='pause_".$key."'>&nbsp;";
      }

      /* If PAUSED add resume action
       */  
      if(in_array($task['STATUS'],array("paused")) && $this->acl_is_writeable("")){
        $prio_actions.= "<input class='center' type='image' src='images/status_start.png' 
          title='"._("Resume job")."' name='resume_".$key."'>&nbsp;";
      }

      /* If PAUSED or WAITING add execution action
       */  
      if(in_array($task['STATUS'],array("paused","waiting")) && $this->acl_is_writeable("")){
        $prio_actions.= "<input class='center' type='image' src='images/rocket.png' 
          title='"._("Execute now")."' name='execute_process_".$key."'>&nbsp;";
      }

      /* Add logview button, currently ever.
       */  
      if($this->acl_is_readable("")){
        $action .= "<input type='image' src='plugins/goto/images/view_logs.png' name='log_view_".$key."' 
          class='center' title='"._("View logs")."' alt='"._("View logs")."'>&nbsp;";
      }

      /* If PAUSED or WAITING add edit action
       */  
      if(in_array($task['STATUS'],array("waiting")) && $this->acl_is_writeable("")){
        $action.= "<input type='image' src='images/lists/edit.png' name='edit_task_".$key."' 
          class='center' title='"._("Edit")."' alt='"._("Edit")."'>";
      }

      /* If PROCESSING add abort action
       */  
      if(in_array($task['STATUS'],array("processing")) && preg_match("/install/",$task['HEADERTAG']) && $this->acl_is_writeable("")){
        $action.= "<img src='images/empty.png' alt=''>";
        $action.= "<input class='center' type='image' src='images/small_error.png' 
          title='"._("Abort job")."' name='abort_process_".$key."'>";
      }

      /* If WAITING or ERROR add remove action
       */  
      if( $this->acl_is_removeable() && in_array($task['STATUS'],array("waiting","error","processed"))){
        $action.= "<input type='image' src='images/lists/trash.png' name='remove_task_".$key."' 
          class='center' title='"._("Remove")."' alt='"._("Remove")."'>";
      }
      if($this->acl_is_writeable("") && in_array($task['STATUS'],array("processing")) && !preg_match("/install/",$task['HEADERTAG'])){
        $action.= "<input type='image' src='images/lists/trash.png' name='remove_task_".$key."' 
          class='center' title='"._("Remove")."' alt='"._("Remove")."'>";
      }

      /* Create entry display name and tooltip */
      $color = "";
      $display = $task['MACADDRESS'];
      $tooltip = "";
      if(isset($task['PLAINNAME']) && !preg_match("/none/i",$task['PLAINNAME'])){
        $display = $task['PLAINNAME'];
        $tooltip = " title='".$task['MACADDRESS']."' ";
      }


      $display2= $task['HEADERTAG'];
      
      /* Check if this event exists as Daemon class 
       * In this case, display a more accurate entry.
       */ 
      if(isset($this->events['QUEUED'][$task['HEADERTAG']])){
        $evt_name   = $this->events['QUEUED'][$task['HEADERTAG']];
        $event_type = $this->events['BY_CLASS'][$evt_name];
        $display2   = $event_type['s_Menu_Name'];

        if(strlen($display2) > 20){
          $display2 = substr($display2,0,18)."...";
        }

        if(isset($event_type['ListImage']) && !empty($event_type['ListImage'])){
          $display2 = $event_type['ListImage']."&nbsp;".$display2;
        }
      } 

      $status = $task['STATUS'];
  
      if($status == "waiting"){
        $status = "<img class='center' src='plugins/goto/images/clock.png' alt=''>&nbsp;"._("Waiting");
      }
      if($status == "error"){
        $status = "<img class='center' src='images/false.png' alt=''>&nbsp;"._("Error");
      }
      if($status == "processed"){
        $status = "<img class='center' src='images/true.png' alt=''>&nbsp;"._("Processed");
      }

      /* Special handling for all entries that have 
          STATUS == "processing" && PROGRESS == NUMERIC
       */
      if($status == "processing" && isset($task['PROGRESS'])){
        $percent = $task['PROGRESS'];

        /* Show activation? */
        if ($percent == "goto-activation"){
          $status = "<img class='center' src='images/lists/off.png' alt=''>&nbsp;"._("Locked");

        /* Show hardware detect? */
        } elseif ($percent == "goto-hardware-detection") {
          $status = "<img class='center' src='plugins/goto/images/hardware.png' alt=''>&nbsp;"._("Detection");

        /* Real percent */
        } else {
         if (preg_match('/install/', $task['HEADERTAG'])){
            $status = "<img src='progress.php?x=80&y=13&p=".$task['PROGRESS']."' alt=''
                          id='progress_".preg_replace("/:/","_",$task['MACADDRESS'])."'>";
          } else {
            $status = preg_replace('/ /', '&nbsp;', _("in progress"));
          }
        }
      }

      // Check whether this is a periodical job or not.
      $period = "";
      if(isset($task['PERIODIC']) && !preg_match("/none/i",$task['PERIODIC'])){
        $tmp = explode("_", $task['PERIODIC']);
        if(count($tmp) == 2){
          $period= $tmp[0]."&nbsp;"._($tmp[1]);
        }
      }

      /* Create each field */
      $field0 = array("string" => "<input type='checkbox' id='item_selected_".$task['ID']."' name='item_selected_".$key."'>" ,
                      "attach" => "style='width:20px;".$color."'");
      $field1 = array("string" => $display,
                      "attach" => $tooltip."style='".$color."'");
      $field1a= array("string" => $display2,
                      "attach" => "style='".$color.";width:120px;'");
      $field1b= array("string" => $period,
                      "attach" => "style='".$color.";width:60px;'");
      if ($task['TIMESTAMP'] == "19700101000000"){
	      $field2 = array("string" => _("immediately"),"attach" => "style='".$color.";width:140px;'");
      } else {
	      $field2 = array("string" => date("d.m.Y H:i:s",strtotime($task['TIMESTAMP'])),"attach" => "style='".$color.";width:140px;'");
      }
      $field3 = array("string" => $status,"attach" => "style='".$color.";width:80px;'");
      $field4 = array("string" => $prio_actions.$action,"attach" => "style='".$color.";text-align:right;width:140px;border-right:0px;'");
      $divlist->AddElement(array($field0,$field1,$field1a,$field1b,$field2,$field3,$field4));
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

    $count = $this->o_queue->number_of_queued_entries($this->event_tags);
    if(!$count) $count = $this->range;
    $divlist->SetListFooter(range_selector($count, $this->start, $this->range,"range"));
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
        msg_dialog::display(_("Error"), sprintf(_("Cannot update queue entry: %s"),$id) , ERROR_DIALOG);
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
        msg_dialog::display(_("Error"), sprintf(_("Cannot update queue entries.")) , ERROR_DIALOG);
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
      $tmp->set_type(TRIGGERED_EVENT);
      $this->recently_removed = $update_ids;
      
      if(!$this->o_queue->append($tmp)){
        msg_dialog::display(_("Error"), sprintf(_("Cannot update queue entry: %s"),$id) , ERROR_DIALOG);
        return(FALSE);
      }
    }else{
      msg_dialog::display(_("Error"),
          sprintf(_("Required class '%s' cannot be found: job not aborted!"),
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
        msg_dialog::display(_("Error"), sprintf(_("Cannot update queue entry: %s"),$id) , ERROR_DIALOG);
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
    
      /* Skip entries which will be removed within the next seconds */
      if(isset($entry['MACADDRESS']) && in_array($entry['MACADDRESS'],$this->recently_removed)){
        continue;
      }
      $this->entries[$entry['ID']]= $entry;
    }
    $this->recently_removed = array();
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
  
    /* Save filter settings */ 
    $gotomasses_filter = session::get("gotomasses_filter");
    foreach(array("range","sort_by","sort_dir") as $attr){
      $gotomasses_filter[$attr] = $this->$attr;
    }
    session::set("gotomasses_filter",$gotomasses_filter);
 
    /* Page changed. */
    if(isset($_GET['start'])){
      $start = $_GET['start'];
      if(is_numeric($start) || $start == 0){
        $this->start = $start;
      }
    }

    /* Check start stop and reset if necessary */
    $count = $this->o_queue->number_of_queued_entries($this->event_tags);
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
          "plShortName"   => _("System deployment"),
          "plDescription" => _("Provide a mechanism to automatically activate systems"),
          "plSelfModify"  => FALSE,
          "plDepends"     => array(),
          "plPriority"    => 0,
          "plSection"     => array("addon"),
          "plCategory"    => array("gotomasses" => array("objectClass" => "none", "description" => _("System deployment"))),
          "plProvidedAcls" => array("Comment"   => _("Description")) 
          ));
  }
}
// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
