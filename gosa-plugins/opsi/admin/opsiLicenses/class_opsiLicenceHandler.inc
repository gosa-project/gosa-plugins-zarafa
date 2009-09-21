<?php

class opsiLicenceHandler extends gosaSupportDaemon  {



  function listLicenses()
  {
    $data = "<header>opsi_getLicensePools_listOfHashes</header>".
      "<source>GOSA</source>".
      "<target>GOSA</target></header>";
    $res = $this->_send($data, TRUE);

  }


  /* @brief Create a license pool at Opsi server.
   * @param licensePoolId The name of the pool (optional).
   * @param description The description of the pool (optional).
   * @param productIds A list of assigned porducts of the pool (optional).
   * @param windowsSoftwareIds A list of windows software IDs associated to the pool (optional).
   */
  function opsi_createLicensePool()
  {

  }

  /*
   * @brief 
   *    Delete licnese pool by license pool ID. 
   *    A pool can only be deleted if there are no software licenses bound to the pool.
   *    The fixed parameter deleteLicenses=True specifies that 
   *      all software licenses bound to the pool are being deleted.
   * @param licensePoolId The name of the pool.
   */
  function opsi_deleteLicensePool()
  {

  }


  /*  
   * @brief 
   *     Create a license contract, create a software 
   *       license and add the software license to the license pool
   * @param licensePoolId The name of the pool the license should be assigned.
   * @param licenseKey The license key.
   * @param partner Name of the license partner (optional).
   * @param conclusionDate Date of conclusion of license contract (optional)
   * @param notificationDate Date of notification that license is running out soon (optional).
   * @param notes This is the place for some notes (optional)
   * @param softwareLicenseId Identificator of a license (optional).
   * @param licenseTyp Typ of a licnese, either "OEM", "VOLUME" or "RETAIL" (optional).
   * @param maxInstallations The number of clients use this license (optional).
   * @param boundToHost The name of the client the license is bound to (optional).
   * @param expirationDate The date when the license is running down (optional).
   */
  function opsi_createLicense()
  {

  }


  /*
   * @brief Assign a software license to a host
   * @param hostid Something like client_1.intranet.mydomain.de
   * @param licensePoolId The name of the pool.
   */
  function opsi_assignSoftwareLicenseToHost()
  {

  }


  /* 
   * @brief Unassign a software license from a host.
   * @param hostid Something like client_1.intranet.mydomain.de
   * @param licensePoolId The name of the pool.
   */
  function opsi_unassignSoftwareLicenseFromHost()
  {}


  /* 
   * @brief Unassign all software licenses from a host
   * @param hostid Something like client_1.intranet.mydomain.de
   */
  function opsi_unassignAllSoftwareLicensesFromHost()
  {}


  /* 
   * @brief Returns expirationDate, boundToHost, maxInstallation, licenseTyp, licensePoolIds and licenseKeys for a given softwareLicense ID.
   * @param softwareLicenseId Identificator of a license.
   */
  function opsi_getSoftwareLicense_hash()
  {}


  /* 
   * @brief Return productIds, windowsSoftwareIds and description for a given licensePoolId
   * @param licensePoolId The name of the pool.
   */
  function opsi_getLicensePool_hash()
  {}


  /* 
   * @brief Returns softwareLicenseId, notes, licenseKey, hostId and licensePoolId for optional given licensePoolId and hostId
   * @param hostid Something like client_1.intranet.mydomain.de (optional).
   * @param licensePoolId The name of the pool (optional).
   */
  function opsi_getSoftwareLicenseUsages_listOfHashes()
  {}


  /* 
   * @brief Return licensePoolId, description, productIds and windowsSoftwareIds for all found license pools.
   */
  function opsi_getLicensePools_listOfHashes()
  {}


  /* @brief 
   *   Returns the assigned licensePoolId and licenses, 
   *    how often the product is installed and at which host
   *    and the number of max and remaining installations for a given OPSI product.
   * @param productId Identificator of an OPSI product.
   */
  function opsi_getLicenseInformationForProduct()
  {}



}

// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
