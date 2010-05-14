<?php 
class migrate_faiProfileRDN extends migrateRDN implements propertyMigration
{
	protected $filter = "(migrate_faiProfileRDN(objectClass=gosaAccount)((objectClass=sambaSamAccount))%s)";
}
?>
