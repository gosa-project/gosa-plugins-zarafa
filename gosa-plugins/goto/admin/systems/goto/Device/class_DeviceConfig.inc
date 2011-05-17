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
    public $usedElementsList = NULL;
    public $assignableElementsList = NULL;

    public $usedElements = array();
    public $assignableElements = array();

    /*! \brief  Initialize the plugin and its widgets. 
     *          Finally call init() to load values from the backend.
     */ 
    function __construct(&$config, $dn, $parent)
    {
        plugin::plugin($config, $dn);

        $this->is_account = $this->initially_was_account = FALSE;

        $this->_device = &$parent;
        $this->deviceUUID = $parent->deviceUUID;

        // Create assignable Elements list.
        $this->assignableElementsList = new sortableListing();
        $this->assignableElementsList->setDeleteable(FALSE);
        $this->assignableElementsList->setEditable(TRUE);
        $this->assignableElementsList->setWidth("100%");
        $this->assignableElementsList->setHeight("200px");
        $this->assignableElementsList->setHeader(array(_("Name")));
        $this->assignableElementsList->setColspecs(array('*'));
        $this->assignableElementsList->setAcl('rwcdm');

        // Create used Elements list.
        $this->usedElementsList = new sortableListing();
        $this->usedElementsList->setDeleteable(TRUE);
        $this->usedElementsList->setInstantDelete(FALSE);
        $this->usedElementsList->setEditable(FALSE);
        $this->usedElementsList->setWidth("100%");
        $this->usedElementsList->setHeight("200px");
        $this->usedElementsList->setHeader(array(_("Name")));
        $this->usedElementsList->setColspecs(array('*'));
        $this->usedElementsList->setAcl('rwcdm');

        // Create parameter Elements list.
        $this->parameterList = new sortableListing();
        $this->parameterList->setDeleteable(TRUE);
        $this->parameterList->setInstantDelete(FALSE);
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

        // Fill list of assignable elements
        $rpc = $this->config->getRpcHandle();
        $res = $rpc->listAssignableElements('squeeze/1.0');
        $this->assignableElements = $res;
        $data = $lData = array();;
        foreach($this->assignableElements as $name => $element){
            if(in_array($name, $this->usedElements)) continue;
            $data[$name] = $element;
            $desc = (isset($element['description'])) ? $element['description'] : '';
            $lData[$name] = array('data' => array($name, $desc));
            
        }
        $this->assignableElementsList->setListData($data, $lData);

        // Build list of assigned Elements
        $data = $lData = array();;
        foreach($this->usedElements as $name){
            $element = $this->assignableElements[$name];
            $data[$name] = $element;
            $desc = (isset($element['description'])) ? $element['description'] : '';
            $lData[$name] = array('data' => array($name, $desc));
        }
        $this->usedElementsList->setListData($data, $lData);
        
        // Build up parameter list
        $data = $lData = array();;
        foreach($this->usedElements as $name){
            $parameter = $this->assignableElements[$name]['parameter'];
            foreach($parameter as $name => $desc){
                $data[$name] = NULL;
                $lData[$name] = array('data' => array($desc));
            }
            $value = (isset($this->parameterValues[$name])) ? $this->parameterValues[$name] : "<i>"._('unset')."</i>";
            $lData[$name] = array('data' => array($name, $value));
        }
        $this->parameterList->setListData($data, $lData);
        


        $smarty = get_smarty();
        $smarty->assign("assignableElementsList", $this->assignableElementsList->render());
        $smarty->assign("usedElementsList", $this->usedElementsList->render());
        $smarty->assign("parameterList", $this->parameterList->render());
        return($display.$smarty->fetch(get_template_path('goto/Device/DeviceConfig.tpl', TRUE)));
    }


    function save_object()
    {

        // Act on Add-Element actions
        $this->assignableElementsList->save_object();
        $action = $this->assignableElementsList->getAction();
        if($action['action'] == 'edit'){
            $this->usedElements[] = $this->assignableElementsList->getKey($action['targets'][0]);
            $this->usedElements = array_unique($this->usedElements);
        }

        // Act on Remove-Item actions
        $this->usedElementsList->save_object();
        $action = $this->usedElementsList->getAction();
        if($action['action'] == 'delete'){
            $name = $this->usedElementsList->getKey($action['targets'][0]);
            $this->usedElements = array_remove_entries_ics(array($name), $this->usedElements);
        }
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
