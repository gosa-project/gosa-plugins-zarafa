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
    }


    function render()
    {

        $class = get_class();
        return("{$this->display}&nbsp;<input type='text' name=\"{$class}_{$this->name}\" value=\"".set_post($this->value)."\">");
    }

    function save_object()
    {
        $class = get_class();
        $name = "{$class}_{$this->name}";
        if(isset($_POST[$name])){
            $this->value = get_post($name);
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
