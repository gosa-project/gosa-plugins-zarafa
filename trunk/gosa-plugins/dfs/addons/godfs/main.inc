<?php


/* Remove locks created by this plugin
*/
if ($remove_lock){
  if(session::is_set('dfsManagment') && session::is_set('edit')){
    $dfsManagment  = session::get('dfsManagment');
    $dfsManagment->remove_lock();
  }
}

/* Remove this plugin from session
*/
if ( $cleanup ){
  session::un_set('dfsManagment');
}else{

     # create dfsManagment object
     if (!session::is_set('dfsManagment')){ 
       session::set('dfsManagment',new dfsManagment($config, $ui));
     }
     $dfsManagment  = session::get('dfsManagment');
     $output = $dfsManagment->execute();

     # do we have to reset
     if (isset($_GET['reset']) && $_GET['reset'] == 1) {
       del_lock ($ui->dn);
       session::un_set('dfsManagment');
     }
 
     # create page header
     if (get_object_info() != "") {
       $display = print_header(get_template_path('plugins/dfs/images/dfs.png'), 
                                _("Distributed File System Administration"),
                                "<img alt=\"\" class=\"center\" src=\"".
                                get_template_path('images/lists/locked.png').
                                "\"> ".get_object_info());
     } else {
       $display = print_header(get_template_path('plugins/dfs/images/dfs.png'),
                                _("Distributed File System Administration"));
     }

     $display.= $output;
    
     # show page
     session::set('dfsManagment',$dfsManagment);
  }

?>
