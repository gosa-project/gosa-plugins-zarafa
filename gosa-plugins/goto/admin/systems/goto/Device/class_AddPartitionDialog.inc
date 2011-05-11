<?php

class AddPartitionDialog
{
    public $partitionObject;
    public $disks = array();

    // Partition properties
    public $size = 1000;
    public $fsType = 'ext3';
    public $mountPoint = '';
    public $usedDisks = array();
    public $forcePrimary = FALSE;
    public $encrypt = FALSE;
    public $size_max_value = 1000;
    public $size_options = 0;
    
    // Attributes managed by this plugin.
    public $attributes = array("size", "fsType", "mountPoint", "forcePrimary", "encrypt", 
            "size_options", "size_max_value");


    /*! \brief  Constructs the Dialog and loads all required informations
     *          to be able to add partitions, raid devices, volumes groups ...
     *  @param  Config          The GOsa configuration object.
     *  @param  remoteObject    The remote partition object.
     */
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
    

    /*! \brief     Generates the HTML output for this plugin. 
     *  @return    String   HTML content of the plugin. 
     */
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


    /*! \brief     Saves posted values. 
     */
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


    /*! \brief     Stores the changes back to the remote table model. 
     *  @return    TRUE on success else false.
     */
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

        // Collect options
        $size = $this->size;
        $maxSize = NULL;
        if($this->size_options == 2){
            $maxSize = $this->size_max_value;
        }
        $grow = $this->size_options == 1;
        $format = FALSE;
        $boot = FALSE;
        $primary = $this->forcePrimary;
        $fsType = $this->fsType;
        $fsOptions = "";
        $encrypt = $this->encrypt;
        $passphrase = "";
        $disk = array_shift($this->usedDisks);

        // We've to create a raid disk
        if($this->fsType == "raid"){
            $target="raid.";
            $id = 0;
            while($id < 100 && in_array($target.str_pad($id, 2, '0', STR_PAD_LEFT)  , $usedTargets)){
                $id ++;
            }
            $target = $target.str_pad($id, 2, '0', STR_PAD_LEFT);
            $fsType = $fsOptions = NULL;
        }else

        // We've to create a raid disk
        if($this->fsType == "pv"){
            $target="pv.";
            $id = 0;
            while($id < 100 && in_array($target.str_pad($id, 2, '0', STR_PAD_LEFT)  , $usedTargets)){
                $id ++;
            }
            $target = $target.str_pad($id, 2, '0', STR_PAD_LEFT);
            $fsType = $fsOptions = NULL;
        }else{
        
            // Add normal-physical partition
            $target = $this->mountPoint;
            if($fsType == "swap"){
                $target = "swap";
            }
        }

        // Add partition to remote model
        $this->partitionObject->addPartition($target,$size, $maxSize, $grow, $format, $boot, $primary,
                $fsType, $fsOptions , $encrypt, $passphrase, $disk);
        return($this->partitionObject->success());
    }
}
?>
