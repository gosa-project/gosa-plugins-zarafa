<?php

class InstallRecipe extends plugin
{

    public $kickstartBootstrapMethod = '';
    public $kickstartBootstrapMethodList = NULL;

    public $kickstartConfigManagement = '';
    public $kickstartConfigManagementList = NULL;

    public $kickstartTemplateDN = "";
    public $kickstartTemplateDNList = array(
        'dc=intranet,dc=gonicus,dc=de' => 'Intranet-Tpl',
        'ou=systems,dc=intranet,dc=gonicus,dc=de' => 'System-Tpl');

    public $kickstartMirrorDN = "";
    public $kickstartMirrorDNList = array(
        'dc=intranet,dc=gonicus,dc=de' => 'Intranet',
        'ou=systems,dc=intranet,dc=gonicus,dc=de' => 'System');
    public $kickstartKernelPackage = "";
    public $kickstartKernelPackageList = array('kernel1','kk1');
    public $kickstartKeyboardlayout = "";
    public $kickstartKeyboardlayoutList = array("104","105");
    public $kickstartSystemLocale = "";
    public $kickstartSystemLocaleList = array('de','en_EN.UTF-8');
    public $kickstartTimezone = "";
    public $kickstartTimeUTC = "";
    public $kickstartNTPServer = array();
    public $kickstartRootEnabled = "";
    public $kickstartRootPasswordHash = "";
    public $kickstartPartitionTable = "";

    public $objectclasses = array('installRecipe');
    public $attributes = array("kickstartTemplateDN","kickstartKeyboardlayout","kickstartSystemLocale",
            "kickstartTimezone","kickstartTimeUTC","kickstartNTPServer","kickstartMirrorDN",
            "kickstartRootEnabled","kickstartRootPasswordHash","kickstartKernelPackage",
            "kickstartPartitionTable","kickstartConfigManagement","kickstartBootstrapMethod");

    public $setKickstartRootPasswordHash = FALSE;

    private $kickstartNTPServerList = NULL;

    function __construct(&$config, $dn)
    {
        plugin::plugin($config, $dn);

        // Preset some values for new accounts
        if(!$this->is_account){

            // Preset the device timezone
            $tz = timezone::get_default_timezone();
            $this->kickstartTimezone = $tz['name'];
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
        $this->kickstartNTPServer = array();
        if(isset($this->attrs['kickstartNTPServer']['count'])){
            for($i = 0; $i < $this->attrs['kickstartNTPServer']['count']; $i++){
                $this->kickstartNTPServer[] = $this->attrs['kickstartNTPServer'][$i];
            }
        }

        // Prepare NTP servers list 
        $this->kickstartNTPServerList= new sortableListing($this->kickstartNTPServer);
        $this->kickstartNTPServerList->setDeleteable(true);
        $this->kickstartNTPServerList->setEditable(false);
        $this->kickstartNTPServerList->setColspecs(array('*'));
        $this->kickstartNTPServerList->setWidth("100%");
        $this->kickstartNTPServerList->setHeight("70px");

        // Load list of bootstrap methods.
        $this->loadInstallationMethods();
    }

    
    function loadInstallationMethods()
    { 
        $this->kickstartBootstrapMethodList = array();
        $this->kickstartConfigManagementList = array();
        $rpc = $this->config->getRpcHandle();
        $res = $rpc->getSupportedBaseInstallMethods();
        foreach($res as $name => $method){
            $this->kickstartBootstrapMethodList[$name] = $method['name'];
            foreach($method['methods'] as $m){
                $this->kickstartConfigManagementList[$name][$m] = $m; 
            }
        }
        
        if(empty($this->kickstartBootstrapMethod)){
            $this->kickstartBootstrapMethod = key($this->kickstartBootstrapMethodList);
        }
        if(empty($this->kickstartConfigManagement)){
            $this->kickstartConfigManagement = key($this->kickstartConfigManagementList[$this->kickstartBootstrapMethod]);
        }
    }
    

    function execute()
    {
        if($this->setKickstartRootPasswordHash){
            $this->dialog = TRUE;
            $smarty = get_smarty();
            $smarty->assign('hashes', set_post($this->hashes));
            $smarty->assign('hash', set_post($this->hash));
            return($smarty->fetch(get_template_path('goto/Device/SetPassword.tpl', TRUE)));
        }

        $this->kickstartNTPServerList->setAcl($this->getacl('kickstartNTPServer'));
        $this->kickstartNTPServerList->update();

        plugin::execute();    
        $smarty = get_smarty();
        $smarty->assign('timezones', $this->timezones);
        $smarty->assign('kickstartKeyboardlayoutList', $this->kickstartKeyboardlayoutList);
        $smarty->assign('kickstartKernelPackageList', $this->kickstartKernelPackageList);
        $smarty->assign('kickstartTemplateDNList', $this->kickstartTemplateDNList);
        $smarty->assign('kickstartMirrorDNList', $this->kickstartMirrorDNList);
        $smarty->assign('kickstartSystemLocaleList', $this->kickstartSystemLocaleList);
        $smarty->assign('kickstartNTPServerList', $this->kickstartNTPServerList->render());

        $smarty->assign('kickstartBootstrapMethod', $this->kickstartBootstrapMethod);
        $smarty->assign('kickstartBootstrapMethodList', $this->kickstartBootstrapMethodList);
        $smarty->assign('kickstartConfigManagement', $this->kickstartConfigManagement);

        $cfgList = $this->kickstartConfigManagementList[$this->kickstartBootstrapMethod];
        $smarty->assign('kickstartConfigManagementList', $cfgList);
        foreach($this->attributes as $attr){
            $smarty->assign($attr, $this->$attr);
        }
        $this->dialog = false;
        return($smarty->fetch(get_template_path('goto/Device/InstallRecipe.tpl', TRUE)));
    }


    function save_object()
    {

        
        if(isset($_POST['InstallRecipePosted'])){

            $currentInstallMethod = $this->kickstartConfigManagement;
            plugin::save_object();
            $this->kickstartRootEnabled = isset($_POST['kickstartRootEnabled']);
            $this->kickstartTimeUTC = isset($_POST['kickstartTimeUTC']);


            if(isset($_POST['setKickstartRootPasswordHash'])){
                $this->setKickstartRootPasswordHash = true;
            }

            $this->kickstartNTPServerList->save_object();
            $this->kickstartNTPServer = $this->kickstartNTPServerList->getMaintainedData();
            if(isset($_POST['kickstartNTPServer_Input']) && isset($_POST['kickstartNTPServer_Add'])){
                $add = get_post('kickstartNTPServer_Input');
                if(!in_array($add, $this->kickstartNTPServer) && !empty($add)){
                    $this->kickstartNTPServer[] = $add;
                }
            }
            $this->kickstartNTPServerList->setListData($this->kickstartNTPServer);

            if($currentInstallMethod != $this->kickstartConfigManagement){
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
            $this->kickstartRootPasswordHash = @$test->generate_hash(get_post('rootPassword'));
        }

    }


    function updateRecipeTab()
    {
        $mode = $this->kickstartConfigManagement;
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
        if(!$this->kickstartRootEnabled) $this->kickstartRootPasswordHash = "";
        $this->kickstartRootEnabled = ($this->kickstartRootEnabled)?'TRUE':'FALSE';
        $this->kickstartTimeUTC = ($this->kickstartTimeUTC)?'TRUE':'FALSE';
        

        plugin::save();

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
                        "kickstartTemplateDN" => _("Template"),
                        "kickstartBootstrapMethod" => _("Bootstrap method"),
                        "kickstartConfigManagement" => _("Config management"),
                        "kickstartKeyboardlayout" => _("Keyboard layout"),
                        "kickstartSystemLocale" => _("System locale"),
                        "kickstartTimezone" => _("Timezone"),
                        "kickstartTimeUTC" => _("Time"),
                        "kickstartNTPServer" => _("NTP-Server"),
                        "kickstartMirrorDN" => _("Kickstart mirror"),
                        "kickstartRootEnabled" => _("Root login enabled"),
                        "kickstartRootPasswordHash" => _("Root password hash"),
                        "kickstartKernelPackage" => _("Kernal package"),
                        "kickstartPartitionTable" => _("Partition table")
                        )
                        )
                        );
    }
}

?>
