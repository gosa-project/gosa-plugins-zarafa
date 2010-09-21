<?php

class TemplateWidget
{
    private $name = "unnamed";
    private $value = "";

    function __construct(&$config, $name, $value)
    {
        $this->name = $name;
        $this->value = $value;
        $this->config = &$config;
    }


    function render()
    {
        return("<input type='text' name=\"{$this->name}\" value=\"".set_post($this->value)."\">");
    }

    function save_object()
    {
    }

    function getValue()
    {
    }

    function getName()
    {
        return($this->name);
    }

    function setValue()
    {
    }

}

?>
