<?php

class mimetabs extends tabs
{
  var $parent = FALSE;

  function mimetabs($config, $data, $dn,$category)
  {
    tabs::tabs($config, $data, $dn,$category);

    /* Add references/acls/snapshots */
    $this->addSpecialTabs();
  }

  function save($ignore_account= FALSE)
  {
    $baseobject= $this->by_object['mimetype'];

    /* Check for new 'dn', in order to propagate the
       'dn' to all plugins */
    if($this->parent->IsReleaseManagementActivated()){
      $new_dn= "cn=".$baseobject->cn.",".$this->parent->mime_release;
    }else{
      $new_dn= "cn=".$baseobject->cn.",".get_ou('mimetypeou').$baseobject->base;
    }

    /* Move group? */
    if ($this->dn != $new_dn){

      /* Write entry on new 'dn' */
      if ($this->dn != "new"){
        $baseobject->move($this->dn, $new_dn);
        $this->by_object['mimetype']= $baseobject;
      }

      /* Happen to use the new one */
      $this->dn= $new_dn;
    }

    tabs::save();
  }

}
// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
?>
