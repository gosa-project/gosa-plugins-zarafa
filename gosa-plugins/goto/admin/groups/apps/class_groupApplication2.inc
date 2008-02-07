<?php
class appgroup2 extends plugin
{
  /* CLI vars */
  var $cli_summary= "Manage application groups";
  var $cli_description= "Some longer text\nfor help";
  var $cli_parameters= array("eins" => "Eins ist toll", "zwei" => "Zwei ist noch besser");


  var $config;

  /* Contains the menu structure in an array.
   */
  var $a_Structure= array();
  var $a_Structure_on_load = array();

  public function __construct(&$config, $dn= NULL, $parent= NULL)
  {
    plugin::plugin($config,$dn,$parent);
    $this->dn = $dn; 
    $this->_load_menu_structure();
    $this->a_Structure_on_load = $this->a_Structure;
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

          $priority = -1;
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
    $smarty->assign("entries",$this->_get_all_entries(TRUE));
    $display= $smarty->fetch (get_template_path('app_list.tpl', TRUE, dirname(__FILE__)));
    return($display);
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
  }

  
  function _edit_entry_edit($id)
  {
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


  function _move_entry_up($id)
  {
    $all = $this->_get_all_entries(TRUE);
    $parent = FALSE;
    foreach($all as $entry){
      if(isset($entry['UNIQID']) && $entry['UNIQID'] == $id){
        if($parent != FALSE){
          $this->__switch_entries($id,"up",$parent);
          return;
        }
      }else{
        if(in_array($entry['TYPE'],array("CLOSE","OPEN"))){
          $parent = $entry['PARENT'];
        }else{
          $parent = $entry['UNIQID'];
        }
      }
    }
  }


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
          $this->__switch_entries($id,"down",$parent);
          return;
        }
      }
    }
  }


   
  function _get_all_entries($add_tags = FALSE,$cur = NULL,$depth = 0)
  {
    $ret = array();
    $depth ++;
    if($cur == NULL){
      $cur = &$this->a_Structure;
    }
    foreach($cur as $key => $entry){

      $tmp = $entry;
      if(!$add_tags){
        $ret[$tmp['UNIQID']] = &$cur[$key];
        if(isset($entry['ENTRIES']) && count($entry['ENTRIES'])){
          $ret = array_merge($ret,$this->_get_all_entries($add_tags,&$cur[$key]['ENTRIES'],$depth));
        }
      }else{
      if(isset($tmp['ENTRIES'])){
        unset($tmp['ENTRIES']);
      }
        $ret[] = $tmp;
        if(isset($entry['ENTRIES']) && count($entry['ENTRIES'])){
          $ret[] = array("TYPE" => "OPEN", "PARENT" => $entry['UNIQID']);
          $ret = array_merge($ret,$this->_get_all_entries($add_tags,&$cur[$key]['ENTRIES'],$depth));
          $ret[] = array("TYPE" => "CLOSE" , "PARENT" => $entry['UNIQID']);
        }
      }
    }
    return($ret);
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



  function PrepareForCopyPaste($source)
  {
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
  
  function multiple_execute()
  {
    return($this->execute());
  }

  function init_multiple_support($attrs,$all)
  {
    // Do nothing here
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
