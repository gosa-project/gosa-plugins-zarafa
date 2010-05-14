<?php 
class migrate_sambaMachineAccountRDN extends migrateRDN implements propertyMigration
{
	protected $filter = "(migrate_sambaMachineAccountRDN(objectClass=gosaAccount)((objectClass=sambaSamAccount))%s)";
}
?>
