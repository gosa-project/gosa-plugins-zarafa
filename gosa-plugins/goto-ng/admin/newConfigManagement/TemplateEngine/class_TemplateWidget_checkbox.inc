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

        $str .= "<input type='hidden' name='{$this->postName}_posted' value='1'>";
        return($str);
    }

    function save_object()
    {
        if(isset($_POST["{$this->postName}_posted"])){
            $this->value = isset($_POST[$this->postName]);
        }
    }
}
?>
