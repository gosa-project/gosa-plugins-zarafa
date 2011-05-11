<?php

class AddPartitionDialog
{
    public $partitionObject;
    public $disks = array();

    public $size = 1000;
    public $fsType = 'ext3';
    public $mountPoint = '';
    public $usedDisks = array();
    public $forcePrimary = FALSE;
    public $encrypt = FALSE;
    public $size_max_value = 1000;
    public $size_options = 0;
    

    public $attributes = array("size", "fsType", "mountPoint", "forcePrimary", "encrypt", 
            "size_options", "size_max_value");

    function __construct($config, $partitionObject)
    {
        $this->partitionObject = &$partitionObject;
        $this->config = &$config;

        // Prepare filesystem types
        $this->fsTypes = array();
        $this->fsTypes['ext2'] = 'ext2';
        $this->fsTypes['ext3'] = 'ext3';
        $this->fsTypes['ext4'] = 'ext4';
        $this->fsTypes['pv'] = _('Physical volume (LVM)');
        $this->fsTypes['raid'] = _('Software raid');
        $this->fsTypes['swap'] = _("Swap");
        $this->fsTypes['vfat'] = "vfat";
        $this->fsTypes['cfs'] = "xfs";

        // Load selectable disks
        $disks = $this->partitionObject->getDisks();
        $this->disks = array();
        foreach($disks as $disk){
            $this->disks[$disk['device']] = $disk['device'];
        }
        if(!count($this->usedDisks)){
            $this->usedDisks[] = key($this->disks);
        }

    }
    
    function execute()
    {
        $smarty = get_smarty();
        foreach($this->attributes as $attr){
            $smarty->assign($attr, $this->$attr);
        }
        $used = array();
        foreach($this->disks as $disk){
            $used[$disk] = in_array($disk, $this->usedDisks);
        }
        $smarty->assign("disk_selected", $used);

        foreach(array("forcePrimary", "encrypt") as $attr){
            $smarty->assign("{$attr}_selected", $this->$attr != FALSE);
        }
        $smarty->assign('fsTypes', $this->fsTypes);
        $smarty->assign('size', $this->size);
        $smarty->assign('disks', $this->disks);

        return($smarty->fetch(get_template_path("goto/Device/AddPartitionDialog.tpl", TRUE)));
    }

    function save_object()
    {
        // Get posted string values 
        foreach($this->attributes as $attr){
            if(isset($_POST[$attr])){
                $this->$attr = get_post($attr);
            }
        }
    
        // Get boolean values
        foreach(array("forcePrimary", "encrypt") as $attr){
            $this->$attr = isset($_POST[$attr]);
        }

        // Get selected disks
        $this->usedDisks = array();
        foreach($this->disks as $disk){
            if(isset($_POST['disk_selected_'.$disk])){
                $this->usedDisks[] = $disk;
            }
        }
    }

    function save()
    {
        // Get all currently used partitions
        $partitions = $this->partitionObject->getPartitions();
        $raids = $this->partitionObject->getRaidDevices();
        $usedTargets = array();
        foreach($partitions as $part){
            $usedTargets[] = $part['target'];
        }
        foreach($raids as $part){
            $usedTargets[] = $part['target'];
        }

        // We've to create a raid disk
        if($this->fsType == "raid"){
            $target="raid.";
            $id = 0;
            while($id < 100 && in_array($target.str_pad($id, 2, '0', STR_PAD_LEFT)  , $usedTargets)){
                $id ++;
            }
            $target = $target.str_pad($id, 2, '0', STR_PAD_LEFT);
            
            // Get first selected disk
            $disk = array_shift($this->usedDisks);
            $this->partitionObject->addPartition($target,$this->size +0, NULL, FALSE, TRUE, FALSE, FALSE, 
                    NULL, NULL , FALSE, NULL, $disk);  
            return($this->partitionObject->success());
        }else

        // We've to create a raid disk
        if($this->fsType == "pv"){
            $target="pv.";
            $id = 0;
            while($id < 100 && in_array($target.str_pad($id, 2, '0', STR_PAD_LEFT)  , $usedTargets)){
                $id ++;
            }
            $target = $target.str_pad($id, 2, '0', STR_PAD_LEFT);
            
            // Get first selected disk
            $disk = array_shift($this->usedDisks);
            $this->partitionObject->addPartition($target,$this->size +0, NULL, FALSE, TRUE, FALSE, FALSE, 
                    NULL, NULL , FALSE, NULL, $disk);  
            return($this->partitionObject->success());
        }else{
        
            // Add normal-physical partition
            $target = $this->mountPoint;
            $disk = array_shift($this->usedDisks);
            $this->partitionObject->addPartition($target,$this->size +0, NULL, FALSE, TRUE, FALSE, FALSE, 
                    NULL, NULL , FALSE, NULL, $disk);  
            return($this->partitionObject->success());
        }
    }
}
?>
