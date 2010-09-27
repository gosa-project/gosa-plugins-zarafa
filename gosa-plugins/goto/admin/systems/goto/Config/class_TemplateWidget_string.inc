<?php

class TemplateWidget_string extends TemplateWidget
{
    function render()
    {
        return("<input type='text' name=\"{$this->postName}\" value=\"".set_post($this->value)."\">");
    }
}

?>
