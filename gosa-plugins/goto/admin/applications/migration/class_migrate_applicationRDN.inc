<?php 
class migrate_applicationRDN extends migrateRDN implements propertyMigration
{
	protected $filter = "(migrate_applicationRDN(objectClass=gosaAccount)((objectClass=sambaSamAccount))%s)";
}
?>
