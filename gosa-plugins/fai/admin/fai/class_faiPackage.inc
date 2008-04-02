<?php

class faiPackage extends plugin
{
  /* attribute list for save action */
  var $ignore_account   = TRUE;

  /* Attributes for this Object */
  var $attributes       = array("cn","description","FAIpackage","FAIdebianRelease","FAIdebianSection", "FAIinstallMethod");

  /* ObjectClasses for this Object*/
  var $objectclasses    = array("top","FAIclass","FAIpackageList","FAIrepository");

  /* Class name of the Ldap ObjectClass for the Sub Object */
  var $subClass         = "";
  var $subClasses       = array("top","FAIclass","FAIscriptEntry");

  /* Class name of the php class which allows us to edit a Sub Object */
  var $subClassName     = "";      

  /* Attributes to initialise for each subObject */
  var $subAttributes    = array("cn","description"); 
  var $sub64coded       = array();

  var $ConfiguredPackages = array();

  /* Specific attributes */
  var $cn               = "";       // The class name for this object
  var $description      = "";       // The description for this set of partitions
  var $is_dialog        = false;    // specifies which buttons will be shown to save or abort
  var $SubObjects       = array();  // All leafobjects of this object

  var $FAIdebianRelease          = ""; // The selected release
  var $FAIdebianSection          = array(); // selected section
  var $FAIinstallMethod          = "aptitude"; // hard coded
  var $mirror                    = ""; // selected mirror

  var $servers          = array();  // All available servers
  var $releases         = array();  // All possible releases 
  var $sections         = array();  // All section types

  var $list             = NULL;

  var $mirrors          = array();  // The combination of server/release/section
  var $confDir          = "";
  var $usedPackages     = array();
  var $buffer           = NULL; 
  var $strID            ="";
  var $newDialogShown   =false;

  var $FAIstate         = "";
  var $view_logged      = FALSE;
  var $base;
  var $FAIpackage ;

  var $FAIinstallMethods  = array( "install", "ninstall", "remove", 
      "dselect-upgrade", "taskinst", "taskrm",
      "hold", "clean", "aptitude", "aptitude-r",
      "pending", "dpkgc" );


  function faiPackage (&$config, $dn= NULL)
  {
    /* Load Attributes */
    plugin::plugin ($config, $dn);

    /* If "dn==new" we try to create a new entry
     * Else we must read all objects from ldap which belong to this entry.
     * First read SubObjects from ldap ... and then the partition definitions for the SubObjects.
     */
    if($dn != "new"){
      $this->dn =$dn;

      /* Get FAIstate */
      if(isset($this->attrs['FAIstate'][0])){
        $this->FAIstate = $this->attrs['FAIstate'][0];
      }

      /* Check if there are already some packages in this list 
       */
      $this->usedPackages = array();
      if(isset($this->attrs['FAIpackage'])){
        unset($this->attrs['FAIpackage']['count']);
        foreach($this->attrs['FAIpackage'] as $pkg){
          $this->usedPackages[$pkg] = $pkg;
        }
        ksort($this->usedPackages);
      }  

      /* Fetch all package configurations from ldap 
       */
      $PackageFilter = "";
      foreach($this->usedPackages as $name){
        $PackageFilter .= "(FAIpackage=".$name.")";
      }
      $PackageFilter = "(&(objectClass=FAIdebconfInfo)(|".$PackageFilter."))";

      /* Search for configuration objects */ 
      $ldap = $this->config->get_ldap_link();
      $ldap->cd($this->dn);
      $ldap->search($PackageFilter,array("FAIvariable","FAIvariableType",
            "FAIvariableContent","FAIpackage","FAIdebianSection","FAIstate"));

      /* Walk through configurations and append them to our list of ConfiguredPackages 
       */
      while($attr = $ldap->fetch()){

        /* Skip objects, that are tagged as removed */
        if(isset($object['FAIstate'][0])){
          if(preg_match("/removed$/",$attr['FAIstate'][0])){
            continue;
          }
        }

        $tmp =array(); 
        $tmp['Name']  = $attr['FAIvariable'][0];
        $tmp['Type']  = $attr['FAIvariableType'][0];

        if (isset($attr['FAIvariableContent'][0])){
          if(!in_array($attr['FAIvariableType'],array("multiselect"))){
            $tmp['Value'] = $attr['FAIvariableContent'][0];
          }else{
            $content = array();        
            unset($attr['FAIvariableContent']['count']);
            foreach($attr['FAIvariableContent'] as $attr){
              $tmp['Value'][] = $attr;
            }
          }
          $this->ConfiguredPackages[$attr['FAIpackage'][0]][$attr['FAIvariable'][0]]=$tmp;
        }
      }

      if (isset($this->attrs['FAIdebianSection']['count'])){
        unset($this->attrs['FAIdebianSection']['count']);
      }
      if((isset($this->attrs['FAIdebianSection']))&&(is_array($this->attrs['FAIdebianSection']))){
        $this->FAIdebianSection = array();
        foreach($this->attrs['FAIdebianSection'] as $sec){
          $this->FAIdebianSection[$sec]=$sec;
        }
      }

      if((isset($this->attrs['FAIdebianSection']))&&(is_string($this->attrs['FAIdebianSection']))){
        $this->FAIdebianSection=array($this->attrs['FAIdebianSection']=>$this->attrs['FAIdebianSection']) ;
      }
      $this->confDir = CONFIG_DIR."/fai/";
      $this->FAIpackage = array();

    } // ENDE  dn != new  

    $methods = array();
    foreach($this->FAIinstallMethods as $method){
      $methods[$method] = $method;
    }
    $this->FAIinstallMethods = $methods;

    /* Check if we exist already - no need to ask for revisions, etc. then */
    if ($this->dn != "new"){
      $this->newDialogShown= true;
    }
  }


  function execute()
  {
    /* Call parent execute */
    plugin::execute();

    if($this->is_account && !$this->view_logged){
      $this->view_logged = TRUE;
      new log("view","fai/".get_class($this),$this->dn);
    }

    /* Fill templating stuff */
    $smarty= get_smarty();
    $display= "";

    $smarty->assign( "FAIinstallMethods", $this->FAIinstallMethods );

    if(!$this->is_account){

      /* Assemble release name */
      $faifilter = session::get('faifilter');
      $tmp= preg_replace('/,'.normalizePreg(get_ou('faiou')).'.*$/', '', $faifilter['branch']);
      $tmp= preg_replace('/ou=/', '', $tmp);
      $rev= array_reverse(split(',', $tmp));
      $this->FAIdebianRelease= "";
      foreach ($rev as $part){
        $this->FAIdebianRelease.= "/$part";
      }
      $this->FAIdebianRelease= preg_replace('#^/#', '', $this->FAIdebianRelease);

      /* Assemble sections */
      $repos= $this->getServerInfos();
      if(isset($repos[$this->FAIdebianRelease])){
        $this->FAIdebianSection= $repos[$this->FAIdebianRelease];
        $this->FAIdebianSection= array_unique($this->FAIdebianSection);
      }

      /* Assign Repository settings*/ 
      $this->is_account     = true;
    }

    /* Assign variables */
    foreach($this->attributes as $attrs){
      $smarty->assign($attrs,$this->$attrs);
    }

    /* Generate package list */
    $this->list= $this->genPkgs();

    /* + was pressed to open the package dialog */
    if(isset($_POST['Addpkg']) && $this->acl_is_writeable("FAIpackage")){
      $this->dialog = new faiPackageEntry($this->config, $this->dn,$this->list,$this->usedPackages);
      $this->is_dialog =true;
    }

    /* Delete selected package */ 
    if(isset($_POST['Delpkg']) && $this->acl_is_writeable("FAIpackage")){
      if($this->FAIstate != "freeze"){
        foreach($_POST['usedPackages'] as $del){
          if(isset($this->usedPackages[$del])){
            unset($this->usedPackages[$del]);
          }
        }
      }
    }

    /* Abort package selection dialog */ 
    if(isset($_POST['CancelSubObject'])){
      $this->dialog = false;
      $this->is_dialog=false;
    }

    /* attach new packages */
    if(isset($_POST['SaveSubObject'])) {
      if($this->FAIstate != "freeze"){
        $this->dialog->save_object();
        if(count($this->dialog->check())){
          foreach($this->dialog->check() as $msgs){
            msg_dialog::display(_("Error"), $msgs, ERROR_DIALOG);
          }
        }else{
          $use = $this->dialog->save();
          $this->usedPackages = $use;
          $this->dialog = false;
          $this->is_dialog=false;
          ksort($this->usedPackages);
        }
      }else{
        $this->dialog = false;
        $this->is_dialog=false;
      }
    }

    /* Configuration dialog open*/
    if((isset($_POST['Conpkg']))&&(isset($_POST['usedPackages']))&&(!empty($_POST['usedPackages'][0])) && $this->acl_is_writeable("FAIdebconfInfo")){
      $pkg_config = array();
      $pkg = $_POST['usedPackages'][0];
      if(isset($this->ConfiguredPackages[$pkg])){
        $pkg_config = $this->ConfiguredPackages[$pkg];
      }
      $this->dialog = new faiPackageConfiguration($this->config, $this->dn,$pkg, $this->FAIdebianRelease , $pkg_config);
      $this->is_dialog =true;
    }

    /* Configuration dialog open*/
    if($this->FAIstate != "freeze" && $this->acl_is_writeable("FAIpackage")){
      if((isset($_POST['Markpkg']))&&(isset($_POST['usedPackages']))&&(!empty($_POST['usedPackages']))){
        foreach($_POST['usedPackages'] as $pkg){
          if (isset($this->usedPackages[$pkg])){
            unset($this->usedPackages[$pkg]);
            if (preg_match('/^-/', $pkg)){
              $pkg= preg_replace('/^-/', '', $pkg);
            } else {
              $pkg= preg_replace('/^/', '-', $pkg);
            }
            $this->usedPackages[$pkg]= $pkg;
          }
        }
      }
    }

    /* Save Configuration */
    if(isset($_POST['SaveObjectConfig'])){
      if($this->FAIstate != "freeze"){
        $this->ConfiguredPackages= array_merge($this->ConfiguredPackages,$this->dialog->save());
      }
      $this->dialog = false;
      $this->is_dialog=false;
    }

    /* cancel configuration */     
    if(isset($_POST['CancelObjectConfig'])){
      $this->dialog = false;
      $this->is_dialog=false;
    }

    /* Display dialog */ 
    if($this->is_dialog){
      return $this->dialog->execute();
    }

    /* Assign section to smarty */
    $strsec = "";
    foreach($this->FAIdebianSection as $sec){
      $strsec .= $sec." ";
    }

    $tmp = $this->plInfo();
    foreach($tmp['plProvidedAcls'] as $name => $translated){
      $smarty->assign($name."ACL",$this->getacl($name,preg_match("/freeze/",$this->FAIstate)));
    }

    $smarty->assign("releases",$this->releases);
    $smarty->assign("release" ,$this->FAIdebianRelease);
    $smarty->assign("sections",$this->sections);
    $smarty->assign("section" ,$strsec);
    $smarty->assign("usedPackages",$this->printUsedPackages());
    $display.= $smarty->fetch(get_template_path('faiPackage.tpl', TRUE));
    return($display);
  }

  /* Delete me, and all my subtrees
   */
  function remove_from_parent()
  {
    $ldap = $this->config->get_ldap_link();
    $ldap->cd ($this->dn);

    $faifilter = session::get('faifilter');
    $use_dn = preg_replace("/".normalizePreg(FAI::get_release_dn($this->dn))."/i", $faifilter['branch'], $this->dn);
    if($faifilter['branch'] == "main"){
      $use_dn = $this->dn;
    }

    FAI::prepare_to_save_FAI_object($use_dn,array(),true);

    new log("remove","fai/".get_class($this),$use_dn,$this->attributes);

    foreach($this->ConfiguredPackages as $pkgname => $attrs){
      foreach($attrs as $name => $attr){
        $pkgdn =  "FAIvariable=".$name.",".$this->dn;
        $use_dn = preg_replace("/".normalizePreg(FAI::get_release_dn($this->dn))."/i", $faifilter['branch'], $pkgdn);
        if($faifilter['branch'] == "main"){
          $use_dn = $obj['dn'];
        }
        FAI::prepare_to_save_FAI_object($use_dn,array(),true);
      }
    }
    $this->handle_post_events("remove");
  }


  /* Save data to object 
   */
  function save_object()
  {
    if($this->FAIstate == "freeze") return;  
    plugin::save_object();
  }


  /* Check supplied data */
  function check()
  {
    /* Call common method to give check the hook */
    $message= plugin::check();

    if(count($this->usedPackages)==0){
      $message[]= _("Please select a least one package!");
    }

    if((empty($this->FAIdebianRelease))||(empty($this->FAIdebianSection))){
      $message[]=_("Please choose a valid release/section combination for your repository setup!");
    }

    return ($message);
  }

  function printUsedPackages(){
    $a_ret=array(); 
    if(is_array($this->usedPackages)) {
      foreach($this->usedPackages as $usedName){

        $config = 0;

        foreach($this->ConfiguredPackages as $name => $value){
          if($name == $usedName){
            $config ++;
          }
        }

        $c_str ="";
        if($config){
          $c_str = " - "._("package is configured");
        }

        /* Adapt used name if we're marked for removal */
        $dsc= "";
        if (preg_match('/^-/', $usedName)){
          $dsc= " - "._("Package marked for removal");
          // Generally a bad idea here, because the toggel triggers on -, not on !
          //$usedName= preg_replace('/^-/', '! ', $usedName);
        }else{
          $usedName2= $usedName;
        }

        if(isset($this->list[$usedName][1])){
          $a_ret[$usedName] = $usedName2." [".$this->list[$usedName][1]."]".$c_str.$dsc;
        }else{
          $a_ret[$usedName] = $usedName2.$c_str.$dsc;
        }
      }
    }
    return($a_ret);
  }


  function genPkgs()
  {
    $start = microtime(1);
    if($this->buffer === NULL || !count($this->buffer)){
      $this->buffer = array();
      $q = new gosaSupportDaemon();
      $attrs = array("distribution", "package","version", "section", "description", "timestamp","template=''");
      $ret = $q->FAI_get_packages($this->FAIdebianRelease,$attrs);
      if($q->is_error()){
        msg_dialog::display(_("Service infrastructure"),msgPool::siError($q->get_error()),ERROR_DIALOG);
      }else{
        foreach($ret as $attr){
          $this->buffer[$attr['PACKAGE']] = array($attr['PACKAGE'],$attr['VERSION'],$attr['SECTION'],$attr['DESCRIPTION']);
        }
      }
    }
    
#    printf("%0.6f",(microtime(1) - $start));
    return $this->buffer;
  }


  /* Save to LDAP */
  function save()
  {

    /* Assemble release name */
    if($this->FAIdebianRelease == "ClearFromCopyPaste"){
      $faifilter = session::get('faifilter');
      $tmp= preg_replace('/,'.normalizePreg(get_ou('faiou')).'.*$/', '', $faifilter['branch']);
      $tmp= preg_replace('/ou=/', '', $tmp);
      $rev= array_reverse(split(',', $tmp));
      $this->FAIdebianRelease= "";
      foreach ($rev as $part){
        $this->FAIdebianRelease.= "/$part";
      }
      $this->FAIdebianRelease= preg_replace('#^/#', '', $this->FAIdebianRelease);
    }

    plugin::save();

    $ldap = $this->config->get_ldap_link();

    $this->attrs['FAIpackage'] = array();
    foreach($this->usedPackages as $pkg => $obj){
      $this->attrs['FAIpackage'][] = $pkg;
    } 

    $this->attrs['FAIdebianSection'] = array();
    foreach($this->FAIdebianSection as $sec){
      $this->attrs['FAIdebianSection'][] = $sec;
    }

    FAI::prepare_to_save_FAI_object($this->dn,$this->attrs);
    if (!$ldap->success()){
      msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $this->dn, 0, get_class()));
    }
    
    if($this->initially_was_account){
      new log("modify","fai/".get_class($this),$this->dn,$this->attributes);
    }else{
      new log("create","fai/".get_class($this),$this->dn,$this->attributes);
    }

    $ldap->cd($this->dn);

    /* Save Package configurations */
    foreach($this->ConfiguredPackages as $pkgname => $attrs){
      foreach($attrs as $name => $attr){
      
        $pkgattrs = array();

        foreach($attr as $n=>$v){
          if(empty($v)) $v = array();
        }

        /* Set attributes */
        $pkgattrs['objectClass'][]        = "FAIdebconfInfo";

        $pkgattrs['FAIpackage']           = $pkgname;
        $pkgattrs['FAIvariable']          = $name;
        $pkgattrs['FAIvariableType']      = $attr['Type'];
        $pkgattrs['FAIvariableContent']   = $attr['Value'];
        $pkgdn =  "FAIvariable=".$name.",".$this->dn;

	/* Tag object */
	$this->tag_attrs($pkgattrs, $pkgdn, $this->gosaUnitTag);

        if(in_array($pkgattrs['FAIvariableType'],array("boolean","multiselect","password","select","string","text"))){
          FAI::prepare_to_save_FAI_object($pkgdn,$pkgattrs);
        }
      }
    }
  }

  /* Return plugin informations for acl handling */ 
  static function plInfo()
  {
    return (array( 
          "plShortName" => _("Package"),
          "plDescription" => _("FAI Package list"),
          "plSelfModify"  => FALSE,
          "plDepends"     => array(),
          "plPriority"    => 28,
          "plSection"     => array("administration"),
          "plCategory"    => array("fai"),
          "plProvidedAcls" => array(
            "cn"                => _("Name"),
            "description"       => _("Description"),
            "FAIpackage"        => _("Packages"),
            "FAIdebianSection"  => _("Section")."&nbsp;("._("Readonly").")",
            "FAIinstallMethod"  => _("Install Method"),
            "FAIdebconfInfo"    => _("Package configuration"),
            "FAIdebianRelease"  => _("Release")."&nbsp;("._("Readonly").")")
          ));
  }




  function PrepareForCopyPaste($source)
  {
    plugin::PrepareForCopyPaste($source);

    if(isset($source['FAIstate'][0])){
      $this->FAIstate = $source['FAIstate'][0];
    }

    $this->FAIdebianRelease = "ClearFromCopyPaste";

    if(isset($source['FAIpackage'])){
      unset($source['FAIpackage']['count']);
      foreach($source['FAIpackage'] as $pkg){
        $this->usedPackages[$pkg] = $pkg;
      }
      ksort($this->usedPackages);
    }else{
      $this->usedPackages = array();
    }

    if((isset($source['FAIdebianSection']))&&(is_array($source['FAIdebianSection']))){
      $this->FAIdebianSection = array();
      for($i = 0 ; $i < $source['FAIdebianSection']['count'] ; $i ++){
        $this->FAIdebianSection[$source['FAIdebianSection'][$i]]=$source['FAIdebianSection'][$i];
      }
    }

    /* Create one filter with all package names, 
       instead of calling $ldap->search for every single package 
     */
    $PackageFilter = "";
    foreach($this->usedPackages as $name){
      $PackageFilter .= "(FAIpackage=".$name.")";
    }
    $PackageFilter = "(&(objectClass=FAIdebconfInfo)(|".$PackageFilter."))";

    /* Search for configuration objects */ 
    $ldap = $this->config->get_ldap_link();
    $ldap->cd($source['dn']);
    $ldap->search($PackageFilter,array("FAIvariable","FAIvariableType","FAIvariableContent","FAIpackage","FAIdebianSection","FAIstate"));

    /* Walk through configurations and append them to our list of ConfiguredPackages */
    while($attr = $ldap->fetch()){

      /* Skip objects, that are tagged as removed */
      if(isset($object['FAIstate'][0])){
        if(preg_match("/removed$/",$attr['FAIstate'][0])){
          continue;
        }
      }

      $tmp =array(); 
      $tmp['Name']  = $attr['FAIvariable'][0];
      $tmp['Type']  = $attr['FAIvariableType'][0];

      if (isset($attr['FAIvariableContent'][0])){
        if(!in_array($attr['FAIvariableType'],array("multiselect"))){
          $tmp['Value'] = $attr['FAIvariableContent'][0];
        }else{
          $content = array();        
          unset($attr['FAIvariableContent']['count']);
          foreach($attr['FAIvariableContent'] as $attr){
            $tmp['Value'][] = $attr;
          }
        }
        $this->ConfiguredPackages[$attr['FAIpackage'][0]][$attr['FAIvariable'][0]]=$tmp;
      }
    }
  }


  function getServerInfos()
  {
    $ret = array();
    $ldap = $this->config->get_ldap_link();
    $ldap->cd($this->config->current['BASE']);
    $ldap->search("(objectClass=FAIrepositoryServer)",array("*"));
    while($attrs = $ldap->fetch()){
      if(isset($attrs['FAIrepository'])){
        for($i =0 ; $i < $attrs['FAIrepository']['count']; $i++){
          $obj = $attrs['FAIrepository'][$i];
          $tmp = split("\|",$obj);
          if(count($tmp)==4){
            foreach(split(",",$tmp[3]) as $sec){
              if(!empty($sec)){
                $ret[$tmp[2]][] =  $sec;
              }
            }
          }
        }
      }
    }
    return($ret);
  }


}

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
