<?php
/*
   This code is part of GOsa (https://gosa.gonicus.de)
   Copyright (C) 2008  Fabian Hickert

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */



/*! \brief  This is the opsi base class, it handles 
  .          gosa daemon requests and prepares data for opsi plugins.
 */
class opsi 
{


  /*! \brief 		Returns a list of netboot products.
    @param
    @return 		
   */
  public static function opsi_get_netboot_products()
  {
    /*  <xml>
        <header>gosa_opsi_get_netboot_products</header> 
        <source>GOSA</source> 
        <target>00:01:6c:9d:b9:fa</target>
        </xml>	
        <xml> 
        <header>gosa_opsi_get_netboot_products</header> 
        <source>GOSA</source> 
        <target>GOSA</target> 
        <hostId>limux-cl-2.intranet.gonicus.de</hostId>
        </xml> */

  }


  /*! \brief 		Returns a list of all local products.
    @param
    @return 		
   */
  public static function gosa_opsi_get_local_products()
  {
    /*  <xml> 
        <header>gosa_opsi_get_local_products</header> 
        <source>GOSA</source> 
        <target>GOSA</target> 
        </xml>
        <xml> 
        <header>gosa_opsi_get_local_products</header> 
        <source>GOSA</source> 
        <target>GOSA</target> 
        <hostId>limux-cl-2.intranet.gonicus.de</hostId>
        </xml> */

  }


  /*! \brief 		Returns a list of all product properties. \ 
    .           Additionally you can specify the host parameter to \
    .           get host specific product properties
    @param
    @return 		
   */
  public static function gosa_opsi_get_product_properties()
  {
    /*  <xml> 
        <header>gosa_opsi_get_product_properties</header> 
        <source>GOSA</source> 
        <target>GOSA</target> 
        <productId>firefox</productId>
        </xml>
        <xml> 
        <header>gosa_opsi_get_product_properties</header> 
        <source>GOSA</source> 
        <target>GOSA</target> 
        <productId>firefox</productId> 
        <hostId>limux-cl-2.intranet.gonicus.de</hostId> 
        </xml>
     */
  }


  /*! \brief 		Set product properties, globally or per host. 
    @param
    @return 		
   */
  public static function gosa_opsi_set_product_properties()
  {
    /* 	<xml> 
        <header>gosa_opsi_set_product_properties</header> 
        <source>GOSA</source> 
        <target>00:01:6c:9d:b9:fa</target> 
        <productId>firefox</productId> 
        <item>
        <name>askbeforeinst</name>
        <value>false</value>
        </item>
        </xml>
        <xml> 
        <header>gosa_opsi_set_product_properties</header> 
        <source>GOSA</source> 
        <target>00:01:6c:9d:b9:fa</target> 
        <hostId>limux-cl-2.intranet.gonicus.de</hostId> 
        <productId>firefox</productId> 
        <item> 
        <name>askbeforeinst</name> 
        <value>false</value> 
        </item> 
        </xml> 
     */
  }


  /*! \brief 		Returns the clients hardware setup.
    @param
    @return 		
   */
  public static function gosa_opsi_get_client_hardware()
  {
    /*	<xml> 
        <header>gosa_opsi_get_client_hardware</header> 
        <source>GOSA</source> 
        <target>GOSA</target> 
        <hostId>limux-cl-2.intranet.gonicus.de</hostId> 
        </xml> 
     */
  }


  /*! \brief 		Returns the clients software setup.
    @param
    @return 		
   */
  public static function gosa_opsi_get_client_software()
  {
    /*  <xml> 
        <header>gosa_opsi_get_client_software</header> 
        <source>GOSA</source> 
        <target>GOSA</target> 
        <hostId>limux-cl-2.intranet.gonicus.de</hostId> 
        </xml> i
     */
  }


  /*! \brief 		Returns a list of all opsi clients.
    @param
    @return 		
   */
  public static function gosa_opsi_list_clients()
  {
    /*  <xml> 
        <header>gosa_opsi_list_clients</header> 
        <source>GOSA</source> 
        <target>00:01:6c:9d:b9:fa</target> 
        </xml>
     */
  }


  /*! \brief 		Deletes the given opsi client.
    @param
    @return 		
   */
  public static function gosa_opsi_del_client()
  {
    /*  <xml> 
        <header>gosa_opsi_del_client</header> 
        <source>GOSA</source> 
        <target>00:01:6c:9d:b9:fa</target> 
        <hostId>limux-cl-2.intranet.gonicus.de</hostId>
        </xml>
     */
  }


  /*! \brief 		Triggers install/reinstall of an opsi client.
    @param
    @return 		
   */
  public static function job_opsi_install_client()
  {
    /*  <xml> 
        <header>job_opsi_install_client</header> 
        <source>GOSA</source> 
        <target>00:01:6c:9d:b9:fa</target> 
        <hostId>limux-cl-2.intranet.gonicus.de</hostId> 
        <macaddress>00:11:25:4b:8c:e5</macaddress> 
        </xml>
     */
  }


  /*! \brief 		Adds a new opsi client.
    @param
    @return 		
   */
  public static function gosa_opsi_add_client()
  {
    /*  <xml> 
        <header>gosa_opsi_add_client</header> 
        <source>GOSA</source> 
        <target>00:01:6c:9d:b9:fa</target> 
        <hostId>limux-cl-2.intranet.gonicus.de</hostId> 
        <macaddress>00:11:25:4b:8c:e5</macaddress> 
        <description>Test halt</description> 
        <ip>1.2.3.4</ip> 
        <notes>Im a note</notes> 
        </xml>
     */
  }


  /*! \brief 		Adds a given product to a client.
    @param
    @return 		
   */
  public static function gosa_opsi_add_product_to_client()
  {
    /*  <xml> 
        <header>gosa_opsi_add_product_to_client</header> 
        <source>GOSA</source> 
        <target>00:01:6c:9d:b9:fa</target> 
        <macaddress>00:11:25:4b:8c:e5</macaddress> 
        <hostId>limux-cl-2.intranet.gonicus.de</hostId> 
        <productId>winxppro</productId> 
        </xml>
     */
  }


  /*! \brief      Removes a given product from a client.
    @param
    @return
   */
  public static function gosa_opsi_del_product_from_client()
  {
    /* <xml> 
       <header>gosa_opsi_del_product_from_client</header> 
       <source>GOSA</source> 
       <target>00:01:6c:9d:b9:fa</target> 
       <hostId>limux-cl-1.intranet.gonicus.de</hostId> 
       <macaddress>00:11:25:4b:8c:e5</macaddress> 
       <productId>softprod</productId>  
       </xml>
     */
  }
}

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
