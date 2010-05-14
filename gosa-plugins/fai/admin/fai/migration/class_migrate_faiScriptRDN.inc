<?php 
class migrate_faiScriptRDN extends migrateRDN implements propertyMigration
{
	protected $filter = "(&(objectClass=FAIscript)%s)";
}
?>
