<?php

class DeviceConfig extends plugin
{
    private $TemplateEngine = NULL;

    private $idToName = array();
    private $currentItemName = "";

    function __construct(&$config, $dn)
    {
        $this->TemplateEngine = new TemplateEngine($config);
        $this->TemplateEngine->setTemplate('puppet.tpl');
        $this->config = $config;

        $str = '{
            "PuppetModule": {
                "options": {
                    "dependency": {
                        "description": "Modules that are needed to be installed for this module",
                            "required": false,
                            "value": [

                                ],
                            "syntax": "^[a-zA-Z0-9_+\\\\./-]+(\\\\[[<=>]+[a-zA-Z0-9_+\\\\.-]+\\\\])?$",
                            "type": "list",
                            "display": "Module dependencies"
                    },
                        "version": {
                            "description": "The version of the puppet module",
                            "required": true,
                            "value": "",
                            "syntax": "^[a-zA-Z0-9_+.-]+$",
                            "type": "textarea",
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

        $this->itemConfig = json_decode($str, TRUE);
        $this->TemplateEngine->load($this->itemConfig);
        
        // Check for the root object
        if(!isset($this->itemConfig['root'])){
            echo 'No root!';
        }

        // Set current item to 'root'.
        $this->currentItemDescriptor = &$this->itemConfig['root'];
        $this->currentItemValues = array();
        $this->currentItem = array();

        $this->addItem('root','root',array());
        $this->setCurrentItem('root');
        $this->addItem('PuppetModule','test1',
                array(
                    'dependency' => array('mozilla-2','toaster-2.0'),
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
        $this->addItem('PuppetTemplate','temp1',array('name' => 'temp1', 'file' => 'kekse.tpl'));

        $this->setCurrentItem('root');
    }


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

    function setCurrentItem($item)
    {
        if($this->currentItemName == $item) return;

        // Save eventually changed values
        if($this->currentItem){
            foreach($this->TemplateEngine->getWidgets() as $widget){
                $this->currentItem['values'][$widget->getName()] = $widget->getValue();
            }
        }


        $this->currentItemName = $item;
        $this->currentItem = &$this->currentItemValues[$item];
        $this->currentItemType = $this->currentItem['type'];
        $this->currentItemDescriptor =&$this->itemConfig[$this->currentItem['type']];
        $this->TemplateEngine->setType($this->currentItemType);
        $this->TemplateEngine->setValues($this->currentItem['values']);
    }


    function execute()
    {
        $smarty = get_smarty();
        
        // Assign possible sub-container objects.
        $smarty->assign('subModule', $this->currentItemDescriptor['container']);
        $smarty->assign('containerName', $this->currentItemDescriptor['name']);
        $smarty->assign('containerDescription', $this->currentItemDescriptor['description']);

        $smarty->assign('navigator', $this->renderNavigator());
        


        $smarty->assign('template',$this->TemplateEngine->render());
        return($smarty->fetch(get_template_path('goto/Config/DeviceConfig.tpl', TRUE)));
    }

    function save_object()
    {
        $this->TemplateEngine->save_object();

        if(isset($_POST['addSubModule']) && isset($_POST['subModule'])){
            $sub = get_post('subModule');
            if(in_array($sub, $this->currentItemDescriptor['container'])){
                
            }
        }
    
        if(isset($_GET['item'])){
            $name = $this->idToName[$_GET['item']];
            $this->setCurrentItem($name);
        }
    }
}
?>
