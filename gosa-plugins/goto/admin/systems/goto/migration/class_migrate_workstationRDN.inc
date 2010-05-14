<?php 
class migrate_workstationRDN extends migrateRDN implements propertyMigration
{
	protected $filter = "(migrate_workstationRDN(objectClass=gosaAccount)((objectClass=sambaSamAccount))%s)";
}
?>
