<?php

/*! \brief  This class allows to manage backend config items and packages.
 */
class newConfigManagement extends plugin
{
    var $initTime;
    var $plHeadline = "Config management";
    var $plDescription = "Config management";

    var $selectedContainer;

    var $dataModel = NULL;
    var $listing = NULL;

    var $cfgTypeMap = NULL;
    var $cfgItemMap = NULL;

    var $addableContainerItems = array();
    var $currentObject = NULL;


    /*! \brief  Initialize the plugin and finally update the data model.
     */
    function __construct($config, $dn)
    {
        $this->config = &$config;
        $this->listing = new ConfigManagementListing($this->config, get_userinfo(), $this);

        // Load the template engine and tell her what template
        //  to use for the HTML it produces.
        $this->TemplateEngine = new TemplateEngine($config);

        // Request an update of the data model
        $this->loadInstallationMethods();
        $this->updateDataModel();
        $this->listing->setListingTypes($this->getListingTypes());
    }


    /*! \brief  Sets the installation method to the given method.
     *          Updates the template engine and adds the initial root
     *           object for the selected method.
     *  @param  The method to use.
     *  @return TRUE on success else FALSE.
     */
    function setInstallMethod($str)
    {
        if(!isset($this->installationMethods[$str])){
            $this->itemConfig = array();
            $this->invalidInstallMethod =TRUE;
            $this->errorMessage = sprintf(_("Invalid installation method %s selected!"), bold($str));
            msg_dialog::display(_("Setup"), $this->errorMessage, ERROR_DIALOG);
            return(FALSE);
        }else{
    
            $this->TemplateEngine->setTemplate($str.".tpl");
            $this->itemConfig = $this->installationMethods[$str]['items'];
            $this->invalidInstallMethod =FALSE;
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
                $this->errorMessage = sprintf(_("Installation method %s is invalid: no root object found!"), bold($str));
                msg_dialog::display(_("Setup"), $this->errorMessage , ERROR_DIALOG);
                $this->initFailed = TRUE;
                $this->itemConfig = array();
                return(FALSE);
            }
        }
    }


    /*! \brief  Updates all distributions, releases, packages and items in the dataModel
     *          Load information from the backend.
     */
    function updateDataModel()
    {
        // Recreate the data model, to have a clean and fresh instance.
        $this->dataModel = new ConfigManagementDataModel();

        // Load distributions 
        $rpc = $this->config->getRpcHandle();
        $res = $rpc->getDistributions();

        if(!$rpc->success()){
            msg_dialog::display(_("Error"), sprintf(_("Failed to load distributions: %s"), $rpc->get_error()),ERROR_DIALOG);
            return(NULL);
        }else{
            $this->cfgTypeMap = array();
            foreach($res as $dist){
                $dist['__removeable'] = TRUE;
                $this->dataModel->addItem('Distribution','/root', $dist['name'], $dist);
                $this->cfgTypeMap['/root/'.$dist['name']] = $dist['installation_method'];
                foreach($dist['releases'] as $release){
                    $distPath = "/root/{$dist['name']}";
                    $release['__removeable'] = TRUE;
                    $this->dataModel->addItem('Release',$distPath, $release['name'], $release);
                }
            }
        }
    }


    /*! \brief  Keep track of posted values and populate those 
     *           which are interesting for us.
     *          Inspects the _POST and _GET values.
     */
    function save_object()
    {
        // Update the listing class, this is necessary to get post
        //  actions from it.
        $this->listing->save_object();

        // Get the selected distribution and release from the listing widget.
        $cont = $this->listing->getSelectedContainer();
        if(isset($_POST['ROOT'])){
            $this->setCurrentContainer('/root');
        }elseif(isset($_POST['BACK'])){

            $path = $this->selectedContainer;
            if($this->dataModel->itemExistsByPath($path)){
                $data = $this->dataModel->getItemByPath($path);
                if($data['parentPath']){
                    $this->setCurrentContainer($data['parentPath']);
                }
            }
        }else{
            $this->setCurrentContainer($cont);
        }
    }


    /*! \brief  Load extended sub-objecte like 'config items' or 'packages'
     *           for the given release path.
     *  @param  String  The release path to load sub-objects for.
     *  @return NULL 
     */
    function updateItemList($path)
    {
        // Fist get Item and check if it is an release 
        if($this->dataModel->itemExistsByPath($path)){
            $data = $this->dataModel->getItemByPath($path);

            // Only releases can contain config-items.
            if($data['type'] == 'Release' && $data['status'] != "fetched"){

                $rpc = $this->config->getRpcHandle();
                $res = $rpc->listConfigItems($data['name']);
                if(!$rpc->success()){
                    msg_dialog::display(_("Error"), 
                            sprintf(_("Failed to load distributions: %s"), 
                                $rpc->get_error()),ERROR_DIALOG);
                }else{

                    // Sort entries by path length 
                    $sLen = array();
                    foreach($res as $itemPath => $type){
                        $sLen[strlen($itemPath)."_".$itemPath] = $itemPath;
                    }
                    uksort($sLen, "strnatcasecmp");   

                    // Walk through each entry and then try to add it to the model
                    foreach($sLen as $unused => $itemPath){

                        // Do not add the root element '/'
                        if($itemPath == "/") continue;

                        $type = $res[$itemPath];
                
                        // Root installation objects do not have a name, so we use 'root' here.
                        $targetPath = trim($path."/".$itemPath);

                        // Remove trailing and duplicated slashes
                        $targetPath = rtrim($targetPath, '/');
                        $targetPath = preg_replace("/\/\/*/","/", $targetPath);

                        // Extract the items name
                        $name = preg_replace("/^.*\//","", $targetPath);
    
                        // Cleanup the path and then add the item.
                        $targetPath = preg_replace("/[^\/]*$/","", $targetPath);
                        $targetPath = rtrim($targetPath,'/');
                        $this->dataModel->addItem($type, $targetPath, $name, 
                                array(    
                                        '__editable' => TRUE,
                                        '__removeable' => TRUE,
                                        '__path' => $itemPath,
                                        '__release' => $path
                                    ),'-' ); 
                    }
                    $this->dataModel->setItemStatus($path, 'fetched');
                }
            }
        }
    }


    /*! \brief  Sets the currently selected container and item path. 
     *  @param  String  The path of the container to set.
     *  @param  String  The path of the item to set.
     *  @return 
     */
    function setCurrentContainer($cont)
    {
        $this->selectedContainer = $cont;

        // Update list of items within the selected container. 
        $this->updateItemList($this->selectedContainer);

        // Transfer checked values back to the listing class.
        $this->listing->setContainers($this->getContainerList());
        $this->listing->setContainer($cont);

        // Set the correct installation method for the selected item
        if(isset($this->cfgTypeMap[$cont])){
            $method = $this->cfgTypeMap[$cont];
            $this->setInstallMethod($method);
        }

        // Update the list of addable sub objects
        $item = $this->dataModel->getItemByPath($cont);
        if(isset($this->itemConfig[$item['type']]['container'])){
            $this->addableContainerItems = $this->itemConfig[$item['type']]['container'];
        }else{
            $this->addableContainerItems = array();
        }
    }


    /*! \brief  Generate the HTML content for this plugin.
     *          Actually renders the listing widget..
     */
    function execute()
    {
        // Get the selected release and store it in a session variable
        //  to allow the configFilter to access it and display the
        //  packages and items.
        $res = $this->listing->execute();
        $this->listing->setListingTypes($this->getListingTypes());
        return($res);
    }


    /*! \brief      Returns a list of items which will then be displayed 
     *               in the management-list. 
     *              (The management class calls this method from its execute())
     *  @return     Array   A list of items/objects for the listing. 
     */
    function getItemsToBeDisplayed()
    {

        $path = $this->selectedContainer;
        $item = $this->dataModel->getItemByPath($path);
        return($item);
    }


    /*! \brief  Returns a simply list of all distributions.
     *          This list will then be used to generate the entries of the 
     *           ItemSelectors in the listing class.
     */
    function getContainerList()
    {
        $data = $this->dataModel->getItemByPath('/root');
        $res = array();
        $res["/root"] = array("name" => "/", "desc" => "");
        $res = array_merge($res,$this->__recurseItem($data, array('Distribution','Release')));
        return($res);
    }


    /*! \brief  Recursivly walks through an item and collects all path and name info.
     *          The reult can then be used to fill the ItemSelector.
     *  @param  Array   The Item to recurse. 
     *  @param  Array   The type of of objects to collect. 
     *  @param  String  The parent path prefix which should be removed.
     *  @return Array   An array containing Array[path] = name
     */
    function __recurseItem($item, $types, $parent = "")
    {
        $res = array();
        if(1 ||  in_array($item['type'], $types)){
            $path = preg_replace("/".preg_quote($parent,'/')."/","",$item['path']);
            $res[$path] = array('name' => $item['name'],'desc'=>$item['type']);
        }
        if(count($item['children'])){
            foreach($item['children'] as $child){
                $res = array_merge($res, $this->__recurseItem($child, $types, $parent));
            }
        }
        return($res);
    }


    /*! \brief  Intializes this plugin
     *          All available installation methods will be loaded
     */
    function loadInstallationMethods()
    {
        // Reset erros
        $this->rpcError = $this->initFailed = FALSE;

        // Load configuration via rpc.
        $rpc = $this->config->getRpcHandle();

        // Populate install methods on success.
        $res = $rpc->getSupportedInstallMethods();
        if(!$rpc->success()){
            $this->rpcError = TRUE;
            $this->errorMessage = $rpc->get_error();;
            return;
        }
        $this->installationMethods = $res;

        if(!count($this->installationMethods)){
            $this->errorMessage = _("No selectable install methods returned!");
            msg_dialog::display(_("Setup"), $this->errorMessage , ERROR_DIALOG);
            $this->initFailed = TRUE;
            return;
        }else{
            $this->cfgItemMap = array();
            foreach($this->installationMethods as $method => $items){
                foreach($items['items'] as $itemName => $item){
                    $this->cfgItemMap[$itemName] = $method;
        
                    // This enables us to create the first level of config items when 
                    //  a release is selected.
                    if($item['name'] == "/"){
                        $this->installationMethods[$method]['items']['Release'] = 
                            &$this->installationMethods[$method]['items'][$itemName];
                    }
                }
            }
        }
    }


    /*! \brief   Returns a info list about all items we can manage,
     *            this used to fill the listings <objectType> settings.
     *  @return Array   An array with item info.
     */
    function getListingTypes()
    {
        $types= array();
        $types['Distribution']['objectClass'] = 'Distribution';
        $types['Distribution']['label'] = _('Distribution');
        $types['Distribution']['image'] = 'images/lists/edit.png';
        $types['Distribution']['category'] = 'Device';
        $types['Distribution']['class'] = 'Device';

        $types['Release']['objectClass'] = 'Release';
        $types['Release']['label'] = _('Release');
        $types['Release']['image'] = 'images/lists/delete.png';
        $types['Release']['category'] = 'Device';
        $types['Release']['class'] = 'Device';

        $types['Component']['objectClass'] = 'Component';
        $types['Component']['label'] = _('Component');
        $types['Component']['image'] = 'plugins/users/images/select_user.png';
        $types['Component']['category'] = 'Device';
        $types['Component']['class'] = 'Device';

        foreach($this->installationMethods as $method => $items){
            foreach($items['items'] as $itemName => $item){
                $types[$itemName]['objectClass'] = $itemName;
                $types[$itemName]['label'] = $item['name'];
                $types[$itemName]['image'] = 'plugins/fai/images/fai_script.png';
                $types[$itemName]['category'] = 'Device';
                $types[$itemName]['class'] = 'Device';
            }
        }

        return($types);
    }


    /*! \brief      The plugins ACL and plugin-property definition. 
     *  @return 
     */
    public static function plInfo()
    {
        return (array(
                    "plShortName"   => _("Config management"),
                    "plDescription" => _("Config management"),
                    "plSelfModify"  => FALSE,
                    "plDepends"     => array(),
                    "plPriority"    => 0,
                    "plSection"     => array("administration"),
                    "plCategory"    => array(
                        "newConfigManagement" => array("description"  => _("Config management"),
                            "objectClass"  => "FAKE_OC_newConfigManagement")),
                    "plProvidedAcls"=> array()
                    ));
    }


    /*! \brief  Acts on open requests.
     *          (This action is received from the ConfigManagementListing class.)
     *  @param  Array   The items ids. (May contain multiple ids)
     *  @return
     */
    function openEntry($ids)
    {
        $id = $ids[0];
        $item = $this->dataModel->getItemById($id);
        $this->setCurrentContainer($item['path']);
        return;
    }



    /*! \brief   
     *  @param  
     *  @return 
     */
    function remove_lock()
    {
    }


    function editEntry($ids)
    {
        // Update the template engine to use another type of item and
        //  some other values.
        $item = $this->dataModel->getItemById($ids[0]);

        if(isset($this->cfgItemMap[$item['type']])){
            $release = preg_replace("/^.*\//","",$item['values']['__release']);
            $path = $item['values']['__path'];
            $method = $this->cfgItemMap[$item['type']];

            // Load item values on demand
            if($item['status'] == '-'){
                $rpc = $this->config->getRpcHandle();
                $item['values']['itemValues'] = $rpc->getConfigItem($release, $path);
                $this->dataModel->setItemStatus($item['path'], 'fetched');
                $this->dataModel->setItemValues($item['path'], $item['values']);
            }

            $this->setInstallMethod($method);
            $this->TemplateEngine->setValues($item['type'],$item['values']['itemValues']);
            $this->listing->setDialogObject($this->TemplateEngine);
            $this->currentObject = $item;
        }
    }

    function saveItemChanges()
    {
        $item = $this->currentObject;
        $this->listing->clearDialogObject();
        $this->TemplateEngine->save_object();
        $values = array();
        foreach($this->TemplateEngine->getWidgets() as $w){
            $values[$w->getName()] = $w->getValue();
        }
        $release = preg_replace("/^.*\//","",$item['values']['__release']);
        $path = $item['values']['__path'];
        $method = $this->cfgItemMap[$item['type']];

        $rpc = $this->config->getRpcHandle();
        $res = $rpc->setConfigItem($release, $path, $item['type'], $values);
        if(!$rpc->success()){
            msg_dialog::display(_("Error"), sprintf(_("Failed to load distributions: %s"), $rpc->get_error()),ERROR_DIALOG);
            return(NULL);
        }else{
            $item['values']['itemValues'] = $values;
            $this->dataModel->setItemValues($item['path'], $item['values']);
        }
    }
}
?>
