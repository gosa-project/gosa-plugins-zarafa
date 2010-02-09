<?php

class hotplugDialog extends plugin
{
  /* attribute list for save action */
  var $hotplugDevices           = ""; 
  var $hotplugDeviceList        = array(); 
  var $regex                    = "*";
  var $depselect                = "/";
  var $deplist                  = "/";
  var $skipThese                = array();
  var $subtree                  = FALSE;

  function hotplugDialog (&$config,$skipThese = array())
  {
    $this->skipThese  = $skipThese;
    $this->config     = $config;
    $this->depselect   = session::get('CurrentMainBase');
  }

  function execute()
  {
    /* Call parent execute */
    plugin::execute();

    /* Fill templating stuff */
    $smarty= get_smarty();
    $display= "";

    if(isset($_POST['dialogissubmitted'])){
      $this->regex=$_POST['regexHot'];
      $this->depselect = $_POST['depselectHot'];
    }

    $this->subtree = isset($_POST['SubSearch']);

    if((isset($_GET['search']))&&(!empty($_GET['search']))){
      $this->regex=$_GET['search']."*";
      $this->regex=preg_replace("/\*\*/","*",$this->regex);
    }

    $this->deplist=$this->config->idepartments;

    $this->hotplugDevices = $this->getHotplugs();
    $smarty->assign("regexHot"    ,$this->regex);
    $smarty->assign("deplistHot"  ,$this->deplist);
    $smarty->assign("depselectHot",$this->depselect);
    $smarty->assign("apply", apply_filter());
    $smarty->assign("alphabet", generate_alphabet());
    $smarty->assign("search_image", get_template_path('images/lists/search.png'));
    $smarty->assign("tree_image", get_template_path('images/lists/search-subtree.png'));
    $smarty->assign("launchimage", get_template_path('images/lists/action.png'));
    $smarty->assign("deplist", $this->config->idepartments);
    $smarty->assign("subtree", $this->subtree?"checked":"");


    $smarty->assign("hotplugDevices",$this->hotplugDevices);
    $smarty->assign("hotplugDeviceKeys",array_flip($this->hotplugDevices));
    $display.= $smarty->fetch(get_template_path('hotplugDialog.tpl', TRUE,dirname(__FILE__)));
    return($display);
  }

  function check(){
    /* Call common method to give check the hook */
    $message= plugin::check();

    if(empty($_POST['hotplugName'])){
      $message[] = _("Please select a hotplug device!");
    }
    return $message;
  }


  /* Save to LDAP */
  function save()
  {
    $entries = array();
    foreach($_POST['hotplugName'] as $name){
      $entries[$name] = $this->hotplugDeviceList[$name];
    }

    return $entries;
  }

  function getHotplugs()
  {
    $filter = "(&(objectClass=gotoDevice)(|(cn=".$this->regex.")(description=".$this->regex.")))";
    $base = get_ou('deviceRDN').$this->depselect;
    $attrs = array("gotoHotplugDevice", "cn", "dn");
    $cat = "devices";

    if ($this->subtree) {
      $res = get_list($filter, $cat, $this->depselect, $attrs, GL_SUBSEARCH);
    } else {
      $res = get_list($filter, $cat, $base, $attrs);
    }

    $a_return = array();  
    $this->hotplugDeviceList = array();
    foreach ($res as $device) {

      if(isset($device['gotoHotplugDevice'][0])){

        $hot_plug = $device['gotoHotplugDevice'][0];
        $tmp      = preg_split("/\|/",$hot_plug);
        $tmp2     = array();
          
        if(in_array($device['cn'][0],$this->skipThese)){
          continue;
        }

        $tmp2['name']         = $device['cn'][0];
        $tmp2['description']  = $tmp[0];
        $tmp2['id']           = $tmp[1];
        $tmp2['produkt']      = $tmp[2];
        $tmp2['vendor']       = $tmp[3];
        $tmp2['dn']           = $device['dn'];

        $a_return[$device['cn'][0]]= $device['cn'][0]." [".$tmp[0]."] ".$tmp[1];

        $this->hotplugDeviceList[$device['cn'][0]]=$tmp2;
      }
    }
    uksort($a_return,"strnatcasecmp");
    return($a_return);
  }
}

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
