<?php

class TemplateWidget
{
    protected $name = "unnamed";
    protected $value = "";

    protected $description = "";
    protected $required = "";
    protected $type = "";
    protected $display = "";

    function __construct(&$config, $name, $value, $description,$required,$type,$display)
    {
        $this->config = &$config;
        $this->name = $name;
        $this->value = $value;
        $this->description = $description;
        $this->required = $required;
        $this->type = $type;
        $this->display = $display;
        $class = get_class();
        $this->postName = "{$class}_{$this->name}";
    }
    
    function getDisplayName()
    {
        return($this->display);
    }


    function render()
    {
        return("");
    }

    function save_object()
    {
        if(isset($_POST[$this->postName])){
            $this->value = get_post($this->postName);
        }
    }

    function getValue()
    {
        return($this->value);
    }

    function getName()
    {
        return($this->name);
    }

    function setValue($value)
    {
        $this->value = $value;
    }

}

?>
