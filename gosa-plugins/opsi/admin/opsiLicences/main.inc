<?php
/*
 * This code is part of GOsa (http://www.gosa-project.org)
 * Copyright (C) 2003-2008 GONICUS GmbH
 *
 * ID: $$Id: main.inc 13288 2008-12-12 14:54:13Z hickert $$
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


// Remove locks created by this plugin
if ($remove_lock){
  if(session::is_set('opsiLicenses')){
    $macl = session::get('opsiLicenses');
    $macl->remove_lock();
  }
}

/* Remove this plugin from session
*/
if ( $cleanup ){
  $macl = session::get('opsiLicenses');
  $macl->remove_lock();
  session::un_set('opsiLicenses');
}else{

  /* Create object object on demand */
  if (!session::is_set('opsiLicenses')){
    session::set('opsiLicenses',new opsiLicenses($config));
  }
  $opsiLicenses = session::get('opsiLicenses');

  /* Execute formular */
  $opsiLicenses->save_object();
  $output= $opsiLicenses->execute ();

  /* Page header*/
  if (get_object_info() != ""){
    $display= print_header(get_template_path('plugins/rolemanagement/images/plugin.png'),
        _("Roles"),
        "<img alt=\"\" align=\"middle\" src=\"".get_template_path('images/lists/locked.png')."\"> ".
        LDAP::fix(get_object_info()));
  } else {
    $display= print_header(get_template_path('plugins/rolemanagement/images/plugin.png'), _("Role management"));
  }

  $display.= $output;

  /* Store changes  in session */
  session::set('opsiLicenses',$opsiLicenses);
}

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
