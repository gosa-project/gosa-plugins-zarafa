<?php 
class migrate_faiPartitionRDN extends migrateRDN implements propertyMigration
{
	protected $filter = "(&(objectClass=FAIpartitionTable)%s)";
}
?>
