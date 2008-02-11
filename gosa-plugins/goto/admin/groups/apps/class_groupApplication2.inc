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

  var $app_parameter = array();
  var $edit_entry    = array();

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
#      $this->FAIrelease = 0;
    }
#    $this->curbase = $this->config->current['BASE'];
    $this->reload();
  }


  function reload()
  {
    $ret = array();
    $release_info = $this->Releases[$this->FAIrelease];

    if(!isset($this->_cache['ReleaseApps'][$release_info['suffix']])){

      $ldap = $this->config->get_ldap_link();
      $ldap->cd($this->config->current['BASE']);
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
    $res    = get_list($filter,"application", $dn, array("ou","FAIstate"), GL_SUBSEARCH);

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
        $FAIstate = "";
        if(isset($attrs['FAIstate'])){
          $FAIstate = $attrs['FAIstate'][0];
        }
        
        $ret[] = array("name"     => $name, 
                       "FAIstate" => $FAIstate,
                       "dn"       => $attrs['dn'], 
                       "parts"    => $parts,"suffix" => $bb.get_ou('applicationou'));
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

    $base =  array();
    $base['UNIQID'] = uniqid();
    $base['PARENT'] = 0; 
    $base['NAME']   = "";
    $base['TYPE']   = "BASE";
    $base['ENTRIES']= array();
    $base['STATUS'] = "LOADED";
    
    $this->a_Structure[0] = $base;

    while($attrs = $ldap->fetch()){
      $cur = &$this->a_Structure[0]['ENTRIES'];
      $parent_id    = $base['UNIQID'];
      $sub_dn       = preg_replace("/,".normalizePreg($this->dn)."$/","",$attrs['dn']);
      $sub_dn_array = split("\,",$sub_dn);


      for($i = (count($sub_dn_array)-1) ; $i >= 0 ; $i--){
        $name = preg_replace("/^[^=]*+=/","",$sub_dn_array[$i]);
        if($i > 0){
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

          $data = array();
          if(in_array("gotoSubmenuEntry",$attrs['objectClass'])){
            $type = "FOLDER";

            $data['ICON'] = "";
            if(isset($attrs['gosaApplicationIcon'])){
              $data['ICON'] = $ldap->get_attribute($attrs['dn'],"gosaApplicationIcon");
            }

          }elseif(in_array("gotoMenuEntry",$attrs['objectClass'])){
            $type = "ENTRY";
            $data['PARAMETER'] = array();
            if(isset($attrs['gosaApplicationParameter'])){
              for($p = 0 ; $p < $attrs['gosaApplicationParameter']['count'] ; $p ++){
                $tmp = split(":",$attrs['gosaApplicationParameter'][$p]);
                $data['PARAMETER'][$tmp[0]] = $tmp[1];
              }
            }
          }elseif(in_array("FAIbranch",$attrs['objectClass'])){
            $type = "RELEASE";
            if(isset($attrs['FAIstate'][0])){
              $data['FAIstate'] = $attrs['FAIstate'][0];
            }else{
              $data['FAIstate'] = "";
            }
          }

          $data['DN']       = $attrs['dn'];
          $data['NAME']     = $name;
          $data['TYPE']     = $type;
          $data['PRIORITY'] = $priority;
          $data['ENTRIES']  = array();
          $data['UNIQID']   = uniqid();
          $data['PARENT']   = $parent_id;
          $data['STATUS']   = "LOADED";
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

    if(isset($_GET['send'])){
      $id = $_GET['send'];
      $all = $this->_get_all_entries();
      if(isset($all[$id])){
        send_binary_content($all[$id]['ICON'],$id.".jpg","image/jpeg");
        exit;
      }
    }

    if(isset($_GET['r']))
    $this->__construct($this->config,$this->dn);

    if(count($this->edit_entry)){
      if($this->edit_entry['TYPE'] == "ENTRY"){
        $smarty = get_smarty();
        $smarty->assign("type", "ENTRY");
        $smarty->assign("entry",$this->edit_entry);
        $smarty->assign("paras",$this->app_parameter);
        $display= $smarty->fetch (get_template_path('edit_entry.tpl', TRUE, dirname(__FILE__)));
        return($display);
      }
      if($this->edit_entry['TYPE'] == "FOLDER"){
        $smarty = get_smarty();

        session::set("binarytype" , "image/jpeg");
        session::set("binary" , $this->edit_entry['ICON']);
  
        $smarty->assign("rand", microtime(TRUE));
        $smarty->assign("image_set" , strlen($this->edit_entry['ICON']) > 0); 
        $smarty->assign("type", "FOLDER");
        $smarty->assign("entry",$this->edit_entry);
        $display= $smarty->fetch (get_template_path('edit_entry.tpl', TRUE, dirname(__FILE__)));
        return($display);
      }
    }

    $smarty = get_smarty();
    $smarty->assign("plug_id" , $_GET['plug']);

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
              "<img class='center' src='images/select_application.png' alt='"._("application")."'>&nbsp;".$name,
              "attach"=>"style='border:0px;'")
            ));
    }

    
    $smarty->assign("FAIrelease",$this->FAIrelease);
    $smarty->assign("app_list",$div->DrawList());
    $smarty->assign("releases",$this->Releases);
    $smarty->assign("folders" , $this->_get_folder_names());
    $entries = $this->_get_entries_for_release($this->FAIrelease);
    $smarty->assign("entries",$entries);
    $display= $smarty->fetch (get_template_path('app_list.tpl', TRUE, dirname(__FILE__)));
    return($display);
  }

    
  function _get_folder_names()
  {
    $data = $this->_get_entries_for_release($this->FAIrelease);
    $ret = array("BASE" => ".");
    foreach($data as $entry){
      if($entry['TYPE'] == "FOLDER"){
        $ret[$entry['UNIQID']] = $entry['NAME'];
      }
    }
    return($ret);
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
    $all = $this->_get_all_entries();
    $key = $this->_get_release_key($release);
    if(isset($all[$key]) && count($all[$key]['ENTRIES'])){
      $res = $this->_get_all_entries(TRUE,TRUE,&$all[$key]['ENTRIES']);
      return($res);
    } 
    return(array());
  }


  function _save_entry_edit()
  {
    $all    = $this->_get_all_entries();
    $entry  = $this->edit_entry;
    $r_entry= &$all[$entry['UNIQID']];

    if($entry['TYPE'] == "ENTRY"){
      $r_entry['PARAMETER'] = $this->app_parameter;
      $r_entry['STATUS'] = "EDITED";
    }
    if($entry['TYPE'] == "FOLDER"){
      $r_entry['ICON']   = $this->edit_entry['ICON'];
      $r_entry['STATUS'] = "EDITED";
    }
    $this->dialog = FALSE;
    $this->edit_entry = array();
  }


  function _edit_entry_edit($id)
  {
    $all   = $this->_get_all_entries();
    $entry = $all[$id];

    $this->app_parameter = array();
    if($entry['TYPE'] == "ENTRY"){
      $found = FALSE;
      foreach($this->apps as $id => $app){

        if($app['cn'][0] == $entry['NAME']){
          $found = TRUE;
          break;
        }
      }
      if($found){
      
        /* Create a list of editable parameter */
        if(isset($app['gosaApplicationParameter'])){
          for($i = 0 ; $i < $app['gosaApplicationParameter']['count'] ; $i++) {
            $para = $app['gosaApplicationParameter'][$i];
            $tmp  = split(":",$para);
            $this->app_parameter[$tmp[0]] = $tmp[1];
          }
        }

        /* Overwrite parameters with entry parameters */
        foreach($entry['PARAMETER'] as $name => $value){
          $this->app_parameter[$name] = $value;
        }
        
        $this->dialog = TRUE;
        $this->edit_entry = $entry;
      }
    }

    if($entry['TYPE'] == "FOLDER"){
      $this->dialog = TRUE;
      $this->edit_entry = $entry;
    }
  }


  function remove_from_parent()
  {
  }



  function check()
  {
  }


  function _check_missing_release($release)
  {
    $release_info = $this->Releases[$release];

    $parent_id = $this->a_Structure[0]['UNIQID'];
    $cur = &$this->a_Structure[0]['ENTRIES'];
    for($i = 0 ; $i < count($release_info['parts']) ; $i ++){
      $part = $release_info['parts'][$i];
      $found = FALSE;
      foreach($cur as $key => $name){
        if($name['NAME'] == $part){
          $parent_id = $cur[$key]['UNIQID'];
          $cur = &$cur[$key]['ENTRIES'];
          
          $found =TRUE;
          break;
        }
      }
      if(!$found){
        $release           =  array();
        $release['UNIQID'] = uniqid();
        $release['PARENT'] = $parent_id;
        $release['NAME']   = $part;
        $release['TYPE']   = "RELEASE";
        $release['ENTRIES']= array();
        $release['STATUS']   = "ADDED";
        $release['FAIstate'] =  $release_info['FAIstate'];
        $cur[] = $release;
        $i --;
      }
    }
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
      if(preg_match("/app_entry_edit/",$name)){
        $id = preg_replace("/^app_entry_edit/","",$name);
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
      if(preg_match("/^parameter_/",$name) && 
        count($this->edit_entry) && $this->edit_entry['TYPE'] == "ENTRY"){
        $name = preg_replace("/^parameter_/","",$name);
        $this->app_parameter[$name] = $value;
      }
    }
    if(isset($_POST['FAIrelease'])){
      $this->FAIrelease = $_POST['FAIrelease'];
      $this->_check_missing_release($this->FAIrelease);
    }
    if(isset($_GET['act']) && $_GET['act'] == 'depopen'){
      $this->curbase = base64_decode($_GET['depid']);
    }
    if(isset($_POST['add_to_folder']) && isset($_POST['folder'])){
      $folder = $_POST['folder'];
      foreach($_POST as $name => $value){
        if(preg_match("/^AddApp_[0-9]*$/",$name)){
          $this->_add_app_id($folder,preg_replace("/^AddApp_/","",$name));   
        }
      }
    }
    if(isset($_POST['add_menu_to_folder']) && isset($_POST['menu_folder'])){
      $folder = $_POST['menu_folder'];
      $name = $_POST['menu_folder_name'];
      if(strlen($name) > 0 && preg_match("/[a-z ]/i",$name)){
        $this->_add_sub_folder($folder,$name);
      }
    }
    if(isset($_POST['app_entry_save'])){ 
      $this->_save_entry_edit();
    }

    if(isset($_FILES['folder_image']) && isset($_POST['folder_image_upload'])){
      if($_FILES['folder_image']['error'] == 0 && $_FILES['folder_image']['size'] > 0){
        $this->edit_entry['ICON'] = file_get_contents($_FILES['folder_image']['tmp_name']);
      }
    }

    if(isset($_POST['app_entry_cancel'])){
      $this->edit_entry = array();
      $this->dialog = FALSE;
    }
    $this->reload();
  }

  
  function _get_release_key($release,$add_if_missing = FALSE)
  {
    $release_info = $this->Releases[$release];

    if($release_info['name'] == "/"){
      return($this->a_Structure['0']['UNIQID']);
    }

    $cur = &$this->a_Structure[0]['ENTRIES'];
    $s_key = "";
    $found = FALSE;
    foreach($release_info['parts'] as $name){
      foreach($cur as $key => $obj){
        if($obj['TYPE'] == "RELEASE" && $obj['NAME'] == $name){
          $s_key = $cur[$key]['UNIQID'];
          $cur = &$cur[$key]['ENTRIES'];
          $found = TRUE;
          break;
        }
        $found = FALSE;
      }
    }
    if($found){
      return($s_key);  
    }  
    return(FALSE);
  }

  
  function _add_sub_folder($folder,$name)
  {
    $all = $this->_get_all_entries();
    if($folder == "BASE"){
      $folder = $this->_get_release_key($this->FAIrelease,TRUE);
    }
    
    if(isset($all[$folder])){
      $a_folder = array();
      $a_folder['STATUS'] = "ADDED";
      $a_folder['NAME']   = $name;
      $a_folder['UNIQID'] = uniqid();
      $a_folder['PARENT'] = $folder;      
      $a_folder['TYPE']   = "FOLDER";
      $all[$folder]['ENTRIES'][] = $a_folder;
    }
  }


  /* !\brief Remove the given id from the menu structure.
      @param  String  ID to of the entry we want to remove.
      @return Boolean TRUE on success
   */
  function _remove_entry_id($id)
  {
    $all = $this->_get_all_entries();
    if(isset($all[$id])){
      $all[$id]['STATUS'] = "REMOVED";
      return(TRUE);
    }
    return(FALSE);
  }

  
  /* !\brief Remove the given id from the menu structure.
      @param  String  ID to of the entry we want to remove.
      @return Boolean TRUE on success
   */
  function _add_entry($folder_id,$entry,$pos = 0)
  {
    $all = $this->_get_all_entries();

    if(isset($all[$folder_id])){

      $folder  = &$all[$folder_id];
      $entries = $entry['ENTRIES'];

      $entry['UNIQID'] = uniqid();     
      $entry['PARENT'] = $folder_id;
      $entry['ENTRIES']= array();
      $entry['STATUS'] = "ADDED";
      
      $cnt = 0; 
      $new = array();
      $added =FALSE;
      foreach($folder['ENTRIES'] as $key => $obj){
        if($pos == $cnt){
          $new[] = $entry;
          $added = TRUE;
          $cnt ++;
        }
        $new[] = $obj;
      }
      if(!$added){
        $new[] = $entry;
      }
      $all[$folder_id]['ENTRIES'] = $new;
      foreach($entries as $sub){
        $this->_add_entry($entry['UNIQID'],$sub);
      }
      return(TRUE);
    }
    return(FALSE);
  }

  
  function _add_app_id($folder_id,$app_id)
  {
    $all = $this->_get_all_entries();
    if($folder_id == "BASE"){
      $folder_id = $this->_get_release_key($this->FAIrelease);
    }
    if(isset($all[$folder_id]) && isset($this->apps[$app_id])){

      $new = array();
      $new['TYPE']  = "ENTRY";
      $new['NAME']  = $this->apps[$app_id]['cn'][0];
      $new['UNIQID']= uniqid(); 
      $new['PARENT']= $folder_id;
      $new['STATUS']= "ADDED";
      $all[$folder_id]['ENTRIES'][] = $new;
    }
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
      if($entry['TYPE'] == "ENTRY"){
        $found = FALSE;
        foreach($this->apps as $app){
          if($app['cn'][0] == $entry['NAME']){
            $found = TRUE;
            if(isset($app['description'][0])){
              $entry['INFO'] = "[".$app['description'][0]."]";
            }
            break;
          }
        } 
        if(!$found){
          $entry['INFO'] = "<font color='red'>"._("Not available in release.")."</font>";
        }
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
        if($tmp['STATUS'] != "REMOVED"){
          $ret[] = $tmp;
          if(isset($entry['ENTRIES']) && count($entry['ENTRIES'])){
            $add = false;
            foreach($entry['ENTRIES'] as $entry){
              if($entry['STATUS'] != "REMOVED"){
                $add = TRUE;
                break;
              }
            }

            if($add){
              $ret[] = array("TYPE" => "OPEN", "PARENT" => $entry['UNIQID']);
              $ret = array_merge($ret,$this->_get_all_entries($add_tags,$skip_release,&$cur[$key]['ENTRIES']));
              $ret[] = array("TYPE" => "CLOSE" , "PARENT" => $entry['UNIQID']);
            }
          }
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

    print_a($o_to,$o_from);
  
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

          $o_from['STATUS'] = "ADDED";

          /* Adding new */
          $tmp2 = array();  
          $cnt = 0; 
          foreach($o_to_sub['ENTRIES'] as $key => $entry){
            $cnt ++;
            if($entry['UNIQID'] == $to){
              break;
            }
          }
          if($type == "up"){
            $cnt --;
 
         }
         $this->_add_entry($o_to_sub['UNIQID'],$o_from,$cnt);

          /* Removing old */
          $tmp = array();
          if(!$this->_remove_entry_id($from)){
            return(FALSE);
          }
        } 
      }else{

        /***********************
         * Target is NOT parent container
         *  + Parent Folder
         *  |-> Source 
         *  + Destination
         ************************/

        /* Removing old */
        $o_to   = $all[$to];
        $o_from = $all[$from];
        $this->_add_entry($to,$o_from);
        $this->_remove_entry_id($from);
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
      $parent = &$all[$o_to['PARENT']];

      if($o_to['PARENT'] == $o_from['PARENT']){
        $tmp = $all[$to];
        $all[$to]   = $o_from;
        $all[$from] = $tmp;
  
        /* Ensure that the app priority is updated */
        foreach($parent['ENTRIES'] as $key => $entry){
          $parent['ENTRIES'][$key]['STATUS'] = "EDITED";
        }
      }
    }
  }


  function save()
  {
    $ldap = $this->config->get_ldap_link();
    $all = $this->_get_all_entries();
    $prio = 0;
    $Actions = array("Remove" => array(),"Edit" => array() , "Add" => array());

    foreach($all as $entry){
      $prio ++;
      $cur = $entry;
      $dn = "";

      do{  
        if($cur['TYPE'] == "ENTRY"){
          $dn.= "cn=".$cur['NAME'].",";
        }elseif($cur['TYPE'] == "FOLDER"){
          $dn.= "cn=".$cur['NAME'].",";
        }elseif($cur['TYPE'] == "RELEASE"){
          $dn.= "ou=".$cur['NAME'].",";
        }elseif($cur['TYPE'] == "BASE"){
        }
        if(!isset($all[$cur['PARENT']])){
          $cur = NULL;
        }else{
          $cur = $all[$cur['PARENT']];
        }
      }while(is_array($cur));

      $cur_dn = $dn.$this->dn;

      $attrs = array();
      switch($entry['TYPE']){
        case "ENTRY"    :
        { 
          $attrs['objectClass'] = "gotoMenuEntry";
          $attrs['cn']          = $entry['NAME'];
          $attrs['gosaApplicationPriority'] = $prio;
          $attrs['gosaApplicationParameter'] = array(); 
     
          foreach($entry['PARAMETER'] as $name => $value){
            $attrs['gosaApplicationParameter'][] = $name.":".$value; 
          }
        }
        break;
        case "FOLDER"   : 
        { 
          $attrs['objectClass'] = "gotoSubmenuEntry";
          $attrs['cn']          = $entry['NAME'];
          $attrs['gosaApplicationPriority'] = $prio;
          $attrs['gosaApplicationIcon']     = $entry['ICON'];
        }
        break;
        case "RELEASE"  : 
        { 
          $attrs['ou']            = $entry['NAME'];
          $attrs['objectClass']   = array();
          $attrs['objectClass'][] = "top";
          $attrs['objectClass'][] = "organizationalUnit";
          $attrs['objectClass'][] = "FAIbranch";
          if(!empty($entry['FAIstate'])){
            $attrs['FAIstate']      = $entry['FAIstate'];
          }
        }
        break;
      }
  
      if($entry['STATUS'] == "LOADED"){
        continue;
      }
      if($entry['STATUS'] == "REMOVED"){
        $Actions['Remove'][$cur_dn] = $cur_dn;
      }
      if($entry['STATUS'] == "EDITED"){
        $Actions['Edit'][$cur_dn] = $attrs;
      }
      if($entry['STATUS'] == "ADDED"){
        $Actions['Add'][$cur_dn] = $attrs;
      }
    }

    $ldap = $this->config->get_ldap_link();
    $ldap->cd($this->config->current['BASE']);
    foreach($Actions['Remove'] as $dn){
      $ldap->cd($dn);
      $ldap->cat($dn);
      if($ldap->count()){
        $ldap->rmdir_recursive($dn);
      }
    }
    foreach($Actions['Add'] as $dn => $data){
      $ldap->cd($dn);
      $ldap->cat($dn);
      if(!$ldap->count()){
        $ldap->add($data);
      }
    }
    foreach($Actions['Edit'] as $dn => $data){
      $ldap->cd($dn);
      $ldap->cat($dn);
      if($ldap->count()){
        $ldap->modify($data);
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
