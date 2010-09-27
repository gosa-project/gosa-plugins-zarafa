<?php

class TemplateWidget_textarea extends TemplateWidget
{
    function render()
    {
        return("<textarea type='text' name=\"{$this->postName}\">".set_post($this->value)."</textarea>");
    }

}

?>
