<?php

class divListMasses extends MultiSelectWindow
{

  /* Current base */
  var $selectedBase       = "";
  var $departments        = array();

  var $parent             ;
  var $ui                 ;

  var $wake = TRUE;  
  
  function divListMasses (&$config,$parent)
  {
    MultiSelectWindow::MultiSelectWindow($config, "gotoMasses", array("gotomasses"));

    $this->parent       = $parent;
    $this->ui           = get_userinfo();
    $this->SetSummary(_("List of system deployment tasks"));
    $this->SetHeadpageMode();
    $this->EnableCloseButton(FALSE);
    $this->EnableSaveButton(FALSE);
    $this->SetInformation(_("This menu allows you to add, remove and change the properties of system deployment tasks."));

    foreach($this->parent->get_actions() as $name => $desc){
      $this->$name = TRUE;
      $this->AddCheckBox($name,sprintf(_("Display entries with action %s."),$desc),
                               sprintf(_("Display entries with action %s."),$desc),TRUE);
    }
    $plug  = $_GET['plug'];

    /* Toggle all selected / deselected */
    $chk = "<input type='checkbox' id='select_all' name='select_all'
               onClick='toggle_all_(\"^item_selected_[0-9]*$\",\"select_all\");' >";

    /* set Page header */
    $this->AddHeader(array("string"=> $chk,          "attach"=>"style='width:20px;'"));

    $this->AddHeader(array("string"=>"<a href='?plug=".$plug."&amp;sort=object'>"._("Target")."</a>"));
    $this->AddHeader(array("string"=>"<a href='?plug=".$plug."&amp;sort=schedule'>"._("Schedule")."</a>",
                                      "attach"=>"style='width:100px;'"));
    $this->AddHeader(array("string"=>"<a href='?plug=".$plug."&amp;sort=action'>"._("Type")."</a>",
                                      "attach"=>"style='width:80px;'"));
    $this->AddHeader(array("string"=>_("Action"),
                                      "attach"=>"style='border-right:0px;width:40px;'"));
  }

  function GenHeader()
  {
    
    /* Display add button if allowed */
    $header = "<div style='padding:5px'>";
    if($this->parent->acl_is_createable()){
      $header .= "<input type='image' src='images/gotomasses_new_task.png' name='new_task' class='center'>&nbsp;";
    }

    /* Display add button if allowed */
    if($this->parent->acl_is_removeable()){
      $header .= "<input type='image' src='images/edittrash.png' name='remove_multiple_task' class='center'>&nbsp;";
    }
    $header .= "</div>";

    /* Create divlist */
    $this->SetListHeader($header);

  }

  function execute()
  {
    $this->ClearElementsList();
    $this->GenHeader();
  }


  function setEntries($tasks)
  {
    /* Create edit link */
    $plug = $_GET['plug'];
    $edit_link = "<div style='width:100%;overflow:hidden;'><nobr><a href='?plug=".$_GET['plug']."&act=edit&id=%key%'>%str%</nobr></div>";

    /* Create action filter array, to sort out those actions we do not want to see */
    $allowed_action = array();
    foreach($this->parent->get_actions() as $name => $desc){
      if($this->$name){
        $allowed_action[] = $name;
      }
    }

    foreach($tasks as $key => $task){

      /* Skip unchecked actions */ 
      if(!in_array($task['Action'],$allowed_action)){
        continue;
      }
 
      $action = "<input type='image' src='images/edit.png' name='edit_task_".$key."' class='center' alt='"._("Edit")."'>";
      if($this->parent->acl_is_removeable()){
        $action.= "<input type='image' src='images/edittrash.png' name='remove_task_".$key."' class='center' alt='"._("Reomve")."'>";
      }
      /* Create each field */
      $field0 = array("string" => "<input type='checkbox' id='item_selected_".$key."' name='item_selected_".$key."'>" ,
          "attach" => "style='width:20px;'");

      $field1 = array("string" => preg_replace(array("/%key%/","/%str%/"),array($key,$this->parent->target_to_string($task)),$edit_link));
      $field2 = array("string" => $this->parent->time_to_string($task),"attach" => "style='width:100px;'");
      $field3 = array("string" => $this->parent->action_to_string($task),"attach" => "style='width:80px;'");
      $field4 = array("string" => $action,"attach" => "style='text-align:right;width:40px;border-right:0px;'");
      $this->AddElement(array($field0,$field1,$field2,$field3,$field4));
    }
  }


  function Save()
  {
    MultiSelectWindow :: Save();  
  }


  function save_object()
  {
    /* Save automatic created POSTs like regex, checkboxes */
    MultiSelectWindow :: save_object();  
  }
}
// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
