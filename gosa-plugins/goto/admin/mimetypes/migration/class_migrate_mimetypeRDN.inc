<?php 
class migrate_mimetypeRDN extends migrateRDN implements propertyMigration
{
	protected $filter = "(migrate_mimetypeRDN(objectClass=gosaAccount)((objectClass=sambaSamAccount))%s)";
}
?>
