<?php

class TemplateWidget_checkbox extends TemplateWidget
{
    function render()
    {
        $str = "";
        $disabled = (!$this->enabled)? "disabled" : "";
        $str .= "<input type='checkbox' value='1' name='{$this->postName}' {$disabled}";
            if($this->value) $str .= " checked ";
        $str .= ">";
        return($str);
    }

    function save_object()
    {
        $this->value = isset($_POST[$this->postName]);
    }
}
?>
