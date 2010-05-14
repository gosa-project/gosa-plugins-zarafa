<?php 
class migrate_faxBlocklistRDN extends migrateRDN implements propertyMigration
{
	protected $filter = "(migrate_faxBlocklistRDN(objectClass=gosaAccount)((objectClass=sambaSamAccount))%s)";
}
?>
