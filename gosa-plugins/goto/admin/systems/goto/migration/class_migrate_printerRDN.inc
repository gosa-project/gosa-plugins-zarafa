<?php 
class migrate_printerRDN extends migrateRDN implements propertyMigration
{
	protected $filter = "(migrate_printerRDN(objectClass=gosaAccount)((objectClass=sambaSamAccount))%s)";
}
?>
