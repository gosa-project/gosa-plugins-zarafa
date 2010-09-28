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
        $this->itemContainerSelector = new releaseSelector(array('cn=root'=>''), 'cn=root', 'cn=root');
        $this->base = "cn=root";

        $this->navigationList= new sortableListing();
        $this->navigationList->setDeleteable(true);
        $this->navigationList->setEditable(true);
        $this->navigationList->setColspecs(array('*'));
        $this->navigationList->setWidth("100%");
        $this->navigationList->setHeight("140px");
        $this->navigationList->setAcl("rwcdm");
        $this->navigationList->sortingEnabled(FALSE);

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
            return;
        }

        // Set current item to 'root' and append some dummy entries.
        $this->addItem($root,'root',array());
        $this->setCurrentItem('root');
        $this->addItem('PuppetModule','test1',
                array(
                    'dependency' => '',
                    'version' => '2.4-f',
                    'name'  => 'Thundebird',
                    'description' => 'Mozilla mail client')
                );
        $this->addItem('PuppetModule','test2',
                array(
                    'version' => 1,
                    'name'  => 'Firefox',
                    'description' => 'Test Module')
                );
        $this->setCurrentItem('test1');
        $this->addItem('PuppetTemplate','temp1',
                array(
                    'name' => 'temp1', 
                    'file' => 'kekse.tpl')
                );
        $this->setCurrentItem('temp1');
        $this->addItem('PuppetTemplate','tep1',
                array(
                    'name' => 'tep1', 
                    'file' => 'kekse.tpl')
                );

        $this->setCurrentItem('root');
        $this->rebuildListing();
    }


    /*! \brief   Overridden render method of class mangement.
     *            this allows us to add a release selection box.
     */
    function renderList()
    {
        session::set('DEVICE_ITEMS', $this->currentItemValues);
        $this->rebuildListing();
        $filter = $this->getFilter();
        $headpage = $this->getHeadpage();
        $this->itemContainerSelector->update();
        $headpage->update();
        $smarty = get_smarty();
        $smarty->assign("RELEASE", $this->itemContainerSelector->render());
        $display = $headpage->render();
        return($this->getHeader().$display);
    }


    /*! \brief   Build up a list of items useable for the itemSelector.
     */
    function getContainerList($array = NULL, $base = "")
    {
        $array = ($array == NULL)?$this->currentItemValues['root']: $array;
        $ret = array('cn=root' => '');
        $name = $array['name'];
        $ret["cn={$name}{$base}"] = $array['type'];
        if(count($array['children'])){
            $base = ",cn={$name}{$base}";
            foreach($array['children'] as $subItem){
                $ret = array_merge($ret, $this->getContainerList($subItem, $base));
            }
        }
        return($ret);    
    }


    /*! \brief   Update the management class and tell her which 
     *            items are available for the itemSelector (baseSelector).
     */
    function rebuildListing()
    {
        // Collect item container list to be able to render the fake-base selector
        $this->itemContainerSelector->setBases($this->getContainerList());
        $this->itemContainerSelector->update(true);
        $this->itemContainerSelector->setBase($this->base);

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
        $headpage = new listing($contents,TRUE);
        $headpage->setBase($this->base);
        $headpage->setFilter($filter);

        parent::__construct($this->config, $this->ui, "services", $headpage);
    }



    /*! \brief      Prepares an item list which can then be used in 
     *               sortableListings to display the items of this configuration.
     *  @return     Array   Containing 'data' and 'lData' for a sortableListing.
     */
    function getItemList($array=NULL,&$res = NULL, $depth = 0)
    {
        // Prepare values.
        $array = ($array == NULL)? $this->currentItemValues['root']: $array;
        if(!$res){
            $res = array('data' => array(), 'lData' => array());
        }

        // Prepare the entry name
        $str = $array['name'];
        if($this->currentItemName == $array['name']){
            $str = "<b>".$array['name']."</b>";
        }
        $add = '&nbsp;&nbsp;';
        $str = str_pad('', ($depth*strlen($add)), $add, STR_PAD_LEFT).$str;

        // Add the current entry
        $res['data'][] = $array;
        $res['lData'][] = array('data' => array($str, "<i>".$array['type']."</i>"));

        // Add sub-entries
        if(count($array['children'])){
            $depth ++;
            foreach($array['children'] as $subItem){
                $this->getItemList($subItem, $res, $depth);
            } 
        }

        return($res);
    }


    /*! \brief      Add a new child-item to the currently selected one. 
     *               
     *  @param  String  type    The 'type' of the new object, eg. 'KickstartTemplate'
     *  @param  String  name    The 'name' of the new object.
     *  @param  Array   values  The initial values for this object.
     *  @return 
     */
    function addItem($type,$name, $values)
    {
        $current = &$this->currentItem; 
        $this->idToName[] = $name;
        $new = array(
                'children' => array(),
                'type' => $type, 
                'name' => $name, 
                'values' => $values);
        $this->currentItemValues[$name] = $new;
        $current['children'][$name] = &$this->currentItemValues[$name];
    }


    /*! \brief      Selects an item as active and takes care 
     *               of required post/get handling. 
     *  @param  String  The name of the item we want to select.
     *  @return 
     */
    function setCurrentItem($item)
    {
        // Do nothing if we're already where we wanted to switch to.
        if($this->currentItemName == $item) return;

        // Save eventually changed values
        if($this->currentItem){
            foreach($this->TemplateEngine->getWidgets() as $widget){
                $this->currentItem['values'][$widget->getName()] = $widget->getValue();
            }
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




    /*! \brief  Renders the HTML content for the device-config plugin. 
     *  @return String  The generated HTML code. 
     */
    function _execute()
    {
        $smarty = get_smarty();

        // Generate item list
        $list = $this->getItemList();
        $this->navigationList->setListData($list['data'],$list['lData']);
        $this->navigationList->update();
        $smarty->assign('navigationList',$this->navigationList->render());

        // Assign current item info
        $smarty->assign('containerName', $this->currentItemDescriptor['name']);
        $smarty->assign('containerDescription', $this->currentItemDescriptor['description']);

        // Assign the generated HTML of Widgets.
        $smarty->assign('template',$this->TemplateEngine->render());
        return($smarty->fetch(get_template_path('goto/Config/DeviceConfig.tpl', TRUE)));
    }


    /*! \brief  Keep track of posted values, some may be interesting for us. 
     *          Tell the template engine to take care of posted values too.
     *  @param  String
     *  @return 
     */
    function save_object()
    {
        $this->TemplateEngine->save_object();

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
                $this->setCurrentItem($name);
            }
        }

        // Another item selected?
        $this->navigationList->save_object();
        $action = $this->navigationList->getAction();
        if($action['action'] == 'edit'){
            $item = $this->navigationList->getData($action['targets'][0]);
            $this->setCurrentItem($item['name']);
        }

        $this->itemContainerSelector->update();
        $this->base = $this->itemContainerSelector->getBase();
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

    function save(){}
}
?>
