<?php



class migrateFaiRDN extends migrateRDN implements propertyMigration
{
    protected $property = NULL;
    protected $config = NULL;
    protected $found = array();
    protected $filter ="";

    // Additional suffixes or prefixes 
    // e.g. for 'faiScriptRDN' (ou=scripts,) moving to new destination lets say 
    //  to 'ou=FAIscripts,' would break stuff with having 'ou=fai,ou=systems,ou=config' 
    // prepended.
    // 
    protected $suffix = ""; 
    protected $prefix = ""; 

    public $faiBaseRDN;

    function __construct($config,$property)
    {
        $this->property = &$property;
        $this->config = &$config;
    }   

    function checkForIssues()
    {
        $this->found = array();
        $ldap= $this->config->get_ldap_link();
        $ldap->cd($this->config->current['BASE']);
        $ldap2= $this->config->get_ldap_link();
        $ldap2->cd($this->config->current['BASE']);

        // If the userRDN wasn't empty, then only search for users inside of the old userRDN.
        $initialValue = $this->prefix.$this->property->getValue().$this->suffix;
        $targetValue = $this->prefix.$this->property->getValue(TRUE).$this->suffix;

        $prefixProp  = $this->config->configRegistry->getProperty('faiManagement','faiBaseRDN');
        $prefix      = $prefixProp->getValue();
        $prefixNew   = $prefixProp->getValue(TRUE);

        $dnMatch = "";
        if(!empty($initialValue) || !empty($prefix)){
            foreach(preg_split("/,/", $initialValue.$prefix) as $rdnPart){
                if(empty($rdnPart)) continue;
                list($namingAttrs, $container) = preg_split("/=/",$rdnPart,2);
                $container = trim($container,', ');
                $dnMatch.= "({$namingAttrs}:dn:={$container})";
            }
        }

        // Collect all FAI releases - this is nescessary to detect the release part
        //  of the dn
        $ldap->cd($this->config->current['BASE']);
        $releases = array();
        $ldap->search("(objectClass=FAIbranch)",array('dn'));
        while($attrs = $ldap->fetch()){
            $releases[$attrs['dn']] = $attrs['dn'];
        }

        // Search for users
        $filter = sprintf($this->filter,$dnMatch);
        $ldap->search($filter,array('dn'));
        $found = FALSE;
        while($attrs = $ldap->fetch()){
            $dn = $attrs['dn'];
            $dnTo = $dn;

            // Detect the releaes part of the dn.
            $releasePart = $dn; 
            while(!isset($releases[$releasePart]) && preg_match("/,/", $releasePart)){
                $releasePart = preg_replace("/^[^,]*+,/","",$releasePart);
            }
            if(!isset($releases[$releasePart])){
                // Release not found;
                continue;
            }

            // Detect the base part of the releasePart
            $basePart = $releasePart; 
            while(!isset($this->config->idepartments[$basePart]) && preg_match("/,/", $basePart)){
                $basePart = preg_replace("/^[^,]*+,/","",$basePart);
            }
            if(!isset($this->config->idepartments[$basePart])){
                // Base not found;
                continue;
            }

            $releaseRDN = substr($releasePart,0,strlen($releasePart) - strlen($basePart));

            // Now remove the faiBaseRDN from the release DN.
            $releaseRDN = preg_replace("/".preg_quote($prefix,'/')."/","", $releaseRDN);

            // Remove all parts from the dn to geht the objects name. 
            $name = substr($dn,0, (strlen($dn)  - strlen($initialValue.$releaseRDN.$prefix.$basePart)));

            // Build new container name and check if it exists.
            $newContainer = $targetValue.$releaseRDN.$prefixNew.$basePart;
            if(!$ldap->dn_exists($newContainer)){
                $this->found['add'][$newContainer] = array(); 
            }

            // Check if the target name has changed.
            $dnTo = $name.$newContainer;
            if($dn != $dnTo){
                $this->found['move'][] = array('from' => $dn, 'to' => $dnTo);
                $found = TRUE;
            }
        }
        return($found);
    }

    function execute()
    {
        $str = "";
        if(count($this->found['add'])) {
            $str.= "<br><h3>"._("Objects that will be added")."</h3>";
            foreach($this->found['add'] as $dn => $attrs){
                $str.= $dn."<br>";
            }
        }
        if(count($this->found['move'])) {
            $str.= "<br><h3>"._("Objects that will be moved")."</h3>";
            $str.="<pre>";
            foreach($this->found['move'] as $id => $data){
                $checked = (!isset($_POST["migrateNow".get_class($this)])) ? 'checked':'';
                $str.= "<input $checked type='checkbox' value='1' name='migrateEntry_{$id}'>";
                $str.= sprintf(_("Moving object '%s' to '%s'"), $data['from'], $data['to'])."<br>";
            }
            $str.="</pre>";
        }
        $str.= "<button name='migrateNow".get_class($this)."'>"._("Migrate")."</button>";
        return($str);
    }
   
    function save_object()
    {
        if(isset($_POST["migrateNow".get_class($this)])){
            $ldap = $this->config->get_ldap_link();
            $ldap->cd($this->config->current['BASE']);

            // Try to add the new container objects
            foreach($this->found['add'] as $dn => $data){
                $ldap->cd($this->config->current['BASE']);
                $ldap->create_missing_trees(ldap::convert($dn));
            }

            // Now move the objects to the new traget
            $tmp = new plugin($this->config,NULL);
            foreach($this->found['move'] as $id => $data){
                if(isset($_POST["migrateEntry_{$id}"])){
                    $tmp->move($data['from'], $data['to']);
                }
            }
            $this->checkForIssues();
        }
    } 

    function check()
    {
        return(array());
    }
} 


?>
