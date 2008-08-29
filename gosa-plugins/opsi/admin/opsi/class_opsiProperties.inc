<?php


/*! \brief  Displays opsi product properties.
 */
class opsiProperties extends plugin
{
  private $product;
  private $cfg;
  private $hostId;


  /*! \brief  Initializes the class.
      @param  Object  The gosa configuration object
      @param  String  The product name.
      @param  Array   The product config (e.g. aray('attr' => 'value'))
      @param  String  The host id if necessary
   */
  public function __construct($config,$product,$cfg = array() ,$hostId = "")
  {
    $this->config  = $config;
    $this->product = $product;
    $this->cfg     = $cfg;
    $this->hostId  = $hostId;
  }


  /*! \brief  Display html dialog which allows to 
                configure the properties
      @return String  HTML content
   */
  public function execute()
  {
    $smarty = get_smarty();
    $smarty->assign("cfg",$this->cfg);
    $smarty->assign("cfg_count",count($this->cfg));
    return($smarty->fetch(get_template_path("properties.tpl",TRUE,dirname(__FILE__))));
  }


  /*! \brief  Save the posted property updates 
   */
  public function save_object()
  {
    foreach($this->cfg as $name => $value){
      if(isset($_POST['value_'.$name])){
        $this->cfg[$name] = validate(get_post('value_'.$name));
      }
    }
  }


  /*! \brief  Sets the current config array.
   */
  public function set_cfg($cfg) 
  {
    $this->cfg = $cfg;
  }


  /*! \brief  Returns the current product config
      @return Array.
   */
  public function get_cfg() 
  {
    return($this->cfg);
  }

  
  /*! \brief  Returns the ID of the product.
      @param  String  Product ID
   */
  public function get_product()
  {
    return($this->product);
  }

  
  /*! \brief  Sets the current product ID, 
              The name is only used to remember the currently edited product.
   */
  public function set_product($name)
  {
    $this->product = $name;
  }


  /*! \brief  Retuns the hostId.
   */
  public function get_hostId()
  {
    return($this->hostId);
  }
}
// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
