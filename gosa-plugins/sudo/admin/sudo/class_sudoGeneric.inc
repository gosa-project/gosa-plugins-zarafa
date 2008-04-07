<?php
/*
 * This code is part of GOsa (http://www.gosa-project.org)
 * Copyright (C) 2003-2008 GONICUS GmbH
 *
 * ID: $$Id: class_sudo.inc 9975 2008-03-25 14:09:30Z hickert $$
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


/*! \brief Sudo generic class. Allow setting User/Host/Command/Runas
           for a sudo role object.
 */
class sudo extends plugin
{
  /* Group attributes */
  var $cn= "";
  var $description= "";

  var $sudoUser   = array("ALL");
  var $sudoCommand= array();
  var $sudoHost   = array("ALL");
  var $sudoRunas  = array("ALL");

  var $objectclasses = array("top","sudoRole");
  var $attributes    = array("cn","description","sudoUser","sudoCommand","sudoHost","sudoRunas");

  var $is_account = TRUE;

  var $dialog;

  /*! \brief  Returns to the base department for sudo roles.
              This department is then used to store new roles.
      @param  Object  GOsa configuration object.
      @return String sudo store department
   */
  public static function get_sudoers_ou($config)
  {
    /***
      GET sudo base
     ***/
    $base ="";
    if(empty($base)){
      /* Default is ou=sudoers,BASE */
      $base = "ou=sudoers,".$config->current['BASE'];
    }else{

      /* Append base to given sudoers ou if missing */
      if(!preg_match("/".normalizePreg($config->current['BASE'])."$/i",$base)){
        if(!preg_match("/,$/",$base)){
          $base = $base.",".$config->current['BASE'];
        }else{
          $base = $base.$config->current['BASE'];
        }
      }
    }
    return($base);
  }

  /*! \brief  Initializes this sudo class, with all required attributes.
      @param  Object $config  GOsa configuration object.
      @param  String $db      "new" or the sudo role dn.
      @return .
   */
  function sudo(&$config, $dn= NULL)
  {
    plugin::plugin ($config, $dn);

    if($this->initially_was_account){
      foreach(array("sudoUser","sudoCommand","sudoHost","sudoRunas") as $attr){
        $this->$attr = array();
        if(isset($this->attrs[$attr])){
          $tmp = array();
          for($i = 0 ; $i < $this->attrs[$attr]['count']; $i++){
            $tmp[] = $this->attrs[$attr][$i];
          }
          $this->$attr = $tmp;
        }
      }
    }
  }


  /*! \brief  Creates the sudo generic ui. 
      @return String  The generated HTML content for this plugin. 
   */
  function execute()
  {
    /* Call parent execute */
    plugin::execute();

    /*********************
       Add users 
     *********************/ 
  
    if(isset($_POST['list_sudoUser']) && !is_object($this->dialog)){
      $used = array();
      foreach($this->sudoUser as $name){
        $used[] = preg_replace("/^!/","",$name);
      }
      $this->dialog =new target_list_users($this->config,$used);
    }
   
    /* Add selected hosts  to the sudoUser list */ 
    if(isset($_POST['SaveMultiSelectWindow']) && $this->dialog instanceof target_list_users){
      foreach($this->dialog->list_get_selected_items() as $id){
        $cn = trim($this->dialog->list[$id]['uid'][0]);
        if(!in_array($cn,$this->sudoUser) && !in_array("!".$cn,$this->sudoUser)){
          $this->sudoUser[] = $cn;
        }
      }   
      unset($this->dialog);
      $this->dialog = NULL;
    }    


    /*********************
       Add systems 
     *********************/ 
  
    if(isset($_POST['list_sudoHost']) && !is_object($this->dialog)){
      $used = array();
      foreach($this->sudoHost as $name){
        $used[] = preg_replace("/^!/","",$name);
      }
      $this->dialog =new target_list_systems($this->config,$used);
    }
   
    /* Add selected hosts  to the sudoHost list */ 
    if(isset($_POST['SaveMultiSelectWindow']) && $this->dialog instanceof target_list_systems){
      foreach($this->dialog->list_get_selected_items() as $id){
        $cn = trim($this->dialog->list[$id]['cn'][0]);
        if(!in_array($cn,$this->sudoHost) && !in_array("!".$cn,$this->sudoHost)){
          $this->sudoHost[] = $cn;
        }
      }   
      unset($this->dialog);
      $this->dialog = NULL;
    }    


    /*********************
       Dialog handling / display / close  
     *********************/ 
  
    if(isset($_POST['CloseMultiSelectWindow']) && is_object($this->dialog)){
      unset($this->dialog);
      $this->dialog = NULL;
    }    

    if(is_object($this->dialog)){
      return($this->dialog->execute());
    }

 
    /*********************
       NEGATE values 
     *********************/ 
    foreach($_POST as $name => $value){
      if(preg_match("/^neg_/",$name)){
        $attr = preg_replace("/^neg_([^_]*)_.*$/","\\1",$name);
        $value= preg_replace("/^neg_[^_]*_([0-9]*)_.*$/","\\1",$name);
  
        $attrs = $this->$attr;
        if(isset( $attrs[$value])){
          $v =  $attrs[$value];
          if(preg_match("/^!/",$v)){
            $attrs[$value] = preg_replace("/^!/","",$v);
          }else{
            $attrs[$value] = "!".$v;
          }
          $this->$attr = $attrs;  
        }
        break; // Do it once, image inputs will be posted twice
      }
    }
  
    /*********************
       Delete values 
     *********************/ 
    foreach($_POST as $name => $value){
      if(preg_match("/^del_/",$name)){
        $attr = preg_replace("/^del_([^_]*)_.*$/","\\1",$name);
        $value= preg_replace("/^del_[^_]*_([0-9]*)_.*$/","\\1",$name);
        $attrs = $this->$attr;
        if(isset( $attrs[$value])){
          unset($attrs[$value]);
          $this->$attr = $attrs;  
        }
        break; // Do it once, image inputs will be posted twice
      }
    }


    /*********************
       ADD values 
     *********************/ 
    foreach(array("sudoUser","sudoHost","sudoRunas") as $attr){
      if(isset($_POST["add_".$attr]) && isset($_POST['new_'.$attr]) && !empty($_POST['new_'.$attr])){
        if(preg_match("/^[a-z\.0-9]*$/i",$_POST['new_'.$attr])){
          $attrs = $this->$attr;
          $attrs[] =  trim($_POST['new_'.$attr]); 
          $this->$attr = $attrs;
        }else{
          msg_dialog::display(_("Error"),msgPool::invalid($attr,$_POST['new_'.$attr],"/[a-z0-9]/"));
        }
      }
    }

    foreach(array("sudoCommand") as $attr){
      if(isset($_POST["add_".$attr]) && isset($_POST['new_'.$attr])){
        $attrs = $this->$attr;
        $attrs[] =  trim($_POST['new_'.$attr]); 
        $this->$attr = $attrs;
      }
    }


    $smarty = get_smarty();
    foreach($this->attributes as $attr){
      $smarty->assign($attr,$this->$attr);
    }

    $divlist_sudoUser = new divSelectBox("divlist_sudoUser");
    $divlist_sudoUser->SetHeight("90");
    $divlist_sudoHost = new divSelectBox("divlist_sudoHost");
    $divlist_sudoHost->Setheight("90");
    $divlist_sudoRunas = new divSelectBox("divlist_sudoRunas");
    $divlist_sudoRunas->Setheight("90");
    $divlist_sudoCommand = new divSelectBox("divlist_sudoCommand");
    $divlist_sudoCommand->Setheight("90");

    $neg_img= "<img src='images/negate.png' alt='!' class='center'>"; 
    $option = "<input type='image' src='images/negate.png'     name='neg_%ATTR%_%KEY%' class='center'>"; 
    $option.= "<input type='image' src='images/edittrash.png'  name='del_%ATTR%_%KEY%' class='center'>"; 
    foreach(array("sudoUser","sudoCommand","sudoHost","sudoRunas") as $attr){
      foreach($this->$attr as $key => $entry){
        $entry = preg_replace("/^!/",$neg_img,$entry);
        $list_name = "divlist_".$attr;
        $$list_name->AddEntry(
          array(
            array("string" => $entry),
            array("string" => preg_replace(array("/%KEY%/","/%ATTR%/"),array($key,$attr),$option),
              "attach" => "style='width:40px; border-right: 0px;'")));
      }
    }

    $smarty->assign("divlist_sudoUser",$divlist_sudoUser->DrawList());
    $smarty->assign("divlist_sudoHost",$divlist_sudoHost->DrawList());
    $smarty->assign("divlist_sudoRunas",$divlist_sudoRunas->DrawList());
    $smarty->assign("divlist_sudoCommand",$divlist_sudoCommand->DrawList());
    
    return($smarty->fetch(get_template_path('generic.tpl', TRUE)));
  }


  /*! \brief  Remove this sudo role from the ldap server 
   */
  function remove_from_parent()
  {
    plugin::remove_from_parent();

    $ldap = $this->config->get_ldap_link();
    $ldap->cd($this->dn);
    $ldap->rmdir($this->dn);

    /* Send signal to the world that we've done */
    $this->handle_post_events("remove");
  }


  /*! \brief  Save all relevant HTML posts. 
   */
  function save_object()
  {
    plugin::save_object();

    if(is_object($this->dialog)){
      $this->dialog->save_object();
    }
  }


  /*! \brief  Save changes into the ldap database.
   */
  function save()
  {
    plugin::save(); 
    $this->cleanup();

    $ldap = $this->config->get_ldap_link();
    $ldap->cd($this->config->current['BASE']);
    if($this->is_new){
      $ldap->create_missing_trees(preg_replace('/^[^,]+,/', '', $this->dn));
      $ldap->cd($this->dn);
      $ldap->add($this->attrs);

      /* Send signal to the world that we've done */
      $this->handle_post_events("create");
    }else{
      $ldap->cd($this->dn);
      $ldap->modify($this->attrs);;

      /* Send signal to the world that we've done */
      $this->handle_post_events("modify");
    }

    if (!$ldap->success()){
      msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $this->dn, LDAP_DEL, get_class()));
    }
  }


  /*! \brief  Check the given input.
      @return Array   All error messages in an array();
   */
  function check()
  {
    $message = plugin::check();
    return ($message);
  }


  /*! \brief  Add ACL object
      @return Returns the ACL object.
   */
  static function plInfo()
  {
    return (array(  
          "plShortName" => _("Sudo"),
          "plDescription" => _("Sudo role"),
          "plSelfModify"  => FALSE,
          "plDepends"     => array(),
          "plPriority"    => 0,
          "plSection"     => array("admin"),
          "plCategory"    => array("sudo" => array("objectClass" => "sudoRole", "description" => _("Sudo role"))),

          "plProvidedAcls"    => array(
            "cn"                => _("Name"),
            "description"       => _("Description"),
            "sudoUser"          => _("Users"),
            "sudoHost"          => _("Host"),
            "sudoCommand"       => _("Command"),
            "sudoRunas"         => _("Run as user"))
        ));
  }
}
// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
