<?php


define('DISK' , 0);
define('PARTITION' , 1);
define('RAID_DEVICE' , 2);
define('VOLUME_GROUP' , 3);
define('VOLUME' , 4);

class AddPartitionDialog
{
    public $partitionObject;
    public $disk = array();
    public $diskList = array();
    public $paritions = array();
    public $selected_type = DISK;

    // Disk properties
    public $d_name = "";

    // LVM Volume group properties
    public $vg_name = "";
    public $vg_partitions = array();
    public $volumeGroups = array();
    public $freeLvmPartitions = array();

    // Volume properties 
    public $v_name = "";
    public $v_group = "";
    public $v_mountPoint = "";
    public $v_fsType = "ext3";
    public $v_fsOptions = "";
    public $v_size = 1000;
    public $v_encrypt = FALSE;
    public $volumeGroupList = array();

    // Raid device properties
    public $r_fsType = 'ext3';
    public $r_fsOptions = '';
    public $r_mountPoint = '';
    public $r_raidLevel = 0;
    public $raidLevelList = array();
    public $raidDevices = array();
    public $freeRaidPartitions = array();
    public $r_partitions = array();
    public $r_spares = 0;
    public $r_encrypt = FALSE;

    // Partition properties
    public $p_size = 1000;
    public $p_format = FALSE;
    public $p_bootable = FALSE;
    public $p_fsType = 'ext3';
    public $p_mountPoint = '';
    public $p_used_disk = array();
    public $p_forcePrimary = FALSE;
    public $p_encrypt = FALSE;
    public $p_size_max_value = 1000;
    public $p_size_options = 0;
    
    // Attributes managed by this plugin.
    public $attributes = array("selected_type");

    // Partitions attributes
    public $p_attributes = array("p_size", "p_fsType", "p_mountPoint", "p_forcePrimary", "p_encrypt", 
            "p_size_options", "p_size_max_value", "p_used_disk", "p_format", "p_bootable");

    // Raid device attributes
    public $r_attributes = array("r_fsType", "r_mountPoint", "r_raidLevel", "r_partitions", 
            "r_spares","r_encrypt", "r_fsOptions");

    // Volume group attributes
    public $vg_attributes = array("vg_name", "vg_partitions");

    // Volume  attributes
    public $v_attributes = array("v_name", "v_group", "v_mountPoint", "v_fsType", 
            "v_size", "v_encrypt", "v_fsOptions");

    // Disk  attributes
    public $d_attributes = array("d_name");

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
        foreach($this->partitionObject->getFsTypes() as $type){
            $this->fsTypes[$type] = $type;
        }
        $this->fsTypes['pv'] = _('Physical volume (LVM)');
        $this->fsTypes['raid'] = _('Software raid');

        // Prepare list of available raid level
        $this->raidLevelList = array();
        foreach($this->partitionObject->getRaidLevels() as $lvl){
            $this->raidLevelList[$lvl] = sprintf(_("Raid %s"), $lvl);
        }

        // Load values from backend
        $this->disks = $this->partitionObject->getDisks();
        $this->partitions = $this->partitionObject->getPartitions();
        $this->raidDevices = $this->partitionObject->getRaidDevices();
        $this->volumeGroups = $this->partitionObject->getVolumeGroups();
        $this->freeLvmPartitions = $this->partitionObject->getUnassignedPhysicalVolumes();
        $this->freeRaidPartitions = $this->partitionObject->getUnassignedRaidPartitions();
        foreach($this->volumeGroups as $vg){
            $this->volumeGroupList[$vg['name']] = $vg['name'];
        }
        $this->diskList = array();
        foreach($this->disks as $disk){
            $this->diskList[$disk['device']] = $disk['device'];
        }
 
        // Select first disk as default.    
        $this->p_used_disk = key($this->diskList);

        // Preselect partition creation if we've already created a disk 
        if(count($this->disks)){
            $this->selected_type = PARTITION;
        }
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
        $fsTypes = $this->fsTypes;
        $attrs = $bool_attrs = array();
        switch($this->selected_type){
            case PARTITION: {
                    $attrs = $this->p_attributes;
                    $bool_attrs = array("p_forcePrimary", "p_encrypt", "p_format", "p_bootable");
                    break;
                }
            case RAID_DEVICE: {
                    $attrs = $this->r_attributes;
                
                    // Do not allow to create raid devices in raid devices.
                    unset($fsTypes['raid']);
                    $bool_attrs = array("r_encrypt");
                    break;
                }
            case VOLUME_GROUP: {
                    $attrs = $this->vg_attributes;
                    break;
                }
            case VOLUME: {
                    $attrs = $this->v_attributes;
                    unset($fsTypes['raid']);
                    unset($fsTypes['pv']);
                    $bool_attrs = array("v_encrypt");
                    break;
                }
            case DISK: {
                    $attrs = $this->d_attributes;
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
        
        $smarty->assign("deviceUsage", $this->partitionObject->getDeviceUsage());
        $smarty->assign('fsTypes', $fsTypes);
        $smarty->assign('raidLevelList', $this->raidLevelList);
        $smarty->assign('freeRaidPartitions', $this->freeRaidPartitions);
        $smarty->assign('disks', $this->diskList);
        $smarty->assign('volumeGroupList', $this->volumeGroupList);
        $smarty->assign('freeLvmPartitions', $this->freeLvmPartitions);
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
                    $bool_attrs = array("p_forcePrimary", "p_encrypt", "p_format", "p_bootable");
                    break;
                }
            case RAID_DEVICE: {
                    $attrs = $this->r_attributes;
                    $bool_attrs = array("r_encrypt");
                    $this->r_partitions = array();
                    foreach($this->freeRaidPartitions as $key => $part){
                        if(isset($_POST['r_partition_'.$key])){
                            $this->r_partitions[] = $part;
                        }
                    }
                    break;
                }
            case VOLUME_GROUP: {
                    $attrs = $this->vg_attributes;
                    $this->vg_partitions = array();
                    foreach($this->freeLvmPartitions as $key => $part){
                        if(isset($_POST['vg_partition_'.$key])){
                            $this->vg_partitions[] = $part;
                        }
                    }
                    break;
                }
            case VOLUME: {
                    $attrs = $this->v_attributes;
                    $bool_attrs = array("v_encrypt");
                    break;
                }
            case DISK: {
                    $attrs = $this->d_attributes;
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
        if($this->selected_type == DISK){
            
            // Get volume group properties
            $name = $this->d_name;
            $this->partitionObject->addDisk($name);
            return($this->partitionObject->success());

        }elseif($this->selected_type == VOLUME_GROUP){
            
            // Get volume group properties
            $devices = $this->vg_partitions;
            $name = $this->vg_name;
            $this->partitionObject->addVolumeGroup($name, $devices, $format=TRUE, $use_existing=FALSE, NULL);
            return($this->partitionObject->success());

        }elseif($this->selected_type == VOLUME){
            
            // Get volume properties
            $name = $this->v_name;
            $fsType = $this->v_fsType;
            $fsOptions = $this->v_fsOptions;
            $group = $this->v_group;
            $target = $this->v_mountPoint;
            $size = intval($this->v_size);
            $encrypt = $this->v_encrypt;
            
            $maxSize = NULL;
            $grow = FALSE;
            $format = TRUE;
            $use_existing = FALSE;

            if($fsType == "swap"){
                $target = "swap";
            }
      
            $this->partitionObject->addVolume($target, $name, $group, $size, $maxSize,
                $grow, $format, $use_existing, $fsType, $fsOptions);

            return($this->partitionObject->success());

        }elseif($this->selected_type == RAID_DEVICE){

            // Get raid device properties
            $devices = $this->r_partitions;
            $fsType = $this->r_fsType;
            $raidLevel = $this->r_raidLevel;
            $spares = $this->r_spares;
            $encrypt = $this->r_encrypt;

            $name = $this->partitionObject->getNextRaidDevice();

            // Check selected target 
            $partitions = $this->partitions;
            $raids = $this->raidDevices;
            $usedTargets = array();
            foreach($partitions as $part){
                $usedTargets[] = $part['target'];
            }
            foreach($raids as $part){
                $usedTargets[] = $part['target'];
            }
            if($fsType == "pv"){
                $target = $this->partitionObject->getNextPhysicalVolumeName();
                $fsType = $fsOptions = NULL;
            }else{
                $target = $this->r_mountPoint;
                $fsType = $this->r_fsType;
                $fsOptions = $this->r_fsOptions;
            }
            if($fsType == "swap"){
                $target = "swap";
            }
       
            $this->partitionObject->addRaidDevice($target, $name, $raidLevel, $spares, $fsType,$fsOptions, 
                TRUE, FALSE, $devices);

            return($this->partitionObject->success());

        }elseif($this->selected_type == PARTITION){

            // Get all currently used partitions
            $partitions = $this->partitions;
            $raids = $this->raidDevices;
            $usedTargets = array();
            foreach($partitions as $part){
                $usedTargets[] = $part['target'];
            }
            foreach($raids as $part){
                $usedTargets[] = $part['target'];
            }

            // Collect options
            $size = intval($this->p_size);
            $maxSize = NULL;
            if($this->p_size_options == 2){
                $maxSize = $this->p_size_max_value;
            }
            $grow = $this->p_size_options == 1;
            $format = $this->p_format == 1;
            $boot = $this->p_bootable == 1;
            $primary = $this->p_forcePrimary;
            $fsType = $this->p_fsType;
            $fsOptions = "";
            $encrypt = $this->p_encrypt;
            $passphrase = "";
            $disk = $this->p_used_disk;

            // We've to create a raid disk
            if($this->p_fsType == "raid"){
                $target = $this->partitionObject->getNextRaidName();
                $fsType = $fsOptions = NULL;
                $encrypt = $format = FALSE;
            }else

            // We've to create a raid disk
            if($this->p_fsType == "pv"){
                $target = $this->partitionObject->getNextPhysicalVolumeName();
                $fsType = $fsOptions = NULL;
                $encrypt = $format = FALSE;
            }else{
            
                // Add normal-physical partition
                $target = $this->p_mountPoint;
                if($fsType == "swap"){
                    $target = "swap";
                    $encrypt = $format = FALSE;
                }
            }

            // Add partition to remote model
            $this->partitionObject->addPartition($target,$size, $maxSize, $grow, $format, $boot, $primary,
                    $fsType, $fsOptions , $encrypt, $passphrase, $disk);
            return($this->partitionObject->success());
        }
    }
}
?>
