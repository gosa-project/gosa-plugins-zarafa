<?php


/*! \brief  The opsi client base class.
            This class can be implemented in tow different ways:
              * as standalone opsi client
              * as part of the samba tabs 
            both types will be detected automatically.

            This class allows to edit the properties of an opsi client
             and its products.
 */
class opsiGeneric extends plugin
{
  /* Contains a list of all available netboot products 
   */
  private $a_availableNetbootProducts = array();
  private $a_initial_availableNetbootProducts = array();
  private $s_selectedNetbootProduct = "";  
  private $s_initial_selectedNetbootProduct = "";  

  /* Contains a list of all available local products
   */
  private $a_availableLocalProducts = array();
  private $a_selectedLocalProducts = array();
  private $a_initial_selectedLocalProducts = array();

  /* Internal veriables 
   */ 
  private $opsi;            // The opsi handle
  public  $parent = NULL;   // The parent object (in case of samba)

  private $hostId       = ""; // The host Id of the currently edit opsi host  
  public  $mac          = ""; // The hosts mac address
  public  $note         = ""; // A note
  public  $description  = ""; // The client description

  public  $initial_mac          = ""; 
  public  $initial_note         = ""; 
  public  $initial_description  = ""; 

  private $init_failed = FALSE; // Is true if the opsi communication failed
  private $parent_mode = TRUE;  // Is true if this is a standlone plugin. (Not samba)
  private $is_installed= FALSE; // Is true is the hast is already installed.

  public $attributes = array("mac","note","description");


  /*! \brief  Initialize this class 
      @param  Object    The GOsa base config.
      @param  String    The Id of the host that we want to edit.
      @param  Object    The parent object. (in case of samba)
   */
  public function __construct($config,$hostId,&$parent = NULL)
  {
    /* Create opsi handle
     */
    $this->opsi = new opsi($config); 
    
    /* Check if we are are part of a windows workstation 
     */
    $this->parent = $parent;
    if($parent instanceof wingeneric){
      $this->parent_mode = FALSE;
    }

    /* Get hostId 
     */
    if($hostId != "new"){
      if(preg_match("/^opsi:/",$hostId)){
        $this->hostId = preg_replace("/^opsi:=([^,]*),.*$/","\\1",$hostId);
      }elseif($this->parent instanceof wingeneric){
        $this->hostId = $this->parent->cn;
        $this->hostId = preg_replace('/\$$/',"",$this->hostId);
      }
    }
  
    /* Try to plugin */
    $this->init();
  }
  

  /*! \brief  Try to load opsi client informations from the 
               gosa support daemon.
   */
  private function init()
  {
    $err = FALSE;
    $this->init_failed = FALSE;
    $this->initially_was_account = FALSE; 

    /* Try to load client infos from the gosa support daemon
     */
    if(! ($this->hostId == "new" || !empty($this->mac)  )){
      $list = $this->opsi->list_clients($this->hostId);
      $err |= $this->opsi->is_error();

      /* Walk through all returned opsi clients and try to detect 
          one that matches our hostId.
         #FIXME Implement an opsi method which returns infos for only one opsi client, not all. 
       */
      foreach($list as $entry){
        if(preg_match("/^".normalizePreg($this->hostId)."$/i",$entry['NAME'][0]['VALUE'])){
          $this->initially_was_account = TRUE; 
          foreach(array(
                "is_installed" => "LASTSEEN",
                "description"  => "DESCRIPTION",
                "mac"          => "MAC", 
                "note"         => "NOTES") as $des => $src){
            $des2 = "initial_".$des;
            $this->$des2 = $this->$des = $entry[$src][0]['VALUE'];
          } 
          break;
        }
      }
    }

    /* Read informations about available netboot products. 
        If not already done, before.
     */
    if(!$err && !count($this->a_availableNetbootProducts)){
      $this->a_availableNetbootProducts = $this->opsi->get_netboot_products();
      ksort($this->a_availableNetbootProducts);
      $err |= $this->opsi->is_error();
    }

    /* Read informations about available netboot products. 
        If not already done, before.
     */
    if(!$err && !count($this->a_availableLocalProducts)) {
      $this->a_availableLocalProducts   = $this->opsi->get_local_products();
      ksort($this->a_availableLocalProducts);
      $err |= $this->opsi->is_error();
    }

    /* Get products selected by this host.
     */
    if(!$err && !empty($this->hostId)) {
      $tmp = array_keys($this->opsi->get_netboot_products($this->hostId));
      $err |= $this->opsi->is_error();
      if(count($tmp) && !$err && !isset($this->a_availableNetbootProducts[$this->s_selectedNetbootProduct]['CFG'])){
        $this->s_selectedNetbootProduct = $tmp[0];
      
        /* Read configuration for "Netboot Products" */
        if(isset($this->a_availableNetbootProducts[$this->s_selectedNetbootProduct])){
          $CFG = $this->opsi->get_product_properties($this->s_selectedNetbootProduct,$this->hostId);
          $this->a_availableNetbootProducts[$this->s_selectedNetbootProduct]['CFG'] = $CFG;
        }
      }
      $err |= $this->opsi->is_error();
    }

    /* Get all selected local products 
     */
    if(!$err && !empty($this->hostId) && !count($this->a_selectedLocalProducts)) {
      $tmp = $this->opsi->get_local_products($this->hostId); 
      $err |= $this->opsi->is_error();
      $this->a_selectedLocalProducts = $tmp;
    }

    /* Load product configuration for all already selected products.
     */
    if(!$err && !empty($this->hostId)) {
      foreach($this->a_selectedLocalProducts as $name => $data){
        if(!$err && !isset($this->a_selectedLocalProducts[$name]['CFG'])){
          $CFG = $this->opsi->get_product_properties($name,$this->hostId);
          $err |= $this->opsi->is_error();
          $this->a_selectedLocalProducts[$name]['CFG'] = $CFG;
        }
      }
    }
  
    /* Check if everything went fine else reset everything and display a retry button 
     */
    if($err){
      $this->init_failed = TRUE;
      
    }else{

      /* Remember initial settings */ 
      $this->is_account = $this->initially_was_account = TRUE;
      $this->a_initial_selectedLocalProducts = $this->a_selectedLocalProducts;
      $this->s_initial_selectedNetbootProduct = $this->s_selectedNetbootProduct;
      $this->a_initial_availableNetbootProducts = $this->a_availableNetbootProducts;

      /* Ensure that a valid netboot is selected product is.
       */
      if(empty($this->s_selectedNetbootProduct)){
        $this->s_selectedNetbootProduct = key($this->a_availableNetbootProducts);
      }
    }
  }


  /*! \brief  Check given data.
      @return Array   Returns an array with all issues.
   */
  public function check()
  {
    return(array());
    $messages = plugin::check();

    if(empty($this->hostId)){
      $messages[] = msgPool::required(_("Name"));
    }elseif(!preg_match("/\./",$this->hostId)){

      /* The hostId must contain a domain part 
       */
      $messages[] = msgPool::invalid(_("Name"),$this->hostId,"",
          _("The client name must contain a domain part (e.g. '.company.de')."));
    }elseif(preg_match("/[^a-z0-9\.\-_]/",$this->hostId)){
      $messages[] = msgPool::invalid(_("Name"),$this->hostId,"/[a-z0-9\.\-_]/");
    }

    /* Ensure that the mac address is valid
     */
    if(!tests::is_mac($this->mac) || empty($this->mac)){
      $messages[] = msgPool::invalid(_("MAC address"),$this->mac,"","00:0C:7F:31:33:F1");
    }
    return($messages);
  }


  /*! \brief  Create the html ui of this plugin
      @return String  HTML content.
   */
  public function execute()
  {
    $display ="";

    /* The pluign initialization failed due to communication problems with the gosa daemon. 
       A retry button will be displayed here.
     */
    if($this->init_failed){
      $smarty = get_smarty();
      $smarty->assign("init_failed",TRUE);
      $smarty->assign("message",$this->opsi->get_error());
      return($smarty->fetch(get_template_path("generic.tpl",TRUE,dirname(__FILE__))));
    }  

    /* If we are not a stand alone opsi client, we must be a samba client 
       which has the opsi tab enabled.
       Check if the opsi is added or removed and display state buttons.
     */
    if(!$this->parent_mode){
      if(isset($_POST['modify_state'])){
        if($this->is_account){
          $this->is_account= FALSE;
        }elseif(!$this->is_account){
          $this->is_account= TRUE;
        }
      }
      if($this->is_account){
        $display = $this->show_disable_header(msgPool::removeFeaturesButton(_("Opsi")), 
            msgPool::featuresEnabled(_("Opsi")));
      }else{
        $display = $this->show_enable_header(msgPool::addFeaturesButton(_("Opsi")), 
            msgPool::featuresDisabled(_("Opsi")));
        return($display);
      } 
    } 

    /* Check if we have a sub dialog opened
     */
    if(is_object($this->dialog)){
      $this->dialog->save_object();
      return($this->dialog->execute());
    }

    /* Create HTML output of this plugin
     */
    $smarty = get_smarty();
    foreach($this->attributes as $attr){
      $smarty->assign($attr,$this->$attr);
    }

    $smarty->assign("parent_mode", $this->parent_mode);
    $smarty->assign("is_installed", $this->is_installed);
    $smarty->assign("init_failed",FALSE);
    $divSLP = new divSelectBox();
    $divALP = new divSelectBox();

    /* Create list of available local products 
     */
    foreach($this->a_availableLocalProducts as $name => $data){
      if(isset($this->a_selectedLocalProducts[$name])) continue;

      $add_tab  = array("string"   => "<input type='image' src='images/back.png' name='add_lp_".$name."'>");
      $name_tab = array("string"   => $name);
      $desc_tab = array("string"   => "<div style='height: 14px;overflow:hidden;'>".$data['DESC']."</div>",
          "attach"   => "title='".$data['DESC']."' style='border-right:0px;'");
      $divALP->AddEntry(array($add_tab,$name_tab,$desc_tab));
    }

    /* Create list of selected local products 
     */
    ksort($this->a_selectedLocalProducts);
    foreach($this->a_selectedLocalProducts as $name => $data){

      $name_tab = array("string"   => $name);
      $desc_tab = array(
          "string" => "<div style='height: 14px;overflow:hidden;'>".$data['DESC']."</div>",
          "attach" => "title='".$data['DESC']."'");

      /* Only display edit button, if there is something to edit 
       */
      $edit = "<img src='images/empty.png' alt=' '>";
      if(count($data['CFG'])){
        $edit = "<input type='image' src='images/lists/edit.png' name='edit_lp_".$name."'>";
      }
      $del  = "<input type='image' src='images/lists/trash.png' name='del_lp_".$name."'>";  

      $opt_tab  = array("string" => $edit.$del,
          "attach" => "style='border-right:0px; width: 40px; text-align:right;'");
      $divSLP->AddEntry(array($name_tab,$desc_tab,$opt_tab));
    }

    /* Check if netboot product is configurable 
     */
    $cfg_able =FALSE;
    if(isset($this->a_availableNetbootProducts[$this->s_selectedNetbootProduct]['CFG'])){
      $cfg_able = count($this->a_availableNetbootProducts[$this->s_selectedNetbootProduct]['CFG']);
    }

    $smarty->assign("netboot_configurable",$cfg_able);
    $smarty->assign("hostId", $this->hostId);
    $smarty->assign("divSLP", $divSLP->DrawList());
    $smarty->assign("divALP", $divALP->DrawList());
    $smarty->assign("SNP", $this->s_selectedNetbootProduct);
    $smarty->assign("ANP", $this->a_availableNetbootProducts);
    return($display.$smarty->fetch(get_template_path("generic.tpl",TRUE,dirname(__FILE__))));
  }


  /*! \brief  Save modifications using the gosa support daemon.
   */
  public function save()
  {
    
    /* Check if we have to create a new opsi client
        or just have to save client modifications.
     */
    if(!$this->initially_was_account && $this->is_account){
      $res = $this->opsi->add_client($this->hostId,$this->mac,$this->note,$this->description);
      if($this->opsi->is_error()){
        msg_dialog::display(_("Error"),msgPool::siError($this->opsi->get_error()),ERROR_DIALOG);    
        return;
      }
    }else{

      /* Update client modifcations.
          -Only if necessary  
       */
      if($this->note != $this->initial_note || 
          $this->description != $this->initial_description ||
          $this->mac != $this->initial_mac){
        $this->opsi->modify_client($this->hostId,$this->mac,$this->note,$this->description);
        if($this->opsi->is_error()){
          msg_dialog::display(_("Error"),msgPool::siError($this->opsi->get_error()),ERROR_DIALOG);
          return;
        }
      }
    }


    /***********
      Detect local netboot product changes
       - Check which products were removed.
       - Check which products were added. 
     ***********/


    /* Detect which products were removed an which added.
     */
    $del = array_diff_assoc($this->a_initial_selectedLocalProducts,$this->a_selectedLocalProducts);
    $add = array_diff_assoc($this->a_selectedLocalProducts,$this->a_initial_selectedLocalProducts);

    /* Remove products from client
     */
    foreach($del as $name => $data){
      $this->opsi->del_product_from_client($name,$this->hostId);
      if($this->opsi->is_error()){
        msg_dialog::display(_("Error"),msgPool::siError($this->opsi->get_error()),ERROR_DIALOG);    
        return;
      }
    }
    
    /* Add products to client
       And set the product properties.
     */
    foreach($add as $name => $data){
      $this->opsi->add_product_to_client($name,$this->hostId);
      if($this->opsi->is_error()){
        msg_dialog::display(_("Error"),msgPool::siError($this->opsi->get_error()),ERROR_DIALOG);    
        return;
      }
      if(!empty($data['CFG'])){
        $this->opsi->set_product_properties($name,$data['CFG'],$this->hostId);
        if($this->opsi->is_error()){
          msg_dialog::display(_("Error"),msgPool::siError($this->opsi->get_error()),ERROR_DIALOG);    
          return;
        }
      }
    }

    /* Save local product properties 
     */
    foreach($this->a_selectedLocalProducts as $name => $data){
      if(isset($del[$name]) || isset($add[$name])) continue;

      /* Update product properties if there are changes 
       */
      $diffs = $this->get_config_changes($data['CFG'],$this->a_initial_selectedLocalProducts[$name]['CFG']);
      if(count($diffs)){
        $this->opsi->set_product_properties($name,$diffs,$this->hostId);
        if($this->opsi->is_error()){
          msg_dialog::display(_("Error"),msgPool::siError($this->opsi->get_error()),ERROR_DIALOG);    
          return;
        }
      }
    }

    /*********
      Detect Netboot product changes
       - Check if another netboot product was selected. 
       - Check if the product properties were changes.
     *********/

    /* Update used netboot product. 
     */
    if($this->s_selectedNetbootProduct != $this->s_initial_selectedNetbootProduct){
      if(!empty($this->s_initial_selectedNetbootProduct)){
        $this->opsi->del_product_from_client($this->s_initial_selectedNetbootProduct,$this->hostId);
        if($this->opsi->is_error()){
          msg_dialog::display(_("Error"),msgPool::siError($this->opsi->get_error()),ERROR_DIALOG);    
          return;
        }
      }
      $this->opsi->add_product_to_client($this->s_selectedNetbootProduct,$this->hostId);
      if($this->opsi->is_error()){
        msg_dialog::display(_("Error"),msgPool::siError($this->opsi->get_error()),ERROR_DIALOG);    
        return;
      }
    }

    /* Check if we have to update the netboot product properties 
        This is the case, if this product is newly selected.
        Or if there was at least one configuration attribute modified.
     */
    $cfg_1 = $cfg_2 = array();
    if(isset($this->a_availableNetbootProducts[$this->s_selectedNetbootProduct]['CFG'])){
      $cfg_1 = $this->a_availableNetbootProducts[$this->s_selectedNetbootProduct]['CFG'];
    }
    if(isset($this->a_initial_availableNetbootProducts[$this->s_selectedNetbootProduct]['CFG'])){
      $cfg_2 = $this->a_initial_availableNetbootProducts[$this->s_selectedNetbootProduct]['CFG'];
    }
    $diffs = $this->get_config_changes($cfg_1,$cfg_2);
    $to_update = array();
    if( !$this->initially_was_account || 
        $this->s_selectedNetbootProduct != $this->s_initial_selectedNetbootProduct){
      $to_update = $this->a_availableNetbootProducts[$this->s_selectedNetbootProduct]['CFG'];
    }elseif(count($diffs)){
      $to_update = $diffs;
    }

    if(count($to_update)){
      $name = $this->s_selectedNetbootProduct;
      $this->opsi->set_product_properties($name,$to_update,$this->hostId);
      if($this->opsi->is_error()){
        msg_dialog::display(_("Error"),msgPool::siError($this->opsi->get_error()),ERROR_DIALOG);
        return;
      }
    }
  }

  
  public function get_config_changes($c1,$c2)
  {
    /* Get key which are not present in both entries 
     */
    $res = array();
    foreach($c2 as $name => $value){
      if(!isset($c1[$name]) || $c1[$name]['DEFAULT'] != $c2[$name]['DEFAULT']){
        $res[$name] = $c2[$name];
      }
    }
    foreach($c1 as $name => $value){
      if(!isset($c2[$name]) || $c2[$name]['DEFAULT'] != $c1[$name]['DEFAULT']){
        $res[$name] = $c1[$name];
      }
    }
    return($res);
  }


  /*! \brief  Removes the opsi client 
   */  
  public function remove_from_parent()
  {
    $this->opsi->del_client($this->hostId);
    if($this->opsi->is_error()){
      msg_dialog::display(_("Error"),msgPool::siError($this->opsi->get_error()),ERROR_DIALOG);
      return;
    }
  }


  /*! \brief  Save html posts 
   */
  public function save_object()
  {
    /* Init failed; reinit is triggered here.
     */
    if(isset($_POST['reinit']) && $this->init_failed){
      $this->init();
    }

    /* Property are currently edited, close the dialog. 
     */
    if(isset($_POST['cancel_properties']) && is_object($this->dialog)){
      $this->dialog = NULL;
    }
  
    /* Save product property changes 
     */
    if(isset($_POST['save_properties']) && ($this->dialog instanceof opsiProperties)){
      $this->dialog->save_object();
      $pro = $this->dialog->get_product();
      $CFG = $this->dialog->get_cfg();
      if(isset($this->a_selectedLocalProducts[$pro])){
        $this->a_selectedLocalProducts[$pro]['CFG'] = $CFG;
        $this->dialog = NULL;
      }elseif($this->s_selectedNetbootProduct == $pro){
        $this->a_availableNetbootProducts[$pro]['CFG'] = $CFG;
        $this->dialog = NULL;
      }else{
        trigger_error("Fatal, unknown product was configured.");
      }
    }

    /* Save html post
     */
    if(isset($_POST['opsiGeneric_posted'])){

      plugin::save_object();

      /* Get hostId 
       */
      if(isset($_POST['hostId']) && $this->parent_mode){
        $this->hostId = get_post('hostId');
      }

      /* Send actions like 'install' or 'wake' to the si server 
       */
      if(isset($_POST['opsi_action']) && isset($_POST['opsi_trigger_action']) && $this->parent_mode){
        $action = $_POST['opsi_action'];
        if($action == "install"){
          $this->install_client(); 
        }
      }

      /* Get selected netboot product.
       */
      if(isset($_POST['opsi_netboot_product'])){
        $SNP = trim($_POST['opsi_netboot_product']);
        if(isset($this->a_availableNetbootProducts[$SNP])){

          if(!isset($this->a_availableNetbootProducts[$SNP]['CFG'])){
            $CFG = $this->opsi->get_product_properties($SNP);
            $this->a_availableNetbootProducts[$SNP]['CFG'] = $CFG;
            if($this->opsi->is_error()){
              $this->init_failed = TRUE;
              return;
            }
          }
          $this->s_selectedNetbootProduct = $SNP;
        }
      }

      /* Add/remove/edit local products 
       */
      foreach($_POST as $name => $value){

        /* Check if netboot product configuration is requested 
         */
        if(preg_match("/^configure_netboot/",$name)){
          $pro = $this->s_selectedNetbootProduct;
          $cfg = $this->a_availableNetbootProducts[$pro]['CFG'];
          $this->dialog = new opsiProperties($this->config,$pro,$cfg,$this->hostId);
          break;
        }
      
        /* Add product 
         */
        if(preg_match("/^add_lp_/",$name)){
          $product = preg_replace("/^add_lp_(.*)_.$/","\\1",$name);
          if(isset($this->a_availableLocalProducts[$product]) && !isset($this->a_selectedLocalProducts[$product])){
            $this->a_selectedLocalProducts[$product] = $this->a_availableLocalProducts[$product];
            $CFG = $this->opsi->get_product_properties($product);
            if($this->opsi->is_error()){
              $this->init_failed = TRUE;
              return;
            }
            $this->a_selectedLocalProducts[$product]['CFG'] = $CFG;
          }
          break;
        }
  
        /* Delete product 
         */
        if(preg_match("/^del_lp_/",$name)){
          $product = preg_replace("/^del_lp_(.*)_.$/","\\1",$name);
          if(isset($this->a_selectedLocalProducts[$product])){
            unset($this->a_selectedLocalProducts[$product]);
          }
          break;
        }
      
        /* Edit a product  
         */
        if(preg_match("/^edit_lp_/",$name)){
          $product = preg_replace("/^edit_lp_(.*)_.$/","\\1",$name);
          $this->dialog = new opsiProperties($this->config,
              $product,$this->a_selectedLocalProducts[$product]['CFG'],$this->hostId);
          break;
        }
      }   
    }
  }


  /* Triggers client installation 
   */
  function install_client()
  {
    $this->opsi->send_action("install",$this->hostId,$this->mac);
    if($this->opsi->is_error()){
      msg_dialog::display(_("Error"),msgPool::siError($this->opsi->get_error()),ERROR_DIALOG);
    }
  }


  /* Return plugin informations for acl handling */
  static function plInfo()
  {
    return (array(
          "plShortName"   => _("Generic"),
          "plDescription" => _("Opsi generic"),
          "plSelfModify"  => FALSE,
          "plDepends"     => array(),
          "plPriority"    => 1,
          "plSection"     => array("administration"),
          "plCategory"    => array("opsi" => array("description"  => _("Opsi client"),
                                                     "objectClass"  => "dummy_class_opsi")),
          "plProvidedAcls"=> array()
          ));
  }

}


// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
