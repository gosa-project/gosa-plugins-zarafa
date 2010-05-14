<?php 
class migrate_faiPackageRDN extends migrateRDN implements propertyMigration
{
	protected $filter = "(migrate_faiPackageRDN(objectClass=gosaAccount)((objectClass=sambaSamAccount))%s)";
}
?>
