<?php

class DevicePartition 
{
    public $object =NULL;
    public $partitionString =NULL;

    public $addDialog = NULL;
    public $initialized = FALSE;

    function __construct($config, $partitionString)
    {
        $this->config = &$config;
        $this->partitionString = $partitionString;

       #$this->partitionString = "disk sda --initlabel --none;
       #        part /boot --size 1000 --format --fstype ext3 --ondisk sda;
       #        part /test --size 1000 --fstype ext3 --ondisk sda;
       #        part pv.00 --size 10 --bootable --ondisk sda;
       #        part raid.00 --size 5000 --bootable --ondisk sda;
       #        part pv.01 --size 10009 --ondisk sda;
       #        part raid.01 --size 1000 --bootable --ondisk sda;
       #        part pv.03 --size 1000 --bootable --ondisk sda;
       #        part raid.02 --size 981 --bootable --ondisk sda;
       #        raid pv.02 --level 0 --name md0 --format raid.01 raid.00;
       #        volgroup garnele --format pv.01 pv.00 pv.02;
       #        logvol /homejj --size 8978 --format --fstype ext3 --name garnele_home --vgname garnele;
       #        logvol /opt --size 7041 --format --fstype ext3 --name garnele_opt --vgname garnele;";

        // Prepare lists
        $this->entryList = new sortableListing();
        $this->entryList->setDeleteable(false);
        $this->entryList->setEditable(false);
        $this->entryList->setWidth("100%");
        $this->entryList->setHeight("400px");
        $this->entryList->setHeader(array(_("Type"),_("Target"),_("Size"),_("Uses device")." / "._("Used by"),
                    _("Filesystem"), _("Options"), _("Details"), "-"));
        $this->entryList->setColspecs(array('*','*','*', '*'));
        $this->entryList->setAcl('rwcdm');
        $this->entryList->setReorderable(FALSE);
        $this->entryList->sortingEnabled(FALSE);
    }

    function execute()
    {
        /*****
         * Handle add-partition dialog
         *****/

        // Create button was pressed, show dialog to add new partitions
        if(isset($_POST['create_partition'])){
            $this->addDialog = new AddPartitionDialog($this->config, $this->object); 
        }

        // Abort partition add
        if(isset($_POST['cancel_partition_add'])){
            $this->addDialog = NULL;
        }

        // Save created partitions
        if(isset($_POST['save_partition_add'])){
            $this->addDialog->save_object();
            if($this->addDialog->save()){
                $this->addDialog = NULL;
            }
        }

        // Show partition dialog 
        if($this->addDialog){
            $this->addDialog->save_object();
            return($this->addDialog->execute());
        }


        /*****
         * Open partition object on demand
         *****/

        // Open remote parition object
        if(!$this->initialized){
            $rpc = $this->config->getRpcHandle();
            $this->object = $rpc->openObject('libinst.preseed.diskdefinition', $this->partitionString);
            if(!$rpc->success()){
                $message = _("An error occured while loading partition information.");
                $message = sprintf(_("An error occured while loading partition information, error was: '%s'."), $rpc->get_error());
                $smarty = get_smarty();
                $smarty->assign('error', TRUE);
                $smarty->assign('errorMsg', $message);
                return($smarty->fetch(get_template_path('goto/Device/DevicePartition.tpl', TRUE)));
            }else{
                $this->initialized = TRUE;
            }
        }


        /*****
         * Fill listing
         *****/

        // Receive list informations
        $map = array();
        $map['disks'] = 'getDisks';
        $map['partitions'] = 'getPartitions';
        $map['raids'] = 'getRaidDevices';
        $map['volumeGroups'] = 'getVolumeGroups';
        $map['volumes'] = 'getVolumes';
        $map['deviceUsage'] = 'getDeviceUsage';
        foreach($map as $target => $func){
            $$target = @$this->object->$func();
            if(!$this->object->success()){
                $message = sprintf(_("An error occured while loading partition information, error was: '%s'."), $this->object->getError());
                $smarty = get_smarty();
                $smarty->assign('error', TRUE);
                $smarty->assign('errorMsg', $message);
                return($smarty->fetch(get_template_path('goto/Device/DevicePartition.tpl', TRUE)));
            }
        }

        // Create a mapping that show which partitions was used for which raid.
        $map_partToRaid = array();
        $raidTargets = array();
        foreach($raids as $raid){
            $raidTargets[] = $raid['target'];
            foreach($raid['devices'] as $devName){
                $map_partToRaid[$devName] = $raid['name'];
            }
        }
        
        // Create a mapping that shows which partition was used in which volumeGroup.
        $map_partToVolumeGroup = array();
        foreach($volumeGroups as $vol){
            foreach($vol['partitions'] as $part){
                $map_partToVolumeGroup[$part] = $vol['name'];
            }
        }

        // Create partition name map        
        $partNames = array();

        // Create separators to create a visible indentation 
        $seps = array();
        $seps[0] = "";
        $seps[1] = str_pad("", 18, "&nbsp;");
        $seps[2] = str_pad("", 36, "&nbsp;");

        // Prepare images
        $partitionImg = image('plugins/goto/images/partition.png','',_("Partition"));
        $diskImg = image('plugins/goto/images/disk.png','',_("Disk"));
        $raidImg = image('plugins/goto/images/raid.png', '', _('Raid'));
        $volumeGroupImg = image('plugins/goto/images/volumeGroups.png', '', _('Volume group'));
        $volumeImg = image('plugins/goto/images/volume.png','',_('Volume'));

        // Add raid devices
        $lData = array();
        if(count($raids)){
        
            // Add raid header
            $lData[] = array("data" => array("<b>"._("Raid devices")."</b>"));
    
            // Add raids
            foreach($raids as $id =>  $raid){
                $str = "{$seps[1]}{$raidImg}&nbsp;{$raid['name']} ("._("Raid")."{$raid['level']})";
                $delImg = image('images/lists/trash.png', "delete_raid_{$id}", _("Remove raid device"));

                $devsStr = "";
                foreach($raid['devices'] as $devName){
                    $devsStr .= $partitionImg."&nbsp;".$devName.", ";
                }
                $devsStr = rtrim($devsStr, ', ');

                $target = $raid['target'];
                if(isset($map_partToVolumeGroup[$target])){
                    $target = $volumeGroupImg."&nbsp;".$target;
                }

                $fsType = (isset($raid['fsType'])) ? $raid['fsType'] :"";
                $fsOptions = (isset($raid['fsOptions'])) ? $raid['fsOptions'] :"";

                $size = "";
                if(isset($deviceUsage['raid'][$raid['target']])){
                    $size = "<i>".$this->__convertPartSize($deviceUsage['raid'][$raid['target']]['size'])."</i>";
                }

                $lData[] = array("data" => array($str, 
                    $target,
                    $size,
                    $devsStr,
                    $fsType,
                    $fsOptions,
                    '',
                    $delImg
                    ));
            }
        }

        // Add volume groups
        if(count($volumeGroups)){
        
            // Add LVM volume groups header
            $lData[] = array("data" => array("<b>"._("LVM Volume Groups")."</b>"));
    
            // Add volume groups
            foreach($volumeGroups as $id =>  $vg){
                $str = "{$seps[1]}{$volumeGroupImg}&nbsp;{$vg['name']}";
                $delImg = image('images/lists/trash.png', "delete_vg_{$id}", _("Remove volume group"));

                // Build up a list of all used partitions
                $partStr = "";
                foreach($vg['partitions'] as $partName){
                    if(in_array_strict($partName, $raidTargets)){
                        $img = $raidImg;
                    }else{
                        $img = $partitionImg;
                    }
                    $partStr .= "{$img}&nbsp;{$partName}, ";
                }
                $partStr = rtrim($partStr, ", ");

                $size = "";
                if(isset($deviceUsage['vg'][$vg['name']])){
                    $size = "<b>".$this->__convertPartSize($deviceUsage['vg'][$vg['name']]['size'])."</b>";
                }

                // Add entry to the list.
                $lData[] = array("data" => array($str, '', $size, $partStr, '', '','',$delImg));

                // Add volumes 
                foreach($volumes as $vid => $volume){
                    $delImg = image('images/lists/trash.png', "delete_v_{$vid}", _("Remove volume"));
                    if($volume['volGroup'] == $vg['name']){
                        $str = "{$seps[2]}{$volumeImg}&nbsp;{$volume['name']}";
                        $lData[] = array("data" => array(
                                    $str, 
                                    $volume['target'],
                                    $this->__convertPartSize($volume['size']),
                                    '',
                                    $volume['fsType'],
                                    $volume['fsOptions'],
                                    '',
                                    $delImg
                                    ));

                        

                    }
                }
            }
        }

        // Collect all disk devices names
        $diskNames = array();
        foreach($disks as $disk){
            $diskNames[] = $disk['device'];
        }

        // Add physical disks to the listing.
        if(count($disks)){
        
            // Add disks
            $lData[] = array("data" => array("<b>"._("Disks")."</b>"));
            foreach($disks as $id =>  $disk){
                $str = "{$seps[1]}{$diskImg}&nbsp;{$disk['device']}";

                $size = "";
                if(isset($deviceUsage['disk'][$disk['device']])){
                    $size = "<b>".$this->__convertPartSize($deviceUsage['disk'][$disk['device']]['size'])."</b>";
                }

                // Create remove icon
                $delImg = image('images/lists/trash.png', "delete_disk_{$id}", _("Remove disk"));
                $lData[] = array("data" => array($str,"",$size,"","","","", $delImg));

                // Add partitions
                foreach($partitions as $pid => $part){

                    if(!in_array_strict($part['onDisk'], $diskNames)){
                        
                        echo "Cannot display '{$part['target']}' the given disk '{$part['onDisk']}' does not exists!<br>";
                        continue;
                    }

                    if($part['onDisk'] == $disk['device']){
                    
                        // Prepare columns
                        $str = "{$seps[2]}{$partitionImg}&nbsp;{$part['target']}";
                        $device = "";
                        $fsType = $part['fsType'];
                        $target = "";
                        if(preg_match("/^(swap|\/)/", $part['target'])){
                            $target = $part['target'];
                        }

                        // Do we have a raid here? Then update the device column 
                        //  to point to the raid name
                        if(isset($map_partToRaid[$part['target']])){
                            $device = sprintf(_("Used by: %s"), $raidImg."&nbsp;".$map_partToRaid[$part['target']]);
                            $fsType.= " "._("Software raid");
                            $fsType = trim($fsType); 
                        }elseif(isset($map_partToVolumeGroup[$part['target']])){
                            $device = sprintf(_("Used by: %s"), $volumeGroupImg."&nbsp;".$map_partToVolumeGroup[$part['target']]);
                            $fsType.= " "._("Physical volume LVM");
                            $fsType = trim($fsType); 
                        }

                        // Create property icons
                        $emptyImage = image("images/empty.png");
                        $formatImg = ($part['format']) ? image('plugins/goto/images/formatDisk.png','',_("Format partition")) : $emptyImage;
                        $primaryImg = ($part['primary']) ? image('plugins/goto/images/primary.png','',_("Primary partition")) : $emptyImage;
                        $bootImg = ($part['bootable']) ? image('images/true.png','',_("Bootable")) : $emptyImage;
                        $encryptImg = ($part['encrypted']) ? image('images/lists/locked.png','',_("Encrypted")) : $emptyImage;

                        // Create remove icon
                        $delImg = image('images/lists/trash.png', "delete_part_{$pid}", _("Remove partition"));

                        // Add entry to the listing
                        $lData[] = array("data" => array(
                                    $str, 
                                    $target, 
                                    $this->__convertPartSize($part['size'], $part['grow'], $part['maxSize']),
                                    $device,
                                    $fsType,
                                    $part['fsOptions'],
                                    "{$primaryImg} {$formatImg} {$bootImg} {$encryptImg}",
                                    $delImg));
                    }
                }
            }
        }

        // Updated columns length for all entries to avoid render errors.
        $length = 8;
        foreach($lData as $id => $entry){
            while(count($lData[$id]['data']) < $length){
                $lData[$id]['data'][] = '';
            }
        }
        $this->entryList->setListData($lData, $lData);
        $this->entryList->update();

        $smarty = get_smarty();
        $smarty->assign('error', FALSE);
        $smarty->assign('list', $this->entryList->render());
        return($smarty->fetch(get_template_path('goto/Device/DevicePartition.tpl', TRUE)));
    }


    /*! \brief  Keep posted HTML values and acts on remove requests.
     */
    function save_object()
    {
        // Create a map whcih points to the correct remove method for each device.
        $map = array();
        $map['part'] = array('func' => 'delPartition', 'name' => _("Partition"));
        $map['raid'] = array('func' => 'delRaidDevice', 'name' => _("Raid device"));
        $map['disk'] = array('func' => 'delDisk', 'name' => _("Disk"));
        $map['vg'] = array('func' => 'delVolumeGroup', 'name' => _("Volume group"));
        $map['v'] = array('func' => 'delVolume', 'name' => _("Volume"));

        // Walk through posts and search for remove requests.
        foreach($_POST as $name => $value){
            foreach($map as $type => $data){

                // Remove paritions
                if(preg_match("/^delete_{$type}_[0-9]*$/", $name)){
                    $id = preg_replace("/^delete_{$type}_/i", "", $name) + 0;
                    $func = $data['func'];
                    @$this->object->$func($id);
                    if(!$this->object->success()){
                        $msg = sprintf(_("Failed to remove '%s': %s"), $data['name'], $this->object->getError());
                        msg_dialog::display(_("Error"), $msg, ERROR_DIALOG);
                    }
                }
            }
        }
    }


    /*! \brief  Returns to created partition table string, which can then be
     *          saved back to the server.
     */
    function save()
    {
        return($this->object->dump());
    }


    /*! \brief  Convert a given size value to a human readable format.
     */
    function __convertPartSize($size, $grow = False, $maxSize = NULL)
    {
        $str = $size." "._("MB");

        if($maxSize != NULL){
            $str .= " - ".$this->__convertPartSize($maxSize);
        }elseif($grow){
            $str .= " - ...  <i>("._("growing").")</i>";
        }
        return($str);
    }
}

?>
