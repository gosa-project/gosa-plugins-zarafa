<?php

class filterRepository {

  static function query($base, $scope, $filter, $attributes, $category, $objectStorage= "")
  {
    return(array());
    $list = servRepository::getRepositoryList();

    $data = array();
    foreach($list as $key => $entry){   
      $entry['objectClass'] = array('FAKE_OC_FaiRepository');
      $item = array('count' => count($entry));
      $item['dn'] = $key;
      foreach($entry as $name => $value){   
        $item[] = $name;
        if(!is_array($value)) $value=array($value);
        $value= array_values($value);
        $value['count'] = count($value);
        $item[$name] = $value;
         
        
      }
      $data[] = $item;
    }
    return($data);
  }
}

?>
