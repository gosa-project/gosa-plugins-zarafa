<?php
/*
* This code is part of GOsa (http://www.gosa-project.org)
* Copyright (C) 2003-2008 GONICUS GmbH
*
* ID: $$Id: class_opsiLicenses.inc 13520 2009-03-09 14:54:13Z hickert $$
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


class licenseGeneric extends plugin {

  // The variables this plugin takes care of.
  var $cn = "";
  var $description ="";
  var $telephoneNumber = "";
  var $facsimileTelephoneNumber = "";

  // The objects base 
  var $base = "";
 
  // Keep track if possible ng aming modifications
  var $orig_dn = "";
  var $orig_cn = "";
  var $orig_base = "";

  // The object classes written by this plugin
  var $objectclasses = array("top","organizationalLicense");

  // The list of occupants  ([dn])
  var $licenseOccupant = array();
  
  // The licenseOccupant cache, dn=>attrs
  var $licenseOccCache = array();

  // A list of attributes managed by this plugin
  var $attributes = array("cn","description",
    "telephoneNumber","facsimileTelephoneNumber","licenseOccupant");
 

  /* Initialize the class 
   */
  function __construct($config,$dn){
    plugin::plugin($config,$dn);
    $this->is_account = TRUE;

    // Initialize list of occupants
    $this->licenseOccupant = array();
    if(isset($this->attrs['licenseOccupant'])){
      for($i=0;$i<$this->attrs['licenseOccupant']['count']; $i++){
        $this->licenseOccupant[] = $this->attrs['licenseOccupant'][$i];
      }
    }

    // Detect the objects base
    if ($this->dn == "new"){
      $this->base = session::get('CurrentMainBase');
    } else {
      $this->base= preg_replace("/^[^,]+,".preg_quote(get_ou("licenseRDN"), '/i')."/","",$this->dn);
    }

    // Keep track of naming attribute modifications
    $this->orig_base = $this->base;
    $this->orig_dn = $dn;
    $this->orig_cn = $this->cn;

    // Reload the occupant cache. 
    $this->reload();
  }


  /* Keep occupant cache up to date. 
   * Else, we may have entries we can't display.
   */
  function reload()
  {
    // Entries can't be added twice. 
    $attrs = array("description", "objectClass", "uid","cn");
    $this->licenseOccupant = array_unique($this->licenseOccupant);
    $this->licenseOccupant = array_values($this->licenseOccupant);
  
    $ldap = $this->config->get_ldap_link();
    foreach($this->licenseOccupant as $dn){
      if(!isset($this->licenseOccCache[$dn])){
        if($ldap->dn_exists($dn)){
          $ldap->cat($dn, $attrs);

          $tmp = $ldap->fetch();
          if(!isset($tmp['cn'])){
         
            // Extract the namingAttribute out of the dn.
            $cn = preg_replace("/^[^=]*+=([^,]*).*$/","\\1",$tmp['dn']);
            if(isset($tmp['uid'])){
              $cn = $tmp['uid'][0];
            }
            if(isset($tmp['description'])){
              $cn.= " [".$tmp['description'][0]."]";
            }
            $tmp['cn'][0] = $cn;
          }

          $this->licenseOccCache[$dn] = $tmp;
        }
      }
    }

  }
  

  function getOccupants(){
    return($this->licenseOccupant);
  }

  /* Generate HTML output of this plugin.
   */
  function execute()
  {
    // Get list of possible ldap bases, will be selectable in the ui.
    $tmp = $this->allowedBasesToMoveTo();


    /***************
     * Dialog handling
     ***************/

    if(isset($_POST['edit_membership']) && !$this->dialog instanceOf plugin){
      $this->dialog = new occupantSelect($this->config,$this->dn,$this);
    }
    if(isset($_POST['delete_membership']) && !$this->dialog instanceOf plugin){
      if(isset($_POST['members'])){
        foreach($_POST['members'] as $id){
          if(isset($this->licenseOccupant[$id])){
            unset($this->licenseOccupant[$id]);
          }
        }
        $this->reload();
      }
    }

    if(isset($_POST['add_object_cancel']) && $this->dialog instanceOf plugin){
      $this->dialog = NULL;
    }
    if(isset($_POST['add_object_finish']) && $this->dialog instanceOf plugin){
      $ret = $this->dialog->save();
      foreach($ret as $key => $entry){
        $this->licenseOccupant[] = $entry['dn'];
        $this->licenseOccCache[$entry['dn']] = $entry['attrs'];
      }
      $this->reload();
      $this->dialog = NULL;
    }

    if($this->dialog instanceOf plugin){
      $this->dialog->save_object();
      return($this->dialog->execute());
    }


    /***************
     * Template handling
     ***************/

    // Get smarty instance and assign required variables.
    $smarty = get_smarty();
    $smarty->assign("bases", $tmp);
    $smarty->assign("base_select",$this->base);
    $smarty->assign("members",$this->convert_list());
    foreach($this->attributes as $attr){
      $smarty->assign($attr,$this->$attr);
    }

    // Assign current permissions for each attribute. 
    $tmp = $this->plInfo();
    foreach($tmp['plProvidedAcls'] as $attr => $desc){
      $smarty->assign($attr."ACL",$this->getacl($attr));
    }
    return($smarty->fetch(get_template_path('licenseGeneric.tpl',TRUE,dirname(__FILE__))));
  }


  /* Check user input and return a list of 'invalid input' messages.
   */
  function check()
  {
    $message = plugin::check();

    // Set the new acl base 
    if($this->dn == "new") {
      $this->set_acl_base($this->base);
    }

    // Check if we are allowed to create/move this user
    if($this->orig_dn == "new" && !$this->acl_is_createable($this->base)){
      $message[]= msgPool::permCreate();
    }elseif($this->orig_dn != "new" && 
        !$this->acl_is_moveable($this->base) && 
        ($this->orig_base != $this->base || $this->orig_cn != $this->cn )){ 
      $message[]= msgPool::permMove();
    }

    /* must: cn */
    if ($this->cn == ""){
      $message[]= msgPool::required(_("Name"));
    }

    // Check if this name is uniq for licenses.
    $ldap= $this->config->get_ldap_link();
    $ldap->cd($this->config->current['BASE']);
    $ldap->search("(&(objectClass=organizationalLicense)(cn=$this->cn))", array("cn"));
    $ldap->fetch();
    if ($ldap->count() != 0 && ( $this->dn == 'new' || $this->cn != $this->orig_cn)){
      $message[]= msgPool::duplicated(_("Name"));
    }

    return($message);
  }
  

  /* Returns list of occupants as <html><option> statements.
   */
  function convert_list()
  {
    $temp= "";
    $icon = " style=\"background-image:url('plugins/generic/images/head.png');\" ";
    foreach ($this->licenseOccupant as $key => $dn){
      if(isset($this->licenseOccCache[$dn])){
        $entry = $this->licenseOccCache[$dn];
        $name = $entry['cn']['0'];
        if(isset($entry['description'][0])){
          $name .= " [".$entry['description'][0]."]";
        }
      }else{
        $name = _("Unknown")."&nbsp;".$dn;
      }
      $temp.= "<option {$icon} title='{$dn}' value='$key' class='select'>{$name}</option>\n";
    }
    return ($temp);
  }

 
  /* Removes the object from the ldap database
   */ 
  function remove_from_parent()
  {
    plugin::remove_from_parent();

    // Remove this object.
    $ldap= $this->config->get_ldap_link();
    $ldap->rmdir($this->dn);
    if (!$ldap->success()){
      msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $this->dn, 0, get_class()));
    }

    // Log action.
    new log("remove","licenses/".get_class($this),$this->dn,array_keys($this->attrs),$ldap->get_error());

    // Trigger remove signal
    $this->handle_post_events("remove");
  }


  /* Saves object modifications
   */  
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

    // Finally write data with selected 'mode'
    $this->cleanup();
    $ldap->cd ($this->dn);
    $ldap->$mode ($this->attrs);
    if (!$ldap->success()){
      msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), 
        $this->dn, LDAP_MOD, get_class()));
      return (1);
    }

    // Send modify/add events
    $this->handle_post_events($mode);

    // Update ACL dependencies too 
    if($this->dn != $this->orig_dn && $this->orig_dn != "new"){
      $tmp = new acl($this->config,$this->parent,$this->dn);
      $tmp->update_acl_membership($this->orig_dn,$this->dn);
    }

    // Log action
    if($mode == "modify"){
      new log("modify","users/".get_class($this),$this->dn,array_keys($this->attrs),$ldap->get_error());
    }else{
      new log("create","users/".get_class($this),$this->dn,array_keys($this->attrs),$ldap->get_error());
    }

    return 0;
  }


  /* This avoids that users move themselves out of their rights.
   */
  function allowedBasesToMoveTo()
  {
    $bases  = $this->get_allowed_bases();
    return($bases);
  }

  
  /* Save HTML inputs
   */
  function save_object()
  {
    plugin::save_object();  
    if(isset($_POST['base'])){
      $this->base = get_post('base');
    }
  }  


  function PrepareForCopyPaste($source)
  {
    plugin::PrepareForCopyPaste($source);

    /* Load member objects */
    $this->licenseOccupant = array();
    if (isset($source['licenseOccupant'])){
      foreach ($source['licenseOccupant'] as $key => $value){
        if ("$key" != "count"){
          $value= @LDAP::convert($value);
          $this->licenseOccupant["$value"]= "$value";
        }
      }
    }
    $this->reload();
  }


  function getCopyDialog()
  {
    $smarty = get_smarty();
    $smarty->assign("cn",     $this->cn);
    $str = $smarty->fetch(get_template_path("paste_generic.tpl",TRUE,dirname(__FILE__)));
    $ret = array();
    $ret['string'] = $str;
    $ret['status'] = "";
    return($ret);
  }

  function saveCopyDialog()
  {
    if(isset($_POST['cn'])){
      $this->cn = $_POST['cn'];
    }
  }

 
  static function plInfo()
  {
    return (array(
          "plShortName"   => _("Generic"),
          "plDescription" => _("License generic"),
          "plSelfModify"  => FALSE,
          "plDepends"     => array(),
          "plPriority"    => 1,
          "plSection"     => array("administration"),
          "plCategory"    => array("licenses" => array("description"  => _("Licenses"),
              "objectClass"  => "organizationalLicense")),
          "plProvidedAcls"=> array(
            "cn"                => _("Name"),
            "description" => _("Description"),
            "base" => _("Base"),
            "telephoneNumber" => _("Telefon number"),
            "facsimileTelephoneNumber" => _("Fax number"),
            "licenseOccupant" => _("Occupants"))
          ));
  }
}


// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
