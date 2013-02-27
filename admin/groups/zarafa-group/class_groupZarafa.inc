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


class zarafagroup extends plugin
{
    /* Internal */
    var $AclTypes                   = array();
//    var $memberUid                  = array();  // Group members
    var $folder_acls                = array();
    var $mailAddressSelect          = FALSE;
    var $view_logged                = FALSE;

    /* Zarafa variables */
    var $zarafaSendAsPrivilege      = array();
    var $zarafaSecurityGroup        = "";
    var $zarafaHidden               = "";
    var $zarafaAccount              = "0";

    /* Dialogs */
    var $sendAsSelect               = FALSE;
    var $addZarafaUser              = FALSE;

    /* Needed object classes and attributes */
    var $objectclasses = array("zarafa-group");
    var $attributes    = array("zarafaHidden", "zarafaSecurityGroup",
                                 "zarafaSendAsPrivilege", "zarafaAccount");

    var $memberList = FALSE;

    var $multiple_support = FALSE; // Not tested yet

    var $uid = "";
    var $cn = "";
    var $orig_cn = "";
    var $show_effective_memeber = FALSE;

    var $plHeadline = "Zarafa group plugin";
    var $plDescription = "Plugin to alter zarafa attributes for your groups";

    function __construct(&$config, $dn= NULL, $base_object= NULL)
    {
        plugin::plugin($config, $dn);

        /* Get attributes from parent object */
        foreach(array("uid","cn") as $attr)
        {
            if(isset($this->parent->by_object['group']) &&
                    isset($this->parent->by_object['group']->$attr))
            {
                $this->$attr = $this->parent->by_object['group']->$attr;
            }
            elseif(isset($this->attrs[$attr]))
            {
                $this->$attr = $this->attrs[$attr][0];
            }
        }
        $this->orig_cn = $this->uid = $this->cn;

        /* Remember account status */
        $this->initially_was_account = $this->is_account;

        /* Load acls
         * The most used acl will be used as member acl, this
        * shortens the listed acls.
        * This may be merged/overwritten by the mail methods.
        */
        if(isset($this->attrs['acl']))
        {
            for($i = 0; $i < $this->attrs['acl']['count'] ; $i++)
            {
                $str = trim($this->attrs['acl'][$i]);

                /* Be carefull here, since kolab22 uses spaces in the acls (herbert read anon/post)
                 */
                $name = trim(preg_replace("/^([^\s]*).*$/","\\1",$str));
                $acl  = trim(preg_replace("/^[^\s]*+\s/","",$str));

                if($name == "anyone")
                    $name = "__anyone__";

                $this->folder_acls[$name] = $acl;
            }
        }

        /* Summarize most used ACLs as member acl */
        if(count($this->folder_acls) > 2)
        {
            $acl_usage = array();
            //            $most_acl = $this->folder_acls['__member__'];
            $most_cnt = 0;
            $member = $this->get_member();

            foreach($this->folder_acls as $user => $acl)
            {
                if(preg_match("/^__/",$user))
                    continue;
                if(!in_array($user,$member['mail']))
                    continue;
                if(!isset($acl_usage[$acl]))
                    $acl_usage[$acl]=0;

                $acl_usage[$acl] ++;

                if($acl_usage[$acl] > $most_cnt)
                {
                    $most_cnt = $acl_usage[$acl];
                    //$most_acl = $acl;
                }
            }

            //$this->folder_acls['__member__'] = $most_acl;
            foreach($this->folder_acls as $name => $acl)
            {
                if(preg_match("/^__/",$name))
                    continue;

                if(/*$acl == $most_acl && */in_array($name,$member['mail']))
                    unset($this->folder_acls[$name]);
            }
        }

        /* Initialize configured values
         */
        if($this->is_account)
        {
            /* Load attributes containing arrays */
            foreach(array("zarafaSendAsPrivilege") as $val)
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
        }

        // Prepare lists
        $this->memberList = new sortableListing();
        $this->memberList->setDeleteable(true);
        $this->memberList->setInstantDelete(false);
        $this->memberList->setEditable(false);
        $this->memberList->setWidth("100%");
        $this->memberList->setHeight("300px");
        $this->memberList->setHeader(array('~',_("Given name"),_("Surname"),_("UID")));
        $this->memberList->setColspecs(array('20px','*','*','*','20px'));
        $this->memberList->setDefaultSortColumn(1);
    }

    /*! \brief  Returns all group members once with 'dn' and once with 'mail'.
     This function is used to summarize ACLs by member acls.
    @return Array   Containing all members, with mail and dn
    */
    function get_member()
    {
        $member = array('all' => array(), 'mail' => array());
        $ldap = $this->config->get_ldap_link();
        $ldap->cd($this->config->current['BASE']);

        if(isset($this->parent->by_object['group']))
        {
            foreach($this->parent->by_object['group']->memberUid as $uid)
            {
                if(!isset($this->parent->by_object['group']->dnMapping[$uid]))
                    continue;

                $dn = $this->parent->by_object['group']->dnMapping[$uid];
                $member['all'][$uid] = $uid;

                if($ldap->object_match_filter($dn,"(&(objectClass=zarafa-user))"))
                {
                    $ldap->cat($dn);
                    $attrs = $ldap->fetch();
                }
            }
        }
        else
        {
            if(!isset($this->parent->by_object['group']->memberUid))
                return($member);

            $users = get_list("(&(objectClass=person)(objectClass=zarafa-user)(uid=*))",
                    "users",$this->config->current['BASE'],
                    array("uid","objectClass"/*,$uattrib*/),GL_SUBSEARCH | GL_NO_ACL_CHECK);

            //             foreach($users as $user)
                //             {
                //                 $member['all'][$user['uid'][0]] = $user['dn'];
                //                 if(isset($user[$uattrib]) &&
                //                    in_array("gosaMailAccount",$user['objectClass']) &&
                //                    (in_array($user['uid'][0], $this->attrs['memberUid'])))
                //                 {
                //                     $member['mail'][$user['uid'][0]] = $user[$uattrib][0];
                //                 }
                //             }
                }

                return $member;
    }

    /*
     * This function fills your plugin with life and produces the output
     */
    function execute()
    {

        /* Call parent execute */
        plugin::execute();

        /*******************
         Zarafa user dialog
        *******************/

        if(isset($_POST['add_zarafa_member']))
        {
            $this->addZarafaUser = new addZarafaUsers($this->config, get_userinfo());
            $this->dialog = TRUE;
        }

        if(isset($_POST['addZarafaUser_cancel']))
        {
            $this->addZarafaUser = FALSE;
            $this->dialog = FALSE;
        }

        if(isset($_POST['addZarafaUser_save']) && $this->addZarafaUser instanceOf addZarafaUsers)
        {
            if($this->acl_is_writeable("memberUid"))
            {
                $list = $this->addZarafaUser->save();
                foreach($list as $entry)
                {
                    $val = $entry['uid'];
                    $this->addMember($val[0]);
                    $this->is_modified= TRUE;
                }

                $this->addZarafaUser= FALSE;
                $this->dialog= FALSE;
            }
            else
            {
                msg_dialog::display(_("Error"), _("Please select an entry!"), ERROR_DIALOG);
            }
        }

        if($this->addZarafaUser instanceOf addZarafaUsers)
        {
            $used  = array();

            if(is_array($this->parent->by_object['group']->memberUid))
                $used['uid'] = array_values($this->parent->by_object['group']->memberUid);
            elseif(!empty($this->parent->by_object['group']->memberUid))
                $used['uid'][] = $this->parent->by_object['group']->memberUid;

            // Build up blocklist
            session::set('filterBlacklist', $used);
            return($this->addZarafaUser->execute());
        }

        if(isset($_POST['delete_zarafa_member']) && isset($_POST['group_member_list']))
            $this->delMember($_POST['group_member_list']);

        /****************
         Account status
        ****************/
        if(!$this->multiple_support_active)
        {
            if(isset($_POST['modify_state']))
            {
                if($this->is_account && $this->acl_is_removeable())
                    $this->is_account= FALSE;
                elseif(!$this->is_account && $this->acl_is_createable())
                    $this->is_account= TRUE;
            }

            if($this->is_account)
            {
                $reason = "";
                $display= $this->show_disable_header(msgPool::removeFeaturesButton(_("Zarafa")),msgPool::featuresEnabled(_("Zarafa")));
            }
            else
          {
                $mailGroup = $this->parent->by_object['mailgroup'];
                $reason = "";

//                 if(!$mailGroup->is_account)
//                 {
//                     $display= $this->show_disable_header(msgPool::addFeaturesButton(_("Zarafa")), $reason, TRUE, TRUE);
//                 }
//                 else
//               {
                    $display= $this->show_disable_header(msgPool::addFeaturesButton(_("Zarafa")),msgPool::
                          featuresDisabled(_("Zarafa")));
//                 }

                return ($display);
            }
        }

        // Act on list events
        foreach(array('memberList') as $list)
        {
            // Check if list is available, depends on multi- or sinlge- group editing.
            if($this->$list)
            {
                $this->$list->save_object();
                $action = $this->$list->getAction();
                if($action['action'] == 'delete' && preg_match("/w/",$this->getacl("memberUid")))
                {
                    foreach ($action['targets'] as $id)
                    {
                        $value = $this->$list->getKey($id);
                        unset ($this->parent->by_object['group']->members["$value"]);
                        $this->parent->by_object['group']->removeUser($value);
                    }
                    $this->parent->by_object['group']->reload();
                }
            }
        }

        /****************
         Send as privilege
        ****************/
        if(isset($_POST['add_local_sendas']))
        {
            $this->sendAsSelect=  new sendAsSelect_group($this->config, get_userinfo());
            $this->dialog= TRUE;
        }

        if(isset($_POST['sendAs_cancel']))
        {
            $this->sendAsSelect= FALSE;
            $this->dialog= FALSE;
        }

        if(isset($_POST['sendAs_save']) && $this->sendAsSelect instanceOf sendAsSelect_group)
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

        if($this->sendAsSelect instanceOf sendAsSelect_group)
        {
            $used  = array();

            if(is_array($this->zarafaSendAsPrivilege))
                $used['dn'] = array_values($this->zarafaSendAsPrivilege);
            elseif(!empty($this->zarafaSendAsPrivilege))
                $used['dn'][] = $this->zarafaSendAsPrivilege;

            $used['dn'][] = $this->dn;

            // Build up blocklist
            session::set('filterBlacklist', $used);
            return($this->sendAsSelect->execute());
        }

        if(isset($_POST['delete_sendas']) && isset($_POST['sendas_list']))
            $this->delSendas($_POST['sendas_list']);

        /****************
         SMARTY- Assign smarty variables
        ****************/

        /* Load templating engine */
        $smarty= get_smarty();

        /* Assign acls */
        $tmp = $this->plInfo();
        foreach($tmp['plProvidedAcls'] as $name => $translation)
            $smarty->assign($name."ACL",$this->getacl($name));

        foreach($this->attributes as $name)
            $smarty->assign($name,$this->$name);

        $smarty->assign("only_local", "");

        /*******************
         Multi edit support
        *******************/
//         foreach($this->attributes as $attr)
//         {
//             if(in_array($attr,$this->multi_boxes))
//                 $smarty->assign("use_".$attr,TRUE);
//             else
//                 $smarty->assign("use_".$attr,FALSE);
//         }

        $smarty->assign("multiple_support",$this->multiple_support_active);
        $smarty->assign("zarafaSecurityGroup", $this->zarafaSecurityGroup?"checked":"");
        $smarty->assign("zarafaHidden", $this->zarafaHidden?"checked":"");
        $smarty->assign("zarafaAccount", $this->zarafaAccount?"checked":"");
        $smarty->assign("gosaMailAlternateAddress", $this->parent->by_object['mailgroup']->gosaMailAlternateAddress);
        $smarty->assign("gosaMailForwardingAddress", $this->parent->by_object['mailgroup']->gosaMailForwardingAddress);

        /* get ldap link */
        $ldap = $this->config->get_ldap_link();
//         $members = array();

//         /* Load memberUid attribute but show only the zarafa-user */
//         foreach($this->parent->by_object['group']->memberUid as $val)
//         {
//             $ldap->search("(&(objectClass=zarafa-user)(uid=" . $val . "))", array("zarafa-user"));
//             if($ldap->count() == 1)
//                 array_push($members, $val);
//         }

//         $smarty->assign("memberUid", $members);

        $this->memberList->setDeleteable($this->zarafaAccount?TRUE:FALSE);

        $this->memberList->setAcl($this->parent->by_object['group']->getacl("memberUid"));
        $data = $lData = array();
        foreach($this->parent->by_object['group']->members as $uid => $member)
        {
            $ldap->search("(&(objectClass=zarafa-user)(uid=" . $uid . "))", array("zarafa-user"));
            if($ldap->count() != 1)
                continue;

            $data[$uid] = $member;
            $givenName = $sn = _("Unknown");
            if(isset($member['sn']) && isset($member['sn'][0])) $sn = $member['sn'][0];
            if(isset($member['givenName']) && isset($member['givenName'][0])) $givenName = $member['givenName'][0];
            $image = image('images/false.png');
            if(isset($member['sn'])){
                $image = image('plugins/users/images/zarafa_user_small.png');
            }
            $lData[$uid] = array('data' => array($image,$sn, $givenName, $uid));
        }
        $this->memberList->setListData($data, $lData);
        $this->memberList->update();

        $smarty->assign("memberList", $this->memberList->render());

        $display.= $smarty->fetch(get_template_path('groupZarafa.tpl', TRUE, dirname(__FILE__)));
        return ($display);
    }

    /* Save data to object */
    function save_object()
    {
        /* Save ldap attributes */
        plugin::save_object();

        if(!$this->dialog)
        {
            foreach(array("zarafaSecurityGroup", "zarafaHidden", "zarafaAccount") as $val)
            {
                if(isset($_POST[$val]))
                    $this->$val = 1;
                else
                    $this->$val = 0;
            }
        }

        //         if (isset($_POST['mailedit']))
            //         {
            //             if(isset($_POST['show_effective_memeber']))
                //             {
                //                 $this->show_effective_memeber = !$this->show_effective_memeber;
                //             }

                //             $mail = $this->mail;
                //             $server = $this->gosaMailServer;
                //             plugin::save_object();

                //             if($this->initially_was_account)
                    //             {
                    //                 $this->gosaMailServer = $server;
                    //             }
                    //             if($this->initially_was_account)
                        //             {
                        //                 $this->mail = $mail;
                        //             }

                            /* Handle posted ACL changes.
                             Add/del member acls.
                            */
                        //             if(isset($_POST['mail_acls_posted']))
                            //             {
                            //                 $new_acls = array();
                            //                 foreach(array("__anyone__","__member__") as $attr){
                            //                     $pname = base64_encode($attr);
                            //                     if(get_post('acl_value_'.$pname)){
                            //                         $new_acls[$attr] = get_post('acl_value_'.$pname);
                            //                     }else{
                            //                         $new_acls[$attr] = $this->folder_acls[$attr];
                            //                     }
                            //                 }

                            //                 foreach($this->folder_acls as $user => $acl){
                            //                     $pname = base64_encode($user);
                            //                     if($user == "__member__" || $user == "__anyone__") continue;
                            //                     if(isset($_POST['remove_acl_user_'.$pname])){
                            //                     }elseif(isset($_POST['acl_user_'.$pname])){
                            //                         if($user != get_post('acl_user_'.$pname)){
                            //                             $new_acls[get_post('acl_user_'.$pname)] = get_post('acl_value_'.$pname);
                            //                         }else{
                            //                             $new_acls[$user] = get_post('acl_value_'.$pname);
                            //                         }
                            //                     }else{
                            //                         $new_acls[$user] = $acl;
                            //                     }
                            //                 }
                            //                 if(isset($_POST['add_acl_user'])){
                            //                     $new_acls[_('New')] = $this->folder_acls['__anyone__'];
                            //                 }
                            //                 $this->folder_acls = $new_acls;
                            //             }

                            /* Handle GOsa mail delivery flags.
                             */

                            /* Assemble mail delivery mode
                             The mode field in ldap consists of values between braces, this must
                            be called when 'mail' is set, because checkboxes may not be set when
                            we're in some other dialog.

                            Example for gosaMailDeliveryMode [LR        ]
                            L - Local delivery
                            R - Reject when exceeding mailsize limit
                            S - Use spam filter
                            V - Use vacation message
                            C - Use custom sieve script
                            I - Only insider delivery */
                            //             $tmp= preg_replace("/[^a-z]/i","",$this->gosaMailDeliveryMode);
                            //             if($this->acl_is_writeable("gosaMailDeliveryModeL")){
                            //                 if(!preg_match("/L/",$tmp) && !isset($_POST['drop_own_mails'])){
                            //                     $tmp.="L";
                            //                 }elseif(preg_match("/L/",$tmp) && isset($_POST['drop_own_mails'])){
                            //                     $tmp = preg_replace("/L/","",$tmp);
                            //                 }
                            //             }

                            //             $opts = array(
                            //                     "R"   => "use_mailsize_limit",
                            //                     "S"   => "use_spam_filter",
                            //                     "V"   => "use_vacation",
                            //                     "C"   => "own_script",
                            //                     "I"   => "only_local");

                            //             foreach($opts as $flag => $post)
                                //             {
                                //                 if($this->acl_is_writeable("gosaMailDeliveryMode".$flag))
                                    //                 {
                                    //                     if(!preg_match("/".$flag."/",$tmp) && isset($_POST[$post]))
                                        //                     {
                                        //                         $tmp.= $flag;
                                        //                     }
                                        //                     elseif(preg_match("/".$flag."/",$tmp) && !isset($_POST[$post]))
                                        //                     {
                                        //                         $tmp = preg_replace("/".$flag."/","",$tmp);
                                        //                     }
                                        //                 }
                                        //             }

                                        //             $tmp= "[$tmp]";
                                        //             if ($this->gosaMailDeliveryMode != $tmp)
                                            //             {
                                            //                 $this->is_modified= TRUE;
                                            //             }
                                            //             $this->gosaMailDeliveryMode= $tmp;
                                            //        }
    }


    /* Save data to LDAP, depending on is_account we save or delete */
    function save()
    {
        $ldap= $this->config->get_ldap_link();

        /* Create acls */
//         $this->acl = array("anyone ".$this->folder_acls['__anyone__']);
//         $member = $this->get_member();
//         $new_folder_acls = array("anyone" => $this->folder_acls['__anyone__']);

//         foreach($this->folder_acls as $user => $acls)
//         {
//             if(preg_match("/^__/",$user)) continue;
//             $this->acl[] = $user." ".$acls;
//             $new_folder_acls[$user]=$acls;
//         }

//         $this->folder_acls = $new_folder_acls;
//         $this->acl = array_unique($this->acl);

        /* Call parents save to prepare $this->attrs */
        plugin::save();

        /* Save data to LDAP */
        $ldap->cd($this->dn);
        $this->cleanup();
        $ldap->modify($this->attrs);

        if(!$ldap->success())
            msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $this->dn, LDAP_MOD, get_class()));

        if($this->initially_was_account)
            new log("modify","groups/".get_class($this),$this->dn,array_keys($this->attrs),$ldap->get_error());
        else
           new log("create","groups/".get_class($this),$this->dn,array_keys($this->attrs),$ldap->get_error());

        /* Optionally execute a command after we're done */
        if($this->initially_was_account == $this->is_account)
        {
            if($this->is_modified)
                $this->handle_post_events("modify");
        }
        else
            $this->handle_post_events("add");
    }

    /*! \brief  Removes the mailAccount extension from ldap
     */
    function remove_from_parent()
    {
        /* Cancel if there's nothing to do here */
        if(!$this->initially_was_account)
            return;

        /* Remove GOsa attributes */
        plugin::remove_from_parent();

        /* Zero arrays */
        $this->attrs['zarafaSendAsPrivilege']= array();

        $this->cleanup();

        @DEBUG (DEBUG_LDAP, __LINE__, __FUNCTION__, __FILE__,$this->attributes, "Save");
        $ldap= $this->config->get_ldap_link();
        $ldap->cd($this->dn);
        $ldap->modify ($this->attrs);

        /* Add "view" to logging class */
//        new log("remove","users/".get_class($this),$this->dn,array_keys($this->attrs),$ldap->get_error());
        if (!$ldap->success()){
            msg_dialog::display(_("LDAP error"), msgPool::ldaperror($ldap->get_error(), $this->dn, LDAP_MOD, get_class()));
        }

        /* Optionally execute a command after we're done */
        $this->handle_post_events("remove",array("uid" => $this->uid));
    }

    /*
     * This method can be used to validate
     * the input given by the user.
     * It is also possible to use this method
     * to check the requirements for the zarafa plugin
     * to be present. This method will be called everytime
     * the user wants to apply changes in any tab.
     * */
    function check()
    {
        //return an empty array if there is no zarafa account enabled
        if(!$this->is_account)
            return array();

        $ldap= $this->config->get_ldap_link();

        /* Call common method to give check the hook */
        $messages= plugin::check();

        //check for the mailgroup plugin to be activated
        $mailGroup = $this->parent->by_object['mailgroup'];
        //if(!$mailGroup->is_account)
        //    $messages[]= "Zarafa requires mail plugin enabled";

        return $messages;
    }

    /* Adapt from template, using 'dn' */
    function adapt_from_template($dn, $skip= array())
    {
        plugin::adapt_from_template($dn, $skip);

        foreach(array("gosaMailAlternateAddress", "gosaMailForwardingAddress") as $val)
        {
            if(in_array($val, $skip))
                continue;

            $this->$val= array();
            if(isset($this->attrs["$val"]["count"]))
            {
                for($i= 0; $i<$this->attrs["$val"]["count"]; $i++)
                {
                    $value= $this->attrs["$val"][$i];
                    foreach(array("sn", "givenName", "uid") as $repl)
                    {
                        if(preg_match("/%$repl/i", $value))
                        {
                            $value= preg_replace ("/%$repl/i", $this->parent->$repl, $value);
                        }
                    }
                    array_push($this->$val, $value);
                }
            }
        }
    }

    /* Return plugin informations for acl handling  */
    static function plInfo()
    {
        return (array(
                "plShortName"   => _("Zarafa"),
                "plDescription" => _("Zarafa group"),
                "plSelfModify"  => FALSE,
                "plDepends"     => array(),
                "plPriority"    => 10,
                "plSection"     => array("administration"),
                "plCategory"    => array("groups"),
                "plProvidedAcls"=> array(
                        "zarafaSecurityGroup"    => _("Zarafa Securitygroup"),
                        "zarafaHidden"           => _("Zarafa Hidden"),
                        "zarafaAccount"          => _("Zarafa Account"),
                        "zarafaSendAsPrivilege"     => _("Zarafa send as privilege"),
                        "gosaMailAlternateAddress"  => _("Mail alternative addresses"))
        ));
    }

    /* Remove given ACL for given member (uid,mail) ..
     */
    function removeUserAcl($index )
    {
        if(isset($this->imapacl[$index]))
            unset($this->imapacl[$index]);
    }

    function multiple_execute()
    {
        return $this->execute();
    }

    function init_multiple_support($attrs,$all)
    {
        plugin::init_multiple_support($attrs,$all);

        $this->gosaMailForwardingAddress = array();
        if(isset($attrs['gosaMailForwardingAddress']))
        {
            for($i = 0 ; $i < $attrs['gosaMailForwardingAddress']['count'] ; $i++)
            {
                $this->gosaMailForwardingAddress[] = $attrs['gosaMailForwardingAddress'][$i];
            }
        }

        $this->gosaMailForwardingAddress_Some = array();
        if(isset($all['gosaMailForwardingAddress']))
        {
            for($i = 0 ; $i < $all['gosaMailForwardingAddress']['count'] ; $i++)
            {
                if(!in_array($all['gosaMailForwardingAddress'][$i],$this->gosaMailForwardingAddress))
                {
                    $this->gosaMailForwardingAddress_Some[] = $all['gosaMailForwardingAddress'][$i];
                }
            }
        }
    }

    function multiple_save_object()
    {
        if(isset($_POST['multiple_mail_group_posted']))
        {
            plugin::multiple_save_object();

            foreach(array("kolabFolderType") as $attr)
            {
                if(isset($_POST['use_'.$attr]))
                {
                    $this->multi_boxes[] = $attr;
                }
            }

            /* Collect data and re-assign it to the imapacl array */
            if ($this->acl_is_writeable("acl"))
            {
                $this->imapacl= array();
                $this->imapacl['%members%']= $_POST['member_permissions'];
                $this->imapacl['anyone']= $_POST['default_permissions'];
                foreach ($this->indexed_user as $nr => $user)
                {
                    if (!isset($_POST["user_$nr"]))
                        continue;

                    if ($_POST["user_$nr"] != $user ||
                            $_POST["perm_$nr"] != $this->indexed_acl[$nr])
                    {
                        $this->is_modified= TRUE;
                    }
                    $this->imapacl[$_POST["user_$nr"]]= $_POST["perm_$nr"];
                }
            }
        }
    }

    /* Return selected values for multiple edit */
    function get_multi_edit_values()
    {
        $ret = plugin::get_multi_edit_values();
        $ret['Forward_some'] = $this->gosaMailForwardingAddress_Some;
        $ret['Forward_all'] = $this->gosaMailForwardingAddress;
        if(in_array('kolabFolderType',$this->multi_boxes)){
            $ret['kolabFolderTypeType'] = $this->kolabFolderTypeType;
            $ret['kolabFolderTypeSubType'] = $this->kolabFolderTypeSubType;
        }
        if(in_array("acl",$this->multi_boxes)){
            $ret['imapacl'] = $this->imapacl;
        }
        return($ret);
    }

    function set_multi_edit_values($attrs)
    {
        $forward = array();
        foreach($attrs['Forward_some'] as $addr)
        {
            if(in_array($addr,$this->gosaMailForwardingAddress))
            {
                $forward[] = $addr;
            }
        }
        foreach($attrs['Forward_all'] as $addr){
            $forward[] = $addr;
        }
        plugin::set_multi_edit_values($attrs);
        $this->gosaMailForwardingAddress = $forward;
    }

    function postable_acls()
    {
        $ret = array();
        foreach($this->folder_acls as $name => $acl)
        {
            $ret[$name] = array("name" => $name,"acl" => $acl,"post_name" => base64_encode($name));
        }
        return($ret);
    }

    function addSendas($newSendAs)
    {
        if(empty($newSendAs))
            return;

        if($this->acl_is_writeable("zarafaSendAsPrivilege"))
        {
            if(!is_array($this->zarafaSendAsPrivilege))
                $this->zarafaSendAsPrivilege = array($this->zarafaSendAsPrivilege);

            array_push($this->zarafaSendAsPrivilege, $newSendAs);

            foreach($this->zarafaSendAsPrivilege as $key => $value)
                if(is_null($value) || $value=="")
                unset($this->zarafaSendAsPrivilege[$key]);

            $this->zarafaSendAsPrivilege = array_unique($this->zarafaSendAsPrivilege);
            sort($this->zarafaSendAsPrivilege);
            reset($this->zarafaSendAsPrivilege);
            $this->is_modified= TRUE;
        }
        else
   {
            msg_dialog::display(_("Permission error"), _("You have no permission to modify these addresses!"), ERROR_DIALOG);
        }
    }

    function delSendas($addresses)
    {
        if($this->acl_is_writeable("zarafaSendAsPrivilege"))
        {
            if(!is_array($addresses))
                $addresses = array($addresses);

            $this->zarafaSendAsPrivilege = array_remove_entries($addresses, $this->zarafaSendAsPrivilege);
            $this->is_modified = TRUE;
        }
        else
   {
            msg_dialog::display(_("Permission error"), _("You have no permission to modify these addresses!"), ERROR_DIALOG);
        }
    }

    /*
     * Add members to the posix database.
     * To keep it stupid and simple we edit the data from posix
     * to synchronize the members from the tabs.
     */
    function addMember($newMember)
    {
        if(empty($newMember) || is_array($newMember))
            return;

        if($this->acl_is_writeable("memberUid"))
        {
//             if(!is_array($this->parent->by_object['group']->memberUid))
//                 $this->parent->by_object['group']->memberUid = array($this->parent->by_object['group']->memberUid);

//             array_push($this->parent->by_object['group']->memberUid, $newMember);

//             foreach($this->parent->by_object['group']->memberUid as $key => $value)
//                 if(is_null($value) || $value=="")
//                     unset($this->parent->by_object['group']->memberUid[$key]);

            $this->parent->by_object['group']->addUser($newMember);
            $this->parent->by_object['group']->members["$newMember"]= $this->parent->by_object['group']->allusers[$newMember];
            $this->parent->by_object['group']->reload();

//             $this->parent->by_object['group']->memberUid = array_unique($this->parent->by_object['group']->memberUid);
//             sort($this->parent->by_object['group']->memberUid);
//             reset($this->parent->by_object['group']->memberUid);
            $this->is_modified= TRUE;
        }
        else
        {
            msg_dialog::display(_("Permission error"), _("You have no permission to modify these addresses!"), ERROR_DIALOG);
        }
    }

    /*
     * Add members to the posix database.
     * To keep it stupid and simple we edit the data from posix
     * to synchronize the members from the tabs.
     */
    function delMember($members)
    {
        if($this->acl_is_writeable("memberUid"))
        {
            if(!is_array($members))
                $members = array($members);

            //$this->parent->by_object['group']->memberUid = array_remove_entries($members, $this->parent->by_object['group']->memberUid);
            foreach($members as $member)
                $this->parent->by_object['group']->removeUser($member);

            $this->parent->by_object['group']->reload();
            $this->is_modified = TRUE;
        }
        else
        {
            msg_dialog::display(_("Permission error"), _("You have no permission to modify these addresses!"), ERROR_DIALOG);
        }
    }

    function get_effective_member_acls()
    {
        $tmp = array();
        $member = $this->get_member();
        foreach($member['mail'] as $uid => $mail)
        {
            /* Do not save overridden acls */
            if(isset($this->folder_acls[$mail]))
            {
                continue;
            }

            //            $tmp[$mail]  = $this->folder_acls['__member__'];
        }
        return($tmp);
    }
    /*
     * If there is a reason
     * the zarafa plugin can't be removed from this group account
     * you can check it here and return a message.
     * If the message is "" the plugin is free for deletion
     */
    function allow_remove()
    {
        $reason = "";
        return $reason;
    }
}

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
