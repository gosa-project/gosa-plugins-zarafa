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


/********

  __construct($config)
  get_hosts_for_system_management()
  get_netboot_products($host = "")
  get_local_products($host = "")
  get_product_properties()
  set_product_properties()
  get_client_hardware()
  get_client_software()
  list_clients()
  del_client()
  job_opsi_install_client()
  add_client()
  add_product_to_client()
  del_product_from_client()

 ********/


/*! \brief  This is the opsi base class, it handles 
  .          gosa daemon requests and prepares data for opsi plugins.
 */
class opsi extends gosaSupportDaemon 
{
  private $config = NULL;
  protected $use_alternative_xml_parse_method = TRUE;

  /*! \brief 		Create opsi object.
    @param
    @return 		
   */
  public function __construct($config)
  {
    $this->config = $config;
    gosaSupportDaemon::__construct($config);
    $this->target = "00:01:6c:9d:b9:fa";
  }


  /******************
    Opsi handling 
   ******************/

  function get_hosts_for_system_management()
  {
    $res = $this->list_clients();

    $data = array();
    foreach($res as $entry){
      if(!isset($entry['MAC'][0]['VALUE'])) $entry['MAC'][0]['VALUE'] = "";
      $data[] = array(
        "dn"          => "opsi:=".$entry['NAME'][0]['VALUE'].",".get_ou("winstations").$this->config->current['BASE'],
        "objectClass" => array("gosa_opsi_client"),
        "cn"          => array(0 => $entry['NAME'][0]['VALUE']),
        "description" => array(0 => $entry['DESCRIPTION'][0]['VALUE']),
        "macAddress"  => array(0 => $entry['MAC'][0]['VALUE']),
        "opsi_notes"  => array(0 => $entry['NOTES'][0]['VALUE']));
    }
    return($data);
  }


  /*! \brief  Maps all xml to array conversion to an alternative method
                then used in the parent class 'gosaSupportDaemon'.
              The alternative method is able to handle more complex data.
   */
  private function xml_to_array($xml,$alternative_method = FALSE)
  {
    return(gosaSupportDaemon::xml_to_array($xml,TRUE));
  }


  /******************
    SI Communication functions
   ******************/


  /*! \brief 		Returns a list of netboot products.
    @param
    @return 		
   */
  public function get_netboot_products($host = "")
  {
    /* Append host attribute to query data 
     */
    $data = array();
    if(!empty($host)){
      $data['hostId'] = trim($host);
    }

    $res    = $this->send_data("gosa_opsi_get_netboot_products",$this->target,$data,TRUE);
    $items = array();
    if(isset($res['XML'][0]['ITEM'])){
      foreach($res['XML'][0]['ITEM'] as $entry){
        $e = array("DESC" => $entry['DESCRIPTION'][0]['VALUE'],
                   "NAME" => $entry['PRODUCTID'][0]['VALUE']);
        $items[$entry['PRODUCTID'][0]['VALUE']] = $e;
      }
    }
    return($items);
  }


  /*! \brief 		Returns a list of all local products.
    @param
    @return 		
   */
  public function get_local_products($host = "")
  {
    /* Append host attribute to query data 
     */
    $data = array();
    if(!empty($host)){
      $data['hostId'] = trim($host);
    }

    $res    = $this->send_data("gosa_opsi_get_local_products",$this->target,$data,TRUE);
    $items = array();
    if(isset($res['XML'][0]['ITEM'])){
      foreach($res['XML'][0]['ITEM'] as $entry){
        $e = array("DESC" => $entry['DESCRIPTION'][0]['VALUE'],
                   "NAME" => $entry['PRODUCTID'][0]['VALUE']);
        $items[$entry['PRODUCTID'][0]['VALUE']] = $e; 
      }
    }
    return($items);
  }


  /*! \brief 		Returns a list of all product properties. \ 
    .           Additionally you can specify the host parameter to \
    .           get host specific product properties
    @param
    @return 		
   */
  public function get_product_properties($productId,$hostId = "")
  {
    $data = array("productId" => $productId);

    /* Append host attribute to query data 
     */
    if(!empty($hostId)){
      $data['hostId'] = trim($hostId);
    }
   
    /* Check parameter */ 
    if(empty($productId)){
      trigger_error("No valid product id given, check parameter 1.");
      return(array());
    }

    /* Query SI server */
    $res    = $this->send_data("gosa_opsi_get_product_properties",$this->target,$data,TRUE);
    $items  = array();
    if(isset($res['XML'][0]['ITEM'])){   
      foreach($res['XML'][0]['ITEM'] as $entry){
        foreach($entry as $name => $val){
          $items[$name] = $val['0']['VALUE'];
        }
      }
    }
    return($items);
  }


  /*! \brief 		Set product properties, globally or per host. 
    @param
    @return 		
   */
  public function set_product_properties($productId,$cfg,$hostId = "")
  {
    $data = array("productId" => $productId);

    /* Append host attribute to query data 
     */
    if(!empty($hostId)){
      $data['hostId'] = trim($hostId);
    }
   
    /* Check parameter */ 
    if(empty($productId)){
      trigger_error("No valid product id given, check parameter 1.");
      return(array());
    }

    if(!count($cfg)) return;
    
    /* Add properties */
    $data['item'] = array();
    foreach($cfg as $name => $value){
      $data['item'][] = "<name>".$name."</name><value>".$value."</value>";
    }  

    /* Query SI server */
    $res    = $this->send_data("gosa_opsi_set_product_properties",$this->target,$data,TRUE);
  }


  /*! \brief 		Adds a given product to a client.
    @param
    @return 		
   */
  public function add_product_to_client($productId,$hostId)
  {
    $data = array("productId" => $productId,"hostId" => $hostId);

    /* Check parameter */ 
    if(empty($productId)){
      trigger_error("No valid product id given, check parameter 1.");
      return;
    }
    if(empty($hostId)){
      trigger_error("No valid host id given, check parameter 2.");
      return;
    }

    /* Query SI server */
    $res    = $this->send_data("gosa_opsi_add_product_to_client",$this->target,$data,TRUE);
  }


  /*! \brief      Removes a given product from a client.
    @param
    @return
   */
  public function del_product_from_client($productId,$hostId)
  {
    $data = array("productId" => $productId,"hostId" => $hostId);

    /* Check parameter */ 
    if(empty($productId)){
      trigger_error("No valid product id given, check parameter 1.");
      return;
    }
    if(empty($hostId)){
      trigger_error("No valid host id given, check parameter 2.");
      return;
    }

    /* Query SI server */
    $res    = $this->send_data("gosa_opsi_del_product_from_client",$this->target,$data,TRUE);
  }


  /*! \brief 		Returns the clients hardware setup.
    @param
    @return 		
   */
  public function get_client_hardware($hostId)
  {
    $data = array("hostId" => $hostId);

    /* Check parameter */ 
    if(empty($hostId)){
      trigger_error("No valid host id given, check parameter 1.");
      return;
    }

    /* Query SI server */
    $res    = $this->send_data("gosa_opsi_get_client_hardware",$this->target,$data,TRUE);
    print_a($res);
  }


  /*! \brief 		Returns the clients software setup.
    @param
    @return 		
   */
  public function get_client_software()
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
  public function list_clients()
  {
    $data   = array();
    $res    = $this->send_data("gosa_opsi_list_clients",$this->target,$data,TRUE);
    $items  = array();
    if(isset($res['XML'][0]['ITEM'])){
      $items = $res['XML'][0]['ITEM'];
    }
    return($items);
  }


  /*! \brief 		Deletes the given opsi client.
    @param
    @return 		
   */
  public function del_client()
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
  public function job_opsi_install_client()
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
  public function add_client($hostId,$ip,$macaddress,$notes,$description)
  {
    $data = array("hostId" => $hostId,"ip" => $ip,"macaddress" => $macaddress);

    if(empty($hostId)){
      trigger_error("No valid host id given, check parameter 1.");
      return;
    }
    if(empty($ip)){
      trigger_error("No valid ip address given, check parameter 2.");
      return;
    }
  
    /* Add optional attributes */ 
    foreach(array("notes","description") as $attr) {
      if(!empty($$attr)){
        $data[$attr] = $$attr;
      }
    }

    /* Query SI server */
    $res    = $this->send_data("gosa_opsi_add_client",$this->target,$data,TRUE);
  }


}
// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
