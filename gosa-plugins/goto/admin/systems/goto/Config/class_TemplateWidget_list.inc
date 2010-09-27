<?php

class TemplateWidget_list extends TemplateWidget
{
    function render()
    {
        $str = "";
        foreach($this->value as $id => $val){
            $str.= "{$val}, ";
        }
        return($str);
    }

}


?>
