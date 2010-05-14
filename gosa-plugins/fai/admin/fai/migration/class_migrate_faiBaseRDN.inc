<?php 
class migrate_faiBaseRDN extends migrateFaiRDN implements propertyMigration
{
    protected $filter = "(&(objectClass=nothing)%s)";
}
?>
