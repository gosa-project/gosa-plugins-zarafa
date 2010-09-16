<?php

class installRecipe extends plugin
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
    }

    function execute()
    {
        plugin::execute();    
        $smarty = get_smarty();
        foreach($this->attributes as $attr){
            $smarty->assign($attr, $this->$attr);
        }
        return($smarty->fetch(get_template_path('goto/devices/installRecipe.tpl', TRUE)));
    }


    function save()
    {
        plugin::save();
        $this->cleanup();
        $ldap=$this->config->get_ldap_link();
        $ldap->cd($this->config->current['BASE']);
        $ldap->cd($this->dn);
        $ldap->modify($this->attrs);
        echo $ldap->get_error();
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
                    "plCategory"    => array("registeredDevice"),
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
