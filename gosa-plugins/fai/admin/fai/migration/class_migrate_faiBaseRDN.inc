<?php 
class migrate_faiBaseRDN extends migrateRDN implements propertyMigration
{
	protected $filter = "(migrate_faiBaseRDN(objectClass=gosaAccount)((objectClass=sambaSamAccount))%s)";
}
?>
