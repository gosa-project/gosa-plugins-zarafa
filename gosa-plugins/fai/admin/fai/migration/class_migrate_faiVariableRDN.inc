<?php 
class migrate_faiVariableRDN extends migrateRDN implements propertyMigration
{
	protected $filter = "(&(objectClass=FAIvariable)%s)";
}
?>
