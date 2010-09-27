<?php

class TemplateWidget_list extends TemplateWidget
{
    function __construct(&$config, $name, $value, $description,$required,$type,$display)
    {
        parent:: __construct($config, $name, $value, $description,$required,$type,$display);

        $this->listWidget= new sortableListing($this->value);
        $this->listWidget->setEditable(false);
        $this->listWidget->setDeleteable(true);
        $this->listWidget->setColspecs(array('*'));
        $this->listWidget->setWidth("100%");
        $this->listWidget->setHeight("70px");
        $this->listWidget->setAcl("rwcdm");
    }


    function render()
    {
        $str = $this->display."&nbsp;";
        $this->listWidget->setListData($this->value);
        $this->listWidget->update();
        $str .= $this->listWidget->render();
        $str .= "<input type='text' name='{$this->postName}_Input'>";
        $str .= "<button name='{$this->postName}_Add'>".msgPool::addButton()."</button>";
        return($str);
    }

    function save_object()
    {
        $this->listWidget->save_object();
        $action = $this->listWidget->getAction();
        if($action['action'] == 'delete'){
            $id = $this->listWidget->getKey($action['targets'][0]);
            unset($this->value[$id]);
            $this->value = array_values($this->value);
        }

        if(isset($_POST["{$this->postName}_Add"]) && isset($_POST["{$this->postName}_Input"])){
            $input = get_post("{$this->postName}_Input");
            $this->value[] = $input;
        }
    }
}


?>
