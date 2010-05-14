<?php 
class migrate_systemIncomingRDN extends migrateRDN implements propertyMigration
{
	protected $filter = "(migrate_systemIncomingRDN(objectClass=gosaAccount)((objectClass=sambaSamAccount))%s)";
}
?>
