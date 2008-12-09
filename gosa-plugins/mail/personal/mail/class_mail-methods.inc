<?php


class mailMethod{

  /* Allow modification of account_ids for existing mail accounts */
  protected $modifyableMail = TRUE;

  /* Enforces same value for 'mail' as used for 'cn' */
  protected $mailEqualsCN   = FALSE; 

  /* the attribute used to create accounts */ 
  protected $uattrib        = "mail";  // Naming attribute for accounts, e.g. imap.

  /* The account prefixes, keep the '.' here! See FAQ cyrusUseSlashes */
  protected $user_prefix    = "user.";  
  protected $share_prefix   = "share.";

  /* Create accounts in cyrus style with '/' instead of '.' */
  protected $cyrusUseSlashes= FALSE;

  /* The atribute mapping for this class  Source --> Destination */
  protected $attributes     = array();
  protected $userObjectClasses = array();
  protected $shareObjectClasses = array();

  /* Enabled mail domain selection. If enabled getMailDomains must the domain parts */ 
  protected $enableDomainSelection= FALSE;
  protected $enableQuota          = TRUE;
  protected $enableSieveManager   = FALSE;
  protected $enableVacationRange  = TRUE;
  protected $enableFolderTypes    = FALSE;

  /* Default values */
  protected $quotaValue   = 0;  
  protected $quotaUsage   = 0;  

  /* Method internal */
  protected $type               = "user"; 
  protected $account_id         = "";
  protected $initial_account_id = "";
  protected $connected          = FALSE;
  protected $error              = "";
  protected $parent             = NULL;   
  protected $MailServer         = "";
 

  /*! \brief  Constructs the mail class 
      @param  Object  Config  The GOsa configuration object
      @param  Object  Plugin  The initator
      @param  String          Open "user" or "group" account.
   */
  function __construct(&$config, &$parent, $type = "user")
  {
    $this->parent = $parent;
    $this->config = $config;

    /* Create a refernce to the mail selected server 
     */
    if(isset($this->parent->gosaMailServer)){
      $this->MailServer = &$this->parent->gosaMailServer;
    }else{
      trigger_error("mailMethod with invalid parent object initialized.");
    }

    if(!in_array($this->type,array("user","group"))){
      trigger_error("Unknown mail class type used '".$type."'.");
    }else{
      $this->type = $type;
    }
  }

  
  /*! \brief  Intialize attributes and config settings.
   */
  protected function init()
  {
    /* Get config value for cyrusUseSlashes
     */
    if ($this->config->get_cfg_value("cyrusUseSlashes") == "true"){
      $this->cyrusUseSlashes = TRUE;
      @DEBUG (DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, "cyrusUseSlashes: <b>Enabled</b>","");
    }else{
      @DEBUG (DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, "cyrusUseSlashes: <b>Disabled</b>","");
    }

    /* Check if the mail account identification attribute
       is overridden in the configuration file
     */
    if($this->config->get_cfg_value("mailAttribute","mail") != ""){
      $new_uattrib= strtolower($this->config->get_cfg_value("mailAttribute"));
      if(in_array($new_uattrib,array("mail","uid"))){
        $this->uattrib = $new_uattrib;
      }else{
        @DEBUG (DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, "<b>".$new_uattrib."</b>",
            "Unsupported 'mailAttribute' in gosa configuration specified");
        msg_dialog::display(_("Configuration error"), 
            sprintf(_("The configured mail attribute '%s' is unsupported!"), $new_uattrib), ERROR_DIALOG);
      }
    }
    $this->build_account_id();
    $this->initial_account_id = $this->account_id;
  }

  
  public function fixAttributesOnLoad()
  {
    foreach($this->attributes as $source => $dest){
      if(isset($this->parent->attrs[$source])){
        $this->parent->attrs[$dest] = $this->parent->attrs[$source];
      }
      if(isset($this->parent->$source)){
        $this->parent->$dest = $this->parent->$source;
      }
    }
  }


  public function mailEqualsCN()
  {
    return($this->mailEqualsCN);
  }
  

  public function fixAttributesOnRemove()
  {
    /* Remove objectClasses  
     */ 
    if($this->type == "user"){
      $this->parent->attrs['objectClass'] = 
        array_remove_entries_ics($this->userObjectClasses, $this->parent->attrs['objectClass']);
    }else{
      $this->parent->attrs['objectClass'] = 
        array_remove_entries_ics($this->shareObjectClasses, $this->parent->attrs['objectClass']);
    }
    foreach($this->attributes as $source => $dest){
      $this->attrs[$dest]   = array();
      $this->attrs[$source] = array();
    }
  }

  public function fixAttributesOnStore()
  {
    foreach($this->attributes as $source => $dest){
      if(isset($this->parent->attrs[$dest])){
        $this->parent->attrs[$source] = $this->parent->attrs[$dest ];
      }
      if(isset($this->parent->$dest)){
        $this->parent->$source = $this->parent->$dest;
      }
    }

    if($this->type == "user"){
      $ocs = $this->userObjectClasses;
    }else{
      $ocs = $this->shareObjectClasses;
    }
    foreach($ocs as $oc){
      if(!in_array($oc, $this->parent->attrs['objectClass'])){
        $this->parent->attrs['objectClass'][] = $oc;
      }
    }
  }


  /*! \brief  Connect services like imap.
              Not necessary for the base class.
      @return Boolean True if this method is connected else false.
   */
  public function connect()
  {
    $this->reset_error();
    @DEBUG (DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__,"<b>Connect method</b>: ".get_class($this),"");
    @DEBUG (DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__,"<b>Current server</b>: ".$this->MailServer,"");
 
    $this->connected = TRUE;
    return(TRUE);
  }


  /*! \brief  Returns the connection status of this method.
      @return Boolean True if this method is connected else false.
   */
  public function is_connected()
  {
    return($this->connected);
  }


  /*! \brief  Disconnect this method. Close services like imap connection.
              Not necessary for the base class.
   */
  public function disconnect()
  {
    $this->reset_error();
    if($this->is_connected()){
      @DEBUG (DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__,"<b>Disconnect method</b>: ".get_class($this),"");
      $this->connected =FALSE; 
    }
  }

  
  /*! \brief  Returns true the current object represents a valid account
              (Some methods may check imap accounts here.)
      @return Boolean TRUE if this is a valid account else FALSE
  */
  public function account_exists()
  {
    $this->reset_error();
    return(TRUE);
  }
  

  /*! \brief  Returns the last error occurred 
      @return String  The last error message.
   */
  public function get_error(){
    return($this->error);
  }


  public function isModifyableMail()
  {
    return($this->modifyableMail);
  }


  /*! \brief  Returns TRUE if the action caused an error.
      @return Boolean TRUE on error else FALSE
   */
  public function is_error(){
    return($this->error != "");
  }


  /*! \brief  Resets the error message.
   */
  public function reset_error(){
    $this->error = "";
  }


  public function get_account_id()
  {
    $this->build_account_id();
    return($this->account_id);
  }

  /*! \brief  Create a new account id, like 'user/name@domain.com'.
   */
  protected function build_account_id()
  {
    /* Build account identicator */
    if($this->type == "user"){
      $str = $this->user_prefix;
    }else{
      $str = $this->share_prefix;
    }

    /* Create account prefix and respect "cyrusUseSlashes" 
       Do not replace escaped dots for cyrusUseSlashes.
     */
    $uattrib = $this->uattrib;
    if($this->cyrusUseSlashes){
      $str = preg_replace('/([^\\\\])\./',"\\1/",$str);
    }
    $str = preg_replace("/\\\\([\.\/])/","\\1",$str);
    $str = trim(strtolower($str . $this->parent->$uattrib));

    if($this->account_id != $str){
      $this->account_id = $str;
      @DEBUG (DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, "accountID generated: <b>".$str."</b>","");
    }
  }


  /*! \brief  Creates a valid folder id for a given folder name.
               e.g. $folder_id = "INBOX/test"  &&  $this->account_id = "share/mailbox@domain.de"
               will result in "share/mailbox/test@domain.de"
              This function is mainly used to read and write folder permissions.
      @return String A valid folder id
   */
  public function create_folder_id($folder, $type = "")
  {

    if(!empty($folder)){
      $folder = trim(preg_replace("/^INBOX[\.\/]*/i","",$folder));
    }
    if(!empty($folder)){
      $folder = "/".$folder;
    }

    /* Build account identicator */
    if($type == ""){
      $type = $this->type;
    }
    if($type == "user"){
      $str = $this->user_prefix;
    }else{
      $str = $this->share_prefix;
    } 

    /* Create account prefix and respect "cyrusUseSlashes"
       Do not replace escaped dots for cyrusUseSlashes.
     */
    $uattrib = $this->uattrib;
    if($this->cyrusUseSlashes){
      $str = preg_replace('/([^\\\\])\./',"\\1/",$str);
    }
    $str = preg_replace("/\\\\([\.\/])/","\\1",$str);
    $str = trim(strtolower($str . $this->parent->$uattrib));

    if(preg_match("/\@/",$this->parent->$uattrib)){
      list($mail,$domain) = split("\@",$this->parent->$uattrib);
      $str = trim(strtolower($str . $mail . $folder . "@" . $domain));
    }else{
      $str = trim(strtolower($str . $this->parent->$uattrib));
    }
    return($str) ;
  }


  /*! \brief  Returns the configured mail method for the given parent object, 
                initialized and read for use.
      @return mailMethod  The configured mailMethod.
   */
  public function get_method()
  {
    $methods = mailMethod::get_methods();
    if ($this->config->get_cfg_value("mailmethod") != ""){
      $method= $this->config->get_cfg_value("mailmethod");
      $cls = get_correct_class_name("mailMethod$method");
      if(isset($methods[$cls])){
        @DEBUG (DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, "Selected mailMethod: <b>".$cls."</b>","");
        $tmp = new $cls($this->config,$this->parent,$this->type);
        $tmp->init();
        return($tmp);
      }else{
        @DEBUG (DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, "Invalid mailMethod defined <b>".$cls.
            "</b> falling back to <b>".get_class($this)."</b>","");

        /* Print out configuration errors directly, we can't catch them everywhere. 
         */
        msg_dialog::display(_("Configuration error"), 
            sprintf(_("Mail method '%s' is unknown!"), $method), ERROR_DIALOG);
      }
    }

    /* If no valued mailMethod could be detected, return the base class.
     */
    $this->init();
    return($this);
  }


  /*! \brief Saves sieve settings 
   */
  public function saveSieveSettings()
  {
    $this->reset_error();
    return(TRUE);
  }

  
  /*! \brief  Creates or Updates the mailAccount represented by this class. 
   */
  public function updateMailbox()
  {
    $this->reset_error();
    return(TRUE);
  }


  /*! \brief  Update shared folder dependencies 
   */
  public function updateSharedFolder()
  {
    $this->reset_error();
    return(TRUE);
  }


  /*! \brief  Removes the mailbox represented by this class,
               and update shared folder ACLs .
   */
  public function deleteMailbox()
  {
    $this->reset_error();

    @DEBUG (DEBUG_MAIL, __LINE__, __FUNCTION__, __FILE__, "<b>".$this->account_id."</b>" ,
        "<b>Remove account</b> from server :".$this->MailServer);

    return(TRUE);

    /* No imap actions here, just updated shared folder membership 
     */
    $ldap = $this->config->get_ldap_link();
    $ldap->cd($this->config->current['BASE']);
    $ldap->search("(&(objectClass=posixGroup)(objectClass=gosaMailAccount)(memberUid=".$account."))",array('dn','cn'));
    if(class_exists("grouptabs")){
      while($attrs = $ldap->fetch()){
        $tmp = new grouptabs($this->config, $this->config->data['TABS']['GROUPTABS'], $attrs['dn']);
        if(isset($tmp->by_object['mailgroup'])){
          $tmp->by_object['mailgroup']->members= $tmp->by_object['group']->memberUid;
          if(!$this->is_account){
            $tmp->by_object['mailgroup']->removeUserAcl($account);
            $tmp->by_object['mailgroup']->removeUserAcl($this->mail);
          }
          $tmp->by_object['mailgroup']->save();
        }
      }
    }
    return(TRUE);
  }

  
  /*! \brief  Returns the used mail attribute (mail,uid)
      @param  String  One out of 'mail','uid'
   */
  public function getUAttrib()
  {
    return($this->uattrib);
  }


  /*! \brief  Returns the used mail attribute (mail,uid)
      @param  String  One out of 'mail','uid'
   */
  public function getUAttribValue()
  {
    $uattrib = $this->getUAttrib();
    return($this->parent->$uattrib);
  }

  
  /*! \brief  Returns whether the quota settings are enabled or not 
      @return Boolean TRUE if enabled else FALSE
   */
  public function quotaEnabled()
  {
    return($this->enableQuota);
  }

  
  /*! \brief  Returns the used quota 
      @return Integer Quota used.
   */
  public function getQuotaUsage()
  {
    return(-1);
  }


  /*! \brief  Returns the quota restrictions.
      @return Integer Quota restrictions.
   */
  public function getQuota($quotaValue)
  {
    return($quotaValue);
  }

  
  /*! \brief  Sets the mail quota
   */
  public function setQuota($number)
  {
    if(!is_numeric($number)){
      $number = (int) $number;
      if(!$number){
        $number = 0;
      }
    }
    $this->quotaValue = $number; 
    return(TRUE);
  }


  /*! \brief  Returns true whether the domain is selectable or not 
   */
  public function domainSelectionEnabled()
  {
    return($this->enableDomainSelection);
  } 


  /*! \brief Returns a list of configured mail domains 
      @return Array A list of mail domains
   */
  public function getMailDomains()
  {
    return(array("Unconfigured"));
  } 

  
  /*! \brief  Returns the used Spamlevels for this mailmethod 
   */
  public function getSpamLevels()
  {
    $spamlevel= array();
    for ($i= 0; $i<21; $i++){
      $spamlevel[]= $i;
    }
    return($spamlevel);
  }

  
  /*! \brief  Returns the list of configured mailbox folders
      @return Array The mailbox folders.
   */
  public function getMailboxList()
  {
    return(array("INBOX"));
  }


  /*! \brief  Returns whether the vacation range is selectable or not
      @return Boolean TRUE, FALSE
   */
  public function vacationRangeEnabled()
  {
    return($this->enableVacationRange);
  }

  
  /*! \brief  Returns true if the sieveManagement is allowed
      @return Boolean TRUE, FALSE
   */
  public function allowSieveManagement()
  {
    return($this->enableSieveManager);
  } 


  /*! \brief  Checks dependencies to other GOsa plugins.
   */
  public function accountCreateable(&$reason = ""){
    return(TRUE);
  }


  /*! \brief  Checks whether this account is removeable or not.
              There may be some dependencies left, eg. kolab.
   */
  public function accountRemoveable(&$reason = ""){
    return(TRUE);
  }

  
  /*! \brief  Returns all mail servers configured in GOsa 
               that are useable with this mailMethod.
      @return Array  All useable mail servers.
  */
  public function getMailServers()
  {
    $mailserver = array();
    $ui = get_userinfo();
    foreach ($this->config->data['SERVERS']['IMAP'] as $key => $val){
      if( $this->MailServer == $key ||
          preg_match("/r/",$ui->get_permissions($val['server_dn'],"server/goImapServer",""))){
        $mailserver[]= $key;
      }
    }
    return($mailserver);
  }

  
  /*! \brief  Returns the available mailMethods
      @return Array   A list of all avaialable mailMethods_
   */ 
  static protected function get_methods()
  {
    global $class_mapping;
    $available = array();
    foreach($class_mapping as $class => $path){
      if($class == "mailMethod") continue;
      if(preg_match("/^mailMethod/",$class)){
        $available[$class] = $class;
      }
    }
    return($available);
  }

  
  /* \brief   Some method require special folder types, "kolab" for example.
      !! Those values are dummy values, the base class doesn't use folder types;
     @return  Array Return folder types.
   */
  public function getAvailableFolderTypes()
  {
    $ret = array();
    $ret['CAT']['mail'] = _("Mail"); 
    $ret['CAT']['peter']= _("User"); 
    $ret['SUB_CAT']['mail']['a'] = "a";
    $ret['SUB_CAT']['mail']['b'] = "b";
    $ret['SUB_CAT']['peter']['tt'] = "tt";
    $ret['SUB_CAT']['peter']['dd'] = "dd";
    $ret['SUB_CAT']['peter']['did'] = "did";
    return($ret);
  }


  /* \brief  Returns the selected folder type. 
      !! Those values are dummy values, the base class doesn't use folder types;
     @return  Array  The folde type.
   */
  public function getFolderType($default)
  {
    if($this->enableFolderTypes && $this->parent->attrs['kolabFolderType'][0]){
      list($cat,$sub) = split("\.",$this->parent->attrs['kolabFolderType'][0]);
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
    $ldap = $this->config->get_ldap_link();
    $ldap->cd ($this->parent->dn);
    $ldap->modify(array("kolabFolderType" => $type['CAT'].".".$type['SUB_CAT']));
    echo "Move me to KOLAB";
  }


  /*! \brief  Returns configured acls 
   */
  public function  getFolderACLs($folder_acls)
  {
    return($folder_acls);
  }


  /*! \brief  Write ACLs back to imap or what ever 
   */
  public function  setFolderACLs($array)
  {
    return(TRUE);
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
    $acls[" "]= _("none");
    return($acls);
  }

  public function folderTypesEnabled()
  {
    return($this->enableFolderTypes);
  }
   
}


// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
