<?php

class DeviceConfig extends plugin
{

    /* Some plugin management related attributes 
     * See class plugin for details.
     */
    public $view_logged = FALSE;
    public $ignore_account = FALSE;
    public $parent = NULL;

    public $parameterList = NULL;
    public $usedItemsList = NULL;
    public $assignableItemsList = NULL;


    /*! \brief  Initialize the plugin and its widgets. 
     *          Finally call init() to load values from the backend.
     */ 
    function __construct(&$config, $dn, $parent)
    {
        plugin::plugin($config, $dn);

        $this->is_account = $this->initially_was_account = FALSE;

        $this->_device = &$parent;
        $this->deviceUUID = $parent->deviceUUID;

        // Create assignable items list.
        $this->assignableItemsList = new sortableListing();
        $this->assignableItemsList->setDeleteable(FALSE);
        $this->assignableItemsList->setEditable(TRUE);
        $this->assignableItemsList->setWidth("100%");
        $this->assignableItemsList->setHeight("200px");
        $this->assignableItemsList->setHeader(array(_("Type")));
        $this->assignableItemsList->setColspecs(array('*'));
        $this->assignableItemsList->setAcl('rwcdm');

        // Create used items list.
        $this->usedItemsList = new sortableListing();
        $this->usedItemsList->setDeleteable(FALSE);
        $this->usedItemsList->setEditable(TRUE);
        $this->usedItemsList->setWidth("100%");
        $this->usedItemsList->setHeight("200px");
        $this->usedItemsList->setHeader(array(_("Type")));
        $this->usedItemsList->setColspecs(array('*'));
        $this->usedItemsList->setAcl('rwcdm');

        // Create parameter items list.
        $this->parameterList = new sortableListing();
        $this->parameterList->setDeleteable(FALSE);
        $this->parameterList->setEditable(TRUE);
        $this->parameterList->setWidth("100%");
        $this->parameterList->setHeight("200px");
        $this->parameterList->setHeader(array(_("Type")));
        $this->parameterList->setColspecs(array('*'));
        $this->parameterList->setAcl('rwcdm');
    }

    function check()
    {
    }
    
    
    function execute()
    {
        // Log account access
        if($this->is_account && !$this->view_logged){
            $this->view_logged = TRUE;
            new log("view","Device/".get_class($this),$this->dn);
        }

        /***************
          Handle account state
         ***************/

        // Allow to add or remove the distribution list extension
        if(isset($_POST['modify_state'])){
            if($this->is_account && $this->acl_is_removeable()){
                $this->is_account= FALSE;
            }elseif(!$this->is_account && $this->acl_is_createable()){
                $this->is_account= TRUE;
            }
        }

        // Show account status-changer
        $display = "";
        if ($this->parent !== NULL){
            if ($this->is_account){
                $display= $this->show_disable_header(_("Remove config mangement"),
                        msgPool::featuresEnabled(_("Config management")));
            } else {
                $display= $this->show_enable_header(_("Add Config management"),
                        msgPool::featuresDisabled(_("Config management")));
                return ($display);
            }
        }

        $smarty = get_smarty();
        $smarty->assign("assignableItemsList", $this->assignableItemsList->render());
        $smarty->assign("usedItemsList", $this->usedItemsList->render());
        $smarty->assign("parameterList", $this->parameterList->render());
        return($display.$smarty->fetch(get_template_path('goto/Device/DeviceConfig.tpl', TRUE)));
    }


    function save_object()
    {
    }


    function save()
    {
    }

    
    function remove_from_parent()
    {
    }
 
    
    static function plInfo()
    {
        return (array(
                    "plShortName"   => _("Device"),
                    "plDescription" => _("Registered device"),
                    "plSelfModify"  => FALSE,
                    "plDepends"     => array(),
                    "plPriority"    => 1,
                    "plSection"     => array("administration"),
                    "plCategory"    => array("Device"),
                    "plProvidedAcls" => array(
                        )));
    }
}
?>
