<?php

class InstallRecipe extends plugin
{
    public $member = array();
    public $ksTemplate = "";
    public $ksKeyboardlayout = "";
    public $ksSystemLocale = "";
    public $ksTimezone = "";
    public $ksTimeUTC = "";
    public $ksNTPServer = "";
    public $ksMirror = "";
    public $ksMirrorTorrent = "";
    public $ksRootEnabled = "";
    public $ksRootPasswordHash = "";
    public $ksKernelPackage = "";
    public $ksPartitionTable = "";

    public $objectclasses = array('installRecipe');
    public $attributes = array("member","ksTemplate","ksKeyboardlayout","ksSystemLocale",
            "ksTimezone","ksTimeUTC","ksNTPServer","ksMirror","ksMirrorTorrent",
            "ksRootEnabled","ksRootPasswordHash","ksKernelPackage","ksPartitionTable");

    function __construct(&$config, $dn)
    {
        plugin::plugin($config, $dn);

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
        $this->timezones = $list;
    }

    function execute()
    {
        plugin::execute();    
        $smarty = get_smarty();
        $smarty->assign('timezones', $this->timezones);
        foreach($this->attributes as $attr){
            $smarty->assign($attr, $this->$attr);
        }
        return($smarty->fetch(get_template_path('goto/Device/InstallRecipe.tpl', TRUE)));
    }


    function save()
    {
        $this->member = array();
        plugin::save();
        #$this->cleanup();

        print_a($this->attrs);

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
                        "ksTemplate" => _("Template"),
                        "ksKeyboardlayout" => _("Keyboard layout"),
                        "ksSystemLocale" => _("System locale"),
                        "ksTimezone" => _("Timezone"),
                        "ksTimeUTC" => _("Time"),
                        "ksNTPServer" => _("NTP-Server"),
                        "ksMirror" => _("Kickstart mirror"),
                        "ksMirrorTorrent" => _("Kickstart torrent mirror"),
                        "ksRootEnabled" => _("Root login enabled"),
                        "ksRootPasswordHash" => _("Root password hash"),
                        "ksKernelPackage" => _("Kernal package"),
                        "ksPartitionTable" => _("Partition table")
                        )
                        )
                        );
    }
}

?>
