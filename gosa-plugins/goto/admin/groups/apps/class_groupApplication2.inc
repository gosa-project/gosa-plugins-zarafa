<?php
class appgroup2 extends plugin
{
  /* CLI vars */
  var $cli_summary= "Manage application groups";
  var $cli_description= "Some longer text\nfor help";
  var $cli_parameters= array("eins" => "Eins ist toll", "zwei" => "Zwei ist noch besser");


  var $config;
  var $curbase;

  /* Contains the menu structure in an array.
   */
  var $a_Structure= array();
  var $a_Structure_on_load = array();
  var $Releases;
  var $FAIrelease = 0;
  var $apps = array();
  var $_cache = array();

  public function __construct(&$config, $dn= NULL, $parent= NULL)
  {
    plugin::plugin($config,$dn,$parent);
    $this->dn = $dn; 
    $this->_load_menu_structure();
    $this->a_Structure_on_load = $this->a_Structure;

    /* Check if we have relase mangement enabled and prepare group application for release management */
    $tmp = $config->search("faiManagement", "CLASS",array('menu','tabs'));
    if(!empty($tmp)){
      $this->enableReleaseManagement = true;
    }

    /* Get required release informations */
    if($this->enableReleaseManagement){
      $this->Releases   = $this->getReleases();
      $this->FAIrelease = 0;
    }
    $this->curbase = $this->config->current['BASE'];
    $this->reload();
  }


  function reload()
  {
    $ret = array();
    $release_info = $this->Releases[$this->FAIrelease];

    if(!isset($this->_cache['ReleaseApps'][$release_info['suffix']])){

      $ldap = $this->config->get_ldap_link();
      $ldap->cd($this->curbase);
      $ldap->search("ou=apps",array("dn"));
      $app_deps = array();
      while($attrs = $ldap->fetch()){
        $app_deps[] = $attrs['dn'];
      }

      foreach($app_deps as $dep){
        $ldap->cd($dep);
        $ldap->search("objectClass=FAIbranch",array("dn"));
        while($attrs = $ldap->fetch()){
          $app_deps[] = $attrs['dn'];
        }
      }

      foreach($app_deps as $dep){
        if(preg_match("/^".normalizePreg($release_info['suffix'])."/",$dep)){
          $ret = array_merge($ret,get_list("(objectClass=gosaApplication)","application",$dep,array("*"),GL_NONE));
        }
      }
      $this->_cache['ReleaseApps'][$release_info['suffix']] = $ret;
    } 
    $this->apps = $this->_cache['ReleaseApps'][$release_info['suffix']];
  }


  function getReleases()
  {
    /* Only display those releases that we are able to read */
    $dn     = $this->config->current['BASE'];
    $filter = "(&(objectClass=organizationalUnit)(objectClass=FAIbranch))";
    $res    = get_list($filter,"application", $dn, array("ou"), GL_SUBSEARCH);

    $ret =array(array("name" => "/" , "parts" => array(),"suffix" => get_ou('applicationou')));
    foreach($res as $attrs){
      if(preg_match("/".get_ou('applicationou')."/",$attrs['dn'])){
        $bb     = preg_replace("/".get_ou('applicationou').".*/","",$attrs['dn']);
        $parts  = array_reverse(split("ou=",$bb));

        $str ="";
        foreach($parts as $key => $part){
          if(empty($part)) {
            unset($parts[$key]);
            continue;
          }
          $part = str_replace(",","",$part);
          $str .= $part."/";
          $parts[$key] = $part;
        }
        $name = preg_replace("/\/$/","",$str);
        if(empty($name)) {
          $name ="/";
        }
        $ret[] = array("name" => $name , "dn" => $attrs['dn'], "parts" => $parts,"suffix" => $bb.get_ou('applicationou'));
      }
    }
    return($ret);
  }


  function getReleases2()
  {
    $ret = array();
    $ret['/'] = array("UNIQID" => 0, "NAME" => "/");
    $tmp = $this->_get_all_entries();
    foreach($tmp as $entry){
      if($entry['TYPE'] == "RELEASE"){
        unset($entry['ENTRIES']);
        $ret[] = $entry;
      }  
    }
    return($ret);
  }


  function _load_menu_structure()
  {
    $this->a_Structure  = array();
    $ldap = $this->config->get_ldap_link();
    $ldap->cd($this->dn);
    $ldap->search("(|(objectClass=gotoSubmenuEntry)(objectClass=FAIbranch)(objectClass=gotoMenuEntry))",array("*"));
    $parent_id = 0;
    while($attrs = $ldap->fetch()){
      $cur = &$this->a_Structure;
      $sub_dn       = preg_replace("/,".normalizePreg($this->dn)."$/","",$attrs['dn']);
      $sub_dn_array = split("\,",$sub_dn);
      for($i = (count($sub_dn_array)-1) ; $i >= 0 ; $i--){
        $name = preg_replace("/^[^=]*+=/","",$sub_dn_array[$i]);
        if($i != 0){
          foreach($cur as $key => $entry){
            if($entry['NAME'] == $name){
              $cur = &$cur[$key]['ENTRIES'];
              $parent_id = $entry['UNIQID'];
            }
          }
        }else{

          $priority = 1;
          if(isset($attrs['gosaApplicationPriority'])){
            $priority= $attrs['gosaApplicationPriority'][0];
          }
          while(isset($cur[$priority])){
            $priority ++;
          }

          if(in_array("gotoSubmenuEntry",$attrs['objectClass'])){
            $type = "FOLDER";
          }elseif(in_array("gotoMenuEntry",$attrs['objectClass'])){
            $type = "ENTRY";
          }elseif(in_array("FAIbranch",$attrs['objectClass'])){
            $type = "RELEASE";
          }

          $data = array();
          $data['DN']       = $attrs['dn'];
          $data['NAME']     = $name;
          $data['TYPE']     = $type;
          $data['PRIORITY'] = $priority;
          $data['ENTRIES']  = array();
          $data['UNIQID']   = uniqid();
          $data['PARENT']   = $parent_id;
          $cur[$priority]   = $data;
          ksort($cur);
        }
      }
    }
  } 


  function execute()
  {
    /* Call parent execute */
    plugin::execute();

    if(isset($_GET['r']))
    $this->__construct($this->config,$this->dn);

    $smarty = get_smarty();

    /* Create application list */
    $div = new divSelectBox("appgroup");
    $div->SetHeight(300);
    $departments = array();
    $res = get_list("(objectClass=gosaDepartment)", "application", $this->curbase,array("description","cn","ou"),GL_SIZELIMIT);
    foreach($res as $value){
      $fdn = $value['dn'];
      $fdn = preg_replace("/".normalizePreg($this->curbase)."/","",$fdn);
      $fdn= @LDAP::fix($fdn);
      if($value["description"][0]!=".."){
        $departments[$value['dn']]= convert_department_dn($fdn)." - [".$value["description"][0]."]";
      }else{
        $departments[$value['dn']]=convert_department_dn($fdn)." ["._("Back")."]";
      }
    }

    /* Create 'open' and 'add' links */
#    if($this->acl_is_writeable("gosaMemberApplication",$this->no_release_acls)){
      $linkadd  = "<a href='?plug=".$_GET['plug']."&amp;act=add&amp;id=%s'>%s</a>";
 #   }else{
  #    $linkadd = "%s";
   # }
    $linkopen = "<a href='?plug=".$_GET['plug']."&amp;act=depopen&amp;depid=%s'>%s</a>";

    /* Create base back entry */
    $base_back = preg_replace("/^[^,]+,/","",$this->curbase);
    if((strlen($base_back)>= strlen($this->config->current['BASE']))&&($this->curbase!=$this->config->current['BASE'])){
      $div->AddEntry(array(
            array("string"=>sprintf($linkopen,base64_encode($base_back),".. ["._("back")."]"),
              "attach"=>"style='border:0px;'")
            ));
    }

    /* Append departments for current base */
    foreach($departments as $key => $app){
      $div->AddEntry(array(
            array("string"=>"<img class='center' src='images/folder.png' alt='"._("department")."'>&nbsp;".sprintf($linkopen,
                base64_encode($key),$app),
              "attach"=>"style='border:0px;'")
            ));
    }

  
    /* Add applications found on this base */
    $used_apps = $this->_get_used_entry_name();
    foreach($this->apps as $key => $app){
      if(in_array($app['cn'][0],$used_apps)){
        continue;
      }
      if(!preg_match("/".get_ou('applicationou').normalizePreg($this->curbase)."$/",$app['dn'])){
        continue;
      }

      $name = $app['cn'][0];
      if(isset($app['description'])){
        $name .= "&nbsp;[".$app['description'][0]."]";
      }
      $div->AddEntry(array(
            array("string"=>sprintf("<input class='center' type='checkbox' value='1' name='AddApp_%s'>",$key).
              "<img class='center' src='images/select_application.png' alt='"._("application")."'>&nbsp;".sprintf($linkadd,$key,$name),
              "attach"=>"style='border:0px;'")
            ));
    }

    
    $smarty->assign("FAIrelease",$this->FAIrelease);
    $smarty->assign("app_list",$div->DrawList());
    $smarty->assign("releases",$this->Releases);
    $entries = $this->_get_entries_for_release($this->FAIrelease);
    $smarty->assign("entries",$entries);
    $display= $smarty->fetch (get_template_path('app_list.tpl', TRUE, dirname(__FILE__)));
    return($display);
  }

 
  function _get_used_entry_name()
  {
    $data = $this->_get_entries_for_release($this->FAIrelease);
    $ret = array();
    foreach($data as $entry){
      if($entry['TYPE'] == "ENTRY"){
        $ret[] = $entry['NAME'];
      }
    }
    return($ret);
  }
 
  function _get_entries_for_release($release,$cur = NULL)
  {
    $release_info = $this->Releases[$release];
    $cur = &$this->a_Structure;

    $found = FALSE; 
    foreach($release_info['parts'] as $name){
      foreach($cur as $key => $obj){
        if($obj['TYPE'] == "RELEASE" && $obj['NAME'] == $name){
          $cur = &$cur[$key]['ENTRIES'];
          $found = TRUE;
          break;
        }
        $found = FALSE;
      }
    }
    if($found){
      return($this->_get_all_entries(TRUE,TRUE,$cur));
    }else{
      return(array());
    }
  }


  function _edit_entry_edit($id)
  {
  }


  function remove_from_parent()
  {
  }


  function save()
  {
  }


  function check()
  {
  }



  /* !\brief Handle ui POSTS, like sort up/down/delete
   */ 
  function save_object()
  {
    foreach($_POST as $name => $value){
      if(preg_match("/del_/",$name)){
        $id = preg_replace("/^del_/","",$name);
        $id = preg_replace("/_(x|y)$/","",$id);
        $this->_remove_entry_id($id);
        break;
      }
      if(preg_match("/edit_/",$name)){
        $id = preg_replace("/^edit_/","",$name);
        $id = preg_replace("/_(x|y)$/","",$id);
        $this->_edit_entry_edit($id);
        break;
      }
      if(preg_match("/up_/",$name)){
        $id = preg_replace("/^up_/","",$name);
        $id = preg_replace("/_(x|y)$/","",$id);
        $this->_move_entry_up($id);
        break;
      }
      if(preg_match("/down_/",$name)){
        $id = preg_replace("/^down_/","",$name);
        $id = preg_replace("/_(x|y)$/","",$id);
        $this->_move_entry_down($id);
        break;
      }
    }
    if(isset($_POST['FAIrelease'])){
      $this->FAIrelease = $_POST['FAIrelease'];
    }
    if(isset($_GET['act']) && $_GET['act'] == 'depopen'){
      $this->curbase = base64_decode($_GET['depid']);
    }
    $this->reload();
  }


  /* !\brief Remove the given id from the menu structure.
      @param  String  ID to of the entry we want to remove.
      @return Boolean TRUE on success
   */
  function _remove_entry_id($id,$start = NULL)
  {
    if($start == NULL){
      $start = &$this->a_Structure;
    }
    foreach($start as $name => $entry){
      if($entry['UNIQID'] == $id){
        unset($start[$name]);
        return(TRUE);
      }
      if(isset($entry['ENTRIES']) && count($start[$name]['ENTRIES'])){
        if($this->_remove_entry_id($id,&$start[$name]['ENTRIES'])){
          return(TRUE);
        }
      }
    }
    return(FALSE);
  }


  /*! \brief move the object identified by the given ID one position up.
      @param  String  The ID of the entry to be moved.
      @return Boolean TRUE on success, else FALSE;
   */
  function _move_entry_up($id)
  {
    $all = $this->_get_all_entries(TRUE);
    $parent = FALSE;
    foreach($all as $entry){
      if(isset($entry['UNIQID']) && $entry['UNIQID'] == $id){
        if($parent != FALSE){
          return($this->__switch_entries($id,"up",$parent));
        }
      }else{
        if(in_array($entry['TYPE'],array("CLOSE","OPEN"))){
          $parent = $entry['PARENT'];
        }else{
          $parent = $entry['UNIQID'];
        }
      }
    }
    return(FALSE);
  }


  /*! \brief move the object identified by the given ID one position down.
      @param  String  The ID of the entry to be moved.
      @return Boolean TRUE on success, else FALSE;
   */
  function _move_entry_down($id)
  {
    $all = $this->_get_all_entries(TRUE);
    $found = FALSE;
    foreach($all as $entry){
      if(isset($entry['UNIQID']) && $entry['UNIQID'] == $id){
        $found = TRUE;
        continue;
      }else{
        if(in_array($entry['TYPE'],array("CLOSE","OPEN"))){
          $parent = $entry['PARENT'];
        }else{
          $parent = $entry['UNIQID'];
        }
        if($found){
          return($this->__switch_entries($id,"down",$parent));
        }
      }
    }
    return(FALSE);
  }


  /*! \brief  Return all entries linear. 
      @param  Boolean   $add_tags  If TRUE, OPEN/CLOSE Tags will be appended.
      @param  &Array    Start here, Pointer to an array.
   */ 
  function _get_all_entries($add_tags = FALSE, $skip_release = FALSE, $cur = NULL)
  {
    $ret = array();
    if($cur == NULL){
      $cur = &$this->a_Structure;
    }
    foreach($cur as $key => $entry){

      if($skip_release && $entry['TYPE'] == "RELEASE"){
        continue;
      }    

      $tmp = $entry;
      if(!$add_tags){
        $ret[$tmp['UNIQID']] = &$cur[$key];
        if(isset($entry['ENTRIES']) && count($entry['ENTRIES'])){
          $ret = array_merge($ret,$this->_get_all_entries($add_tags,$skip_release,&$cur[$key]['ENTRIES']));
        }
      }else{
      if(isset($tmp['ENTRIES'])){
        unset($tmp['ENTRIES']);
      }
        $ret[] = $tmp;
        if(isset($entry['ENTRIES']) && count($entry['ENTRIES'])){
          $ret[] = array("TYPE" => "OPEN", "PARENT" => $entry['UNIQID']);
          $ret = array_merge($ret,$this->_get_all_entries($add_tags,$skip_release,&$cur[$key]['ENTRIES']));
          $ret[] = array("TYPE" => "CLOSE" , "PARENT" => $entry['UNIQID']);
        }
      }
    }
    return($ret);
  }


  /* !\brief Switch one entry with another 
     @param  String  from  The source ID.
     @param  String  type  "up"/"down" type switched.
     @param  String  to    The destination ID.
     return  Boolean TRUE on success 
   */
  function __switch_entries($from,$type,$to)
  {
    $all = $this->_get_all_entries();

    $o_to   = &$all[$to];
    $o_from = &$all[$from];

    /***********************
     * Source == Destination 
     * Move into next parent. 
     ************************/
    if($to == $from){
      $to = $o_to['PARENT'];
      $o_to   = &$all[$to];
    }

    /***********************
     * Target is container 
     ************************/
    if(in_array($o_to['TYPE'],array("FOLDER","RELEASE"))){

      /***********************
       * Move into parent folder  
       *   + Target 
       *   |-> Source
       ************************/
      if($to == $o_from['PARENT']){
 
        /* Check if source is a folder object 
         */
        $o_to_sub = &$all[$o_to['PARENT']]; 
        if(in_array($o_to_sub['TYPE'],array("FOLDER","RELEASE"))){

          /* Removing old */
          $tmp = array();
          foreach($o_to['ENTRIES'] as $key => $entry){
            if($entry['UNIQID'] == $from){
              if(!$this->_remove_entry_id($from)){
                return(FALSE);
              }
            }
          }

          /* Adding new */
          $tmp2 = array();
          foreach($o_to_sub['ENTRIES'] as $key => $entry){

            if($type == "up"){
              if($entry['UNIQID'] == $to){
                $o_from['PARENT'] = $o_to_sub['UNIQID'];
                $tmp2[] = &$o_from;
              }
              $tmp2[] = &$o_to_sub['ENTRIES'][$key];
            }else{

              $tmp2[] = &$o_to_sub['ENTRIES'][$key];
              if($entry['UNIQID'] == $to){
                $o_from['PARENT'] = $o_to_sub['UNIQID'];
                $tmp2[] = &$o_from;
              }
            }
          }
          $all[$o_to_sub['UNIQID']]['ENTRIES'] = $tmp2;
        } 
      }else{

        /***********************
         * Target is NOT parent container
         *  + Parent Folder
         *  |-> Source 
         *  + Destination
         ************************/

        /* Removing old */
        $o_to   = &$all[$to];
        $o_from = &$all[$from];
        $this->_remove_entry_id($from);
        $o_from['PARENT']     = $to;
        $o_to['ENTRIES'][]    = $o_from;

      }
    }else{

      /***********************
       * Source and Destination in some Folder.
       *  + Parent folder
       *  |-> Source
       *  |-> Destination
       ************************/
      $o_to   = &$all[$to];
      $o_from = &$all[$from];

      if($o_to['PARENT'] == $o_from['PARENT']){
        $tmp = $all[$to];
        $all[$to]   = $o_from;
        $all[$from] = $tmp;
      }
    }
  }


  /* Return plugin informations for acl handling  */ 
  static function plInfo()
  {
    return (array(
          "plShortName"   => _("Applications"),
          "plDescription" => _("Group applications"),
          "plSelfModify"  => FALSE,
          "plDepends"     => array(),
          "plPriority"    => 0,
          "plSection"     => array("admin"),
          "plCategory"    => array("groups"),
          "plProvidedAcls"=> array(
            "gosaMemberApplication"     => _("Application"),
            "FAIrelease"                => _("Release"),
            "gosaApplicationParameter"  => _("Application parameter"))
          ));
  }


  function PrepareForCopyPaste($source)
  {
  }


  function multiple_save_object()
  {
    if(isset($_POST['group_apps_multi'])){
      $this->save_object(); 
      plugin::multiple_save_object();    
  
      /* Get posts */
      foreach(array("apps") as $attr){
        if(isset($_POST['use_'.$attr])) {
          $this->multi_boxes[] = $attr;
        }
      }
    }
  }
  

  function get_multi_edit_values()
  {
    $ret = plugin::get_multi_edit_values();

    if(in_array("apps",$this->multi_boxes)){
      $ret['gosaApplicationParameter'] = $this->gosaApplicationParameter;
      $ret['Categories']               = $this->Categories;
      $ret['gosaMemberApplication']    = $this->gosaMemberApplication;
      $ret['FAIrelease']               = $this->FAIrelease;
      $ret['appoption']                = $this->appoption;
    }
    return($ret);
  }
}
// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
