<?php 
class migrate_deviceRDN extends migrateRDN implements propertyMigration
{
	protected $filter = "(migrate_deviceRDN(objectClass=gosaAccount)((objectClass=sambaSamAccount))%s)";
}
?>
