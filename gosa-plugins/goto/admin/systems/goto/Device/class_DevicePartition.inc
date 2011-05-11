<?php



class DevicePartition 
{
    public $object =NULL;
    public $partitionString =NULL;

    public $addDialog = NULL;

    function __construct($config, $partitionString)
    {
        $this->config = &$config;
        $this->partitionString = $partitionString;
        $this->init();
    }

    function init()
    {
        $rpc = $this->config->getRpcHandle();
        $this->object = $rpc->openObject('libinst.preseed.diskdefinition', '');

        $this->object->addDisk('sda');
        $this->object->addDisk('sdb');
        $this->object->addDisk('sdc');
        $this->object->addPartition('/kekse', 2333, NULL, FALSE, TRUE, FALSE, FALSE, 'ext3','ro,user,nosuid' , FALSE, NULL, 'sda');
        $this->object->addPartition('/wurst', 2000, NULL, FALSE, TRUE, FALSE, FALSE, NULL, NULL, FALSE, NULL, 'sda');
        $this->object->addPartition('raid.00', 2000, NULL, FALSE, TRUE, FALSE, FALSE, NULL, NULL, FALSE, NULL, 'sda');
        $this->object->addPartition('raid.01', 2000, NULL, FALSE, TRUE, FALSE, FALSE, NULL, NULL, FALSE, NULL, 'sda');
        $this->object->addPartition('pv.00', 2333, NULL, FALSE, TRUE, FALSE, FALSE, NULL, NULL, FALSE, NULL, 'sda');
        $this->object->addRaidDevice('pv.01', 'md0', '0','0', NULL, NULL, TRUE, FALSE, array('raid.00', 'raid.01'));
        $this->object->addVolumeGroup('garnele', array('pv.00', 'pv.01'));
        $this->object->addVolume('/home', 'garnele_home', 'garnele', 2000);

        // Prepare lists
        $this->entryList = new sortableListing();
        $this->entryList->setDeleteable(false);
        $this->entryList->setEditable(false);
        $this->entryList->setWidth("100%");
        $this->entryList->setHeight("400px");
        $this->entryList->setHeader(array(_("Type"),_("Target"),_("Size"),_("Uses device")." / "._("Used by"),_("Filesystem"), _("Options")));
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
        if(isset($_POST['create_partition'])){
            $this->addDialog = new AddPartitionDialog($this->config, $this->object); 
        }
        if(isset($_POST['cancel_partition_add'])){
            $this->addDialog = NULL;
        }
        if(isset($_POST['save_partition_add'])){
            $this->addDialog->save_object();
            if($this->addDialog->save()){
                $this->addDialog = NULL;
            }
        }
        if($this->addDialog){
            $this->addDialog->save_object();
            return($this->addDialog->execute());
        }


        /*****
         * Fill listing
         *****/

        // Receive list informations
        $disks = $this->object->getDisks();
        $partitions = $this->object->getPartitions();
        $raids = $this->object->getRaidDevices();
        $volumeGroups = $this->object->getVolumeGroups();
        $volumes = $this->object->getVolumes();

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
        $partitionImg = image('plugins/goto/images/partition.png');
        $diskImg = image('plugins/goto/images/disk.png');
        $raidImg = image('plugins/goto/images/raid.png');
        $volumeGroupImg = image('plugins/goto/images/volumeGroups.png');
        $volumeImg = image('plugins/goto/images/volume.png');

        // Add raid devices
        $lData = array();
        if(count($raids)){
        
            // Add raid header
            $lData[] = array("data" => array("<b>"._("Raid devices")."</b>"));
    
            // Add raids
            foreach($raids as $id =>  $raid){
                $str = "{$seps[1]}{$raidImg}&nbsp;{$raid['name']} ({$raid['level']})";

                $devsStr = "";
                foreach($raid['devices'] as $devName){
                    $devsStr .= $partitionImg."&nbsp;".$devName.", ";
                }
                $devsStr = rtrim($devsStr, ', ');

                $target = $raid['target'];
                if(isset($map_partToVolumeGroup[$target])){
                    $target = $volumeGroupImg."&nbsp;".$target;
                }

                $lData[] = array("data" => array($str, 
                    $target,
                    '',
                    $devsStr,
                    $raid['fsType'],
                    $raid['fsOptions']
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

                // Build up a list of all used partitions
                $partStr = "";
                foreach($vg['partitions'] as $partName){
                    if(in_array($partName, $raidTargets)){
                        $img = $raidImg;
                    }else{
                        $img = $partitionImg;
                    }
                    $partStr .= "{$img}&nbsp;{$partName}, ";
                }
                $partStr = rtrim($partStr, ", ");

                // Add entry to the list.
                $lData[] = array("data" => array($str, '', '', $partStr));

                // Add volumes 
                foreach($volumes as $volume){
                    if($volume['volGroup'] == $vg['name']){
                        $str = "{$seps[2]}{$volumeImg}&nbsp;{$volume['name']}";
                        $lData[] = array("data" => array(
                                    $str, 
                                    $volume['target'],
                                    $this->__convertPartSize($volume['size']),
                                    '',
                                    $volume['fsType'],
                                    $volume['fsOptions']
                                    ));

                        

                    }
                }
            }
        }

        // Add physical disks to the listing.
        if(count($disks)){
        
            // Add disks
            $lData[] = array("data" => array("<b>"._("Disks")."</b>"));
            foreach($disks as $id =>  $disk){
                $str = "{$seps[1]}{$diskImg}&nbsp;{$disk['device']}";
                $lData[] = array("data" => array($str));

                // Add partitions
                foreach($partitions as $pid => $part){

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

                        // Add entry to the listing
                        $lData[] = array("data" => array(
                                    $str, 
                                    $target, 
                                    $this->__convertPartSize($part['size']),
                                    $device,
                                    $fsType,
                                    $part['fsOptions']));
                    }
                }
            }
        }

        // Updated columns length for all entries to avoid render errors.
        $length = 6;
        foreach($lData as $id => $entry){
            while(count($lData[$id]['data']) < $length){
                $lData[$id]['data'][] = '';
            }
        }
        $this->entryList->setListData($lData, $lData);
        $this->entryList->update();

        $smarty = get_smarty();
        $smarty->assign('list', $this->entryList->render());
        return($smarty->fetch(get_template_path('goto/Device/DevicePartition.tpl', TRUE)));
    }

    function save_object()
    {

    }

    function save()
    {
        return($this->object->dump());
    }

    function __convertPartSize($size)
    {
        return($size." MB");
    }
}

?>
