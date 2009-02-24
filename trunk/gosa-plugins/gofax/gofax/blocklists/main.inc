<?php

/* Remove locks created by this plugin
*/
if ($remove_lock){
  if(session::is_set('blocklist')){
      $macl = session::get('blocklist');
      $macl->remove_lock();
  }
}

/* Remove this plugin from session
*/
if ( $cleanup ){
  session::un_set('blocklist');
}else{

	/* Create blocklist object on demand */
	if (!session::is_set('blocklist')){
		$blocklist= new blocklist($config, $ui);
	 	$blocklist->set_acl_category("gofaxlist");
		session::set('blocklist',$blocklist);
	}
	$blocklist = session::get('blocklist');
	$blocklist->save_object();
	$output= $blocklist->execute();

	/* Reset requested? */
	if (isset($_GET['reset']) && $_GET['reset'] == 1){
		session::un_set ('blocklist');
	}

	/* Page header*/
	if (get_object_info() != ""){
	        $display= print_header(get_template_path($blocklist->plIcon), _("Blocklist management"), "<img alt=\"\"align=\"middle\" src=\"".get_template_path('images/lists/locked.png')."\"> ".LDAP::fix(get_object_info()));
	} else {
	        $display= print_header(get_template_path($blocklist->plIcon), _("Blocklist management"));
	}
	$display.= $output;

	session::set('blocklist',$blocklist);
}
?>
