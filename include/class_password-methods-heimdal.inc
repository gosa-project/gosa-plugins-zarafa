<?php
/*
   This code is part of GOsa (https://gosa.gonicus.de)
   Copyright (C) 2007 Cajus Pollmeier

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */


class passwordMethodheimdal extends passwordMethod
{

	function passwordMethodheimdal(&$config)  
	{
    $this->config= $config;
	}


	function is_available()
	{
    #TODO
    # Check if heimdal information is available
    # in configuration and return true/false
    #
    # In particular, we need the "heimdal_keygen" to feed kadmin in order to return "true".
    #
    return false;
	}


	function generate_hash($pwd)
	{
    $mode= "kerberos";
    if (isset($this->config->current['KRBSASL']) && preg_match('/^true$/i', $this->config->current['KRBSASL'])){
      $mode= "sasl";
    }

    return "{".$mode."}".$this->attrs['uid'][0]."@".$cfg= $this->config->data['SERVERS']['KERBEROS']['REALM'];
	}


  function remove_from_parent()
  {
    #TODO
    # Find and remove kerberos entry below $this->dn
    # See https://oss.gonicus.de/labs/gosa/ticket/223
  }


  function set_password()
  {
    #TODO
    # Add or modify kerberos entry below $this->dn
    # See https://oss.gonicus.de/labs/gosa/ticket/223
    # Order: create entries, then call the heimdal_keygen hook with the realm (returned by generate_hash)
    #        to let it add the missing kerberos keys.
  }


  function get_hash_name()
  {
    $mode= "kerberos";
    if (isset($this->config->current['KRBSASL']) && preg_match('/^true$/i', $this->config->current['KRBSASL'])){
      $mode= "sasl";
    }
    return "$mode";
  }


  function is_configurable()
  {
    return TRUE;
  }


  function configure()
  {
    #TODO
    # * Handle POST's from and for this dialog
    # * Return filled template for the config sub-dialog.
    # See https://oss.gonicus.de/labs/gosa/ticket/223
  }

}

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
