<?php


class mailMethodKolab extends mailMethodCyrus
{
  /* Allow modification of account_ids for existing mail accounts */
  protected $modifyableMail   = FALSE;

  /* Allow modification of the mail server attribute existing mail accounts */
  protected $modifyableServer = TRUE;

  /* Enforces same value for 'mail' as used for 'cn' */
  protected $mailEqualsCN   = TRUE; 

  /* Enabled mail domain selection. If enabled getMailDomains must the domain parts */ 
  protected $enableDomainSelection= FALSE;
  protected $enableQuota          = TRUE;
  protected $enableSieveManager   = TRUE;
  protected $enableVacationRange  = TRUE;
  protected $enableFolderTypes    = TRUE;

  protected $attributes           = array(
      "kolabDeleteflag" => "kolabDeleteflag",
      "kolabFolderType" => "kolabFolderType",
      "alias" => "gosaMailAlternateAddress",
      "cyrus-userquota" => "gosaMailQuota",
      "kolabHomeServer" => "gosaMailServer");

  protected $user_prefix    = 'user.';
  protected $share_prefix   = 'shared\\.';

  /* Account ID creation */
  protected $user_id    = "%prefix%%uattrib%";  
  protected $share_id   = "%prefix%%cn%@%domain%";

  protected $userObjectClasses = array("kolabInetOrgPerson");
  protected $shareObjectClasses = array("kolabSharedFolder");

  protected $acl_map = array(
      "lrsw"     => "read",
      "lrswp"    => "post",
      "p"        => "external post",
      "lrswip"   => "append",
      "lrswipcd" => "write",
      "lrswipcda"=> "admin",
      "lrswipkxtecda"=> "full-admin",
      " "         => "none");
  public function fixAttributesOnLoad()
  {
    mailMethodCyrus::fixAttributesOnLoad();
  }

  public function fixAttributesOnStore()
  {
    mailMethod::fixAttributesOnStore();
    $this->build_account_id();
    $this->parent->attrs['kolabDeleteflag'] = array();

    /* The folderType is set by ::setFolderType(...) */
    $this->parent->attrs['kolabFolderType'] = array();
    if($this->type == "group"){ 
      $this->parent->attrs['gosaSharedFolderTarget'] = "kolab+shared.".$this->parent->mail;
    }
  }

  public function fixAttributesOnRemove()
  {
    mailMethod::fixAttributesOnRemove();
    foreach($this->attributes as $source => $dest){
      $this->attrs[$dest]   = array();
      $this->attrs[$source] = array();
    }
    if($this->type == "user"){
      $ocs = $this->userObjectClasses;
    }else{
      $ocs = $this->shareObjectClasses;
    }
    $this->parent->attrs['objectClass'] = array_merge( $this->parent->attrs['objectClass'],$ocs);
    $this->parent->attrs['objectClass'] = array_unique($this->parent->attrs['objectClass']);
    $this->parent->attrs['gosaSharedFolderTarget'] =array();
    $this->parent->attrs['kolabDeleteFlag'] = $this->parent->gosaMailServer;
    if($this->folderTypesEnabled()){
      $this->parent->attrs['kolabFolderType'] =array();
    }
  }


  /* \brief   Some method require special folder types, "kolab" for example.
      !! Those values are dummy values, the base class doesn't use folder types;
     @return  Array Return folder types.
   */
  public function getAvailableFolderTypes()
  {
    $ret = array();
    $ret['CAT'][''] = _("Unspecified"); 
    $ret['CAT']['mail']= _("Mails"); 
    $ret['CAT']['task']= _("Tasks"); 
    $ret['CAT']['journal']= _("Journals"); 
    $ret['CAT']['calendar']= _("Calendar"); 
    $ret['CAT']['contact']= _("Contacts"); 
    $ret['CAT']['note']= _("Notes"); 

    $ret['SUB_CAT']['']['']     = _("Unspecified"); 
    $ret['SUB_CAT']['mail'][''] = _("Unspecified"); 
    $ret['SUB_CAT']['mail']['inbox']      = _("Inbox"); 
    $ret['SUB_CAT']['mail']['drafts']     = _("Drafts"); 
    $ret['SUB_CAT']['mail']['sentitems']  = _("Sent"); 
    $ret['SUB_CAT']['mail']['junkemail']  = _("SPAM"); 

    $ret['SUB_CAT']['task']['default']     = _("Default"); 
    $ret['SUB_CAT']['journal']['default']  = _("Default"); 
    $ret['SUB_CAT']['calendar']['default'] = _("Default"); 
    $ret['SUB_CAT']['contact']['default']  = _("Default"); 
    $ret['SUB_CAT']['note']['default']     = _("Default"); 

    return($ret);
  }


  /*! \brief  Checks whether this account is removeable or not.
              There may be some dependencies left, eg. kolab.
   */
  public function accountRemoveable(&$reason = "")
  {
    if(isset($this->parent->attrs)){
      $checkArray  = array("kolabInvitationPolicy","unrestrictedMailSize", "calFBURL","kolabDelegate","kolabFreeBusyFuture");
      foreach($checkArray as $index){
        if(isset($this->parent->attrs[$index])){
          $reason = _("Mail settings cannot be removed while there are delegations configured!");
          return(false);
        }
      }
    }
    return(true);
  }



  /* \brief  Returns the selected folder type. 
      !! Those values are dummy values, the base class doesn't use folder types;
     @return  Array  The folde type.
   */
  public function getFolderType($default)
  {
    if($this->enableFolderTypes && isset($this->parent->attrs['kolabFolderType'][0])){
      $type = $this->parent->attrs['kolabFolderType'][0];
      if(strpos($type,".")){
        list($cat,$sub) = split("\.",$this->parent->attrs['kolabFolderType'][0]);
      }else{
        $sub = "";
        $cat = $type;
      }

      return(array("CAT" => $cat, "SUB_CAT" => $sub));
    }else{
      return($default);
    }
  }

 
  /* \brief  Returns the selected folder type. 
      !! Those values are dummy values, the base class doesn't use folder types;
     @return  Array  The folde type.
   */
  public function setFolderType($type)
  {

    $type_str = trim($type['CAT'].".".$type['SUB_CAT']);
    $type_str = trim($type_str,".");
    $attrs = array();
    if(empty($type_str)){
      $attrs['kolabFolderType'] = array();
    }else{
      $attrs['kolabFolderType'] = $type_str;
    }

    $ldap = $this->config->get_ldap_link();
    $ldap->cd ($this->parent->dn);
    $ldap->modify($attrs);
    if($ldap->success()){
      @DEBUG (DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, "<b>Successful</b>", 
          "<b>MAIL:</b> Set folderType to '".$type_str."'");
    }else{
      @DEBUG (DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, "<b>Failed: </b>".$ldap->get_error(), 
          "<b>MAIL:</b> Set folderType to '".$type_str."'");
    }
  }


  public function setQuota($number)
  {
    mailMethod::setQuota($number);
    @DEBUG (DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, "<b>Skipped, part of kolabd!</b>", 
        "<b>Set account quota</b> on server '".$this->MailServer);
    return(TRUE);
  }

  public function  setFolderACLs($permissions)
  {
    mailMethod::setFolderACLs($permissions);
    @DEBUG (DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, "<b>Skipped, part of kolabd!</b>", 
        "<b>Setting folder ACLs!</b>","");
    return(TRUE);
  }

  public function updateMailbox()
  {
    mailMethod::updateMailbox();
    @DEBUG (DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, "<b>Skipped, part of kolabd!</b>", 
        "<b>Add/Update account</b> on server :".$this->MailServer);
    return(TRUE);
  }

  public function deleteMailbox()
  {
    mailMethod::deleteMailbox();
    @DEBUG (DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, "Skipped, part of kolabd!", 
      "<b>Setting Quota</b>");
    return(TRUE);
  }


  public function allow_remove(&$reason)
  {
    if(isset($this->parent->attrs)){
      if(in_array_ics("kolabDeleteFlag",array_keys($this->parent->attrs))){
        $reason = _("Waiting for kolab to remove mail properties...");
        return(FALSE);
      }elseif(in_array("gosaMailAccount",$this->parent->attrs['objectClass'])){
        $reason = _("Please remove the mail settings first!");
        return(FALSE);
      }
    }
    return(TRUE);
  }
}


// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
