<?php

/*
 * This code is part of GOsa (http://www.gosa-project.org)
 * Copyright (C) 2009-2010 Bas Tichelaar
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

/*
 * Constant defines
 */
define("ZARAFA_POP_FEATURE_ON", "on");
define("ZARAFA_POP_FEATURE_OFF", "off");
define("ZARAFA_POP_FEATURE_DEFAULT", "default");

define("ZARAFA_IMAP_FEATURE_ON", "on");
define("ZARAFA_IMAP_FEATURE_OFF", "off");
define("ZARAFA_IMAP_FEATURE_DEFAULT", "default");

define("ZARAFA_ZPUSH_FEATURE_ON", "on");
define("ZARAFA_ZPUSH_FEATURE_OFF", "off");
define("ZARAFA_ZPUSH_FEATURE_DEFAULT", "default");


class zarafaAccount extends plugin
{
    /* Definitions */
    var $plHeadline     = "Zarafa settings";
    var $plDescription  = "This does something needed for zarafa";
    var $view_logged    = FALSE;
    var $is_account     = FALSE;
    var $initially_was_account = FALSE;

    /* Zarafa attributes */
    var $zarafaSendAsPrivilege     = array();
    var $zarafaQuotaHard           = "";
    var $zarafaQuotaSoft           = "";
    var $zarafaQuotaWarn           = "";
    var $zarafaQuotaOverride       = "0";

    var $zarafaAdmin               = "0";
    var $zarafaSharedStoreOnly     = "0";
    var $zarafaHidden              = "0";
    var $zarafaAccount             = "0";
    var $zarafaResourceCapacity    = "0";
    var $zarafaResourceType        = "";
    //zarafaContact account
    var $zarafaContactAccount      = "0";
    var $initialContactAccount     = FALSE;

    var $zpushControl              = FALSE;

    var $zarafaEnabledFeatures     = array();
    var $zarafaDisabledFeatures    = array();
    var $zarafaUserArchiveServers  = array();
    var $goZarafaDisplayName       = "";

    /* The methods defaults */
    var $sendAsSelect              = FALSE;
    var $initial_uid               = "";

    var $goZarafaPostalCode        = "";
    var $goZarafaLocation          = "";

    /* List of available zarafa archive servers */
    var $availableArchiveServer  = array();
    var $choosenArchiveServer    = "";

    /* Used LDAP attributes && classes
     * It is important to use only zarafa attributes here
     * Access to attributes from the mail plugin and so on
     * is provided by the parent of this plugin
     */
    var $zarafaUserAttributes = array("zarafaResourceType","zarafaResourceCapacity","zarafaAccount",
      "zarafaAdmin","zarafaHidden","zarafaSharedStoreOnly",
      "zarafaSendAsPrivilege","zarafaQuotaHard",
      "zarafaQuotaSoft","zarafaQuotaWarn",
      "zarafaQuotaOverride", "zarafaEnabledFeatures",
      "zarafaDisabledFeatures", "zarafaUserArchiveServers");

    var $zarafaAddonAttributes = array("goZarafaDisplayName", "goZarafaPostalCode", "goZarafaLocation");
    var $zarafaContactAttributes = array("zarafaSendAsPrivilege", "zarafaHidden", "zarafaAccount");

    var $attributes = array();
    var $usedAttributes = array();

    /* Used ldap object classes be sure to use only zarafa classes
     * and in this case we may only need the zarafa-user
     */
    var $objectclasses= array("goZarafaAddons", "zarafa-user", "zarafa-contact");
    var $multiple_support = FALSE;
    var $cn  = "";
    var $sn  = "";
    var $givenName  = "";


    /*! \brief Constructor for the zarafa-user plugin
     * the base class constructor will only be called to
     * get sure that all necessary steps have been made
     * to create an object out of this class.
     * While we have two alternative object-classes
     * we have to procced in different ways.
     */
    function __construct (&$config, $dn= NULL)
    {
        //merge all attributes together
        $this->attributes = array_unique(array_merge(array_merge($this->zarafaUserAttributes, $this->zarafaAddonAttributes), $this->zarafaContactAttributes));

        plugin::plugin($config,$dn);

        //Extra zpush config value 
        $this->zpushControl= $this->config->get_cfg_value("zarafaAccount", "zpushControl");

        /* Is Account? */
        if(isset($this->attrs['objectClass']) && in_array_ics("zarafa-user", $this->attrs['objectClass']) ||
           in_array_ics("zarafa-contact", $this->attrs['objectClass'])){
          $this->is_account= TRUE;
          @DEBUG (DEBUG_TRACE, __LINE__, __FUNCTION__, __FILE__,
            "found", "Object check");

          //check what kind of account we have and remember it 
          if(in_array_ics ("zarafa-contact", $this->attrs['objectClass'])){
            $this->usedAttributes= array_unique(array_merge($this->zarafaContactAttributes, $this->zarafaAddonAttributes));
            $this->zarafaContactAccount = TRUE;
            $this->initialContactAccount = TRUE;
          } else if(in_array_ics ("zarafa-user", $this->attrs['objectClass'])){
            $this->usedAttributes= array_unique(array_merge($this->zarafaUserAttributes, $this->zarafaAddonAttributes));
          }

          $this->initially_was_account = TRUE;
        }

        /* Initialize configured values
         */
        if($this->is_account)
        {
            foreach($this->attributes as $attrs){
              if(isset($this->attrs[$attrs])){
                if(isset($this->attrs[$attrs]["count"])){
                  $this->$attrs = $this->attrs[$attrs][0];
                }else{
                  $this->$attrs = $this->attrs[$attrs];
                }
              }
            }

            /* Get attributes from parent object
             */
            foreach(array("uid","cn","sn","givenName") as $attr)
                $this->$attr = $this->attrs[$attr][0];

            /* Load attributes containing arrays */
            foreach (array("zarafaSendAsPrivilege", "zarafaEnabledFeatures", "zarafaDisabledFeatures", "zarafaUserArchiveServers") as $val)
            {
                $this->$val= array();
                if (isset($this->attrs["$val"]["count"]))
                {
                    for ($i= 0; $i<$this->attrs["$val"]["count"]; $i++)
                    {
                        array_push($this->$val, $this->attrs["$val"][$i]);
                    }
                }
            }

            $this->refreshArchiveServer();
        }
    }

    /* Method to read the available archive server from ldap */
    function refreshArchiveServer(){
      $ldap = $this->config->get_ldap_link();
      $ldap->search("(&(objectClass=goZarafaAddons)(goZarafaArchiveServer=1))", array("cn"));

      $this->availableArchiveServer = array();
      while($attrs= $ldap->fetch()){
        if(isset($attrs["cn"][0]) && !in_array($attrs["cn"][0], $this->zarafaUserArchiveServers)){
          $this->availableArchiveServer[] = $attrs["cn"][0];
        }
      }
    }

    function execute()
    {
        /* Call parent execute */
        $display = plugin::execute();

        /* Log view */
        if($this->is_account && !$this->view_logged)
        {
            $this->view_logged = TRUE;
            new log("view","users/".get_class($this),$this->dn);
        }

        if(isset($_POST['modify_state']))
          $this->is_account= !$this->is_account;

        if(!$this->is_account && $this->parent === NULL)
        {
          $display= "<img alt=\"\" src=\"images/small-error.png\" align=\"middle\">&nbsp;<b>".
            msgPool::noValidExtension(_("Mail"))."</b>";
          $display.= back_to_main();
          return ($display);
        }

        if($this->parent !== NULL)
        {
          if($this->is_account)
          {
            $reason = "";
            $display= $this->show_disable_header(msgPool::removeFeaturesButton(_("Zarafa")),msgPool::featuresEnabled(_("Zarafa")));
          }
          else
          {
            $posixAccount = $this->parent->by_object['posixAccount'];
            $mailAccount = $this->parent->by_object['mailAccount'];
            $reason = "";

            if(!$posixAccount->is_account || !$mailAccount->is_account)
            {
              $display= $this->show_enable_header(msgPool::addFeaturesButton(_("Zarafa")),"The mail and posix plugin must be activ in order to use this plugin",TRUE,TRUE);
            }
            else
            {
              $display= $this->show_enable_header(msgPool::addFeaturesButton(_("Zarafa")),msgPool::featuresDisabled(_("Zarafa")));
            }

            return ($display);
          }
        }

        if(isset($_POST['add_archive']) && isset($_POST['choosenArchiveServer'])){
          $this->zarafaUserArchiveServers[] = $this->availableArchiveServer[$_POST['choosenArchiveServer']];
          $this->refreshArchiveServer();
        }

        if(isset($_POST['delete_archive'])){
          $this->delArchive($_POST['archive_server_list']);
        }

        /****************
         Send as privilege
         ****************/

        if (isset($_POST['add_local_sendas']))
        {
            $this->sendAsSelect=  new sendAsSelect($this->config, get_userinfo());
            $this->dialog= TRUE;
        }

        if (isset($_POST['sendAs_cancel']))
        {
            $this->sendAsSelect= FALSE;
            $this->dialog= FALSE;
        }

        if (isset($_POST['sendAs_save']) && $this->sendAsSelect instanceOf sendAsSelect)
        {
            if($this->acl_is_writeable("zarafaSendAsPrivilege"))
            {
                $list = $this->sendAsSelect->save();

                foreach ($list as $entry)
                {
                    $val = $entry['dn'];
                    $this->addSendas($val);
                    $this->is_modified= TRUE;
                }

                $this->sendAsSelect= FALSE;
                $this->dialog= FALSE;
            }
            else
           {
                msg_dialog::display(_("Error"), _("Please select an entry!"), ERROR_DIALOG);
            }
        }

    if($this->sendAsSelect instanceOf sendAsSelect)
    {
      $used  = array();
      $used['dn'] = array_values($this->zarafaSendAsPrivilege);
      $used['dn'][] = $this->dn;

      // Build up blocklist
      session::set('filterBlacklist', $used);
      return($this->sendAsSelect->execute());
    }

    if(isset($_POST['delete_sendas']))
    {
      $this->delSendas ($_POST['sendas_list']);
    }

    if($this->sendAsSelect instanceOf sendAsSelect)
    {
      return($this->sendAsSelect->execute());
    }

    /****************
      SMARTY- Assign smarty variables
     ****************/
    //print_a($this->parent->by_object['mailAccount']);
    $smarty = get_smarty();

    $SkipWrite = (!isset($this->parent) || !$this->parent) && !session::is_set('edit');
    $tmp  = $this->plInfo();

    foreach($tmp['plProvidedAcls'] as $name => $transl)
    {
      $smarty->assign("$name"."ACL", $this->getacl($name,$SkipWrite));
    }
    foreach($this->attributes as $attr)
    {
      $smarty->assign($attr,$this->$attr);
    }

    $smarty->assign("initially_was_account", $this->initially_was_account);

    $resourcetypes = array(0 => "&nbsp;", "user" => _("User"), "room" => _("Room"), "equipment" => _("Equipment"));
    $smarty->assign("zarafaResourceType_list", $resourcetypes);
    $smarty->assign("mail", $this->parent->by_object['mailAccount']->mail);
    $smarty->assign("zarafaQuotaHard",       $this->zarafaQuotaHard);
    $smarty->assign("zarafaQuotaSoft",       $this->zarafaQuotaSoft);
    $smarty->assign("zarafaQuotaWarn",       $this->zarafaQuotaWarn);
    $smarty->assign("zpushControl", $this->zpushControl);
    $smarty->assign("zarafaQuotaOverride",   $this->zarafaQuotaOverride?"checked":"");
    $smarty->assign("zarafaSharedStoreOnly", $this->zarafaSharedStoreOnly?"checked":"");
    $smarty->assign("zarafaAdmin", $this->zarafaAdmin?"checked":"");
    $smarty->assign("zarafaAccount", $this->zarafaAccount?"checked":"");
    $smarty->assign("zarafaHidden", $this->zarafaHidden?"checked":"");
    $smarty->assign("zarafaContactAccount", $this->zarafaContactAccount?"checked":"");
    $smarty->assign("zarafaEnabledFeatures", $this->zarafaEnabledFeatures);
    $smarty->assign("zarafaDisabledFeatures", $this->zarafaDisabledFeatures);
    $smarty->assign("zarafaUserArchiveServers", $this->zarafaUserArchiveServers);
    $smarty->assign("availableArchiveServer", $this->availableArchiveServer);
    $smarty->assign("choosenArchiveServer", $this->choosenArchiveServer);
    $smarty->assign("gosaMailServer", $this->parent->by_object['mailAccount']->gosaMailServer);
    $smarty->assign("gosaMailAlternateAddress", $this->parent->by_object['mailAccount']->gosaMailAlternateAddress);
    $smarty->assign("gosaMailForwardingAddress", $this->parent->by_object['mailAccount']->gosaMailForwardingAddress);

    /* _Multiple users vars_ */
    foreach($this->attributes as $attr)
    {
      $u_attr = "use_".$attr;
      $smarty->assign($u_attr,in_array($attr,$this->multi_boxes));
    }
    foreach(array("zarafaAccount", "zarafaHidden", "zarafaAdmin", "zarafaSharedStoreOnly", "zarafaQuotaOverride") as $attr)
    {
      $u_attr = "use_".$attr;
      $smarty->assign($u_attr,in_array($attr,$this->multi_boxes));
    }

    return($display.$smarty->fetch(get_template_path("zarafa.tpl",TRUE,dirname(__FILE__))));
  }

  /* Save data to object */
  function save_object()
  {
    if(isset($_POST['zarafaTab']))
    {
      /* Save ldap attributes */
      plugin::save_object();

        foreach(array("zarafaHidden", "zarafaAccount", "zarafaAdmin", "zarafaSharedStoreOnly", "zarafaQuotaOverride", "zarafaContactAccount") as $val)
        {
            if(isset($_POST[$val]))
              $this->$val = 1;
            else
              $this->$val = 0;
        }
        /* $this->goZarafaDisplayName = $this->sn . ", " . $this->givenName; */
        //Get the most up to date name from the user 
        $this->goZarafaDisplayName = $this->parent->by_object['user']->sn.', '.$this->parent->by_object['user']->givenName;
  
        $tMatchArray = array();
        $this->goZarafaLocation = "";
        $this->goZarafaPostalCode = "";
  
        if(preg_match('/[0-9]{5}/', $this->parent->by_object['user']->l, $tArray)){
          $this->goZarafaPostalCode = $tArray[0];
          foreach(preg_split('/[0-9]{5}/', $this->parent->by_object['user']->l) as $addressSubstring){
              $this->goZarafaLocation .= $addressSubstring;
          }
        }else{
          $this->goZarafaLocation = $this->parent->by_object['user']->l;
        }
  
        $this->goZarafaLocation = trim($this->goZarafaLocation);
        $this->goZarafaLocation = preg_replace('!\s+!', ' ', $this->goZarafaLocation);
  
        /*process zarafa features*/
        if(isset($_POST['pop3'])){
          switch($_POST['pop3'])
          {
              case ZARAFA_POP_FEATURE_ON:
                  $this->zarafaEnabledFeatures[] = 'pop3';
    
                  if(($key = array_search('pop3', $this->zarafaDisabledFeatures, true)) !== false)
                      unset($this->zarafaDisabledFeatures[$key]);
                  break;
              case ZARAFA_POP_FEATURE_OFF:
                  $this->zarafaDisabledFeatures[] = 'pop3';
    
                  if(($key = array_search('pop3', $this->zarafaEnabledFeatures, true)) !== false)
                      unset($this->zarafaEnabledFeatures[$key]);
                  break;
              case ZARAFA_POP_FEATURE_DEFAULT:
                  if(($key = array_search('pop3', $this->zarafaEnabledFeatures, true)) !== false)
                      unset($this->zarafaEnabledFeatures[$key]);
    
                  if(($key = array_search('pop3', $this->zarafaDisabledFeatures, true)) !== false)
                      unset($this->zarafaDisabledFeatures[$key]);
                  break;
          }
        }
  
        if(isset($_POST['imap'])){
          switch($_POST['imap'])
          {
              case ZARAFA_IMAP_FEATURE_ON:
                  $this->zarafaEnabledFeatures[] = 'imap';
    
                  if(($key = array_search('imap', $this->zarafaDisabledFeatures, true)) !== false)
                      unset($this->zarafaDisabledFeatures[$key]);
                  break;
              case ZARAFA_IMAP_FEATURE_OFF:
                  $this->zarafaDisabledFeatures[] = 'imap';
    
                  if(($key = array_search('imap', $this->zarafaEnabledFeatures, true)) !== false)
                      unset($this->zarafaEnabledFeatures[$key]);
                  break;
              case ZARAFA_IMAP_FEATURE_DEFAULT:
                  if(($key = array_search('imap', $this->zarafaEnabledFeatures, true)) !== false)
                      unset($this->zarafaEnabledFeatures[$key]);
    
                  if(($key = array_search('imap', $this->zarafaDisabledFeatures, true)) !== false)
                      unset($this->zarafaDisabledFeatures[$key]);
    
                  break;
          }
        }

        if ($this->zpushControl == true){
          if(isset($_POST['zpush'])){
            switch($_POST['zpush'])
            {
            case ZARAFA_ZPUSH_FEATURE_ON:
              $this->zarafaEnabledFeatures[] = 'zpush';

              if(($key = array_search('zpush', $this->zarafaDisabledFeatures, true)) !== false)
                unset($this->zarafaDisabledFeatures[$key]);
              break;
            case ZARAFA_ZPUSH_FEATURE_OFF:
              $this->zarafaDisabledFeatures[] = 'zpush';

              if(($key = array_search('zpush', $this->zarafaEnabledFeatures, true)) !== false)
                unset($this->zarafaEnabledFeatures[$key]);
              break;
            case ZARAFA_ZPUSH_FEATURE_DEFAULT:
              if(($key = array_search('zpush', $this->zarafaEnabledFeatures, true)) !== false)
                unset($this->zarafaEnabledFeatures[$key]);

              if(($key = array_search('zpush', $this->zarafaDisabledFeatures, true)) !== false)
                unset($this->zarafaDisabledFeatures[$key]);

              break;
            }
          }
        }
  
        $this->zarafaEnabledFeatures = array_unique($this->zarafaEnabledFeatures);
        $this->zarafaDisabledFeatures= array_unique($this->zarafaDisabledFeatures);
    }
  }

  /*! \brief  Adds the given mail address to the list of mail forwarders
   */
  function addSendas($address)
  {
    if(empty($address))
        return;

    if($this->acl_is_writeable("zarafaSendAsPrivilege"))
    {
      $this->zarafaSendAsPrivilege[]= $address;
      $this->zarafaSendAsPrivilege= array_unique ($this->zarafaSendAsPrivilege);
      sort ($this->zarafaSendAsPrivilege);
      reset ($this->zarafaSendAsPrivilege);
      $this->is_modified= TRUE;
    }
    else
    {
      msg_dialog::display(_("Permission error"), _("You have no permission to modify these addresses!"), ERROR_DIALOG);
    }
  }

  function delArchive($addresses)
  {
    if($this->acl_is_writeable("zarafaUserArchiveServers")){
      $this->zarafaUserArchiveServers= array_remove_entries ($addresses, $this->zarafaUserArchiveServers);
      $this->is_modified= TRUE;
      $this->refreshArchiveServer();
    }else{
      msg_dialog::display(_("Permission error"), _("You have no permission to modify these addresses!"), ERROR_DIALOG);
    }
  }

  function delSendas($addresses)
  {
    if($this->acl_is_writeable("zarafaSendAsPrivilege")){
      $this->zarafaSendAsPrivilege= array_remove_entries ($addresses, $this->zarafaSendAsPrivilege);
      $this->is_modified= TRUE;
    }else{
      msg_dialog::display(_("Permission error"), _("You have no permission to modify these addresses!"), ERROR_DIALOG);
    }
  }

  /*! \brief  Save the mailAccount settings to the ldap database.
   */
  function save()
  {
    $ldap= $this->config->get_ldap_link();

    /* Save arrays */
    $this->attrs['zarafaSendAsPrivilege']     = $this->zarafaSendAsPrivilege;
    $this->attrs['zarafaEnabledFeatures']     = $this->zarafaEnabledFeatures;
    $this->attrs['zarafaDisabledFeatures']    = $this->zarafaDisabledFeatures;
    $this->attrs['goZarafaDisplayName']       = $this->goZarafaDisplayName;
    $this->attrs['zarafaUserArchiveServers']  = $this->zarafaUserArchiveServers;

    /* Call parents save to prepare $this->attrs */
    plugin::save();

    /* Remove unused attributes and object classes */
    if($this->zarafaContactAccount){
      if(in_array("zarafa-user", $this->attrs['objectClass'])){
        unset($this->attrs['objectClass'][array_search("zarafa-user", $this->attrs['objectClass'])]);
      }
    } else {
      if(in_array("zarafa-contact", $this->attrs['objectClass'])){
        unset($this->attrs['objectClass'][array_search("zarafa-contact", $this->attrs['objectClass'])]);
      }
    }

    /* remember to have only numeric incremental indices in your array */
    $this->attrs['objectClass'] = array_values($this->attrs['objectClass']);

    /* Save data to LDAP */
    $ldap->cd($this->dn);
    $this->cleanup();
    $ldap->modify($this->attrs);

    if(!$ldap->success())
        msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $this->dn, LDAP_MOD, get_class()));

    /* Log last action */
    if($this->initially_was_account)
      new log("modify","users/".get_class($this),$this->dn,array_keys($this->attrs),$ldap->get_error());
    else
      new log("create","users/".get_class($this),$this->dn,array_keys($this->attrs),$ldap->get_error());

    /* Optionally execute a command after we're done */
    if($this->initially_was_account == $this->is_account)
    {
        if($this->is_modified)
            $this->handle_post_events("modify", array("uid" => $this->uid));
    }
    else
    {
        $this->handle_post_events("add", array("uid" => $this->uid));
    }
  }

  /*
   * Use this method for the "special"
   * needs of cleanups to do in this plugin
   */
  function cleanup(){
    foreach ($this->attrs as $index => $value){
      if($this->initialContactAccount && !$this->zarafaContactAccount){
        if(in_array($index, $this->zarafaContactAttributes) &&
          !in_array($index, $this->zarafaUserAttributes)){
            $this->attrs[$index] = array();
            continue;
        }
      }else if($this->zarafaContactAccount){
        if(!in_array($index, $this->zarafaContactAttributes) && 
          in_array($index, $this->zarafaUserAttributes)){
            $this->attrs[$index] = array();
            continue;
          }
      }

      /* Convert arrays with one element to non arrays, if the saved
       *          attributes are no array, too */
      if (is_array($this->attrs[$index]) &&
        count ($this->attrs[$index]) == 1 &&
        isset($this->saved_attributes[$index]) &&
        !is_array($this->saved_attributes[$index])){

          $tmp= $this->attrs[$index][0];
          $this->attrs[$index]= $tmp;
        }

      /* Remove emtpy arrays if they do not differ */
      if (is_array($this->attrs[$index]) &&
        count($this->attrs[$index]) == 0 &&
        !isset($this->saved_attributes[$index])){

          unset ($this->attrs[$index]);
          continue;
        }

      /* Remove single attributes that do not differ */
      if (!is_array($this->attrs[$index]) &&
        isset($this->saved_attributes[$index]) &&
        !is_array($this->saved_attributes[$index]) &&
        $this->attrs[$index] == $this->saved_attributes[$index]){

          unset ($this->attrs[$index]);
          continue;
        }
      /* Remove arrays that do not differ */
      if (is_array($this->attrs[$index]) &&
        isset($this->saved_attributes[$index]) &&
        is_array($this->saved_attributes[$index])){

          if (!array_differs($this->attrs[$index],$this->saved_attributes[$index])){
            unset ($this->attrs[$index]);
            continue;
          }
        }
    }

    /* Update saved attributes and ensure that next cleanups will be successful too */
    foreach($this->attrs as $name => $value){
      $this->saved_attributes[$name] = $value;
    }
  }

  /*! \brief  Check given values
   */
  function check()
  {
      $messages = array();
      /* Call common method to give check the hook */
      $messages = plugin::check();

      $posixAccount = $this->parent->by_object['posixAccount'];
      $mailAccount = $this->parent->by_object['mailAccount'];

      if(!$posixAccount->is_account)
          $messages[] = "Zarafa - plugin requires posix enabled";

      if(!$mailAccount->is_account)
          $messages[] = "Zarafa - plugin requires mail enabled";

      return($messages);
  }

  /*! \brief  Adapt from template, using 'dn'
   */
  function adapt_from_template($dn, $skip= array())
  {
    plugin::adapt_from_template($dn, $skip);

    foreach (array("zarafaSendAsPrivilege", "zarafaEnabledFeatures", "zarafaDisabledFeatures", "zarafaUserArchiveServers") as $val)
    {
      if(in_array($val, $skip))
        continue;

      $this->$val= array();
      if (isset($this->attrs["$val"]["count"]))
      {
        for ($i= 0; $i<$this->attrs["$val"]["count"]; $i++)
        {
          $value= $this->attrs["$val"][$i];
          foreach (array("sn", "givenName", "uid") as $repl)
          {
            if (preg_match("/%$repl/i", $value))
            {
              $value= preg_replace ("/%$repl/i", $this->parent->$repl, $value);
            }
          }

          if (preg_match("/%first/i", $value))
              $value= preg_replace ("/%first/i", $this->parent->givenName[0], $value);

          array_push($this->$val, strtolower(rewrite($value)));
        }
      }
    }
  }

//   /*! \brief  Prepare this class the be useable when editing multiple users at once
//    */
//   function get_multi_edit_values()
//   {
//     $ret = plugin::get_multi_edit_values();
//     if(in_array("zarafaQuotaHard",$this->multi_boxes)){
//       $ret['zarafaQuotaHard'] = $this->gosaMailQuota;
//     }

//     return($ret);
//   }

  /*! \brief  Check given input for multiple user edit
   */
//   function multiple_check()
//   {
//     $message = plugin::multiple_check();

//     if(empty($this->gosaMailServer) && in_array("gosaMailServer",$this->multi_boxes)){
//       $message[]= msgPool::noserver(_("Mail"));
//     }

//     return($message);
//   }

  /*! \brief  ...
   */
//  function set_multi_edit_values($values)
//   {
//     plugin::set_multi_edit_values($values);
//     $tmp= preg_replace("/[^a-z]/i","",$this->gosaMailDeliveryMode);
//     if(isset($values['flag_add'])){
//       foreach($values['flag_add'] as $flag){
//         if(!preg_match("/".$flag."/",$tmp)){
//           $tmp .= $flag;
//         }
//       }
//     }
//     if(isset($values['flag_remove'])){
//       foreach($values['flag_remove'] as $flag){
//         if(preg_match("/".$flag."/",$tmp)){
//           $tmp = preg_replace("/".$flag."/","",$tmp);
//         }
//       }
//     }
//     $this->gosaMailDeliveryMode = "[".$tmp."]";

//   }

  /*! \brief  Removes the mailAccount extension from ldap
   */
  function remove_from_parent()
  {
      /* Cancel if there's nothing to do here */
      if(!$this->initially_was_account)
      {
          return;
      }

      /* Remove GOsa attributes */
      plugin::remove_from_parent();

      /* Zero arrays */
      $this->attrs['zarafaSendAsPrivilege']    = array();
      $this->attrs['zarafaEnabledFeatures']    = array();
      $this->attrs['zarafaDisabledFeatures']   = array();
      $this->attrs['zarafaUserArchiveServers'] = array();
      
      $this->cleanup();

      @DEBUG (DEBUG_LDAP, __LINE__, __FUNCTION__, __FILE__,$this->attributes, "Save");
      $ldap= $this->config->get_ldap_link();
      $ldap->cd($this->dn);
      $ldap->modify ($this->attrs);

      /* Add "view" to logging class */
      new log("remove","users/".get_class($this),$this->dn,array_keys($this->attrs),$ldap->get_error());
      if (!$ldap->success()){
          msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $this->dn, LDAP_MOD, get_class()));
      }

      /* Optionally execute a command after we're done */
      $this->handle_post_events("remove",array("uid" => $this->uid));
  }

  /*! \brief  Initialize plugin to be used as multiple edit class.
   */
//   function init_multiple_support($attrs,$all)
//   {
//     plugin::init_multiple_support($attrs,$all);
//   }

    /*! \brief
     */
//     function get_multi_init_values()
//     {
//         $attrs = plugin::get_multi_init_values();
//         return($attrs);
//     }

    /*! \brief  Display multiple edit dialog
     */
//     function multiple_execute()
//     {
//         return($this->execute());
//     }

    /*! \brief  Save posts from multiple edit dialog
     */
//     function multiple_save_object()
//     {
//         if (isset($_POST['zarafaTab']))
//         {
//             plugin::multiple_save_object();
//             $this->save_object();
//         }
//     }

    function allow_remove()
    {
        return "";
    }

    /*! \brief  ACL settings
     */
    static function plInfo()
    {
        return (array(
          "plShortName"   => _("Zarafa"),
          "plDescription" => _("Zarafa settings"),
          "plSelfModify"  => TRUE,
          "plDepends"     => array("user"),                    // This plugin depends on
          "plPriority"    => 4,                                // Position in tabs
          "plSection"     => array("personal" => _("My account")),
          "plCategory"    => array("users"),
          "plOptions"     => array(),

          "plProperties"  => array(

            array(
              "name"          => "zpushControl",
              "type"          => "bool",
              "default"       => "FALSE",
              "description"   => _("Enable extra control mechanism for z-push access. Adds an additional value for zarafaEnabledFeatures attribute (zpush)."),
              "check"         => "gosaProperty::isBool",
              "migrate"       => "",
              "group"         => "zarafa",
              "mandatory"     => FALSE)),

          "plProvidedAcls"  => array(
              "gosaMailAlternateAddress"  =>  _("Alternate mail addresses"),
              "gosaMailForwardingAddress" =>  _("Forward addresses"),
              "zarafaQuota"               =>  _("Quota size"),
              "zarafaAdmin"               =>  _("Zarafa admin"),
              "zarafaAccount"             =>  _("Zarafa account"),
              "zarafaHidden"              =>  _("Zarafa hidden"),
              "zarafaSharedStoreOnly"     =>  _("Zarafa shared store"),
              "zarafaQuotaOverride"       =>  _("Zarafa quota override"),
              "zarafaSendAsPrivilege"     =>  _("Zarafa send as privilege"),
              "zarafaResource"            =>  _("Zarafa resource"),
              "zarafaEnabledFeatures"     =>  _("Zarafa enabled features"),
              "zarafaResourceCapacity"    =>  _("Zarafa resource capacity"),
              "zarafaDisabledFeatures"    =>  _("Zarafa disabled features"),
              "zarafaUserArchiveServers"  =>  _("Zarafa archive server"),
              "goZarafaPostalCode"        =>  _("The useres Postal code"),
              "goZarafaLocation"          =>  _("Users location"))
          ));
    }

}

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
