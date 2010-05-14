<?php 
class migrate_faiPackageRDN extends migrateRDN implements propertyMigration
{
	protected $filter = "(&(objectClass=FAIpackageList)%s)";
}
?>
