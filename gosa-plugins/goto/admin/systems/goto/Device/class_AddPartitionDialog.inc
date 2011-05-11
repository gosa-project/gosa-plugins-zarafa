<?php

class AddPartitionDialog
{
    public $partitionObject;

    function __construct($config, $partitionObject)
    {
        $this->partitionObject = &$partitionObject;
        $this->config = &$config;
    }
    
    function execute()
    {
        $smarty = get_smarty();
        
        return(get_template_path("goto/Device/AddPartitionDialog.tpl", TRUE));
    }

    function save_object()
    {
    }

    function save()
    {
    }
}
?>
