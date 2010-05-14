<?php 
class migrate_phoneMacroRDN extends migrateRDN implements propertyMigration
{
	protected $filter = "(migrate_phoneMacroRDN(objectClass=gosaAccount)((objectClass=sambaSamAccount))%s)";
}
?>
