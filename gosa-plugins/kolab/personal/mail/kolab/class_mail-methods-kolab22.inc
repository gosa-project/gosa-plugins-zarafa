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
    $domains = array();
    $ldap = $this->config->get_ldap_link();
    $ldap->cd($this->config->current['BASE']);
    $ldap->search("(&(objectClass=kolab)(k=kolab)(postfix-mydestination=*))",array("postfix-mydestination"));

    if($ldap->count() == 1){
      $attrs = $ldap->fetch();
      for($i=0; $i <$attrs['postfix-mydestination']['count']; $i ++ ){
        $domains[] = $attrs['postfix-mydestination'][$i];
      }
      @DEBUG (DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, "<b>Found: ".trim(implode($domains,", "),", ")."</b>",
          "Revceiving 'Mail domains for kolab22'.");
    }elseif($ldap->count() == 0){
      @DEBUG (DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, "<b>No domains found. Check k=kolab and postfix-mydestination</b>",
          "Revceiving 'Mail domains for kolab22'.");
    }else{
      @DEBUG (DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, "<b>Found more than one kolab configuration object. Giving up.</b>",
          "Revceiving 'Mail domains for kolab22'.");
    }

    return($domains);
  } 
}


// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
