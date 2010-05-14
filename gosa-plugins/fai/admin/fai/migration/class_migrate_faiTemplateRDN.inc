<?php 
class migrate_faiTemplateRDN extends migrateRDN implements propertyMigration
{
	protected $filter = "(migrate_faiTemplateRDN(objectClass=gosaAccount)((objectClass=sambaSamAccount))%s)";
}
?>
