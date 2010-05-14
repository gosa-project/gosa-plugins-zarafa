<?php 
class migrate_faiScriptRDN extends migrateRDN implements propertyMigration
{
	protected $filter = "(migrate_faiScriptRDN(objectClass=gosaAccount)((objectClass=sambaSamAccount))%s)";
}
?>
