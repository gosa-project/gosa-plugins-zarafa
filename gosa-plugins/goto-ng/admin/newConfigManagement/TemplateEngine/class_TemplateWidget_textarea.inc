<?php

class TemplateWidget_textarea extends TemplateWidget
{
    function render()
    {
        $title = set_post($this->description);
        $disabled = (!$this->enabled)? "disabled" : "";
        return("<textarea   title=\"{$title}\"
                            style='width:100%'
                            {$disabled} 
                            rows=4 type='text' 
                            name=\"{$this->postName}\">".set_post($this->value)."</textarea>");
    }

}

?>
