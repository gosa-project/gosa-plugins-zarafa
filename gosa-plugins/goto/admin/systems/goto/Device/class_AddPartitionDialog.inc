<?php


define('PARTITION' , 1);
define('RAID_DEVICE' , 2);
define('VOLUME_GROUP' , 3);
define('VOLUME' , 4);



class AddPartitionDialog
{
    public $partitionObject;
    public $disks = array();
    public $selected_type = PARTITION;

    // Partition properties
    public $p_size = 1000;
    public $p_fsType = 'ext3';
    public $p_mountPoint = '';
    public $p_used_disk = array();
    public $p_forcePrimary = FALSE;
    public $p_encrypt = FALSE;
    public $p_size_max_value = 1000;
    public $p_size_options = 0;
    
    // Attributes managed by this plugin.
    public $attributes = array("selected_type");
    public $p_attributes = array("p_size", "p_fsType", "p_mountPoint", "p_forcePrimary", "p_encrypt", 
            "p_size_options", "p_size_max_value", "p_used_disk");

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
    
        // Select first disk as default.    
        $this->p_used_disk = key($this->disks);
    }
    

    /*! \brief     Generates the HTML output for this plugin. 
     *  @return    String   HTML content of the plugin. 
     */
    function execute()
    {
        $smarty = get_smarty();
        
        // Assign base attributes 
        foreach($this->attributes as $attr){
            $smarty->assign($attr, $this->$attr);
        }

        // Assign partition attributes.
        $attrs = $bool_attrs = array();
        switch($this->selected_type){
            case PARTITION: {
                    $attrs = $this->p_attributes;
                    $bool_attrs = array("p_forcePrimary", "p_encrypt");
                    break;
                }
        }

        // Assign properties to smarty.
        foreach($attrs as $attr){
            $smarty->assign($attr, $this->$attr);
        }
        foreach($bool_attrs as $attr){
            $smarty->assign("{$attr}_selected", $this->$attr != FALSE);
        }
        $smarty->assign('fsTypes', $this->fsTypes);
        $smarty->assign('disks', $this->disks);
        return($smarty->fetch(get_template_path("goto/Device/AddPartitionDialog.tpl", TRUE)));
    }


    /*! \brief     Saves posted values. 
     */
    function save_object()
    {
        // Assign partition attributes.
        $attrs = $bool_attrs = array();
        switch($this->selected_type){
            case PARTITION: {
                    $attrs = $this->p_attributes;
                    $bool_attrs = array("p_forcePrimary", "p_encrypt");
                    break;
                }
        }

        // Get posted string values 
        $attrs = array_merge($attrs, $this->attributes);
        foreach($attrs as $attr){
            if(isset($_POST[$attr])){
                $this->$attr = get_post($attr);
            }
        }
    
        // Get boolean values
        foreach($bool_attrs as $attr){
            $this->$attr = isset($_POST[$attr]);
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
        $size = $this->p_size;
        $maxSize = NULL;
        if($this->p_size_options == 2){
            $maxSize = $this->p_size_max_value;
        }
        $grow = $this->p_size_options == 1;
        $format = FALSE;
        $boot = FALSE;
        $primary = $this->p_forcePrimary;
        $fsType = $this->p_fsType;
        $fsOptions = "";
        $encrypt = $this->p_encrypt;
        $passphrase = "";
        $disk = $this->p_used_disk;

        // We've to create a raid disk
        if($this->p_fsType == "raid"){
            $target="raid.";
            $id = 0;
            while($id < 100 && in_array($target.str_pad($id, 2, '0', STR_PAD_LEFT)  , $usedTargets)){
                $id ++;
            }
            $target = $target.str_pad($id, 2, '0', STR_PAD_LEFT);
            $fsType = $fsOptions = NULL;
        }else

        // We've to create a raid disk
        if($this->p_fsType == "pv"){
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
