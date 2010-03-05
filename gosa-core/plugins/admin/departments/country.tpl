<!--////////////////////
	//	COUNTRY (c)
    //////////////////// -->

<table summary="" style="width:100%; vertical-align:top; text-align:left;" cellpadding=4>
 <tr>
   <td style="vertical-align:top; width:50%">
     <h3>{t}Properties{/t}</h3>
     <table summary="">
      <tr>
       <td><LABEL for="c">{t}Country name{/t}</LABEL>{$must}</td>
       <td>
{render acl=$cACL}
    	<input type='text' id="c" name="c" size=25 maxlength=60 value="{$c}" title="{t}Name of country to create{/t}">
{/render}
       </td>
      </tr>
      <tr>
       <td><LABEL for="description">{t}Description{/t}</LABEL>{$must}</td>
       <td>
{render acl=$descriptionACL}
        <input type='text' id="description" name="description" size=25 maxlength=80 value="{$description}" title="{t}Descriptive text for department{/t}">
{/render}
       </td>
      </tr>
	{if !$is_root_dse}
      <tr>
        <td><LABEL for="base">{t}Base{/t}</LABEL>{$must}</td>
        <td>
{render acl=$baseACL}
          {$base}
{/render}
	  </td>
	 </tr>
	{/if}

  {if $manager_enabled}
    <tr>
     <td><label for="manager">{t}Manager{/t}</label></td>
     <td>
{render acl=$managerACL}
        <input type='text' name='manager_name' id='manager_name' value='{$manager_name}' disabled
          title='{$manager}'>
{/render}
{render acl=$managerACL}
        <input type='image' src='images/lists/edit.png' name='editManager' class='center'>
{/render}
        {if $manager!=""}
{render acl=$managerACL}
        <img src='images/info_small.png' title='{$manager}' class='center'>
{/render}
{render acl=$managerACL}
        <input type='image' src='images/lists/trash.png' name='removeManager' class='center'>
{/render}
        {/if}
     </td>
    </tr>
  {/if}
	</table>
  </td>
 </tr>
</table>
<hr>
<table summary="" style="width:100%; vertical-align:top; text-align:left;" cellpadding=4>
 <tr>
   <td style="vertical-align:top; width:100%">
     <h3>{t}Administrative settings{/t}</h3>
{render acl=$gosaUnitTagACL}
     <input id="is_administrational_unit" type=checkbox name="is_administrational_unit" value="1" {$gosaUnitTag}><label for="is_administrational_unit">{t}Tag department as an independent administrative unit{/t}</label>
{/render}
   </td>
  </tr>
</table>
<input type='hidden' name='dep_generic_posted' value='1'>
