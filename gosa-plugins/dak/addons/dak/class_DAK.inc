<?php


class DAK
{

  /* Unused ... */
	public static function get_repositories($config)
	{
    if(!$config instanceOf config){
      trigger_error("Invalid config object given, aborting.");
      return;
    }
    $res = array();
    $ldap = $config->get_ldap_link();
    $ldap->cd($config->current['BASE']);
    $ldap->search("(&(macAddress=*)(FAIrepository=*)(objectClass=FAIrepositoryServer))",array("cn","FAIrepository","macAddress"));
    while($attrs = $ldap->fetch()){
      for($i = 0 ; $i < $attrs['FAIrepository']['count'] ; $i ++){
        list($url,$parent,$release,$sections) = explode("|",$attrs['FAIrepository'][$i]);
        $repo['SECTIONS'] = explode(",",$sections);
        $repo['SERVER']   = $attrs['cn'][0];
        $repo['RELEASE']  = $release;
        $repo['MAC']      = $attrs['macAddress'][0];
        $repo['PARENT']   = $parent;
        $repo['URL']      = $url;
        $repo['DN']       = $attrs['dn'];
        $res[] = $repo;
      }
    }
    return($res);
	}


  /*! \brief  Returns all configured repository server and some 
               informations about release/sections/parent.
      @param  Object  The GOsa configuration object.
      @return Array   Repository infromations.
   */
	public static function get_repositories_by_server($config)
	{
    if(!$config instanceOf config){
      trigger_error("Invalid config object given, aborting.");
      return;
    }
    $res = array();
    $rest = get_sub_list("(&(macAddress=*)(FAIrepository=*)(objectClass=FAIrepositoryServer))",
        "server",get_ou("serverRDN"),$config->current['BASE'],
        array("cn","FAIrepository","macAddress"),GL_SUBSEARCH);

    foreach($rest as $attrs){
      $serv = array();
      $serv['REPOSITORIES'] = array();
      for($i = 0 ; $i < $attrs['FAIrepository']['count'] ; $i ++){
        list($url,$parent,$release,$sections) = explode("|",$attrs['FAIrepository'][$i]);
        $repo['SECTIONS'] = explode(",",$sections);
        $repo['RELEASE']  = $release;
        $repo['PARENT']   = $parent;
        $repo['URL']      = $url;
        $serv['REPOSITORIES'] [] = $repo;
      }
      $serv['MAC']      = $attrs['macAddress'][0];
      $serv['DN']       = $attrs['dn'];
      $serv['SERVER']   = $attrs['cn'][0];
      $res[] = $serv;
    }
    return($res);
	}


  /*! \brief  Returns all keyring entries for the specified server 
      @param  String  The servers mac address.
      @return Array   All keyring entries.
  */
  public static function list_keys($server)
  {
    $o_queue  = new gosaSupportDaemon();
    $data     = $o_queue->DAK_keyring_entries($server);
    if($o_queue->is_error()){
      msg_dialog::display(_("Error"), $o_queue->get_error(), ERROR_DIALOG);
    }
    return($data);
  }


  /*! \brief  Imports the given key into a keyring specified by server 
      @param  String  The mac address of the server that provides the keyring.
      @param  String  The Key to import. 
      @return Boolean TRUE in case of success else FALSE.
   */
  public static function import_key($server,$key)
  {
    $o_queue  = new gosaSupportDaemon();
    $o_queue->DAK_import_key($server,$key);
    if($o_queue->is_error()){
      msg_dialog::display(_("Error"), $o_queue->get_error(), ERROR_DIALOG);
      return(FALSE);
    }
    return(TRUE);
  }


  /*! \brief Removes the given key from a keyring.  
      @param  String  The servers mac address where the keyring is located.
      @param  String  The Key UID to remove.
      @return Boolean TRUE in case of success else FALSE.
   */
  public static function remove_key($server,$key)
  {
    $o_queue  = new gosaSupportDaemon();
    $o_queue->DAK_remove_key($server,$key);
    if($o_queue->is_error()){
      msg_dialog::display(_("Error"), $o_queue->get_error(), ERROR_DIALOG);
      return(FALSE);
    }
    return(TRUE);
  }
}

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
