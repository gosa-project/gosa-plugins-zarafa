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
    
    private $resolvedNamesCache = array();


    /*! \brief  Constructs the device object and keep some 
     *           initial values.
     */
    function __construct(&$config, $dn)
    {
        plugin::plugin($config, $dn);
        $this->base = $this->config->current['BASE'];
        $this->orig_dn = $this->dn;
    }

    
    /*! \brief  Generate a fake uuid, it is not a real uuid.
     *  @return String  A fake uuid.
     */
    function genFakeUuid()
    {
        $strfmt = "xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx";
        for($i=0;$i<strlen($strfmt);$i++)
            if($strfmt[$i] == 'x') $strfmt[$i] = dechex(rand(0,15));
        return($strfmt);
    }


    /*! \brief  Generate the plugins HTML content.
     *  @return String  HTML content
     */
    function execute()
    {
        plugin::execute();    

        if($this->dialog instanceOf singleUserSelect) return($this->dialog->execute());

        $smarty = get_smarty();

        // Assign ACL settings 
        $plInfo = $this->plInfo();
        foreach($plInfo['plProvidedAcls'] as $name => $desc){
            $smarty->assign("{$name}ACL", $this->getacl($name));
        }
    
        // Assign just user names instead of the complete dn.
        $smarty->assign("owner_name", $this->getUserName($this->owner));
        $smarty->assign("manager_name", $this->getUserName($this->manager));

        foreach($this->attributes as $attr){
            $smarty->assign($attr, $this->$attr);
        }
        return($smarty->fetch(get_template_path('goto/Device/Device.tpl', TRUE)));
    }


    /*! \brief  Validate the user input.
     */ 
    function check()
    {   
        $message = plugin::check();
       
        // If one of the registered user attributes is set, 
        //  the others have to be set too.
        $str = $this->manager.$this->deviceUUID.$this->deviceStatus;
        if(!empty($str)){        
            foreach($this->dynClasses['registeredDevice'] as $attr){
                if(empty($this->$attr)){
                    $message[] = "Please set all values for 'registered device' or leave all blank! {$attr}";
                    break;
                }
            }
        }
 
        return($message);
    }

   
    /*! \brief  Detect an object's name by querying the ldap
     *           for the object's cn.
     * @param   String  The object's dn to query for.
     */ 
    function getUserName($dn)
    {   
        // First asked the cache if we've already queried this name.
        if(isset($this->resolvedNamesCache[$dn])) {
            return($this->resolvedNamesCache[$dn]);
        }

        // Try to detect the object's name via ldap search.
        $ldap = $this->config->get_ldap_link();
        $this->resolvedNamesCache[$dn] = "";
        if(!empty($dn)){
            $ldap->cat($dn, array('cn'));
            if($ldap->count()){
                $attrs = $ldap->fetch();
                $this->resolvedNamesCache[$dn] = $attrs['cn'][0];
                return( $attrs['cn'][0]);
            }else{
                $this->resolvedNamesCache[$dn] = "("._("unknown")."!): ".$dn;
            }
        }
        return($this->resolvedNamesCache[$dn]);
    }   
    
 
    /*! \brief  Detects and stores relevant values which where
     *           currently transmitted via $_GET/$_POST.
     */ 
    function save_object()
    {
        plugin::save_object();

        // Change owner requested, initiate a user selection dialog.
        if(isset($_POST['editOwner'])){
            $this->currentUserSelect = "owner";
            $this->dialog = new singleUserSelect($this->config, get_userinfo());
        }

        // Change manager requested, initiate a user selection dialog.
        if(isset($_POST['editManager'])){
            $this->currentUserSelect = "manager";
            $this->dialog = new singleUserSelect($this->config, get_userinfo());
        }

        // The user selection dialog has send that it has finished its job.
        // Store the posted user-dn as manager or owner. 
        if($this->dialog && $this->dialog instanceOf singleUserSelect && count($this->dialog->detectPostActions())){
            $users = $this->dialog->detectPostActions();
            if(isset($users['action']) && $users['action'] == 'userSelected' && isset($users['targets']) && count($users['targets'])){
                $headpage = $this->dialog->getHeadpage();
                $dn = $users['targets'][0];
                $attr = $this->currentUserSelect;
                $this->$attr = $dn;
                $this->dialog = NULL;
            }
        }
        
        // User selection canceled
        if(isset($_POST['add_users_cancel'])){
            $this->dialog = NULL;
        }
      
        // Regenerate a new uuid 
        if(isset($_POST['reloadUUID']))  $this->deviceUUID = $this->genFakeUuid();
    }


    /*! \brief  Save the modified object back to the ldap.
     */ 
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
                        "deviceStatus" => _("Status"),
                        "deviceType" => _("Type"),
                        "macAddress" => _("MAC address"),
                        "ipHostNumber" => _("IP address")
                        )
                        )
                        );
    }
}

?>
