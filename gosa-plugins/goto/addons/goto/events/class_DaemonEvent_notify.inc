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

class DaemonEvent_notify extends DaemonEvent
{
  var $visible_for = USER_EVENT;

  var $users = array();
  var $groups= array(); 

  var $message = "";
  var $subject = "";
  var $from    = "";

  var $attributes = array("from","users","groups","message","subject");

  public function __construct($config,$data = array())
  {
    DaemonEvent::__construct($config,$data);

    $ui = get_userinfo();
    $this->from = $ui->cn; 
 
    $this->message = base64_decode($this->message);
    $this->subject = base64_decode($this->subject);

    $this->s_Menu_Name  = _("Send message");
    $this->s_Event_Name = _("Send message");

    $this->s_Schedule_Action = "job_send_user_msg";
    $this->s_Trigger_Action= "gosa_send_user_msg";
    $this->s_Queued_Action= "trigger_action_notify";
    $this->s_Menu_Image = "images/mailto.png";
    $this->s_List_Image = "images/mailto.png";
    $this->a_targets = array("GOSA"); // Required to get the event send. Maybe this is a wrong value.
  }

  public function execute()
  {
    DaemonEvent::execute();

    /* Check if target add dialog is open */
    if($this->is_target_list_open() && $this->is_new){
      return($this->get_target_add_list());
    }


    $display = $this->get_header();
    $tmp = $this->data;
    $smarty = get_smarty();
    $smarty->assign("data"        , $this->data);
    $smarty->assign("is_new"      , $this->is_new);
    $smarty->assign("timestamp"   , $this->get_time_select());
    
    $smarty->assign("users" , $this->users);
    $smarty->assign("groups" , $this->groups);
    
    $smarty->assign("add_str", msgPool::addButton(_("Target")));
    $smarty->assign("del_str", msgPool::delButton(_("Target")));

    $smarty->assign("from",    xmlentities($this->from));
    $smarty->assign("subject", xmlentities($this->subject));
    $smarty->assign("message", xmlentities($this->message));

    $display.= $smarty->fetch(get_template_path('DaemonEvent_notify.tpl', TRUE, dirname(__FILE__)));
    $display.= $this->get_footer();
    return($display);
  }


  public function check()
  {
    $msgs = DaemonEvent::check();
    if(empty($this->subject)){
      $msgs[] = msgPool::required(_("Subject"));
    }
    if(empty($this->message)){
      $msgs[] = msgPool::required(_("Message"));
    }
    if(empty($this->from)){
      $msgs[] = msgPool::required(_("From"));
    }
    if(!count($this->groups) && !count($this->users)){
      $msgs[] = msgPool::required(_("Target"));
    }
    return($msgs);
  }


  public function save_object()
  {
    DaemonEvent::save_object();

    if(isset($_POST['del_any_target']) && isset($_POST['groups'])){
      foreach($_POST['groups'] as $id){
        if(isset($this->groups[$id])){
          unset($this->groups[$id]);
        }
      }
    }
    if(isset($_POST['del_any_target']) && isset($_POST['users'])){
      foreach($_POST['users'] as $id){
        if(isset($this->users[$id])){
          unset($this->users[$id]);
        }
      }
    }

    if(isset($_POST['subject'])){
      $this->subject = get_post('subject');
    }
    if(isset($_POST['message'])){
      $this->message = get_post('message');
    }
    if(isset($_POST['from'])){
      $this->from = get_post('from');
    }
  }

  public function add_users($targets)
  {
    $add = $targets;
    if(!is_array($add)){
      $add = array($add);
    }
    foreach($add as $target){
      if(!in_array($target,$this->users)){
        $this->users[] = $target;
      }
    }
  }


  public function add_groups($targets)
  {
    $add = $targets;
    if(!is_array($add)){
      $add = array($add);
    }
    foreach($add as $target){
      if(!in_array($target,$this->groups)){
        $this->groups[] = $target;
      }
    }
  }


  /*! \brief  Returns HTML content, displaying a dialog which allows to add new targets.
    @return String HTML content. (EventTargetAddList)
   */
  public function get_target_add_list()
  {
    $this->target_add_list_used = TRUE;

    if($this->target_divlist == NULL){
      $this->target_divlist = new EventTargetAddUserList($this->config,$this);
    }
    $this->target_divlist->execute();

    $smarty = get_smarty();
    $smarty->assign("divlist",$this->target_divlist->Draw());
    return($smarty->fetch(get_template_path('target_list.tpl', TRUE, dirname(__FILE__))));
  }


  /*! \brief Add a target MAC address
      @param Array A List of all target that should be added.
   */
  public function add_targets($targets)
  {
    if(isset($targets['USERS'])){
      $this->add_users($targets['USERS']);
    }
    if(isset($targets['GROUPS'])){
      $this->add_groups($targets['GROUPS']);
    }
  }


  public function save()
  {
    $ret = DaemonEvent::save();
    $ret['delivery_time'] = $ret['timestamp'];
    $ret['users']   = array_values( $ret['users']);
    $ret['groups']  = array_values( $ret['groups']);
    $ret['subject'] = base64_encode($ret['subject']);
    $ret['message'] = base64_encode($ret['message']);
    return($ret);
  }
} 
// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
