<?php 
class migrate_sudoRDN extends migrateRDN implements propertyMigration
{
	protected $filter = "(migrate_sudoRDN(objectClass=gosaAccount)((objectClass=sambaSamAccount))%s)";
}
?>
