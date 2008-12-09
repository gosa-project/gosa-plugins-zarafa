<?php


class mailMethodKolab22 extends mailMethodKolab
{

  /* Allow modification of account_ids for existing mail accounts */
  protected $modifyableMail = FALSE;

  /* Enforces same value for 'mail' as used for 'cn' */
  protected $mailEqualsCN   = TRUE; 

  /* Enabled mail domain selection. If enabled getMailDomains must the domain parts */ 
  protected $enableDomainSelection= TRUE;
  protected $enableQuota          = TRUE;
  protected $enableSieveManager   = TRUE;
  protected $enableVacationRange  = TRUE;
  protected $enableFolderTypes    = TRUE;

  protected $userObjectClasses = array("kolabInetOrgPerson");
  protected $shareObjectClasses = array("kolabSharedFolder");

  /*! \brief Returns a list of configured mail domains 
      @return Array A list of mail domains
   */
  public function getMailDomains()
  {
    return(array("gonicus.de"));
  } 


  /*! \brief  Returns a list of all possible acls.
      @return Array   ACLs.
  */
  public function getAclTypes()
  {
    $acls["lrsw"]     = _("read");
    $acls["lrswp"]    = _("post");
    $acls["p"]        = _("external post");
    $acls["lrswip"]   = _("append");
    $acls["lrswipcd"] = _("write");
    $acls["lrswipcda"]= _("admin");
    $acls["lrswipkxtecda"]= _("super-admin");
    $acls[" "]= _("none");
    return($acls);
  }
}


// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
