<?php 
class migrate_faiTemplateRDN extends migrateRDN implements propertyMigration
{
	protected $filter = "(&(objectClass=FAItemplate)%s)";
}
?>
