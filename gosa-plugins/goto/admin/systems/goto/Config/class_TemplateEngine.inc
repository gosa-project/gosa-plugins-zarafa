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

    function setValues($array)
    {
#        print_a($array);
    }

    function setType($name)
    {
        $this->itemType = $name;
        $this->widgets = array();

        $data = $this->data[$this->itemType];
        if(isset($data['options']) && count($data['options'])){
            foreach($data['options'] as $name => $item){
                $this->widgets[] = new TemplateWidget($this->config, $name, $item['value']);
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
        foreach($this->getWidgets() as $widget){
            $smarty->assign($widget->getName(), $widget->render());
        }
        return($smarty->fetch(get_template_path("goto/Config/{$this->template}", TRUE)));
    }
}


?>
