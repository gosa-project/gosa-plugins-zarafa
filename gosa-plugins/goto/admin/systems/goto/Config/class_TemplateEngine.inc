<?php

class TemplateEngine
{
    private $config;
    private $data = array();    
    private $template = "";

    private $itemType = '';
    private $widgets = array();

    function __construct($config)
    {
        $this->config = &$config;
    }

    function setValues($values)
    {
        foreach($values as $name => $value){
            if(!isset($this->widgets[$name])){
                $class = get_class();
                echo "Unknown option '{$name}' for {$class}!)}<br>";
                 continue;
            }
            $this->widgets[$name]->setValue($value);
        }
    }

    function setType($name)
    {
        $this->itemType = $name;
        $this->widgets = array();

        if(!isset($this->data[$this->itemType])){
            echo "Undefined item type '{$name}'!<br>";
            return;
        }
        $data = $this->data[$this->itemType];
        if(isset($data['options']) && count($data['options'])){
            foreach($data['options'] as $name => $item){

                $widgetClassName = "TemplateWidget_{$item['type']}";
                if(!class_available($widgetClassName)){
                    echo "Unknown widget class {$widgetClassName}! Falling back to default widget.<br>";
                    $widgetClassName = "TemplateWidget_string";
                }

                $this->widgets[$name] = new $widgetClassName($this->config, $name, 
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
            $smarty->assign($widget->getName()."Name", $widget->getDisplayName());
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
