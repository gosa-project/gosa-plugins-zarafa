<?php

/*! \brief  This is the opsi base class, it handles
  .          gosa daemon requests and prepares data for opsi plugins.
 */
class si_mailqueue extends gosaSupportDaemon
{
  private $config = NULL;
  protected $use_alternative_xml_parse_method = TRUE;

  /*! \brief    Create opsi object.
    @param
    @return
   */
  public function __construct($config)
  {
    $this->config = $config;
    gosaSupportDaemon::__construct($config);
    $this->target = "00:01:6c:9d:b9:fa";
  }

    
  /*! \brief  Returns TRUE or FALSE, whether the plugin is enabled or disabled .
      @return Boolean  s.a.
   */
  public function enabled()
  {
    return(TRUE);
  }


  /*! \brief  Returns a list of all mail queue entries 
      @return Array   s.a.
   */
  public function query_mailqueue($server,$sortby,$direction)
  {
    $attrs = array("Size","Recipient","Sender","Arrival","MailID","Hold","Active","Error","Server");

    /* Check sorting paramter 
     */
    if(!in_array($sortby,$attrs)){
      trigger_error("Invalid sorting option '".$sortby."'.");
      $sortby = "Arrival";
    }

    /* Check sorting direction 
     */
    if(!in_array($direction,array("down","up"))){
      trigger_error("Invalid sorting direction '".$direction."'.");
      $direction = "down";
    }

    /* Query mailqueue 
     */
    $ids = array();
    $res = $this->send_data("gosa_mailqueue_query",$server,array(),TRUE);
    $items = array(); 
    if(isset($res['XML'][0])){
      foreach($res['XML'][0] as $name => $entry){
        if(preg_match("/^ANSWER[0-9]*$/",$name)){
          $attrs = array(
              "MSG_SIZE"      => "Size",
              "RECIPIENT"     => "Recipient",
              "SENDER"        => "Sender",
              "ARRIVAL_TIME"  => "Arrival",
              "MSG_ID"        => "MailID");  
          $val = array();
          foreach($attrs as $source => $dest){
            $val[$dest] = $entry[0][$source][0]['VALUE'];
          }
          $ids[] = $val['MailID'];
          $attrs = array(  
              "MSG_HOLD"   => "Hold",
              "MSG_ACTIVE" => "Active",
              "ERROR"      => "Error");
          foreach($attrs as $source => $dest){
            if(isset($entry[0][$source][0]['VALUE'])){
              $val[$dest] = $entry[0][$source][0]['VALUE'];
            }else{
              $val[$dest] = FALSE;
            }
          }

          $val['Server'] = $server;
          $val['Arrival'] = strtotime($val['Arrival']);
          $n = $val[$sortby]."-".microtime(1);
          while(isset($items[$n])){
            $n = $val[$sortby]."-".microtime(1);
          }
          $items[$n] = $val;
        }
      }
    }   

    /* Sort entries by given direction 
     */
    if($direction == "down"){
      ksort($items);
    }else{
      krsort($items);
    }
    return($items);
  }


  public function header($msg_id, $server)
  {
    $data = array();
    $data['msg_id'] = $msg_id;
    $res = $this->send_data("gosa_mailqueue_header",$server,$data,TRUE);
  
    $attrs = array(
        "RECIPIENT",
        "SENDER",
        "SUBJECT");

    $data = array();
    if(isset($res['XML'])){
      foreach($attrs as $attr){
        $data[$attr] = "";
        if(isset($res['XML'][0][$attr])){
          $data[$attr] = $res['XML'][0][$attr][0]['VALUE'];
        }
      }  
    }
    return($data);
  }

 
  /*! \brief  Returns a list of all mail queue entries 
      @return Array   s.a.
   */
  public function send_queue_action($msg_ids,$server, $action)
  {
    $data = array();

    /* Check given msg_ids, must be array.
     */
    if(!is_array($msg_ids)){
      trigger_error("Invalid msg_id given. Array expected.");
      return(FALSE);
    }

    /* Check triggered action 
     */
    $allowed_actions = array("hold","unhold","requeue","del");
    if(!in_array($action,$allowed_actions)){
      trigger_error("Unknown queue action triggered '".$action."'. Request aborted.");  
      return(FALSE);
    }    
    
    $data['msg_id'] = $msg_ids;
    $this->send_data("gosa_mailqueue_".$action,$this->target,$data,FALSE);
    // There is no answer for this requests 
  }
}
// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
