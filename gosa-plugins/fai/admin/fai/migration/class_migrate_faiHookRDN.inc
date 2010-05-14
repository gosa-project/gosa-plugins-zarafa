<?php 
class migrate_faiHookRDN extends migrateRDN implements propertyMigration
{
	protected $filter = "(migrate_faiHookRDN(objectClass=gosaAccount)((objectClass=sambaSamAccount))%s)";
}
?>
