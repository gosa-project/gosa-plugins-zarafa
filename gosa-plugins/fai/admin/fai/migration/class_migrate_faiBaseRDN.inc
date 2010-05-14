<?php 
class migrate_faiBaseRDN extends migrateFaiRDN implements propertyMigration
{
    protected $filter = "(&(objectClass=nothing)%s)";



    function checkForIssues()
    {
    
        // Check every single property which is connected to this property.
        $ret = array('add' => array(), 'move' => array());
        foreach(array(
                    'faiScript'=> 'faiScriptRDN',
                    'faiHook'=> 'faiHookRDN',
                    'faiTemplate'=> 'faiTemplateRDN',
                    'faiVariable'=> 'faiVariableRDN',
                    'faiProfile'=> 'faiProfileRDN',
                    'faiPartitionTable'=> 'faiPartitionRDN',
                    'faiPackage'=> 'faiPackageRDN') as $class => $name){

            if($this->config->configRegistry->propertyExists($class,$name)){
                $prop = $this->config->configRegistry->getProperty($class,$name);
                if($prop->migrationRequired()){
                    $migrator = $prop->getMigrationClass();
                    if($migrator){
                        $tmp = $migrator->getChanges();
                        if(isset($tmp['add'])){
                            $ret['add'] = array_merge($tmp['add'],$ret['add']);
                        }
                        if(isset($tmp['move'])){
                            $ret['move'] = array_merge($tmp['move'],$ret['move']);
                        }
                    }
                }
            }
        }
        $this->found = $ret;
        return(count($ret['add']) || count($ret['move']));
    }
}
?>
