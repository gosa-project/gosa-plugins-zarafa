<?php 
class migrate_faiBaseRDN extends migrateRDN implements propertyMigration
{
    protected $filter = "(&(objectClass=nothing)%s)";
}
?>
