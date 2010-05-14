<?php 
class migrate_faiVariableRDN extends migrateRDN implements propertyMigration
{
	protected $filter = "(migrate_faiVariableRDN(objectClass=gosaAccount)((objectClass=sambaSamAccount))%s)";
}
?>
