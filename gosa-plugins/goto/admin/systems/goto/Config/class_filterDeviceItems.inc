<?php

class filterDeviceItems {

    static function query($base, $scope, $filter, $attributes, $category, $objectStorage= "")
    {
        print_a(array($base, $scope, $filter, $attributes, $category, $objectStorage));
        $ret = array();

        $entry = array();
        $entry['dn'] = 'cn=test,cn=/';
        $entry[] = 'dn';
        $entry['cn'] = array('test', 'count' => 1);
        $entry[] = 'cn';
        $entry['description'] = array('test-item', 'count' => 1);
        $entry[] = 'description';
        $entry['objectClass'] = array('FAKE_OC_DeviceItem','count' => 1);
        $entry[] = 'objectClass';
        $entry['count'] = 4;


        $ret[] = $entry;
        return($ret);
    }
}

?>
