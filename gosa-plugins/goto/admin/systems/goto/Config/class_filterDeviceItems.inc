<?php

class filterDeviceItems {

    static function query($base, $scope, $filter, $attributes, $category, $objectStorage= "")
    {
        // Walk through the data array till we reach the correct level, then 
        //  create the list of entries.
        $data = session::get('DEVICE_ITEMS');
        foreach(array_reverse(preg_split("/[,]*cn=/", $base,0,PREG_SPLIT_NO_EMPTY)) as $name){
            $data = $data[$name]['children'];            
        }

        // Add entries 
        $ret = array();
        $filter = preg_replace("/\*/",'.*', $filter);
        foreach($data as $item){
            filterDeviceItems::addEntry($ret, $item, $scope == 'sub', $filter);
        }
        return($ret);
    }

    static function addEntry(&$ret, $item, $recursive = FALSE, $filter)
    {
        if(preg_match("/".$filter."/",$item['name'])){
            $entry = array();
            $entry['dn'] = $item['name'];
            $entry[] = 'dn';
            $entry['cn'] = array($item['name'], 'count' => 1);
            $entry[] = 'cn';
            $entry['description'] = array($item['type'], 'count' => 1);
            $entry[] = 'description';
            $entry['objectClass'] = array('FAKE_OC_DeviceItem','count' => 1);
            $entry[] = 'objectClass';
            $entry['count'] = 4;
            $ret[] = $entry;
        }

        if($recursive && isset($item['children']) && count($item['children'])){
            foreach($item['children'] as $item){
                filterDeviceItems::addEntry($ret, $item, $recursive,$filter);
            }
        }

    }
}

?>
