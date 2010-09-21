<?php

class TemplateEngine
{
    private $config;
    private $data = array();    
    private $template = "";

    function __construct($config)
    {
        $this->config = &$config;
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
        $res = array();
        $res[] = new TemplateWidget($this->config, 'name', 'herbert');
        return($res);
    }     

    function render()
    {
        $smarty = get_smarty();
        foreach($this->getWidgets() as $widget){
            $smarty->assign($widget->getName(), $widget->render());
        }
        return($smarty->fetch(get_template_path("goto/Config/{$this->template}", TRUE)));
    }
}


?>
