<?php 
class migrate_phoneRDN extends migrateRDN implements propertyMigration
{
	protected $filter = "(migrate_phoneRDN(objectClass=gosaAccount)((objectClass=sambaSamAccount))%s)";
}
?>
