<?php


class Device extends plugin
{

    public $objectclasses = array('top','device');
    public $attributes = array("cn","serialNumber","seeAlso","owner","ou","o","ipHostNumber",
            "l","description","manager","deviceUUID","deviceStatus","macAddress","deviceType");

    public $dynClasses = array(
            'ieee802Device' => array('macAddress'),
            'registeredDevice' => array('deviceStatus','deviceUUID','manager','deviceType'),
            'ipHost' => array('ipHostNumber'));

    public $cn = "";
    public $serialNumber = "";
    public $seeAlso = "";
    public $owner = "";
    public $ou = "";
    public $o = "";
    public $l = "";
    public $description = "";
    public $manager = "";
    public $deviceUUID = "";
    public $deviceStatus = "";
    public $deviceType = "";
    public $ipHostNumber = "";
    public $macAddress = "";

    public $base = "";
    public $orig_dn ="";

    function __construct(&$config, $dn)
    {
        plugin::plugin($config, $dn);
        $this->base = $this->config->current['BASE'];
        $this->orig_dn = $this->dn;
    }

    function execute()
    {
        plugin::execute();    
        $smarty = get_smarty();
        foreach($this->attributes as $attr){
            $smarty->assign($attr, $this->$attr);
        }
        return($smarty->fetch(get_template_path('goto/Device/Device.tpl', TRUE)));
    }


    function save()
    {

        plugin::save();

        // Append and remove dynmic object classes
        foreach($this->dynClasses as $oc => $attrs){
            $this->attrs['objectClass'] = array_remove_entries(array($oc), $this->attrs['objectClass']);
            foreach($attrs as $attr){
                if(isset($this->attrs[$attr]) && !empty($this->attrs[$attr])){
                    $this->attrs['objectClass'][] = $oc;
                    break;
                }
            }
        }

        $this->cleanup();
        $ldap=$this->config->get_ldap_link();
        $ldap->cd($this->config->current['BASE']);
        $ldap->cd($this->dn);

        if($this->initially_was_account){
            $ldap->modify($this->attrs);
        }else{
            $ldap->add($this->attrs);
        }
        echo $ldap->get_error();
    }


    static function plInfo()
    {
        return (array(
                    "plShortName"   => _("Device"),
                    "plDescription" => _("Device"),
                    "plSelfModify"  => FALSE,
                    "plDepends"     => array(),
                    "plPriority"    => 1,
                    "plSection"     => array("administration"),
                    "plCategory"    => array(
                        "Device" => array( 
                            "description"  => _("Device"),
                            "objectClass"  => "Device")),
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
                        "deviceStatus" => _("Stauts"),
                        "macAddress" => _("MAC address"),
                        "ipHostNumber" => _("IP address")
                        )
                        )
                        );
    }
}

?>
