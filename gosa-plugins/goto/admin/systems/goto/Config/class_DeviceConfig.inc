<?php

/*! \brief  A GOsa plugin which generates a device configuration dialog  
 */
class DeviceConfig extends plugin
{
    private $TemplateEngine = NULL;
    private $idToName = array();
    private $currentItemName = "";
    private $currentItemValues = array();
    private $currentItem = array();


    /*! \brief  Constructs the device configuration plugin 
     *  @param  Config  The GOsa configuration object.
     */
    function __construct(&$config, $dn)
    {
        // Load the template engine and tell her what template
        //  to use for the HTML it produces.
        $this->TemplateEngine = new TemplateEngine($config);
        $this->TemplateEngine->setTemplate('puppet.tpl');
        $this->config = $config;

        // CREATE Dummy entry 

        $str = '{
            "PuppetModule": {
                "options": {
                    "dependency": {
                        "description": "Modules that are needed to be installed for this module",
                            "required": false,
                            "value": "0x002",
                            "syntax": "^[a-zA-Z0-9_+\\\\./-]+(\\\\[[<=>]+[a-zA-Z0-9_+\\\\.-]+\\\\])?$",
                            "type": "file",
                            "display": "Module dependencies"
                    },
                        "version": {
                            "description": "The version of the puppet module",
                            "required": true,
                            "value": "",
                            "syntax": "^[a-zA-Z0-9_+.-]+$",
                            "type": "string",
                            "display": "Module  version"
                        },
                        "name": {
                            "description": "The name of the puppet module",
                            "required": true,
                            "value": "",
                            "syntax": "^[a-zA-Z0-9_+.-]+$",
                            "type": "string",
                            "display": "Module name"
                        },
                        "description": {
                            "required": false,
                            "type": "string",
                            "display": "Module description",
                            "value": "",
                            "description": "Text       briefly describing the module contents"
                        }
                },
                    "container": [
                        "PuppetManifest",
                    "PuppetFile",
                    "PuppetTemplate"
                        ],
                    "name": "Module",
                    "description": "Puppet     module"
            },
                "root": {
                    "options": {

                    },
                    "container": [
                        "PuppetModule"
                        ],
                    "name": "Root",
                    "description": "The root item"
                }
        }';        

        // Load the item-configuration description to populate the 
        //  the available modules.
        $this->itemConfig = json_decode($str, TRUE);
        $this->TemplateEngine->load($this->itemConfig);



        // CREATE Dummy entry 

        // Set current item to 'root'.
        $this->addItem('root','root',array());
        $this->setCurrentItem('root');
        $this->addItem('PuppetModule','test1',
                array(
                    'dependency' => '',
                    'version' => '2.4-f',
                    'name'  => 'Thundebird',
                    'description' => 'Mozilla mail client')
                );
        $this->addItem('PuppetModule','test2',
                array(
                    'version' => 1,
                    'name'  => 'Firefox',
                    'description' => 'Test Module')
                );
        $this->setCurrentItem('test1');
        $this->addItem('PuppetTemplate','temp1',
                array(
                    'name' => 'temp1', 
                    'file' => 'kekse.tpl')
                );

        $this->setCurrentItem('root');
    }


    /*! \brief      Renders a navigation to allow to switch between the
     *               active mopdules. 
     *              This method recursivly collects all module entries.
     *  @return HTML content which represents the navigation
     */
    function renderNavigator($array = NULL)
    {
        $array = ($array == NULL)? $this->currentItemValues['root']: $array;
        $str = "<ul>";
        $plug = $_GET['plug'];

        $id = array_search($array['name'],$this->idToName);
        $str .= "<a href='?plug={$plug}&amp;item={$id}'>";
        if($this->currentItemName == $array['name']){
            $str .= "<b>".$array['name']."</b>";
        }else{
            $str .= $array['name'];
        }
        $str .= "&nbsp;<i>(".$array['type'].")</i>";
        $str .= "</a>";

        if(count($array['children'])){
            foreach($array['children'] as $subItem){
                $str .= $this->renderNavigator($subItem);
            } 
        }
        $str  .= "</ul>";
        return($str);
    }


    /*! \brief      Add a new child-item to the currently selected one. 
     *               
     *  @param  String  type    The 'type' of the new object, eg. 'KickstartTemplate'
     *  @param  String  name    The 'name' of the new object.
     *  @param  Array   values  The initial values for this object.
     *  @return 
     */
    function addItem($type,$name, $values)
    {
        $current = &$this->currentItem; 
        $this->idToName[] = $name;
        $new = array(
                'children' => array(),
                'type' => $type, 
                'name' => $name, 
                'values' => $values);
        $this->currentItemValues[$name] = $new;
        $current['children'][$name] = &$this->currentItemValues[$name];
    }


    /*! \brief      Selects an item as active and takes care 
     *               of required post/get handling. 
     *  @param  String  The name of the item we want to select.
     *  @return 
     */
    function setCurrentItem($item)
    {
        // Do nothing if we're already where we wanted to switch to.
        if($this->currentItemName == $item) return;

        // Save eventually changed values
        if($this->currentItem){
            foreach($this->TemplateEngine->getWidgets() as $widget){
                $this->currentItem['values'][$widget->getName()] = $widget->getValue();
            }
        }

        // Set the new item info.
        $this->currentItemName = $item;
        $this->currentItem = &$this->currentItemValues[$item];
        $this->currentItemType = $this->currentItem['type'];
        $this->currentItemDescriptor =&$this->itemConfig[$this->currentItem['type']];

        // Update the template engine to use another type of item and 
        //  some other values.
        $this->TemplateEngine->setValues($this->currentItemType,$this->currentItem['values']);
    }


    /*! \brief  Renders the HTML content for the device-config plugin. 
     *  @return String  The generated HTML code. 
     */
    function execute()
    {
        $smarty = get_smarty();

        // Assign the navigation bar.
        $smarty->assign('navigator', $this->renderNavigator());

        // Assign possible sub-container objects.
        $smarty->assign('subModule', $this->currentItemDescriptor['container']);

        // Assign current item info
        $smarty->assign('containerName', $this->currentItemDescriptor['name']);
        $smarty->assign('containerDescription', $this->currentItemDescriptor['description']);

        // Assign the generated HTML of Widgets.
        $smarty->assign('template',$this->TemplateEngine->render());
        return($smarty->fetch(get_template_path('goto/Config/DeviceConfig.tpl', TRUE)));
    }


    /*! \brief  Keep track of posted values, some may be interesting for us. 
     *          Tell the template engine to take care of posted values too.
     *  @param  String
     *  @return 
     */
    function save_object()
    {
        $this->TemplateEngine->save_object();

        // Add sub-module requested.
        if(isset($_POST['addSubModule']) && isset($_POST['subModule'])){
            $sub = get_post('subModule');
            if(in_array($sub, $this->currentItemDescriptor['container'])){

                // Check if this is a valid item
                if(!isset($this->itemConfig[$sub])) {
                    echo "Invalid item type '{$sub}'!";
                    $values = array();
                }else{
                    $values = $this->itemConfig[$sub]['options'];
                }
                $name = 'test'.rand(0,99999);
                $this->addItem($sub,$name,$values);
                $this->setCurrentItem($name);
            }
        }

        // Module switched
        if(isset($_GET['item'])){
            $name = $this->idToName[$_GET['item']];
            $this->setCurrentItem($name);
        }
    }
}
?>
