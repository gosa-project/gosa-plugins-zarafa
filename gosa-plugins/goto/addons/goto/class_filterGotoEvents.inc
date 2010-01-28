<?php
class filterGotoEvents extends  filterLDAP {
  static function query($base, $scope, $filter, $attributes, $category, $objectStorage= "")
  {


    $o_queue = new gosaSupportDaemon(TRUE,5);
    $events  = DaemonEvent::get_event_types( SYSTEM_EVENT);


    $entries = $o_queue->get_queued_entries($event_tags,0,9999999,"id");
    if ($o_queue->is_error()){
      msg_dialog::display(_("Error"), sprintf(_("Cannot load queue entries: %s"), "<br><br>".$o_queue->get_error()), ERROR_DIALOG);
    }

    /* Assign entries by id.
     */
    $data= array();
    foreach($entries as $entry){

      $item = array();
      
      $item['count'] = 0;
      $item['dn']= implode($entry,", item=");

      foreach($entry as $name => $value){
        $item[] = $name;
        $item[$name]=array('count' => 1, $value);
        $item['count'] ++;
      }


#     /* Skip entries which will be removed within the next seconds */
#     if(isset($entry['MACADDRESS']) && in_array($entry['MACADDRESS'],$this->recently_removed)){
#       continue;
#     }
      $data[] = $item;
    }
    return($data);
  }
}
?>
