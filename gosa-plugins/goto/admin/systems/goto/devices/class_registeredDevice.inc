<?php


class registeredDevice extends plugin
{




    static function plInfo()
    {
        return (array(
                    "plShortName"   => _("Device"),
                    "plDescription" => _("Registered device"),
                    "plSelfModify"  => FALSE,
                    "plDepends"     => array(),
                    "plPriority"    => 1,
                    "plSection"     => array("administration"),
                    "plCategory"    => array(
                        "registeredDevice" => array( "description"  => _("Registered device"),
                            "objectClass"  => "registeredDevice")),
                    ));
    }
}

?>
