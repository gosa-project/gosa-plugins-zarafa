<?php 
class migrate_faiHookRDN extends migrateRDN implements propertyMigration
{
	protected $filter = "(&(objectClass=FAIhook)%s)";
}
?>
