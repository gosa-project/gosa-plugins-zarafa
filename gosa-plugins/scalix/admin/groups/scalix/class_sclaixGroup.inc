<?php

class scalixGroup extends scalixAccount
{
	var $scalixGroup = TRUE;
		
  var $attributes           = array("scalixMailnode", "scalixServerLanguage", "scalixLimitMailboxSize", "scalixMailboxClass");
  var $fakeBooleanAttributes= array("scalixAdministrator", "scalixMailboxAdministrator", "scalixLimitOutboundMail",
                                    "scalixLimitInboundMail", "scalixLimitNotifyUser", "scalixHideUserEntry");
  var $objectclasses        = array("scalixGroupClass");
	
}

?>
