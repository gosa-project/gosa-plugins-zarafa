<?php


class registeredDevice extends plugin
{

    public $objectclasses = array('top','device','registeredDevice');
    public $attributes = array("cn","serialNumber","seeAlso","owner","ou","o",
            "l","description","manager","deviceUUID","deviceStatus");

    function execute()
    {
        plugin::execute();    
        $smarty = get_smarty();
        foreach($this->attributes as $attr){
            $smarty->assign($attr, $this->$attr);
        }
        return($smarty->fetch(get_template_path('goto/devices/registeredDevice.tpl', TRUE)));
    }


    function save()
    {
        plugin::save();
        $this->cleanup();
        $ldap=$this->config->get_ldap_link();
        $ldap->cd($this->config->current['BASE']);
        $ldap->cd($this->dn);
        $ldap->modify($this->attrs);
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
                    "plCategory"    => array(
                        "registeredDevice" => array( "description"  => _("Registered device"),
                            "objectClass"  => "registeredDevice")),
                    "plProvidedAcls" => array(
                        "cn" => _("Name"),
                        "serialNumber" => _("Serial number"),
                        "seeAlso" => _("See also"),
                        "owner" => _("Owner"),
                        "ou" => _("Organizational unit"),
                        "o" => _("Organization"),
                        "l" => _("Location"),
                        "description" => _("Description"),
                        "manager" => _("Manager"),
                        "deviceUUID" => _("Uuid"),
                        "deviceStatus" => _("Stauts")
                        )
                        )
                        );
    }
}

?>
