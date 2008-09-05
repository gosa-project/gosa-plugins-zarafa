<?php

class mailqueue extends plugin
{
  /* Definitions */
  var $plHeadline     = "Mail queue";
  var $plDescription  = "This does something";
  var $plIcon         = "plugins/mail/images/mailqueue.png";

  /* attribute list for save action */
  var $attributes     = array();
  var $objectclasses  = array();

  var $Server         = "all";
  var $ServerList     = array(); // The list of all available servers.
  var $Search         = "*";
  var $Time           = 0;
  var $Page           = 0;
  var $Stat           = "all";
  var $OrderBy        = "Arrival";
  var $SortType       = "up";
  var $disp_header    = false;
  var $range          = 20;   

  /* Logging detection */
  var $view_logged    = FALSE;

  function mailqueue(&$config, $dn= NULL)
  {
    $this->config   = &$config;
    $this->si_queue = new si_mailqueue($this->config);
    $this->getServer();
  }


  function execute()
  {
    /* Call parent execute */
    plugin::execute();

    /* Log view */
    if(!$this->view_logged){
      $this->view_logged = TRUE;
      new log("view","mailqueue/".get_class($this),$this->dn);
    }

    $smarty= get_smarty();
    $error =false;

    /******************
      Handle options 
     ******************/

    $action = $server = $entry = "";
    $types = array("delete","hold","unhold","header","requeue");
    foreach($_POST as $name => $value){
      foreach($types as $type){
        if(preg_match("/^".$type."__/",$name)){
          $action = $type;
          $entry  = preg_replace("/^".$type."__([^_]*)__.*/","\\1",$name); 
          $server = preg_replace("/^".$type."__[^_]*__([^_]*)_.*/","\\1",$name); 
          break;
        }
      }
    }

    /* Send action for given mail id */
    if(in_array($action,array("hold","unhold","requeue"))){
      $this->si_queue->send_queue_action(array($entry),$server,$action);
    }


    /******************
      Display mail header
     ******************/

    if($action == "header"){

      /* Create table which displays the header informations */
      $this->disp_header = $this->si_queue->header($entry);
    }

    /* Back is posted from the header display page */
    if(isset($_POST['back'])){
      $this->disp_header = false;
    }

    /* If there is a header in disp_header, then display it */
    if($this->disp_header){
      $smarty->assign("header",$this->disp_header);
      return ($smarty->fetch (get_template_path('header.tpl', TRUE)));
    }


    /******************
      Query mailqueues 
     ******************/

    if($this->Server == "all"){
      $entries = array();
      foreach($this->ServerList as $mac => $name){
        if(!tests::is_mac($mac)) continue;
        $entries = array_merge($entries,$this->si_queue->query_mailqueue($mac,$this->OrderBy,$this->SortType,$this->range));
        if($this->si_queue->is_error()){
          unset($this->ServerList[$mac]);
          msg_dialog::display(_("Error"),sprintf(_("The server '%s' was removed from the server list, due to communication problems with the GOsa daemon, the error was: %s"),$name,"<b>".msgPool::siError($this->si_queue->get_error()."</b>")),ERROR_DIALOG);
        }
      }
    }else{
      $entries = $this->si_queue->query_mailqueue($this->Server,$this->OrderBy,$this->SortType,$this->Limit);
      if($this->si_queue->is_error()){
        msg_dialog::display(_("Error"),msgPool::siError($this->si_queue->get_error()),ERROR_DIALOG);
      }
    }

    
    /******************
      create html output 
     ******************/

    $smarty->assign("all_ok"        , count($entries));
    $smarty->assign("entries"       , $entries);
    $smarty->assign("plug"          , "?plug=".$_GET['plug']);

    $smarty->assign("r_stats"       , $this->getStats());
    $smarty->assign("stats"         , array_flip($this->getStats()));

    $smarty->assign("stat"          , $this->Stat);
    $smarty->assign("p_server"      , $this->Server);
    $smarty->assign("p_servers"     , $this->ServerList);
    $smarty->assign("p_serverKeys"  , array_flip($this->ServerList));
    $smarty->assign("p_time"        , $this->Time);
    $smarty->assign("p_times"       , $this->getTimes());
    $smarty->assign("p_timeKeys"    , array_flip($this->getTimes()));
    $smarty->assign("search_for"    , $this->Search);
    $smarty->assign("range_selector", range_selector(count($entries), $this->Page, $this->range,"EntriesPerPage")); 
    $smarty->assign("OrderBy"       , $this->OrderBy);

    /* Display sort arrow */
    if($this->SortType == "up"){
      $smarty->assign("SortType","<img src='images/lists/sort-up.png' alt='"._("up")."' border='0'>");
    }else{
      $smarty->assign("SortType","<img src='images/lists/sort-down.png' alt='"._("down")."' border='0'>");
    }
    return ($smarty->fetch (get_template_path('contents.tpl', TRUE)));
  }


  /* return selectable server 
   */
  function getServer()
  {
    $ret= array("all"=>_("All"));

    /* First of all, detect all servers that supports the mailqueue extension 
        -If this fails, the mailqueue(s) can't be queried.
     */
    $hosts          = $this->si_queue->get_hosts_with_module("mailqueue_com");
    $this->si_error = $this->si_queue->is_error();
    if(!count($hosts)){
      return(array());
    }    

    /* Create search filter and try to resolv mac to hostname 
     */
    $filter = "";
    foreach($hosts as $mac){
      $filter .= "(macAddress=".$mac.")";
    }
    $filter = "(&(objectClass=GOhard)(|".$filter."))";
    $res = get_list($filter,"no_acls",$this->config->current['BASE'],
        array("cn","macAddress"),GL_SUBSEARCH | GL_NO_ACL_CHECK); 

    /* Create result array 
     */
    foreach($hosts as $mac){
      $found = FALSE;
      foreach($res as $entry){
        if(preg_match("/^".normalizePreg($mac)."$/i",$entry['macAddress'][0])){
          $ret[$mac] = $entry['cn'][0];
          $found = TRUE;
          break;
        }
      }
      if(!$found){
        $ret[$mac] = $mac;
      }
    }
    $this->ServerList = $ret;
  }


  /* Return selectable times*/
  function getTimes()
  {
    $ret = array();
    $ret['nolimit']=_("no limit"); 
    foreach(array(1,2,4,8,12,24,36,48) as $i){
      if($i == 1){
        $ret[$i] = $i."&nbsp;"._("hour");
      }else{
        $ret[$i] = $i."&nbsp;"._("hours");
      }
    }
    return($ret);
  }


  /* Save post values*/
  function save_object($save_current= FALSE)
  {
    /* Update amount of entries displayed */
    if(isset($_POST['EntriesPerPage'])){
      $this->range = $_POST['EntriesPerPage'];
    }

    if(isset($_POST['p_server']) && isset($this->ServerList[$_POST['p_server']])){
      $this->Server = $_POST['p_server'];
    }

    if(isset($_POST['p_time'])){
      $this->Time = $_POST['p_time'];
    }
    if(isset($_POST['search_for'])){
      $this->Search = $_POST['search_for'];
    }
    if(isset($_POST['Stat'])){
      $this->Stat = $_POST['Stat'];
    }
    if((isset($_GET['start']))&&(is_numeric($_GET['start']))&&($_GET['start']>=0)){
      $this->Page = $_GET['start'];
    }

    if((isset($_GET['sort']))&&(!empty($_GET['sort']))){
      $old = $this->OrderBy;
      $this->OrderBy = $_GET['sort'];
      if($this->OrderBy == $old)
      {
        if($this->SortType== "up"){
          $this->SortType = "down";
        }else{
          $this->SortType = "up";
        }
      }
    }

  }

  /* Return stats */
  function getStats()
  {
    return(array(
          "all"     =>_("All"),
          "hold"    =>_("Hold"),
          "unhold"  =>_("Un hold"),
          "active"  =>_("Active"),
          "nonactive"  =>_("Not active")
          ));
  }

  /* Return plugin informations for acl handling
     #FIXME You can only read attributes within this report plugin */
  static function plInfo()
  {
    return (array(
        "plShortName"   => _("Mail queue"),
        "plDescription" => _("Mail queue addon"),
        "plSelfModify"  => FALSE,
        "plDepends"     => array(),
        "plPriority"    => 1,
        "plSection"     => array("addon"),
        "plCategory"    => array("mailqueue" => array("objectClass" => "none", "description" => _("Mail queue addon"))),

        "plProvidedAcls" => array(
            "unhold_all"      => _("Unhold all messages"),
            "hold_all"        => _("Hold all messages"),
            "del_all"         => _("Delete all messages"),
            "requeue_all"     => _("Requeue all messages"),
            "unhold"          => _("Unhold message"),
            "hold"            => _("Hold message"),
            "del"             => _("Delete message"),
            "requeue"         => _("Requeue message"),
            "query"           => _("Gathering queue data"),
            "header"          => _("Get header information")
          )
        ));
  }



}
// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
