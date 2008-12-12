<?php

/* Remove classes and variables from session
 */
$lock_msg ="";
if ( $cleanup ){
  session::un_set('fonreport');
}else{

	/* Create fonreport object on demand */
	if (!session::is_set('fonreport') || (isset($_GET['reset']) && $_GET['reset'] == 1)){
		session::set('fonreport',new fonreport ($config, $ui));
	}
	$fonreport = session::get('fonreport');

	/* Execute formular */
	$fonreport->save_object();
	$display= $fonreport->execute ();
	$display.= "<input type=\"hidden\" name=\"ignore\">\n";

	/* Page header*/
	$display= print_header(get_template_path('plugins/gofon/images/phonereport.png'), _("Phone reports")).$display;

	/* Store changes  in session */
	session::set('fonreport',$fonreport);
}
?>
