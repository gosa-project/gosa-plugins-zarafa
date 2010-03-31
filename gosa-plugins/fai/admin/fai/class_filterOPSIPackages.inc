<?php

class filterOPSIPackages extends filterLDAP{

  static function query($base, $scope, $filter, $attributes, $category, $objectStorage= "")
  {
    global $config;
    $ui = get_userinfo();
    $ldap = $config->get_ldap_link();
    $ldap->cd($config->current['BASE']);

    // Append opsi stuff ...
    if(class_available('opsi') && $base == get_ou("faiBaseRDN").$config->current['BASE']){
      $opsi = new opsi($config);
      $objects = array();
      if($opsi instanceof opsi && $opsi->enabled()){
        $opsi_acl = $ui->get_permissions($base,"opsi/opsiProperties");
        if(preg_match("/r/",$opsi_acl)){
          $err = FALSE;

          $n_pro = $opsi->get_netboot_products();
          $err |= $opsi->is_error();
          foreach($n_pro as $name => $data){
            $entry = array(
                "cn" => $name,
                "description" => $data['DESC'],
                "type" => "opsi_netboot");
            $objects[$name]['opsi_netboot'] = $entry;
          }
          $l_pro = $opsi->get_local_products();
          $err |= $opsi->is_error();
          foreach($l_pro as $name => $data){
            $entry = array("cn" => $name,
                "description" => $data['DESC'],
                "type" => "opsi_local");
            $objects[$name]["opsi_local"] = $entry;
          }
          if($err){
            msg_dialog::display(_("Error"),msgPool::siError($opsi->get_error()),ERROR_DIALOG);
          }
        }
      }
      $cn_filter = "";
      if(preg_match("/^.*cn=([^)]*).*/",$filter)){
        $cn_filter = trim(preg_replace("/^.*cn=([^)]*).*/","\\1",$filter),"*");
      }

      foreach($objects as $name => $data)
      {
        $data = array_pop($data);
        if(!empty($cn_filter) && !preg_match("/".preg_quote($cn_filter,'/')."/",  $data['cn'])){
          continue;
        }
        $item =array();

        $item['objectClass']  = array('count' => 2, $data['type'],"FAKE_OC_OPSI");
        $item[] = 'objectClass';
        $item['cn']  = $data['cn'];
        $item[] = 'cn';
        $item['description']  = $data['description'];
        $item[] = 'description';
        $item['dn']  = $data['cn'];
        $item[] = 'dn';
        $item['TYPES']  = array($data['type']);
        $item[] = 'TYPES';
        $item['count'] = '5';
        $entries[] = $item;

      }
    }
    return($entries);
  }
}
?>
