<?php
/*
  This code is part of GOsa (https://gosa.gonicus.de)
  Copyright (C) 2004-2007  Cajus Pollmeier

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

class dhcpNetwork extends plugin
{
  /* Used attributes */
  var $options= array();
  var $statements= array();

  /* attribute list for save action */
  var $attributes= array();
  var $objectclasses= array();

  function dhcpNetwork()
  {
    /* This is always an account */
    $this->is_account= TRUE;
  }

  function execute()
  {

    $acl_writeable = preg_match("/w/",$this->parent->getacl(""));

    /* Check for iteraction */
    if ($acl_writeable && isset($_POST['add_dns']) && $_POST['addserver'] != ""){
      if (!preg_match('/^[0-9a-z.-]+$/', get_post('addserver'))){
        msg_dialog::display(_("Error"), msgPool::invalid(_("Name"),
              htmlentities(get_post('addserver')),"/[0-9a-z.-]/"), ERROR_DIALOG);
      } else {
        $servers= array();
        $chk = dhcpAdvanced::getOption('domain-name-servers',$this->options,true);
        if($chk != ""){
          foreach(split(",", $chk) as $val){
            $servers[$val]= $val;
          }
        }
        $servers[get_post('addserver')]= get_post('addserver');

        $tmp= "";
        foreach($servers as $val){
          $tmp.= $val.",";
        }
        $chk = dhcpAdvanced::getOption('domain-name-servers',$this->options);
        if($chk != ""){
          unset($this->options[$chk]);
        }
        $new = "domain-name-servers ".preg_replace('/,$/', '', $tmp);
        $this->options[$new]=$new; 
      }
    }
    if ($acl_writeable && isset($_POST['delete_dns']) && isset($_POST['dnsserver'])){
      $val = dhcpAdvanced::getOption('domain-name-servers', $this->options, true);
      $key = dhcpAdvanced::getOption('domain-name-servers', $this->options);
  
      if($key != ""){
        unset($this->options[$key]);
        $tmp= preg_replace("/(\s*,\s*)?".get_post('dnsserver')."/i",'', $val);
        $tmp= preg_replace("/(\s*)?,(\s*)?$/", '', $tmp);
        if ($tmp != ""){
          $tmp ="domain-name-servers {$tmp}";
          $this->options[$tmp]= $tmp;
        }
      }

    }

    /* Show main page */
    $smarty= get_smarty();

    /* Assign ACLs */
    $smarty->assign("acl",$this->parent->getacl(""));

    /*
     * Assemble options
     */

    /* Router */
    $smarty->assign("routers", dhcpAdvanced::getOption('router',$this->options));

    /* DNS */
    $smarty->assign("domain", dhcpAdvanced::getOption('domain-name',$this->options));
    if (dhcpAdvanced::getOption('domain-name-servers',$this->options) != ""){
      $servers= array();
      foreach(split(",", dhcpAdvanced::getOption('domain-name-servers',$this->options, true)) as $val){
        $servers[$val]= $val;
      }
      $smarty->assign("dnsservers", $servers);
    } else {
      $smarty->assign("dnsservers", "");
    }

    /* Netmask / Broadcast */
    if (dhcpAdvanced::getOption('subnet-mask', $this->options) != ""){
      $key = dhcpAdvanced::getOption('subnet-mask', $this->options);
      $val = dhcpAdvanced::getOption('subnet-mask', $this->options,true);

      unset($this->options[$key]);
      $new = 'subnet-mask '.normalize_netmask($val);
      $this->options[$new]=$new;
      $smarty->assign("subnet_mask", $new);
    } else {
      $smarty->assign("subnet_mask", "");
    }
    $smarty->assign("broadcast_address", dhcpAdvanced::getOption('broadcast-address',$this->options));

    /* Boot stuff */
    if (isset($this->statements['filename'])){
      $smarty->assign("filename", trim($this->statements['filename'], '"'));
    } else {
      $smarty->assign("filename", "");
    }
    if (isset($this->statements['next-server'])){
      $smarty->assign("nextserver", $this->statements['next-server']);
    } else {
      $smarty->assign("nextserver", "");
    }

    /* Set flags */
    $smarty->assign("autohost", "");
    if (isset($this->statements['get-lease-hostnames'])){
      if (preg_match('/^(true|on|yes)$/', $this->statements['get-lease-hostnames'])){
        $smarty->assign("autohost", "checked");
      }
    }
    $smarty->assign("autohostdecl", "");
    if (isset($this->statements['use-host-decl-names'])){
      if (preg_match('/^(true|on|yes)$/', $this->statements['use-host-decl-names'])){
        $smarty->assign("autohostdecl", "checked");
      }
    }

    return $smarty->fetch(get_template_path('dhcp_network.tpl', TRUE,dirname(__FILE__)));
  }

  function remove_from_parent()
  {
  }


  /* Save data to object */
  function save_object()
  {
    /* Only save, if we are "active" */
    if (isset($_POST['routers']) && preg_match("/w/",$this->parent->getacl(""))){

      /*
       * Assemble options
       */

      /* Options */
      foreach (array("routers" => "routers", "domain-name" => "domain", "subnet-mask" => "subnet_mask",
                     "broadcast-address" => "broadcast_address") as $key => $val){

        $ikey = dhcpAdvanced::getOption($key, $this->options);
        if($ikey != ""){
          unset($this->options[$ikey]);
        }
        if ($_POST["$val"] != ''){
          $new = $key." ".$get_post($val);
          $this->options[$new]=$new; 
        }
      }

      /* Statements */
      foreach (array("filename" => "filename", "next-server" => "nextserver") as $key => $val){
        if ($_POST["$val"] == ''){
          unset($this->statements["$key"]);
        } else {
    
          /* Only quote filename values */
          if(in_array($key,array("filename"))){
            $this->statements["$key"]= '"'.get_post("$val").'"';
          }else{
            $this->statements["$key"]= get_post("$val");
          }
        }
      }

      /* Flags */
      if (isset ($_POST['autohost'])){
        $this->statements['get-lease-hostnames']= "true";
      } else {
        unset($this->statements['get-lease-hostnames']);
      }
      if (isset ($_POST['autohostdecl'])){
        $this->statements['use-host-decl-names']= "on";
      } else {
        unset($this->statements['use-host-decl-names']);
      }
    }
  }


  /* Check values */
  function check()
  {
    $message= array();

    /* Check netmask and broadcast */
    foreach(array("subnet-mask" => _("Netmask"), "broadcast-address" => _("Broadcast")) as $key => $typ){
      $ikey = dhcpAdvanced::getOption($key,$this->options);
      $ival = dhcpAdvanced::getOption($key,$this->options,true);
      if ($ikey == ""){
        continue;
      }
      $tmp= preg_replace('/^[^\s]+\s/', '', $ival);
      if (!tests::is_ip($tmp)){
        $message[]= sprintf(_("Error in definition of '%s'!"), $typ);
      }
    }

    return $message;
  }


  /* Save to LDAP */
  function save()
  {
  }
  
}
// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
