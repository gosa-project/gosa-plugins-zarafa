<?php
/*
 * This code is part of GOsa (http://www.gosa-project.org)
 * Copyright (C) 2003-2008 GONICUS GmbH
 *
 * ID: $$Id$$
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

class EventTargetAddList extends MultiSelectWindow
{
  public $display_server = TRUE;
  public $display_workstation = TRUE;
  public $display_ogroup = TRUE;
  public $filter_iprange = FALSE;
  public $filter_mac_addr = FALSE;
  public $filter_fai_release = FALSE;
  public $filter_fai_class = FALSE;

  public $regex  = "*";
  public $ipfrom = "0.0.0.0";
  public $ipto   = "*";
  public $mac_addr = "*:*:*:*:*:*";
  public $fai_release = "*";
  public $fai_class = "*";
  public $_target_list = array();

  public $workstation_list = array();
  public $server_list = array();

  public $array_TextCheckboxes = array();

  public $SubSearch = FALSE;
  /* Define that Draw has to return a smarty object */
  public $DrawReturnsSmartyObject = TRUE;

  function __construct(&$config,$parent)
  {
    MultiSelectWindow::MultiSelectWindow($config, "EventTargetAddList", 
        array("server",
          "workstation",
          "ogroups"));

    $this->parent       = $parent;
    $this->ui           = get_userinfo();


    $this->target_divlist = new MultiSelectWindow($this->config,"EventAddTargedtList","gotomasses");
    $this->SetSummary(_("Targets"));
    $this->EnableCloseButton(FALSE);
    $this->EnableSaveButton(FALSE);

    $this->SetInformation(_("This dialog shows all available targets for the event. Check the targets you want to add and use the 'Apply' button."));

    /* Toggle all selected / deselected */
    $chk = "<input type='checkbox' id='select_all' name='select_all'
      onClick='toggle_all_(\"^item_selected_[0-9]*$\",\"select_all\");' >";

    $this->EnableAplhabet(TRUE);

    /* set Page header */
    $this->AddHeader(array("string"=> $chk,          "attach"=>"style='width:20px;'"));
    $this->AddHeader(array("string"=>"&nbsp;","attach"=>"style='width:20px;'"));
    $this->AddHeader(array("string"=>_("System / Department")));

    //$name,$string,$value,$conn,$image="images/lists/search.png")
    $this->AddRegex("regex" ,"regex"  ,"*"                , TRUE);

    $this->AddCheckBox("display_server","1"     ,_("Display server"),TRUE);
    $this->AddCheckBox("display_workstation","1",_("Display workstation"),TRUE);
    $this->AddCheckBox("display_ogroup","1"     ,_("Display object groups"),TRUE);
    
    /* Add SubSearch checkbox */
    $this->AddCheckBox(SEPERATOR);
    $this->AddCheckBox("SubSearch",  msgPool::selectToView("","subsearch"), msgPool::selectToView("","subsearch_small"), false);

    $regexes = array();
    $regexes[] = $this->AddRegex("ipfrom","ipfrom" ,"0.0.0.0"          , FALSE, "images/lists/search.png", TRUE);
    $regexes[] = $this->AddRegex("ipto"  ,"ipto"   ,"255.255.255.255"  , FALSE, "images/lists/search.png", TRUE);
    $this->AddTextCheckBox("filter_iprange","1"     ,_("Filter by IP range"),FALSE, $regexes);

    $regexes = array();
    $regexes[] = $this->AddRegex("mac_addr", "mac_addr", "*:*:*:*:*:*", FALSE, "images/lists/search.png", TRUE);
    $this->AddTextCheckBox("filter_mac_addr", "1", _("Filter by MAC address"), FALSE, $regexes);

    $regexes = array();
    $regexes[] = $this->AddRegex("fai_release", "fai_release", "*", FALSE, "images/lists/search.png", TRUE);
    $this->AddTextCheckBox("filter_fai_release", "1", _("Filter by FAI release"), FALSE, $regexes);

    $regexes = array();
    $regexes[] = $this->AddRegex("fai_class", "fai_class", "*", FALSE, "images/lists/search.png", TRUE);
    $this->AddTextCheckBox("filter_fai_class", "1", _("Filter by FAI class"), FALSE, $regexes);

    /* Create a list of servers
     */
    $tmp = get_sub_list("(&(macAddress=*)(objectClass=goServer))",
          "server",get_ou("serverRDN"),$config->current['BASE'],
          array("cn","objectClass","description","ipHostNumber","macAddress"),GL_SUBSEARCH);
    foreach($tmp as $server){
      $this->server_list[$server['dn']] = $server;
    }

    /* Create a list of workstations
     */
    $tmp = get_sub_list("(&(macAddress=*)(objectClass=gotoWorkstation))",
          "server",get_ou("workstationRDN"),$config->current['BASE'],
          array("cn","objectClass","description","ipHostNumber","macAddress"),GL_SUBSEARCH);
    foreach($tmp as $server){
      $this->workstation_list[$server['dn']] = $server;
    }

  }


  function execute()
  {
    $this->ClearElementsList();
    $this->AddDepartments($this->selectedBase,2,1);
    $this->setEntries();
    $this->GenHeader();
  }


  function GenHeader()
  {
    $modules = array("server","workstation");

    /* Add base */
    $tmp = array();
    $base = $this->config->current['BASE'];
    $tmp[] = array("dn"=>$this->config->current['BASE']);
    $tmp=  array_merge($tmp,get_list("(&(|(ou=*)(description=*))(objectClass=gosaDepartment))", $modules, $base,
          array("ou", "description"), GL_SIZELIMIT | GL_SUBSEARCH));

    $deps = array();
    foreach($tmp as $tm){
      $deps[$tm['dn']] = $tm['dn'];
    }

    $department = $departments = array();
    $ui= get_userinfo();
    $d = $ui->get_module_departments($modules);
    foreach($d as $department){
      $departments[$department] = $department;
    }

    /* Load possible departments */
    $ids = $this->config->idepartments;
    $first = "";
    $found = FALSE;
    $options = array();
    foreach($ids as $dep => $name){
      if(isset($deps[$dep]) && in_array_ics($dep, $departments)){

        /* Keep first base dn in mind, we could need this
         *  info if no valid base was found
         */
        if(empty($first)) {
          $first = $dep['dn'];
        }

        $value = $ids[$dep];
        if ($this->selectedBase == $dep){
          $found = TRUE;
          $options.= "<option selected='selected' value='".$dep."'>$value</option>";
        } else {
          $options.= "<option value='".$dep."'>$value</option>";
        }
      }
    }

    $listhead = $this->get_default_header();

    /* Add base selection */
    $listhead .= _("Base")."&nbsp; <select name='CurrentMainBase' onChange='mainform.submit()' class='center'>$options</select>".
      " <input class='center' type='image' src='images/lists/submit.png' align='middle'
      title='"._("Submit department")."' name='submit_department' alt='". _("Submit")."'>&nbsp;";

    $this->SetListHeader($listhead);

  }


  function get_selected_targets()
  {
    $a_targets = array();
    foreach($this->list_get_selected_items() as $id){
      if(in_array("gosaGroupOfNames",$this->_target_list[$id]['objectClass'])){
        foreach($this->_target_list[$id]['member'] as $mem_dn){
          if(isset($this->workstation_list[$mem_dn])){
            $a_targets[] = $this->workstation_list[$mem_dn]['macAddress'][0];
          }
          if(isset($this->server_list[$mem_dn])){
            $a_targets[] = $this->server_list[$mem_dn]['macAddress'][0];
          }
        }
      }else{
        if(isset($this->_target_list[$id]['macAddress'][0])){
          $a_targets[] = $this->_target_list[$id]['macAddress'][0];
        }
      }
    }
    return($a_targets);
  }


  function setEntries()
  {
    $_target_list = array();
    if($this->display_server){
      if (!$this->SubSearch) {
        $_target_list = array_merge($_target_list,
              get_list("(&(cn=".$this->regex.")(objectClass=goServer))",
              "server",get_ou("serverRDN").$this->selectedBase,
              array("cn","objectClass","description","ipHostNumber","macAddress", "FAIclass"),GL_NONE));
      }else {
        $_target_list = array_merge($_target_list,
              get_sub_list("(&(cn=".$this->regex.")(objectClass=goServer))",
              "server",get_ou("serverRDN"),$this->selectedBase,
              array("cn","objectClass","description","ipHostNumber","macAddress", "FAIclass"),GL_SUBSEARCH));
      }
    }
    if($this->display_workstation){
      if (!$this->SubSearch) {
        $_target_list = array_merge($_target_list,
              get_list("(&(cn=".$this->regex.")(objectClass=gotoWorkstation))",
              "workstation",get_ou("workstationRDN").$this->selectedBase,
              array("cn","objectClass","description","ipHostNumber","macAddress", "FAIclass"),GL_NONE));
      }else {
        $_target_list = array_merge($_target_list,
              get_sub_list("(&(cn=".$this->regex.")(objectClass=gotoWorkstation))",
              "workstation",get_ou("workstationRDN"), $this->selectedBase,
              array("cn","objectClass","description","ipHostNumber","macAddress", "FAIclass"),GL_SUBSEARCH));
      }
    }
    if($this->display_ogroup){
      if (!$this->SubSearch) {
        $_target_list = array_merge($_target_list,
              get_list("(&(cn=".$this->regex.")(member=*)(objectClass=gosaGroupOfNames))",
              "ogroups",get_ou("ogroupRDN").$this->selectedBase,
              array("cn","objectClass","description","member", "FAIclass"),GL_NONE));
      }else {
        $_target_list = array_merge($_target_list,
              get_sub_list("(&(cn=".$this->regex.")(member=*)(objectClass=gosaGroupOfNames))",
              "ogroups",get_ou("ogroupRDN"), $this->selectedBase,
              array("cn","objectClass","description","member", "FAIclass"),GL_SUBSEARCH));
      }
    }
    $this->_target_list = $_target_list;

    $tmp = array();
    foreach($this->_target_list as $key => $object){
      $tmp[$key] = $object['cn'][0];
    }
    natcasesort($tmp);

    foreach($tmp as $key => $obj){

      $obj = $this->_target_list[$key];
      $name = $obj['cn'][0];
      if(isset($obj['description'])){
        $name .= "&nbsp;[".$obj['description'][0]."]";
      }
      if(isset($obj['macAddress'])){
        $name .= "&nbsp;- ".$obj['macAddress'][0]."";
      }
      if(isset($obj['ipHostNumber'])){
        $name .= "&nbsp;- ".$obj['ipHostNumber'][0]."";
      }

      $img ="";
      if(in_array("goServer",$obj['objectClass'])){
        $img = '<img class="center" src="plugins/systems/images/select_server.png" alt="S" title="'._("Server").'">';

        if($this->filter_iprange){
          if(!isset($obj['ipHostNumber']) || !tests::is_in_ip_range($this->ipfrom,$this->ipto,             $obj['ipHostNumber'][0])) {
            continue;
          }
        }

        if($this->filter_mac_addr) {
          if(!isset($obj['macAddress']) || !ldap_equality_check($obj['macAddress'][0], $this->mac_addr, "[0-9A-F]{2}", FALSE)) {
            continue;
          }
        }

        if($this->filter_fai_release) {
          if(isset($obj['FAIclass'])) {
            $fai_release = split(':', $obj['FAIclass'][0]);
            if (isset($fai_release[1])) {
              $fai_release = $fai_release[1];
            } else {
              continue;
            }

            if (!ldap_equality_check($fai_release, $this->fai_release)) {
              continue;
            }
          }
          else {
            continue;
          }
        }

        if ($this->filter_fai_class) {
          if(isset($obj['FAIclass'])) {
            $fai_classes = split(':', $obj['FAIclass'][0]);
            $fai_classes = split(' ', $fai_classes[0]);

            $found = FALSE;
            foreach($fai_classes as $used_class) {
              if(ldap_equality_check($used_class, $this->fai_class)){
                $found = TRUE;
                break;
              }
            }
            if (!$found) {
              continue;
            }
          }
          else {
            continue;
          }
        }

        if(!isset($this->server_list[$obj['dn']])){
          continue;
        }
      }elseif(in_array("gotoWorkstation",$obj['objectClass'])){
        $img = '<img class="center" src="plugins/systems/images/select_workstation.png" alt="W" title="'._("Workstation").'">';
        if($this->filter_iprange){
          if(!isset($obj['ipHostNumber']) || !tests::is_in_ip_range($this->ipfrom,$this->ipto,$obj['ipHostNumber'][0])) {
            continue;
          }
        }

        if($this->filter_mac_addr){
          if(!isset($obj['macAddress']) || !ldap_equality_check($obj['macAddress'][0], $this->mac_addr, "[0-9A-F]{2}", FALSE)) {
            continue;
          }
        }

        if($this->filter_fai_release) {
          if (isset($obj['FAIclass'])) {
            $fai_release = split(':', $obj['FAIclass'][0]);
            if (isset($fai_release[1])) {
              $fai_release = $fai_release[1];
            } else {
              continue;
            }
            if (!ldap_equality_check($fai_release, $this->fai_release)) {
              continue;
            }
          }
          else {
            continue;
          }
        }

        if ($this->filter_fai_class) {
          if(isset($obj['FAIclass'])) {
            $fai_classes = split(':', $obj['FAIclass'][0]);
            $fai_classes = split(' ', $fai_classes[0]);

            $found = FALSE;
            foreach($fai_classes as $used_class) {
              if(ldap_equality_check($used_class, $this->fai_class)){
                $found = TRUE;
                break;
              }
            }
            if (!$found) {
              continue;
            }
          }
          else {
            continue;
          }
        }

        if(!isset($this->workstation_list[$obj['dn']])){
          continue;
        }
      }elseif(in_array("gosaGroupOfNames",$obj['objectClass'])){
        /* Do not show object groups if ip or mac filter is enabled */
        if($this->filter_iprange || $this->filter_mac_addr) {
          continue;
        }

        if($this->filter_fai_release) {
          if (isset($obj['FAIclass'])) {
            $fai_release = split(':', $obj['FAIclass'][0]);
            if (isset($fai_release[1])) {
              $fai_release = $fai_release[1];
            } else {
              continue;
            }
            if (!ldap_equality_check($fai_release, $this->fai_release)) {
              continue;
            }
          }
          else {
            continue;
          }
        }

        if ($this->filter_fai_class) {
          if(isset($obj['FAIclass'])) {
            $fai_classes = split(':', $obj['FAIclass'][0]);
            $fai_classes = split(' ', $fai_classes[0]);

            $found = FALSE;
            foreach($fai_classes as $used_class) {
              if(ldap_equality_check($used_class, $this->fai_class)){
                $found = TRUE;
                break;
              }
            }
            if (!$found) {
              continue;
            }
          }
          else {
            continue;
          }
        }

        $img = '<img class="center" src="plugins/ogroups/images/select_ogroup.png" alt="O" title="'._("Object group").'">';
      }

      $field1 = array("string" => "<input type='checkbox' id='item_selected_".$key."' name='item_selected_".$key."'>",
                      "attach" => "style='width:20px;'");
      $field2 = array("string" => $img,
                      "attach" => "style='width:20px;'");
      $field3 = array("string" => $name , "attach" => "title='".$obj['dn']."'");
      $this->AddElement(array($field1,$field2,$field3));
    }
  }

  function AddTextCheckbox($name,$value="Unset", $string="Unset", $default=false, $regexes=array())
  {
    $arr = array();
    $MultiDialogFilters = session::get('MultiDialogFilters');
    if(isset($MultiDialogFilters[$this->filterName][$name])){
      $arr['default'] = $MultiDialogFilters[$this->filterName][$name];
      $this->$name = $arr['default'];
    }else {
      $arr['default'] = $default;
      $this->$name = $default;
    }
    $arr['name'] = $name;
    $arr['string'] = $string;
    $arr['value'] = $value;
    $arr['enabled'] = true;
    $arr['regexes'] = $regexes;

    $this->array_TextCheckboxes[] = $arr;

  }

  function Draw() {
    $smarty = MultiSelectWindow::Draw();
    $special_filter = "";
    foreach($this->array_TextCheckboxes as $textcheckbox) {
      $special_filter .= "<table style='width:100%;border-top:1px solid #B0B0B0;'><tr><td>";
      $special_filter .= $this->GetCheckbox($textcheckbox);
      foreach($textcheckbox['regexes'] as $regex) {
        $special_filter .= $this->GetRegex($regex, FALSE);
      }
      $special_filter .= "</table>";
    }
    $smarty->assign("special_filters"   , $special_filter);
    $display = $smarty->fetch(get_template_path('EventTargetAddList.tpl', TRUE, dirname(__FILE__)));
    return($display);
  }

  /*! \brief  Returns a set of elements selected in a MultiSelectWindow
    @return Array[integer]=integer
   */
  protected  function list_get_selected_items()
  {
    $ids = array();
    foreach($_POST as $name => $value){
      if(preg_match("/^item_selected_[0-9]*$/",$name)){
        $id   = preg_replace("/^item_selected_/","",$name);
        $ids[$id] = $id;
      }
    }
    return($ids);
  }

  function save_object()
  {
    MultiSelectWindow::save_object();
    if(isset($_POST['MultiSelectWindow'.$this->filterName])){
      foreach($this->array_TextCheckboxes as $key => $box) {
          $MultiDialogFilters = session::get('MultiDialogFilters');
          if(isset($_POST[$box['name']])){
              $this->array_TextCheckboxes[$key]['default'] = true;
              $this->$box['name'] = true;
          }else{
              $this->array_TextCheckboxes[$key]['default'] = false;
              $this->$box['name'] = false;
          }
          $MultiDialogFilters[$this->filterName][$box['name']] = $this->$box['name'];

      foreach($this->array_TextCheckboxes[$key]['regexes'] as $regex_key => $regex_box) {
          $this->array_TextCheckboxes[$key]['regexes'][$regex_key]['value'] = $_POST[$regex_box['name']];
          $this->$regex_box['name'] = $_POST[$regex_box['name']];
          $MultiDialogFilters[$this->filterName][$regex_box['name']] = $this->$regex_box['name'];
      }
      session::set('MultiDialogFilters',$MultiDialogFilters);
      }
    }
  }
}
// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
