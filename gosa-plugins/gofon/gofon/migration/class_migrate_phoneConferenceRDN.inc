<?php 
class migrate_phoneConferenceRDN extends migrateRDN implements propertyMigration
{
	protected $filter = "(migrate_phoneConferenceRDN(objectClass=gosaAccount)((objectClass=sambaSamAccount))%s)";
}
?>
