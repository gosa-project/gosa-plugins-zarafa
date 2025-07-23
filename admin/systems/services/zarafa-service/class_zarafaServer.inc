<?php

/*
 * This plugin will add some new functionallity to
 * manage your zarafa -server/s.
 */
class zarafaServer extends goService
{
    //needed ldap attributes
    var $attributes = array("zarafaAccount", "zarafaHidden", "zarafaHttpPort", "zarafaSslPort", "zarafaFilePath",
                            "zarafaContainsPublic", "goZarafaArchiveServer");

    //needed ldap object classes
    var $objectclasses = array("zarafa-server", "goZarafaAddons");

    //various other member
    var $plHeadline = "Zarafa server plugin";
    var $plDescrition = "Zarafa server settings";
    var $DisplayName = "Zarafa server";

    //boolean attributes
    var $zarafaAccount = 0;
    var $zarafaHidden = 0;
    var $zarafaContainsPublic = 0;
    var $goZarafaArchiveServer = 0;
    //integer attributes
    var $zarafaHttpPort = 80;
    var $zarafaSslPort = 443;

    //string attributes
    var $zarafaFilePath = "";

    var $count;

    /*
     * Constructor
     */
    public function __construct(&$config, $dn = NULL)
    {
        goService::__construct($config, $dn);
        $this->config = $config;
        $this->DisplayName = _("Zarafa server");
    }

    public function execute()
    {
        //get smarty engine and assign smarty variables
        $smarty = get_smarty();

        $smarty->assign("zarafaAccount", ($this->zarafaAccount)?"checked":"");
        $smarty->assign("zarafaHidden", ($this->zarafaHidden)?"checked":"");
        $smarty->assign("zarafaHttpPort", $this->zarafaHttpPort);
        $smarty->assign("zarafaSslPort", $this->zarafaSslPort);
        $smarty->assign("zarafaContainsPublic", ($this->zarafaContainsPublic)?"checked":"");
        $smarty->assign("zarafaFilePath", $this->zarafaFilePath);
        $smarty->assign("goZarafaArchiveServer", ($this->goZarafaArchiveServer)?"checked":"");

        return($smarty->fetch(get_template_path('zarafaServer.tpl', TRUE, dirname(__FILE__))));
    }

    /*
     * Add the zarafa service entry to the services list
     */
    public function getListEntry()
    {
        $fields = goService::getListEntry();
        $fields['Message']    = _("Zarafa service");
        #$fields['AllowEdit']  = false;
        return($fields);
    }

    public function save_object()
    {
        if(!isset($_POST['zarafa-server-plugin']))
        {
            return;
        }

        goService::save_object();

        //transform checkboxes to 1 or 0
        foreach(array("zarafaAccount", "zarafaHidden", "zarafaContainsPublic", "goZarafaArchiveServer") as $val)
        {
            if(isset($_POST[$val]))
            {
                $this->$val = 1;
            }
            else
          {
               $this->$val = 0;
            }
        }
    }

    /*
     * Save values to ldap
     */
    public function save()
    {
        //call parent implementation
        goService::save();

        //get ldap link
        $ldap= $this->config->get_ldap_link();

        /* Save data to LDAP */
        $ldap->cd($this->dn);
        $this->cleanup();
        $ldap->modify($this->attrs);

        if(!$ldap->success())
            msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $this->dn, LDAP_MOD, __CLASS__));
    }

    /* Return plugin informations for acl handling */
    static function plInfo()
    {
        return (array(
                    "plShortName"   => _("Zarafa"),
                    "plDescription" => _("zarafa service")." ("._("Services").")",
                    "plSelfModify"  => FALSE,
                    "plDepends"     => array(),
                    "plPriority"    => 100,
                    "plSection"     => array("administration"),
                    "plCategory"    => array("server"),

                    "plProvidedAcls"=> array(
                            "start"                 => _("Start"),
                            "stop"                  => _("Stop"),
                            "restart"               => _("Restart"),
                            "zarafaAccount"         => _("Zarafa server"),
                            "zarafaHidden"          => _("Zarafa hidden"),
                            "zarafaHttpPort"        => _("Zarafa HTTP port"),
                            "zarafaSslPort"         => _("Zarafa SSL port"),
                            "zarafaConatinsPublic"  => _("Zarafa node contains public store"),
                            "zarafaFilePath"        => _("Zarafa file path"),
                            "goZarafaArchiveServer" => _("Zarafa Archvie Server"),
                        )
                    ));
    }
}

?>
