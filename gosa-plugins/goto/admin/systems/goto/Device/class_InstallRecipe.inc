<?php

class InstallRecipe extends plugin
{

    public $installBootstrapMethod = '';
    public $installBootstrapMethodList = NULL;

    public $installConfigManagement = '';
    public $installConfigManagementList = NULL;

    public $installTemplateDN = "";
    public $installTemplateDNList = array(
        'dc=intranet,dc=gonicus,dc=de' => 'Intranet-Tpl',
        'ou=systems,dc=intranet,dc=gonicus,dc=de' => 'System-Tpl');

    public $installMirrorDN = "";
    public $installMirrorDNList = array(
        'dc=intranet,dc=gonicus,dc=de' => 'Intranet',
        'ou=systems,dc=intranet,dc=gonicus,dc=de' => 'System');
    public $installKernelPackage = "";
    public $installKernelPackageList = array('kernel1','kk1');
    public $installKeyboardlayout = "";
    public $installKeyboardlayoutList = array("104","105");
    public $installSystemLocale = "";
    public $installSystemLocaleList = array('de','en_EN.UTF-8');
    public $installTimezone = "";
    public $installTimeUTC = "";
    public $installNTPServer = array();
    public $installRootEnabled = "";
    public $installRootPasswordHash = "";
    public $installPartitionTable = "";

    public $objectclasses = array('installRecipe');
    public $attributes = array("installTemplateDN","installKeyboardlayout","installSystemLocale",
            "installTimezone","installTimeUTC","installNTPServer","installMirrorDN",
            "installRootEnabled","installRootPasswordHash","installKernelPackage",
            "installPartitionTable","installConfigManagement","installBootstrapMethod");

    public $setKickstartRootPasswordHash = FALSE;

    public $view_logged = FALSE;
    
    public $ignore_account = FALSE;

    private $installNTPServerList = NULL;

    function __construct(&$config, $dn)
    {
        plugin::plugin($config, $dn);

        // Preset some values for new accounts
        if(!$this->is_account){

            // Preset the device timezone
            $tz = timezone::get_default_timezone();
            $this->installTimezone = $tz['name'];
        }

        // Prepare list of timezones
        $tmp = timezone::_get_tz_zones();
        $list = array();
        foreach($tmp['TIMEZONES'] as $name => $offset){
            if($offset >= 0){
                $list[$name] = $name." ( + ".sprintf("%0.2f",$offset/(60*60))." "._("hours").")";
            }else{
                $offset = $offset * -1;
                $list[$name] = $name." ( - ".sprintf("%0.2f",($offset/(60*60)))." "._("hours").")";
            }
        }  
        uksort($list, 'strnatcasecmp'); 
        $this->timezones = $list;

        // Get list of password hashes
        $tmp = passwordMethod::get_available_methods();
        $this->hashes = array();
        foreach($tmp['name'] as $name){
            $this->hashes[$name] = $name;
        }
        $this->hash = $this->config->get_cfg_value("core","passwordDefaultHash"); 

        // Get list of NTP servers
        $this->installNTPServer = array();
        if(isset($this->attrs['installNTPServer']['count'])){
            for($i = 0; $i < $this->attrs['installNTPServer']['count']; $i++){
                $this->installNTPServer[] = $this->attrs['installNTPServer'][$i];
            }
        }

        // Prepare NTP servers list 
        $this->installNTPServerList= new sortableListing($this->installNTPServer);
        $this->installNTPServerList->setDeleteable(true);
        $this->installNTPServerList->setEditable(false);
        $this->installNTPServerList->setColspecs(array('*'));
        $this->installNTPServerList->setWidth("100%");
        $this->installNTPServerList->setHeight("70px");

        // Load list of bootstrap methods.
        $this->loadInstallationMethods();
    }

    
    function loadInstallationMethods()
    { 
        $this->installBootstrapMethodList = array();
        $this->installConfigManagementList = array();
        $rpc = $this->config->getRpcHandle();
        $res = $rpc->getSupportedBaseInstallMethods();
        foreach($res as $name => $method){
            $this->installBootstrapMethodList[$name] = $method['name'];
            foreach($method['methods'] as $m){
                $this->installConfigManagementList[$name][$m] = $m; 
            }
        }
        
        if(empty($this->installBootstrapMethod)){
            $this->installBootstrapMethod = key($this->installBootstrapMethodList);
        }
        if(empty($this->installConfigManagement)){
            $this->installConfigManagement = key($this->installConfigManagementList[$this->installBootstrapMethod]);
        }
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
                $display= $this->show_disable_header(_("Remove instal profile"),
                        msgPool::featuresEnabled(_("Install profile")));
            } else {
                $display= $this->show_enable_header(_("Add install profile"),
                        msgPool::featuresDisabled(_("Install profile")));
                return ($display);
            }
        }

        /***************
          Root password hash dialog
         ***************/

        if($this->setKickstartRootPasswordHash){
            $this->dialog = TRUE;
            $smarty = get_smarty();
            $smarty->assign('hashes', set_post($this->hashes));
            $smarty->assign('hash', set_post($this->hash));
            return($smarty->fetch(get_template_path('goto/Device/SetPassword.tpl', TRUE)));
        }

        /***************
          Generate HTML content
         ***************/

        $this->installNTPServerList->setAcl($this->getacl('installNTPServer'));
        $this->installNTPServerList->update();

        plugin::execute();    
        $smarty = get_smarty();
        $smarty->assign('timezones', $this->timezones);
        $smarty->assign('installKeyboardlayoutList', $this->installKeyboardlayoutList);
        $smarty->assign('installKernelPackageList', $this->installKernelPackageList);
        $smarty->assign('installTemplateDNList', $this->installTemplateDNList);
        $smarty->assign('installMirrorDNList', $this->installMirrorDNList);
        $smarty->assign('installSystemLocaleList', $this->installSystemLocaleList);
        $smarty->assign('installNTPServerList', $this->installNTPServerList->render());

        $smarty->assign('installBootstrapMethod', $this->installBootstrapMethod);
        $smarty->assign('installBootstrapMethodList', $this->installBootstrapMethodList);
        $smarty->assign('installConfigManagement', $this->installConfigManagement);

        $cfgList = $this->installConfigManagementList[$this->installBootstrapMethod];
        $smarty->assign('installConfigManagementList', $cfgList);
        foreach($this->attributes as $attr){
            $smarty->assign($attr, $this->$attr);
        }
        $this->dialog = false;
        return($display.$smarty->fetch(get_template_path('goto/Device/InstallRecipe.tpl', TRUE)));
    }


    function save_object()
    {

        
        if(isset($_POST['InstallRecipePosted'])){

            $currentInstallMethod = $this->installConfigManagement;
            plugin::save_object();
            $this->installRootEnabled = isset($_POST['installRootEnabled']);
            $this->installTimeUTC = isset($_POST['installTimeUTC']);


            if(isset($_POST['setKickstartRootPasswordHash'])){
                $this->setKickstartRootPasswordHash = true;
            }

            $this->installNTPServerList->save_object();
            $this->installNTPServer = $this->installNTPServerList->getMaintainedData();
            if(isset($_POST['installNTPServer_Input']) && isset($_POST['installNTPServer_Add'])){
                $add = get_post('installNTPServer_Input');
                if(!in_array($add, $this->installNTPServer) && !empty($add)){
                    $this->installNTPServer[] = $add;
                }
            }
            $this->installNTPServerList->setListData($this->installNTPServer);

            if($currentInstallMethod != $this->installConfigManagement){
                $this->updateRecipeTab();
            }
        }
        if(isset($_POST['cancelPassword'])) $this->setKickstartRootPasswordHash =false;
        if(isset($_POST['setPassword'])) {
            $this->setKickstartRootPasswordHash =false;
            $hash = get_post('passwordHash');

            // Not sure, why this is here, but maybe some encryption methods require it.
            mt_srand((double) microtime()*1000000);

            // Generate the requested hash
            $methods = new passwordMethod($this->config, $this->dn);
            $available = $methods->get_available_methods();
            $test = new $available[$hash]($this->config,$this->dn);
            $this->installRootPasswordHash = @$test->generate_hash(get_post('rootPassword'));
        }

    }


    function updateRecipeTab()
    {
        $mode = $this->installConfigManagement;
        if(!isset($this->recipeTabs[$mode])){
            $this->recipeTabs[$mode] = new DeviceConfig($this->config, $this->dn);
            $this->recipeTabs[$mode]->parent = &$this->parent;

            if($this->recipeTabs[$mode]->setInstallMethod($mode)){

                $class= &$this->recipeTabs[$mode];

                // Add some dummy entries for testing.
                if($mode == 'puppet'){ 
                    $id = $class->addItem('PuppetModule','test1',
                            array(
                                'dependency' => array('stulle','Wurst'),
                                'version' => '2.4-f',
                                'name'  => 'Thundebird',
                                'description' => 'Mozilla mail client')
                            );
                    $id = $class->addItem('PuppetModule','test2',
                            array(
                                'dependency' => array('Leipnitz','Dose'),
                                'version' => 1,
                                'name'  => 'Firefox',
                                'description' => 'Test Module')
                            );
                    $class->setCurrentItem($id);
                    $id = $class->addItem('PuppetTemplate','temp1',
                            array(
                                'name' => 'temp1',
                                'data' => 'kekse.tpl')
                            );
                    $class->setCurrentItem($id);
                    $id = $class->addItem('PuppetTemplate','tep1',
                            array(
                                'name' => 'tep1',
                                'data' => 'kekse.tpl')
                            );

                    $class->setCurrentItem($class->getRootItemID());
                }
            }else{
                unset($this->recipeTabs[$mode]);
            }
        }

        // Update the recipe tab to match the selected installation method.
        if($this->is_account && isset($this->recipeTabs[$mode])){

            if(isset($this->parent->by_object['DeviceConfig']) && 
                    $this->recipeTabs[$mode] === $this->parent->by_object['DeviceConfig']){
                return;
            }

            $this->parent->by_name['DeviceConfig'] = $mode ;
            $this->parent->by_object['DeviceConfig'] = &$this->recipeTabs[$mode];
        }else{
            if(isset($this->parent->by_name['DeviceConfig'])){
                unset($this->parent->by_name['DeviceConfig']);
                unset($this->parent->by_object['DeviceConfig']);
            }
        }
    }


    function save()
    {
        // 
        if(!$this->installRootEnabled) $this->installRootPasswordHash = "";
        $this->installRootEnabled = ($this->installRootEnabled)?'TRUE':'FALSE';
        $this->installTimeUTC = ($this->installTimeUTC)?'TRUE':'FALSE';
        
        plugin::save();

        unset($this->attrs['installConfigManagement']);
        unset($this->attrs['installBootstrapMethod']);
        echo "Skipp saving of 'installConfigManagement' and 'installBootstrapMethod', i've no idea where to store them.";

        $this->cleanup();
        $ldap=$this->config->get_ldap_link();
        $ldap->cd($this->config->current['BASE']);
        $ldap->cd($this->dn);
        $ldap->modify($this->attrs);
        new log("modify","Device/".get_class($this),$this->dn,array_keys($this->attrs),$ldap->get_error());
        if (!$ldap->success()){
            msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $this->dn, LDAP_MOD, get_class()));
        }else{
            $this->handle_post_events("modify");
        }
    }

    
    function remove_from_parent()
    {
        plugin::remove_from_parent();
        unset($this->attrs['installConfigManagement']);
        unset($this->attrs['installBootstrapMethod']);
        echo "Skipp saving of 'installConfigManagement' and 'installBootstrapMethod', i've no idea where to store them.";

        $ldap=$this->config->get_ldap_link();
        $ldap->cd($this->config->current['BASE']);
        $ldap->cd($this->dn);
        $ldap->modify($this->attrs);
        new log("remove","Device/".get_class($this),$this->dn,array_keys($this->attrs),$ldap->get_error());
        if (!$ldap->success()){
            msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $this->dn, LDAP_DEL, get_class()));
        }else{
            $this->handle_post_events("remove");
        }
        
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
                        "member" => _("Member"),
                        "installTemplateDN" => _("Template"),
                        "installBootstrapMethod" => _("Bootstrap method"),
                        "installConfigManagement" => _("Config management"),
                        "installKeyboardlayout" => _("Keyboard layout"),
                        "installSystemLocale" => _("System locale"),
                        "installTimezone" => _("Timezone"),
                        "installTimeUTC" => _("Time"),
                        "installNTPServer" => _("NTP-Server"),
                        "installMirrorDN" => _("Kickstart mirror"),
                        "installRootEnabled" => _("Root login enabled"),
                        "installRootPasswordHash" => _("Root password hash"),
                        "installKernelPackage" => _("Kernal package"),
                        "installPartitionTable" => _("Partition table")
                        )
                        )
                        );
    }
}

?>
