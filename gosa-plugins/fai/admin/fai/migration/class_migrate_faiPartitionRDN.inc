<?php 
class migrate_faiPartitionRDN extends migrateRDN implements propertyMigration
{
	protected $filter = "(migrate_faiPartitionRDN(objectClass=gosaAccount)((objectClass=sambaSamAccount))%s)";
}
?>
