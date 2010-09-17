<?php

class DeviceTab extends tabs
{
    function save($ignore_account= FALSE)
    {
        // Prepare object DN
        $baseobject= $this->by_object['Device'];
        $uuid    = preg_replace('/,/', '\,', $baseobject->deviceUUID);

        $rdn = get_ou('Device','DeviceRDN');
        if(empty($uuid)){
            $cn      = preg_replace('/,/', '\,', $baseobject->cn);
            $cn      = preg_replace('/"/', '\"', $cn);
            $this->dn= "cn=".$cn.",".$rdn.$baseobject->base;
        }else{
            $this->dn= "deviceUUID=".$uuid.",".$rdn.$baseobject->base;
        }
        $baseobject->dn= $this->dn;

        // Object moved?
        if($this->dn != $baseobject->orig_dn && $baseobject->orig_dn != "new"){
            $baseobject->update_acls($baseobject->orig_dn,$this->dn);
            $baseobject->move($baseobject->orig_dn,$this->dn);
        }

        // Populate values 
        foreach ($this->by_object as $key => $obj){
            $this->by_object[$key]->dn= $this->dn;
            $this->by_object[$key]->cn= $baseobject->cn;
        }

        // Save now
        tabs::save(TRUE);
    }
}

?>
