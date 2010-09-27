<?php

class TemplateEngine
{
    private $config;
    private $data = array();    
    private $template = "";

    private $itemType = '';

    function __construct($config)
    {
        $this->config = &$config;
    }

    function setValues($values)
    {
        foreach($values as $name => $value){
            $this->widgets[$name]->setValue($value);
        }
    }

    function setType($name)
    {
        $this->itemType = $name;
        $this->widgets = array();

        if(!isset($this->data[$this->itemType])){
            trigger_error("Undefined item type '{$name}'!");
            return;
        }
        $data = $this->data[$this->itemType];
        if(isset($data['options']) && count($data['options'])){
            foreach($data['options'] as $name => $item){

                $this->widgets[$name] = new TemplateWidget($this->config, $name, 
                    $item['value'],
                    $item['description'],
                    $item['required'],
                    $item['type'],
                    $item['display']);
            }
        }
    }

    function load($array)
    {
        $this->data = $array;
    }
  
    function setTemplate($tmpl)
    {
        $this->template = $tmpl;
    }
    
    function getWidgets()
    {
        return($this->widgets);
    }     

    function render()
    {
        $smarty = get_smarty();
        $smarty->assign("type", $this->itemType);
        foreach($this->widgets as $widget){
            $smarty->assign($widget->getName(), $widget->render());
        }
        return($smarty->fetch(get_template_path("goto/Config/{$this->template}", TRUE)));
    }

    function save_object()
    {
        foreach($this->widgets as $widget){
            $widget->save_object();
        }
    }
}


?>
