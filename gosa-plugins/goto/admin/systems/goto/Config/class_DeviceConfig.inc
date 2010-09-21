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
                "PuppetRoot": {
                    "options": {

                    },
                    "container": [
                        "PuppetModule"
                        ],
                    "name": "Root",
                    "description": "The root item"
                }
        }';        

        $str = json_decode($str, TRUE);
        $this->TemplateEngine->load($str);
        $this->TemplateEngine->setTemplate('puppet.tpl');
    }


    function execute()
    {
        $smarty = get_smarty();
        $smarty->assign('template',$this->TemplateEngine->render());
        return($smarty->fetch(get_template_path('goto/Config/DeviceConfig.tpl', TRUE)));
    }
}
?>
