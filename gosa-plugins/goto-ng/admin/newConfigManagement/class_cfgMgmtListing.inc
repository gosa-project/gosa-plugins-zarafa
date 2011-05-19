<?php

class cfgMgmtListing extends listing{

    function renderNavigation()
    {
        $result= array();
        $enableBack = true;
        $enableRoot = true;
        $enableHome = true;

        $listhead ="";
        $enableHome = false;

        /* Draw root button */
        $result["ROOT"]= image('images/lists/root.png', 'ROOT', _("Root"));
        $result["BACK"]= image('images/lists/back.png', 'BACK', _("Go to preceding level"));
        $result["HOME"]= image('images/lists/home.png', 'HOME', _("Go to current users level"));
        $result["RELOAD"]= image('images/lists/reload.png', 'REFRESH', _("Reload list"));

        return ($result);
    }



}

?>
