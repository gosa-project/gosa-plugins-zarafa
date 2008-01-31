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

  var $a_Releases = array();
  var $a_Entries  = array();
  var $a_Folders  = array();
  var $menu_pointer = NULL;

  public function __construct(&$config, $dn= NULL, $parent= NULL)
  {
    plugin::plugin($config,$dn,$parent);
    $this->dn = $dn; 
    $this->_load_menu_structure();
  }


  function _load_menu_structure()
  {
    $this->a_Structure  = array();
    $this->a_Releases   = array();
    $this->a_Entries    = array();
    $this->a_Folders    = array();

    $this->menu_pointer = &$this->a_Structure;

    $ldap = $this->config->get_ldap_link();
    $ldap->cd($this->dn);
    $ldap->search("(|(objectClass=gotoSubmenuEntry)(objectClass=FAIbranch)(objectClass=gotoMenuEntry))",array("*"));
    while($attrs = $ldap->fetch()){
      $cur = &$this->a_Structure;
      $sub_dn       = preg_replace("/,".normalizePreg($this->dn)."$/","",$attrs['dn']);
      $sub_dn_array = split("\,",$sub_dn);

      for($i = (count($sub_dn_array)-1) ; $i >= 0 ; $i--){
        
        $name = preg_replace("/^ou=/","",$sub_dn_array[$i]);
        if($i != 0){
          $cur = &$cur[$name]['ENTRIES'];
        }else{

          $priority = -1;
          if(isset($attrs['gosaApplicationPriority'])){
            $priority= $attrs['gosaApplicationPriority'][0];
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
          $date['ENTRIES']  = array();
          $cur[$name]       = $data;

          switch($type){
            case 'RELEASE': $this->a_Releases[$name] = &$cur[$name];break;
            case 'ENTRY':   $this->a_Entries[$name] = &$cur[$name];break;
            case 'FOLDER':  $this->a_Folders[$name] = &$cur[$name];break;
          }
        }
      }
    }
  } 


  function execute()
  {
    /* Call parent execute */
    plugin::execute();

    $this->__construct($this->config, $this->dn,NULL);
    $smarty = get_smarty();
    $smarty->assign("entries",$this->_get_all_entries());
    $display= $smarty->fetch (get_template_path('app_list.tpl', TRUE, dirname(__FILE__)));
    return($display);
  }


  function _get_all_entries($cur = NULL,$depth = 0)
  {
    $ret = array();
    $depth ++;
    if($cur == NULL){
      $cur = &$this->a_Structure;
    }
    $ret[] = array("TYPE" => "OPEN");
    foreach($cur as $entry){
      $tmp = $entry;
      if(isset($tmp['ENTRIES'])){
        unset($tmp['ENTRIES']);
      }
      $ret[] = $tmp;
      if(isset($entry['ENTRIES'])){
        $ret = array_merge($ret,$this->_get_all_entries(&$entry['ENTRIES'],$depth));
      }
    }
    $ret[] = array("TYPE" => "CLOSE");
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
