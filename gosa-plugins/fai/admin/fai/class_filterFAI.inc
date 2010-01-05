<?php

class filterFAI extends filterLDAP{

  static function query($base, $scope, $filter, $attributes, $category, $objectStorage= "")
  {
  /* Create a new list of FAI object
     * Generate List of Partitions,Hooks,Scripts,Templates,Profiles ...
     */
    $ObjectTypes = array(
        "FAIpartitionTable"  => array("OU"=> get_ou('faiPartitionRDN') , "CHKBOX"=>"ShowPartitions"  ,"ACL" => "faiPartitionTable"),
        "FAIpackageList"     => array("OU"=> get_ou('faiPackageRDN')   , "CHKBOX"=>"ShowPackages"    ,"ACL" => "faiPackage"),
        "FAIscript"          => array("OU"=> get_ou('faiScriptRDN')    , "CHKBOX"=>"ShowScripts"     ,"ACL" => "faiScript"),
        "FAIvariable"        => array("OU"=> get_ou('faiVariableRDN')  , "CHKBOX"=>"ShowVariables"   ,"ACL" => "faiVariable"),
        "FAIhook"            => array("OU"=> get_ou('faiHookRDN')      , "CHKBOX"=>"ShowHooks"       ,"ACL" => "faiHook"),
        "FAIprofile"         => array("OU"=> get_ou('faiProfileRDN')   , "CHKBOX"=>"ShowProfiles"    ,"ACL" => "faiProfile"),
        "FAItemplate"        => array("OU"=> get_ou('faiTemplateRDN')  , "CHKBOX"=>"ShowTemplates"   ,"ACL" => "faiTemplate"));



    global $config;
    $tmp = FAI::get_all_objects_for_given_base($base,$filter);
    $ui = get_userinfo();
    $ldap = $config->get_ldap_link();
    $ldap->cd($config->current['BASE']);

    $result = array();
    foreach($tmp as $entry){

      /* Get some more informations about the object */
      $ldap->cat($entry['dn'], array("cn","description","objectClass","FAIclass","FAIstate","objectClass"));
      $object  = $ldap->fetch();

      /* Walk through possible types */
      foreach($ObjectTypes as $type => $rest){

        $acl = $ui->get_permissions($object['dn'],"fai/".$rest ['ACL']);

        if(in_array($type,$object['objectClass']) && preg_match("/r/",$acl)){

          /* Prepare object */
          unset($object['objectClass']['count']);
          if(!isset($object['description'][0])){
            $object['description'][0]="";
          }

          $dn = preg_replace("/^[^,]*+,/","",$object['dn']);

          /* Clean up object informations */
          $obj                  = array();
          $obj['cn']            = $object['cn'][0];
          $obj['dn']            = $object['cn'][0];
          $obj['acl']           = $acl;
          $obj['class']         = $rest ['ACL'];
          $obj['FAIstate']      = $entry['FAIstate'];
          $obj['description']   = $object['description'][0];
          $obj['objectClass']   = $object['objectClass'];
          $obj['TYPES'][]       = $type;
        
          if(!isset($result[$obj['cn']])){
            $result[$obj['cn']] = $obj;
          }else{
            $result[$obj['cn']]['TYPES'][]=$type;
          }
        }
      }
    }

    // Make results useable for the listing class.
    $entries = array();
    foreach($result as $entry){
      $item=array('count' => count($entry));
      foreach($entry as $name => $value){
        $item[] = $name;
        $item[$name] = $value ;       
      }
      $entries[] = $item;
    }

    return($entries);
  }
}
?>
