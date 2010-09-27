<?php

class DeviceConfig extends plugin
{
    private $TemplateEngine = NULL;

    function __construct(&$config, $dn)
    {
        $this->TemplateEngine = new TemplateEngine($config);
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

        $this->itemConfig = json_decode($str, TRUE);
        
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
                    'version' => 1,
                    'name'  => 'test1',
                    'description' => 'Test Module')
                );
        $this->addItem('PuppetModule','test2',
                array(
                    'version' => 1,
                    'name'  => 'test2',
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
        $str .= "<a href='?plug={$plug}&amp;item={$array['name']}'>";
        if($this->currentItemName == $array['name']){
            $str .= "<b>".$array['name']."</b>";
        }else{
            $str .= $array['name'];
        }
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
        $this->currentItemName = $item;
        $this->currentItem = &$this->currentItemValues[$item];
        $this->currentItemDescriptor =&$this->itemConfig[$this->currentItem['type']];
    }


    function execute()
    {
        $smarty = get_smarty();
        
        // Assign possible sub-container objects.
        $smarty->assign('subModule', $this->currentItemDescriptor['container']);
        $smarty->assign('containerName', $this->currentItemDescriptor['name']);
        $smarty->assign('containerDescription', $this->currentItemDescriptor['description']);

        $smarty->assign('navigator', $this->renderNavigator());
        


        $this->TemplateEngine->load($this->currentItemDescriptor);
        $this->TemplateEngine->setTemplate('puppet.tpl');
        $smarty->assign('template',$this->TemplateEngine->render());
        return($smarty->fetch(get_template_path('goto/Config/DeviceConfig.tpl', TRUE)));
    }

    function save_object()
    {
        if(isset($_POST['addSubModule']) && isset($_POST['subModule'])){
            $sub = get_post('subModule');
            if(in_array($sub, $this->currentItemDescriptor['container'])){
                
            }
        }
    
        if(isset($_GET['item'])){
            $this->setCurrentItem($_GET['item']);
        }
    }
}
?>
