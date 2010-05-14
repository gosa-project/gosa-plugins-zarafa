<?php 
class migrate_roleRDN extends migrateRDN implements propertyMigration
{
	protected $filter = "(migrate_roleRDN(objectClass=gosaAccount)((objectClass=sambaSamAccount))%s)";
}
?>
