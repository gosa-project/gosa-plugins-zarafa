<?php 
class migrate_terminalRDN extends migrateRDN implements propertyMigration
{
	protected $filter = "(migrate_terminalRDN(objectClass=gosaAccount)((objectClass=sambaSamAccount))%s)";
}
?>
