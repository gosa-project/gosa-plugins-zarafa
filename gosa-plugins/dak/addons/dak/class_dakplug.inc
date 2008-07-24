<?php


class dakplug extends plugin
{

	var $tabs = NULL;	
	
	public function __construct($config,NULL)
	{
		$this->tabs = new dak_tabs($config, $config->data['TABS']['DAK_TABS'],"");
	}
	
	public function execute()
	{
		return($this->tabs->execute());
	}

	public function save_object()
	{
		$this->tabs->save_object();
	}	

	public function save() {}
	public function remove_from_parent() {}
	public function check() {}

}

?>
