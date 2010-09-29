<?php

class InstallRecipe extends plugin
{
    public $kickstartTemplateDN = "";
    public $kickstartKeyboardlayout = "";
    public $kickstartSystemLocale = "";
    public $kickstartTimezone = "";
    public $kickstartTimeUTC = "";
    public $kickstartNTPServer = "";
    public $kickstartMirrorDN = "";
    public $kickstartRootEnabled = "";
    public $kickstartRootPasswordHash = "";
    public $kickstartKernelPackage = "";
    public $kickstartPartitionTable = "";

    public $objectclasses = array('installRecipe');
    public $attributes = array("kickstartTemplateDN","kickstartKeyboardlayout","kickstartSystemLocale",
            "kickstartTimezone","kickstartTimeUTC","kickstartNTPServer","kickstartMirrorDN",
            "kickstartRootEnabled","kickstartRootPasswordHash","kickstartKernelPackage","kickstartPartitionTable");

    public $setKickstartRootPasswordHash = FALSE;

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


        plugin::execute();    
        $smarty = get_smarty();
        $smarty->assign('timezones', $this->timezones);
        foreach($this->attributes as $attr){
            $smarty->assign($attr, $this->$attr);
        }
        $this->dialog = false;
        return($smarty->fetch(get_template_path('goto/Device/InstallRecipe.tpl', TRUE)));
    }


    function save_object()
    {
        if(isset($_POST['InstallRecipePosted'])){
            plugin::save_object();
            $this->kickstartRootEnabled = isset($_POST['kickstartRootEnabled']);


            if(isset($_POST['setKickstartRootPasswordHash'])){
                $this->setKickstartRootPasswordHash = true;
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


    function save()
    {
        // 
        if(!$this->kickstartRootEnabled) $this->kickstartRootPasswordHash = "";
        $this->kickstartRootEnabled = ($this->kickstartRootEnabled)?'TRUE':'FALSE';
        

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
