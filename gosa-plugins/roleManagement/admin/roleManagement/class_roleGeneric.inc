<?php
/*
* This code is part of GOsa (http://www.gosa-project.org)
* Copyright (C) 2003-2008 GONICUS GmbH
*
* ID: $$Id: class_roleManagement.inc 13520 2009-03-09 14:54:13Z hickert $$
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/


class roleGeneric extends plugin {

  var $cn = "";
  var $description ="";
  var $telephoneNumber = "";
  var $facsimileTelephoneNumber = "";
  var $x121Address = "";
  var $roleOccupant = array();

  var $base = "";
  var $orig_dn = "";
  var $orig_cn = "";
  var $orig_base = "";

  var $objectclasses = array("top","organizationalRole");
  var $attributes = array("cn","x121Address","description",
    "telephoneNumber","facsimileTelephoneNumber","roleOccupant");
  
  function __construct($config,$dn){
    plugin::plugin($config,$dn);
    $this->is_account = TRUE;
    $this->orig_dn = $dn;
    $this->orig_cn = $this->cn;

    /* Set base */
    if ($this->dn == "new"){
      $this->base = session::get('CurrentMainBase');
    } else {
      $this->base= preg_replace("/^[^,]+,".preg_quote(get_ou("roleRDN"), '/')."/","",$this->dn);
    }
    $this->orig_base = $this->base;
  }


  function execute()
  {
    $smarty = get_smarty();

    /* Create base acls */
    $tmp = $this->allowedBasesToMoveTo();
    $smarty->assign("bases", $tmp);
    $smarty->assign("base_select",$this->base);
    $smarty->assign("members",$this->convert_list());
    foreach($this->attributes as $attr){
      $smarty->assign($attr,$this->$attr);
    }
    $tmp = $this->plInfo();
    foreach($tmp['plProvidedAcls'] as $attr => $desc){
      $smarty->assign($attr."ACL",$this->getacl($attr));
    }
    return($smarty->fetch(get_template_path('roleGeneric.tpl',TRUE,dirname(__FILE__))));
  }


  function check()
  {
    $message = plugin::check();

    /* Set the new acl base */
    if($this->dn == "new") {
      $this->set_acl_base($this->base);
    }

    /* Check if we are allowed to create/move this user
     */
    if($this->orig_dn == "new" && !$this->acl_is_createable($this->base)){
      $message[]= msgPool::permCreate();
    }elseif($this->orig_dn != "new" && 
        !$this->acl_is_moveable($this->base) && 
        ($this->orig_base != $this->base || $this->orig_cn != $this->cn )){ 
      $message[]= msgPool::permMove();
    }

    /* Name already used? */
    $ldap= $this->config->get_ldap_link();
    $ldap->cd($this->config->current['BASE']);
    $ldap->search("(&(objectClass=organizationalRole)(cn=$this->cn))", array("cn"));
    $ldap->fetch();
    if ($ldap->count() != 0 && ( $this->dn == 'new' || $this->cn != $this->orig_cn)){
      $message[]= msgPool::duplicated(_("Name"));
    }


    return($message);
  }
  

  function convert_list()
  {
    $temp= "";
    foreach ($this->roleOccupant as $key => $value){
      $temp.= "
        <option title='".addslashes( $key)."' 
        value=\"$key\" class=\"select\" 
        >".$value['text']."</option>\n";
    }

    return ($temp);
  }

  
  function remove_from_parent()
  {
    plugin::remove_from_parent();

    $ldap= $this->config->get_ldap_link();
    $ldap->rmdir($this->dn);
    if (!$ldap->success()){
      msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $this->dn, 0, get_class()));
    }

    new log("remove","roles/".get_class($this),$this->dn,array_keys($this->attrs),$ldap->get_error());

    /* Trigger remove signal */
    $this->handle_post_events("remove");
  }


  function save(){
    plugin::save();

    /* Save data. Using 'modify' implies that the entry is already present, use 'add' for
       new entries. So do a check first... */
    $ldap = $this->config->get_ldap_link();
    $ldap->cat ($this->dn, array('dn'));
    if ($ldap->fetch()){
      $mode= "modify";
    } else {
      $mode= "add";
      $ldap->cd($this->config->current['BASE']);
      $ldap->create_missing_trees(preg_replace('/^[^,]+,/', '', $this->dn));
    }
    @DEBUG (DEBUG_LDAP, __LINE__, __FUNCTION__, __FILE__,$this->attributes, "Save via $mode");

    /* Finally write data with selected 'mode' */
    $this->cleanup();
    $ldap->cd ($this->dn);
    $ldap->$mode ($this->attrs);
    if (!$ldap->success()){
      msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), 
        $this->dn, LDAP_DEL,            get_class()));
      return (1);
    }

    $this->handle_post_events($mode);

    /* Remove ACL dependencies too */
    if($this->dn != $this->orig_dn && $this->orig_dn != "new"){
      $tmp = new acl($this->config,$this->parent,$this->dn);
      $tmp->update_acl_membership($this->orig_dn,$this->dn);
    }

    if($mode == "modify"){
      new log("modify","users/".get_class($this),$this->dn,array_keys($this->attrs),$ldap->get_error());
    }else{
      new log("create","users/".get_class($this),$this->dn,array_keys($this->attrs),$ldap->get_error());
    }
  }


  /* This avoids that users move themselves out of their rights.
   */
  function allowedBasesToMoveTo()
  {
    /* Get bases */
    $bases  = $this->get_allowed_bases();
    return($bases);
  }

  
  static function plInfo()
  {
    return (array(
          "plShortName"   => _("Generic"),
          "plDescription" => _("Role generic"),
          "plSelfModify"  => FALSE,
          "plDepends"     => array(),
          "plPriority"    => 1,
          "plSection"     => array("administration"),
          "plCategory"    => array("roles" => array("description"  => _("Roles"),
              "objectClass"  => "organizationalRole")),
          "plProvidedAcls"=> array(
            "cn"                => _("Name"),
            "description" => _("Description"),
            "base" => _("Base"),
            "telephoneNumber" => _("Telefon number"),
            "facsimileTelephoneNumber" => _("Fax number"),
            "roleOccupant" => _("Occupants"),
            "x121Address" => _("X.121 Address"))
          ));
  }
}


// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
