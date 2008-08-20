<?php

class opsi_generic extends plugin
{
  private $opsi;
  private $hostId;  

  /* Contains a list of all available netboot products 
   */
  private $a_availableNetbootProducts = array();
  private $s_selectedNetbootProduct = "";  

  /* Contains a list of all available local products
   */
  private $a_availableLocalProducts = array();
  private $a_selectedLocalProducts = array();
  
  public function __construct($config,$hostId)
  {
    $this->opsi = new opsi($config); 
    
    /* Get hostId */
    $this->hostId = preg_replace("/^opsi:=([^,]*),.*$/","\\1",$hostId);

    /* Get product settings */     
    $this->a_availableNetbootProducts = $this->opsi->get_netboot_products();
    $this->a_availableLocalProducts   = $this->opsi->get_local_products();
  
    /* Get selected products */
    $tmp = array_keys($this->opsi->get_netboot_products($this->hostId));  
    $this->s_selectedNetbootProduct = $tmp;
    $tmp = $this->opsi->get_local_products($this->hostId);  
    $this->a_selectedLocalProducts = $tmp;

    /* Load product configuration */
    foreach($this->a_selectedLocalProducts as $name => $data){
      $CFG = $this->opsi->get_product_properties($name,$this->hostId);
      $this->a_selectedLocalProducts[$name]['CFG'] = $CFG;
    }
  }

  public function execute()
  {
    $smarty = get_smarty();
    $divSLP = new divSelectBox();
    $divALP = new divSelectBox();

    /* Create list of available local products 
     */
    ksort($this->a_availableLocalProducts);
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

    $smarty->assign("divSLP", $divSLP->DrawList());
    $smarty->assign("divALP", $divALP->DrawList());
    $smarty->assign("SNP", $this->s_selectedNetbootProduct);
    $smarty->assign("ANP", $this->a_availableNetbootProducts);
    return($smarty->fetch(get_template_path("generic.tpl",TRUE,dirname(__FILE__))));
  }


  public function save_object()
  {
    if(isset($_POST['opsi_generic'])){

      if(isset($_POST['opsi_netboot_product'])){
        $SNP = trim($_POST['opsi_netboot_product']);
        if(isset($this->a_availableNetbootProducts[$SNP])){
          $this->s_selectedNetbootProduct = $SNP;
        }
      }
    
      foreach($_POST as $name => $value){
        if(preg_match("/^add_lp_/",$name)){
          $product = preg_replace("/^add_lp_(.*)_.$/","\\1",$name);
          if(isset($this->a_availableLocalProducts[$product]) && !isset($this->a_selectedLocalProducts[$product])){
            $this->a_selectedLocalProducts[$product] = $this->a_availableLocalProducts[$product];
            $CFG = $this->opsi->get_product_properties($product);
            $this->a_selectedLocalProducts[$product]['CFG'] = $CFG;
          }
          break;
        }
        if(preg_match("/^del_lp_/",$name)){
          $product = preg_replace("/^del_lp_(.*)_.$/","\\1",$name);
          if(isset($this->a_selectedLocalProducts[$product])){
            unset($this->a_selectedLocalProducts[$product]);
          }
          break;
        }
        if(preg_match("/^edit_lp_/",$name)){
          $product = preg_replace("/^edit_lp_(.*)_.$/","\\1",$name);
          $this->dialog = new opsi_product_config($product,$this->hostId);
          break;
        }
      }   
    }
  }
}


// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
