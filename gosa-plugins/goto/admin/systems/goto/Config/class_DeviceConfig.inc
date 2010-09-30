<?php

/*! \brief  A GOsa plugin which generates a device configuration dialog  
 */
class DeviceConfig extends management
{
    private $TemplateEngine = NULL;
    private $idToName = array();
    private $currentItemName = "";
    private $currentItemValues = array();
    private $currentItem = array();

    private $navigationList = NULL;
    public $pl_notify;
    public $read_only;

    private $itemContainerSelector = NULL;

    private $base ;

    public $is_account = FALSE;
    public $ignore_account = FALSE;

    private $baseToDN = array();

    private $addableContainerItems = array();

    /*! \brief  Constructs the device configuration plugin 
     *  @param  Config  The GOsa configuration object.
     */
    function __construct(&$config, $dn)
    {
        // Load the template engine and tell her what template
        //  to use for the HTML it produces.
        $this->TemplateEngine = new TemplateEngine($config);
        $this->TemplateEngine->setTemplate('puppet.tpl');
        $this->config = $config;

        // Set storage points - We do not have any - We just create a fake list which lists all items
        $this->storagePoints = array("");

        // CREATE Dummy entry 
        $str = file_get_contents('/home/hickert/json.txt');
        $items = json_decode($str, TRUE);

        // Load the item-configuration description to populate the 
        //  the available modules.
        $this->itemConfig = $items['items'];
        $this->TemplateEngine->load($this->itemConfig);

        // Detect root item, its name is / 
        $root = NULL;
        foreach($this->itemConfig as $key => $item){
            if($item['name'] == '/') {
                $root = $key;
                break;
            }
        }
        if(!$root){
            echo 'No root found!';
            $this->rebuildListing();
            $this->is_account = FALSE;
            return;
        }

        $this->is_account = TRUE;

        // Set current item to 'root', this is the minimum to get things running.
        $this->addItem($root,'root',array());
        $this->setCurrentItem('root');
        $this->setBase($this->baseToDN['root']);

        $this->addItem('PuppetModule','test1',
                array(
                    'dependency' => array('stulle','Wurst'),
                    'version' => '2.4-f',
                    'name'  => 'Thundebird',
                    'description' => 'Mozilla mail client')
                );
        $this->addItem('PuppetModule','test2',
                array(
                    'dependency' => array('Leipnitz','Dose'),
                    'version' => 1,
                    'name'  => 'Firefox',
                    'description' => 'Test Module')
                );
        $this->setCurrentItem('test1');
        $this->addItem('PuppetTemplate','temp1',
                array(
                    'name' => 'temp1', 
                    'data' => 'kekse.tpl')
                );
        $this->setCurrentItem('temp1');
        $this->addItem('PuppetTemplate','tep1',
                array(
                    'name' => 'tep1',
                    'data' => 'kekse.tpl')
                );

        $this->setCurrentItem('root');
        $this->rebuildListing();
    }


    function editEntry($action="",$target=array(),$all=array(), 
            $altTabClass ="", $altTabType = "", $altAclCategory="")
    {
        $this->setCurrentItem($target[0]);
        $this->dialogObject = $this->TemplateEngine;
        $this->skipFooter = TRUE;
        $this->dialog = TRUE;
    }


    function saveItemChanges()
    {
        // Save eventually changed values
        if($this->currentItem){
            foreach($this->TemplateEngine->getWidgets() as $widget){
                $this->currentItem['values'][$widget->getName()] = $widget->getValue();
            }
        }
        $this->closeDialogs();
    }


    function openEntry($action="",$target=array(),$all=array(), 
            $altTabClass ="", $altTabType = "", $altAclCategory="")
    {
        $this->setBase($this->currentItemValues[$target[0]]['base']);
    }


    /*! \brief   Overridden render method of class mangement.
     *            this allows us to add a release selection box.
     */
    function renderList()
    {
        // Do we represent a valid account
        if (!$this->is_account){
            $str = "<img alt=\"\" src=\"images/small-error.png\" align=\"middle\">&nbsp;<b>".
                msgPool::noValidExtension("GOsa")."</b>";
            return($str);
        }

        // Collect item container list to be able to render the fake-base selector
        if(!$this->itemContainerSelector){
            $this->itemContainerSelector = new releaseSelector(
                    $this->getContainerList(), 
                    $this->base, 
                    $this->currentItemValues['root']['base']);
        }else{
            $this->itemContainerSelector->setBases($this->getContainerList());
        }
        $this->itemContainerSelector->update(true);
        $this->itemContainerSelector->setBase($this->base);

        session::set('DEVICE_ITEMS', $this->currentItemValues);
        $this->rebuildListing();
        $filter = $this->getFilter();
        $headpage = $this->getHeadpage();

        $headpage->update();
        $smarty = get_smarty();
        $smarty->assign("RELEASE", $this->itemContainerSelector->render());
        $display = $headpage->render();
        return($this->getHeader().$display);
    }


    /*! \brief   Build up a list of items useable for the itemSelector.
     */
    function getContainerList($array = NULL)
    {
        $array = ($array == NULL)?$this->currentItemValues['root']: $array;
        $ret[$array['base']] = $array['type'];
        if(count($array['children'])){
            foreach($array['children'] as $subItem){
                $ret = array_merge($ret, $this->getContainerList($subItem));
            }
        }
        return($ret);    
    }


    /*! \brief   Update the management class and tell her which 
     *            items are available for the itemSelector (baseSelector).
     */
    function rebuildListing()
    {
        // Build filter
        if (session::global_is_set(get_class($this)."_filter")){
            $filter= session::global_get(get_class($this)."_filter");
        } else {
            $filter = new filter(get_template_path("goto/Config/DeviceConfig-filter.xml", true));
            $filter->setObjectStorage($this->storagePoints);
        }
        $this->setFilter($filter);

        // Load service xml file and fill in placeholders
        $contents =file_get_contents(get_template_path("goto/Config/DeviceConfig-list.xml", true));


        $items = "";
        foreach($this->addableContainerItems as $item){
            $desc = $this->itemConfig[$item]['description'];
            $items .=
                "<action>".
                "    <name>add_{$item}</name>".
                "    <type>entry</type>".
                "    <image>images/lists/edit.png</image>".
                "    <label>{$desc}</label>".
                "</action>";
        }

        if(!empty($item)){
            $items = 

                "<action>".
                " <type>sub</type>".
                " <image>images/lists/element.png[new]</image>".
                " <label>Create</label>".
                " {$items}".
                "</action>";
        }

        $contents = preg_replace("/%ITEMS%/", $items, $contents);

        $headpage = new listing($contents,TRUE);
        $headpage->setBase($this->base);
        $headpage->setFilter($filter);

        parent::__construct($this->config, $this->ui, "services", $headpage);

        // Register default actions
        $this->registerAction("new",        "newEntry");
        $this->registerAction("edit",       "openEntry"); // !! We forward 'edit' to 'open' to have a department like navigation.
        $this->registerAction("editEntry",  "editEntry");

        $this->registerAction("saveItemChanges", "saveItemChanges");
        $this->registerAction("cancelItemEdit", "closeDialogs");
        $this->registerAction("cancelItemAdd", "closeDialogs");
        $this->registerAction("saveItemAdd", "saveItemAdd");
        foreach($this->itemConfig as $name => $item){
            $this->registerAction("add_{$name}", "newEntry");
        }
    }


    /*! \brief  This method intiates the object creation.
     *
     *  @param  String  'action'  The name of the action which was the used as trigger.
     *  @param  Array   'target'  A list of object dns, which should be affected by this method.
     *  @param  Array   'all'     A combination of both 'action' and 'target'.
     */
    function newEntry($action="",$target=array(),$all=array(), 
            $altTabClass ="", $altTabType = "", $altAclCategory="")
    {
        $toAdd = preg_replace("/^add_/", "",$action);
        $itemToAdd = $this->itemConfig[$toAdd];

        $this->dialogObject = new AddItemDialog($this->config,$toAdd,$itemToAdd);
        $this->dialog = true;
    }

    
    function saveItemAdd()
    {
        if(!$this->dialogObject instanceOf AddItemDialog) return;

        $msgs = $this->dialogObject->save_object();
        $msgs = $this->dialogObject->check();
        if(count($msgs)){
            msg_dialog::displayChecks($msgs);
        }else{
            $itemName = $this->dialogObject->getName();
            $itemCfg = $this->dialogObject->getItemCfg();
            $itemType = $this->dialogObject->getItemType();

            $this->setCurrentItem($this->baseToDN[$this->base]);
    
            $this->addItem($itemType, $itemName);
            $this->closeDialogs();

#           $this->setCurrentItem($itemName);
#           $this->dialogObject = $this->TemplateEngine;
#           $this->skipFooter = TRUE;
#           $this->dialog = TRUE;

        }
    }


    /*! \brief      Add a new child-item to the currently selected one. 
     *               
     *  @param  String  type    The 'type' of the new object, eg. 'KickstartTemplate'
     *  @param  String  name    The 'name' of the new object.
     *  @param  Array   values  The initial values for this object.
     *  @return 
     */
    function addItem($type,$name, $values = array())
    {
        if(!isset($this->itemConfig[$type])){
            echo "Invalid type {$type}, skipping item!";
            return;
        }

        $allValuesSet = TRUE;
        foreach($this->itemConfig[$type]['options'] as $oName => $oValue){
            if(!isset($values[$oName])){
                $values[$oName] = $oValue;
            }
        }

        $current = &$this->currentItem; 
        $this->idToName[] = $name;

        $base = (isset($current['base']))? ",".$current['base'] : '';
        $base = "cn={$name}{$base}";

        $new = array(
                'base' => $base,
                'children' => array(),
                'type' => $type, 
                'name' => $name, 
                'values' => $values);
        $this->currentItemValues[$name] = $new;
        $current['children'][$name] = &$this->currentItemValues[$name];

        $this->baseToDN[$name] = $base;
        $this->baseToDN[$base] = $name;
    }
    

    /*! \brief      Selects an item as active and takes care 
     *               of required post/get handling. 
     *  @param  String  The name of the item we want to select.
     *  @return 
     */
    function setCurrentItem($item)
    {
        if(!isset($this->currentItemValues[$item])){
            echo "Invalid item name {$name}! Skipping selection!";
            return;
        }

        // Set the new item info.
        $this->currentItemName = $item;
        $this->currentItem = &$this->currentItemValues[$item];
        $this->currentItemType = $this->currentItem['type'];
        $this->currentItemDescriptor =&$this->itemConfig[$this->currentItem['type']];

        // Update the template engine to use another type of item and 
        //  some other values.
        $this->TemplateEngine->setValues($this->currentItemType,$this->currentItem['values']);
    }


    /*! \brief  Keep track of posted values, some may be interesting for us. 
     *          Tell the template engine to take care of posted values too.
     *  @param  String
     *  @return 
     */
    function save_object()
    {
        if($this->dialogObject){
            $this->TemplateEngine->save_object();
        }

        // Add sub-module requested.
        if(isset($_POST['addSubModule']) && isset($_POST['subModule'])){
            $sub = get_post('subModule');
            if(in_array($sub, $this->currentItemDescriptor['container'])){

                // Check if this is a valid item
                if(!isset($this->itemConfig[$sub])) {
                    echo "Invalid item type '{$sub}'!";
                    $values = array();
                }else{
                    $values = $this->itemConfig[$sub]['options'];
                }
                $name = 'test'.rand(0,99999);
                $this->addItem($sub,$name,$values);
            }
        }

        // Get selected Items
        $this->itemContainerSelector->update();
        $this->setBase($this->itemContainerSelector->getBase());
    }

   
    function setBase($base)
    {
        $this->base = $base;
        $item = $this->currentItemValues[$this->baseToDN[$base]];
        $this->addableContainerItems = $this->itemConfig[$item['type']]['container'];
    }


    /*! \brief    Forward plugin acls
     */
    function set_acl_base($base)
    {
        $this->acl_base = $base;
    }


    /*! \brief    Forward plugin acls
     */
    function set_acl_category($category)
    {
        $this->acl_category = $category;
    }

    function save()
    {
        foreach($this->currentItemValues as $name => $item){
            foreach($item['values'] as $oName => $oValue){
                echo "<br>{$name} -- <i>{$item['type']}</i>: <b>{$oName}</b>: {$oValue}";
            }
        }
    }


      // Inject user actions
    function detectPostActions()
    {
        $action = management::detectPostActions();
        if(isset($_POST['saveItemEdit'])) $action['action'] = "saveItemChanges";
        if(isset($_POST['saveItemAdd'])) $action['action'] = "saveItemAdd";
        if(isset($_POST['cancelItemEdit'])) $action['action'] = "cancelItemEdit";
        if(isset($_POST['cancelItemAdd'])) $action['action'] = "cancelItemAdd";

        if($action['action'] == 'remove'){
             echo "Löschen geht nicht!";
             $action['action'] = "";
        }
        return($action);
    }

    
    function closeDialogs()
    {
        parent::closeDialogs();
        $this->dialog = false;
    }
   
 
    function check()
    {
        return(array());
    }
}
?>
