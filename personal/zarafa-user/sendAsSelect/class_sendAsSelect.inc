<?php
/*
 * This code is part of GOsa (http://www.gosa-project.org)
 * Copyright (C) 2003-2008 GONICUS GmbH
 *
 * ID: $$Id: class_groupManagement.inc 14766 2009-11-05 14:30:35Z hickert $$
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

class sendAsSelect extends management
{

  protected $skipFooter = TRUE;
  protected $skipHeader = TRUE;

  var $plHeadline = "Mail address selection";

  function __construct($config,$ui)
  {
    $this->config = $config;
    $this->ui = $ui;

    $this->storagePoints = array(get_ou("core", "userRDN"), get_ou("core", "groupRDN"));

    // Build filter
    if (session::global_is_set(get_class($this)."_filter")){
      $filter= session::global_get(get_class($this)."_filter");
    } else {
      $filter = new filter(get_template_path("sendAs-filter.xml", true, dirname(__FILE__)));
      $filter->setObjectStorage($this->storagePoints);
    }
    $this->setFilter($filter);

    // Build headpage
    $headpage = new listing(get_template_path("sendAs-list.xml", true, dirname(__FILE__)));
    $headpage->setFilter($filter);
    $headpage->registerElementFilter("objectName", "sendAsSelect::objectNameFilter");
    parent::__construct($config, $ui, "mail", $headpage);
  }

    static function objectNameFilter($id,$dn,$cn,$sn=NULL,$givenName=NULL)
    {
        if($sn){
            return("{$sn[0]}, {$givenName[0]}");
        }else{
            return("{$cn[0]}");
        }
    }

  function save()
  {
      $act = $this->detectPostActions();
      $headpage = $this->getHeadpage();
      if(!isset($act['targets'])) return(array());
      $ret = array();
      foreach($act['targets'] as $dn)
      {
          $ret[] = $headpage->getEntry($dn);
      }
      return($ret);
  }
}

?>
