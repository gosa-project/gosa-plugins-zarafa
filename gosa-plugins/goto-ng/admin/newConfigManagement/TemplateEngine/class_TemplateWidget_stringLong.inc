<?php

class TemplateWidget_stringLong extends TemplateWidget
{
    function render()
    {
        $desc = set_post($this->description);
        $value = set_post($this->value);
        
        $name = " name=\"{$this->postName}\" ";
        $value = " value=\"{$value}\" ";
        $title = (empty($this->description))?"": " title=\"{$desc}\"";
        
        $disabled = (!$this->enabled)? "disabled" : "";
        return("<input type='text' style='width:100%;' {$title} {$name} {$value} {$disabled}>");
    }
}

?>
