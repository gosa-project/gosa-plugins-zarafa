<?php 
class migrate_faiProfileRDN extends migrateRDN implements propertyMigration
{
	protected $filter = "(&(objectClass=FAIprofile)%s)";
}
?>
