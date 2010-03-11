<?php
/*
 * This code is part of GOsa (http://www.gosa-project.org)
 * Copyright (C) 2003-2008 GONICUS GmbH
 *
 * ID: $$Id: class_gotomasses.inc 10967 2008-05-20 13:18:12Z hickert $$
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */


/*! \brief  The DAK keyring class used to manage DAK keyrings.
            It provides remove/add key mehtods and lists all keys for a selected server.
 */
class dakkeyring extends plugin
{
  private $Servers          = array();
  private $selected_Server  = array();
  private $list             = array();
  public $attributes        = array("selected_Server");


  /*! \brief   Creates a dak_keyring class.
      @param   The GOsa configuration object. 
   */
  public function __construct($config)
  {
    plugin::plugin($config,NULL);

    /* Collect release servers and store them
     */
    $this->Servers = DAK::get_repositories_by_server($this->config);
    if(count($this->Servers)){
      $this->selected_Server  = key($this->Servers);
    }
  }


  /*! \brief  Create HTML output for this plugin.
      @return String  HTML output.
   */
  public function execute()
  {
    plugin::execute();
    
    if(isset($_POST['search'])){
      $this->refresh_list();
    }

    $smarty= get_smarty();
    $smarty->assign("Servers"         , $this->Servers);
    $smarty->assign("selected_Server" , $this->selected_Server);
    $smarty->assign("list",$this->CreateList());
    return($smarty->fetch (get_template_path('dak_keyring.tpl', TRUE, dirname(__FILE__))));
  }


  /*! \brief  Refresh the list of keys for the currently selected server 
              This function is automatically called when a key was added and removed.
   */
  private function refresh_list()
  {
    $details = array();
    foreach($this->list as $entry){
      if(isset($entry['DETAILS']) && $entry['DETAILS'] && isset($entry['ATTRIBUTES']['UID'])){
        $details[] = $entry['ATTRIBUTES']['UID'];
      }
    }
    
    $tmp = DAK::list_keys($this->Servers[$this->selected_Server]['MAC']);
    $this->list = array();
    foreach($tmp as $entry){
      if(isset($entry['ATTRIBUTES']['UID']) && in_array($entry['ATTRIBUTES']['UID'],$details)){
        $entry['DETAILS'] = TRUE;
      }
      $this->list[] = $entry;
    }
  } 


  /*! \brief  Creates the HTML output representing the keylist 
              for this currently selected server 
      @return String HTML list containig all keys for the selected server.
   */
  private function CreateList()
  {
    $divlist = new divlist("DAK_keyring");
    $divlist->SetWidth("100%");
    $divlist->SetHeight("450px");
    $divlist->SetEntriesPerPage(0);

    /* Set default col styles 
     */
    $s0 = "style='width:10px;'";
    $s1 = "style='width:70px;'";
    $s2 = "style='width:50px;'";
    $s3 = "style='width:130px;'";
    $s4 = "";
    $s5 = "style='text-align: right;width:50px; border-right:0px;'";

    /* Add header 
     */
    $h0 = array("string" => "",            "attach" => $s0);
    $h1 = array("string" => _("Key ID"),   "attach" => $s1);
    $h2 = array("string" => _("Length"),   "attach" => $s2);
    $h3 = array("string" => _("Status"),   "attach" => $s3);
    $h4 = array("string" => _("UID"),      "attach" => $s4);
    $h5 = array("string" => _("Action"),   "attach" => $s5);
    $divlist->SetHeader(array($h0,$h1,$h2,$h3,$h4,$h5));

    /* Add entries 
     */
    foreach($this->list as $key => $entry){

      /* Check if all attributes are given
       */
      if(!is_array($entry['UID'])){
        $entry['UID'] = array($entry['UID']);
      }
      $key_id = $length = $created = $expired = $expires = "";

      foreach(array("key_id"=>"UID","length"=>"LENGTH","created"=>"CREATED","expired"=>"EXPIRED","expires"=>"EXPIRES") as $var => $name){
        if(isset($entry['ATTRIBUTES'][$name])){
          $$var = $entry['ATTRIBUTES'][$name];
        }
      }

      $status ="-";
      $title = " title='".sprintf(_("Created: %s"),$created)."' ";
      if(empty($expires) && empty($expired)) $status = "<font $title style='color:green;'>"._("Valid")."</font>";
      if(!empty($expired))                   $status = "<font $title style='color:red; font-size:bold'>"._("Expired: ").$expired."</font>";
      if(!empty($expires))                   $status = "<font $title style='color:green;'>"._("Expires: ").$expires."</font>";

      /* Create detail icon 
       */  
      $hide = "<input type='image' name='details_".$key."' src='plugins/dak/images/forward-arrow.png' alt='-' class='center'>&nbsp;";
      $down = "<input type='image' name='details_".$key."' src='plugins/dak/images/down-arrow.png' alt='+' class='center'>&nbsp;";

      $actions = "<input type='image' class='center' alt='".msgPool::delButton()."' 
                    src='images/lists/trash.png' name='remove_key_".$key."'>";

      /* Add detailed entry 
          (All UIDs are listed, each in a single column)
       */
      if(isset($entry['DETAILS']) && $entry['DETAILS'] == TRUE){
        $first = TRUE;
        foreach($entry['UID'] as $val){

          $f0 = array("string" => "","attach" => $s0);
          $f1 = array("string" => "","attach" => $s1);
          $f2 = array("string" => "","attach" => $s2);
          $f3 = array("string" => "","attach" => $s3);
          $f4 = array("string" => htmlentities(utf8_decode($val),ENT_QUOTES,"UTF-8"), "attach" => $s4);
          $f5 = array("string" => "","attach" => $s5);

          if($first){
            $first = FALSE;
            $f0 = array("string" => $hide         ,"attach" => $s0);
            $f1 = array("string" => $key_id       ,"attach" => $s1);
            $f2 = array("string" => $length       ,"attach" => $s2);
            $f3 = array("string" => $status       ,"attach" => $s3);
            $f4 = array("string" => htmlentities(utf8_decode($val),ENT_QUOTES,"UTF-8"), "attach" => $s4);
            $f5 = array("string" => $actions      ,"attach" => $s5);
          }
          $divlist->AddEntry(array($f0,$f1,$f2,$f3,$f4,$f5));
        }
      }else{

        /* Add non detailed entries, just the first uid is displayed.
         */
        if(count($entry['UID']) > 1){
          $f0 = array("string" => $down,         "attach" => $s0);
        }else{
          $f0 = array("string" => "",         "attach" => $s0);
        }
        $f1 = array("string" => $key_id,       "attach" => $s1);
        $f2 = array("string" => $length,       "attach" => $s2);
        $f3 = array("string" => $status,        "attach" => $s3);
        $f4 = array("string" => htmlentities(utf8_decode($entry['UID'][0]),ENT_QUOTES,"UTF-8"), "attach" => $s4);
        $f5 = array("string" => $actions,"attach" => $s5);
        $divlist->AddEntry(array($f0,$f1,$f2,$f3,$f4,$f5));

      }
    }
    return($divlist->DrawList());
  }


  /*! \brief  Act on the HTML posts for this plugin 
   */
  public function save_object()
  {
    plugin::save_object();

    /* Save html posts, like the selected repository server 
     */
    foreach($this->attributes as $attr){
      if(isset($_POST[$attr])){
        $this->$attr = get_post($attr);
      }
      }
  
    /* Display details icon was clicked
     */
    foreach($_POST as $name => $value){
      if(preg_match("/^remove_key_/",$name)){
        $id = preg_replace("/^remove_key_([0-9]*)_.*$/","\\1",$name);
        $this->remove_key($id);
        $this->refresh_list();
        break;
      }

      if(preg_match("/^details_/",$name)){
        $id = preg_replace("/^details_([0-9]*)_.*$/","\\1",$name);
        if(isset($this->list[$id])){
          if(!isset($this->list[$id]['DETAILS']) || !$this->list[$id]['DETAILS']){
            $this->list[$id]['DETAILS'] = TRUE;
          }else{
            $this->list[$id]['DETAILS'] = FALSE;
          }
          break;
        }
      }
    }

    /* Import new keys 
     */
    if(isset($_POST['import_key']) && isset($_FILES['import'])){
      if($_FILES['import']['size'] != 0){
        $key = file_get_contents($_FILES['import']['tmp_name']);
        if($this->import_key($key)){
          $this->refresh_list();
        }
      }
    }
  }

   
  /*! \brief   Removes the given key from the keyring server.
      @return  Boolean TRUE in case of success else FALSE.
   */ 
  private function remove_key($key)
  {
    if($this->list[$key]){
      $mac = $this->Servers[$this->selected_Server]['MAC'];
      return(DAK::remove_key($mac,$this->list[$key]['ATTRIBUTES']['UID']));
    }
  }
  

  /*! \brief  Imports a new key into the keyring for the currently selected server 
      @return  Boolean TRUE in case of success else FALSE.
   */
  private function import_key($key)
  {
    $mac = $this->Servers[$this->selected_Server]['MAC'];
    return(DAK::import_key($mac,$key));
  }


  /*! \brief  ACL plugin information  
   */
  static function plInfo()
  {
    return (array(
          "plShortName"   => _("DAK keyring")."&nbsp;("._("Addon").")",
          "plDescription" => _("DAK keyring management")."&nbsp;("._("Addon").")",
          "plSelfModify"  => FALSE,
          "plDepends"     => array(),
          "plPriority"    => 98,
          "plSection"       => array("addon"),
          "plCategory"      => array("server"),
          "plProvidedAcls"  => 
              array(
                   ) 
          ));
  }
}
// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
