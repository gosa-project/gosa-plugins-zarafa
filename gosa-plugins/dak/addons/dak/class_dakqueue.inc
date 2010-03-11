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

class dakqueue extends plugin
{
  var $attributes = array("selected_Repository");
  var $selected_Repository = "";  
 
  public function __construct($config)
  {
    plugin::plugin($config,NULL);
  }

  public function execute()
  {
    plugin::execute();

    $smarty= get_smarty();
    $smarty->assign("Repositories"          , DAK::get_repositories($this->config));
    $smarty->assign("selected_Repository" , $this->selected_Repository);
    return($smarty->fetch (get_template_path('dak_queue.tpl', TRUE, dirname(__FILE__))));
  }

  public function save_object()
  {
    plugin::save_object();

    foreach($this->attributes as $attr){
      if(isset($_POST[$attr])){
        $this->$attr = get_post($attr);
      }
    }
  }

  /*! \brief  ACL plugin information
   */
  static function plInfo()
  {
    return (array(
          "plShortName"   => _("DAK queue")."&nbsp;("._("Addon").")",
          "plDescription" => _("DAK queue management")."&nbsp;("._("Addon").")",
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
