<?php

/*! \brief  A GOsa plugin which generates a device configuration dialog  
 */
class DeviceConfig extends management
{

    // Used to render the item-configuration dialog 
    private $TemplateEngine = NULL;

    // A list of all configured items for this device.
    private $allConfiguredItems = array();

    // The currently active item
    // Add add and remove action will be performed on this item
    private $currentItem = array();
    
    // A baseSelector which will be fed with a simulated 
    //  department list. All entries will get a fake base
    //  which can then be used to fill the baseSelector widget.
    private $itemContainerSelector = NULL;

    // The currently selected base within management list.
    private $base ;

    // This array contains a list of all item-types we can add 
    //  to the currently selected item (currentItem) 
    private $addableContainerItems = array();

    // Due to the fakt that we use a fake 'base/dn' for all items
    //  we've to map this 'base/dn' to the items 'id'.
    private $mappingBaseToID = array();

    private $rootItemID;
    private $lastItemID = 1;

    // Some plugin related memebers which are not member of 
    //  the management class. See class plugin.
    public $is_account = FALSE;
    public $ignore_account = FALSE;
    public $pl_notify;
    public $read_only;


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
        $idRoot = $this->addItem($root,'root',array());
        $this->rootItemID = $idRoot;
        $this->setCurrentItem($idRoot);
        $this->setSelectedListItemID($idRoot);

        // Add some dummy entries for testing.
        $id = $this->addItem('PuppetModule','test1',
                array(
                    'dependency' => array('stulle','Wurst'),
                    'version' => '2.4-f',
                    'name'  => 'Thundebird',
                    'description' => 'Mozilla mail client')
                );
        $id = $this->addItem('PuppetModule','test2',
                array(
                    'dependency' => array('Leipnitz','Dose'),
                    'version' => 1,
                    'name'  => 'Firefox',
                    'description' => 'Test Module')
                );
        $this->setCurrentItem($id);
        $id = $this->addItem('PuppetTemplate','temp1',
                array(
                    'name' => 'temp1', 
                    'data' => 'kekse.tpl')
                );
        $this->setCurrentItem($id);
        $id = $this->addItem('PuppetTemplate','tep1',
                array(
                    'name' => 'tep1',
                    'data' => 'kekse.tpl')
                );

        $this->setCurrentItem($idRoot);
        $this->rebuildListing();
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
            echo "Invalid type {$type}, skipping item!<br>";
            return;
        }

        // Add missing values with the item-type defaults.
        $allValuesSet = TRUE;
        foreach($this->itemConfig[$type]['options'] as $oName => $oValue){
            if(!isset($values[$oName])){
                $values[$oName] = (isset($oValue['default']))?$oValue['default']:"";
            }
        }

        // Get the currently selected entry, its the parent for the newly 
        //  added one. 
        $current = &$this->currentItem; 

        // Create a FAKE base to be able to use the management lists
        //  which are currently ldap and thus dn based.
        $base = (isset($current['base']))? ",".$current['base'] : '';
        $base = "{$type}={$name}{$base}";


        if(isset($this->mappingBaseToID[$base])){
            echo "Cannot add entry, dn already in use! {$base}<br>";
            return(null);
        }
    
        // Get next free item slot.
        $id = ($this->lastItemID ++);
        $new = array(
                'base' => $base,
                'children' => array(),
                'id' => $id, 
                'type' => $type, 
                'name' => $name, 
                'values' => $values);

        // Append the entry to the list of all items.
        $this->allConfiguredItems[$id] = $new;

        // Create a child referenc, this creates some kind of entry tree.
        $current['children'][$id] = &$this->allConfiguredItems[$id];

        // Add entries to the list of base and id mappings
        //  this allows us to easily detect the base for an id and vice versa.
        $this->mappingBaseToID[$id] = $base;
        $this->mappingBaseToID[$base] = $id;
        return($id);
    }


    /*! \brief      Selects an item as active.
     *              All further add and remove actions will be performed
     *               on the obejcts children.
     *  @param  String  The 'id' of the item we want to select.
     *  @return 
     */
    function setCurrentItem($item)
    {
        if(!isset($this->allConfiguredItems[$item])){
            echo "Invalid item name {$name}! Skipping selection!";
            return;
        }

        // Set the new item info.
        $this->currentItem = &$this->allConfiguredItems[$item];
        $this->currentItemType = $this->currentItem['type'];
        $this->currentItemDescriptor = $this->itemConfig[$this->currentItem['type']];
    }


    /*! \brief      Removes a given item ID.
     *  @param  String  The 'id' of the item we want to remove.
     *  @return 
     */
    function removeItem($id, &$data = NULL)
    {
        if($data === NULL){
            $data = &$this->allConfiguredItems;
        }

        // Remove the item and its children
        if(isset($data[$id])){
            foreach($data[$id]['children'] as $cid => $item){
                $this->removeItem($cid, $data);
            }
            unset($data[$id]);
        }
    
        // Remove to current id from sub entries
        foreach($data as $key => $item){
            $this->removeItem($id, $data[$key]['children']);
        }
    }


    /*! \brief      Initiate item edit.
     *              An action send from the management list.
     *  @param  See management::editEntry 
     *  @return 
     */
    function editEntry($action="",$target=array(),$all=array(), 
            $altTabClass ="", $altTabType = "", $altAclCategory="")
    {
        $this->setCurrentItem($target[0]);
        $this->dialogObject = $this->TemplateEngine;
        $this->skipFooter = TRUE;
        $this->dialog = TRUE;

        // Update the template engine to use another type of item and 
        //  some other values.
        $this->TemplateEngine->setValues($this->currentItemType,$this->currentItem['values']);
    }


    /*! \brief      Save changes for the currently edited item.
     */
    function saveItemChanges()
    {
        // Save eventually changed values
        if($this->currentItem){

            // Check if everything is fine.
            $msgs = $this->TemplateEngine->check();
            if(count($msgs)){
                msg_dialog::displayChecks($msgs);
            }else{
                foreach($this->TemplateEngine->getWidgets() as $widget){
                    $this->currentItem['values'][$widget->getName()] = $widget->getValue();
                }
                $this->closeDialogs();
            }
        }
    }


    /*! \brief      React on open requests from the management list 
     */
    function openEntry($action="",$target=array(),$all=array(), 
            $altTabClass ="", $altTabType = "", $altAclCategory="")
    {
        $this->setSelectedListItemID($target[0]);
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
            $this->itemContainerSelector = new ItemSelector(
                    $this->getContainerList(), 
                    $this->base, 
                    $this->allConfiguredItems[$this->rootItemID]['base']);
        }else{
            $this->itemContainerSelector->setBases($this->getContainerList());
        }
        $this->itemContainerSelector->update(true);
        $this->itemContainerSelector->setBase($this->base);

        session::set('DEVICE_ITEMS', $this->allConfiguredItems);
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
        $array = ($array == NULL)?$this->allConfiguredItems[$this->rootItemID]: $array;
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

        // Build up device-list configuration   
        $types ="";
        $images = array();
        $images[] = 'images/lists/edit.png';
        $images[] = 'images/caps.png';
        $images[] = 'images/lists/trash.png';
        $images[] = 'images/filter.png';
        $images[] = 'images/find.png';
        $i = 0; 
        foreach($this->itemConfig as $type => $item){
            $desc = $item['description'];
            $img = $images[$i++];
            $types .= 
                "   <objectType>".
                "     <label>{$desc}</label>".
                "     <objectClass>{$type}</objectClass>".
                "     <category>Device</category>".
                "     <class>dummy</class>".
                "     <image>{$img}</image>".
                "   </objectType>";
        }
        $contents = preg_replace("/%TYPES%/", $types, $contents);


        $items = "";
        $i = 0;
        foreach($this->addableContainerItems as $item){
            $desc = $this->itemConfig[$item]['description'];
            $img = $images[$i++];
            $items .=
                "<action>".
                "    <name>add_{$item}</name>".
                "    <type>entry</type>".
                "    <image>{$img}</image>".
                "    <label>{$desc}</label>".
                "</action>";
        }

        if(!empty($items)){
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

    
    /*! \brief   Saves newly created items and adds them as child to 
     *            the currently selected item.
     */
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

            $this->setCurrentItem($this->mappingBaseToID[$this->base]);
    
            $this->addItem($itemType, $itemName);
            $this->closeDialogs();

#           $this->setCurrentItem($itemName);
#           $this->dialogObject = $this->TemplateEngine;
#           $this->skipFooter = TRUE;
#           $this->dialog = TRUE;

        }
    }


    /*! \brief  Keep track of posted values, some may be interesting for us. 
     *          Tell the template engine to take care of posted values too.
     *  @param  String
     *  @return 
     */
    function save_object()
    {
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

        // Get the selected item-id from the item list and populate it.
        $this->itemContainerSelector->update();
        $id  = $this->mappingBaseToID[$this->itemContainerSelector->getBase()]; 
        $this->setSelectedListItemID($id);
    }


    /* \brief   Updates the currenlty seleted item in the management list
     */   
    function setSelectedListItemID($id)
    {
        $this->base = $this->mappingBaseToID[$id];
        $type = $this->allConfiguredItems[$id]['type'];
        $this->addableContainerItems = $this->itemConfig[$type]['container'];
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
        foreach($this->allConfiguredItems as $name => $item){
            foreach($item['values'] as $oName => $oValue){
                echo "<br>{$name} -- <i>{$item['type']}</i>: <b>{$oName}</b>: {$oValue}";
            }
        }
    }


    /*! \brief  Initiates the removal for the given entries
     *           and displays a confirmation dialog.
     *
     *  @param  String  'action'  The name of the action which was the used as trigger.
     *  @param  Array   'target'  A list of object dns, which should be affected by this method.
     *  @param  Array   'all'     A combination of both 'action' and 'target'.
     */
    protected function removeEntryRequested($action="",$target=array(),$all=array())
    {
        foreach($target as $id){
            $this->removeItem($id);
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
