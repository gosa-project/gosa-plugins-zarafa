
<h3>{t}OPSI host{/t}</h3>

{if $init_failed}

<font style='color: #FF0000;'>{msgPool type=siError p=$message}</font>

<button type='submit' name='reinit'>{t}Retry{/t}</button>


{else}

<table style="width: 100%;">
 <tr>
  <td>
   <table>
    {if $standalone}
    <tr>
     <td>{t}Name{/t}{$must}</td>
     <td>
{render acl=$hostIdACL}
		<input style='width:300px;' type='text' name='hostId' value='{$hostId}'>
{/render}
	 </td>
    </tr>
<!--
    <tr>
     <td>{t}MAC address{/t}{$must}</td>
     <td>
{render acl=$macACL}
		<input type='text' name="dummy" value="{$mac}" disabled>
{/render}
	 </td>
	</tr>
-->
	{else}
    <tr>
     <td>{t}Name{/t}</td>
     <td>
{render acl=$hostIdACL}
		<input style='width:300px;' type='text' disabled value="{$hostId}">
{/render}
	 </td>
    </tr>
<!--
    <tr>
     <td>{t}MAC address{/t}{$must}</td>
     <td>
{render acl=$macACL}
		<input type='text' name="mac" value="{$mac}">
{/render}
	 </td>
    </tr>
-->
    {/if}
    <tr>
     <td>{t}Netboot product{/t}</td>
     <td>
{render acl=$netbootProductACL}
      <select name="opsi_netboot_product" onChange="document.mainform.submit();" size=1>
		{foreach from=$ANP item=item key=key}
			<option {if $key == $SNP} selected {/if} value="{$key}">{$key}</option>
		{/foreach}
      </select>
{/render}
      &nbsp;
      {if $netboot_configurable}
		  {image path="images/lists/edit.png" action="configure_netboot" title="{t}Configure product{/t}">
      {else}
<!--		  <input type='image' name='dummy_10' src='images/lists/edit_gray.png'
			title='{t}Configure product{/t}' class='center'>-->
      {/if}
     </td>
    </tr>
   </table>
  </td>
  <td>

   <table>
    <tr>
     <td>{t}Description{/t}</td>
     <td>
{render acl=$descriptionACL}
		<input type='text' name='description' value='{$description}'>
{/render}
	 </td>
    </tr>
    <tr>
     <td>{t}Notes{/t}</td>
     <td>
{render acl=$descriptionACL}
		<input type='text' name='note' value='{$note}'>
{/render}
	 </td>
    </tr>
   </table>
  </td>
 </tr>
 <tr>
  <td colspan="2">
   <hr>
  </td>
 </tr>
 <tr>
  <td style="width:50%;"><h3>{image path="plugins/opsi/images/product.png"}&nbsp;{t}Installed products{/t}</h3>
{render acl=$localProductACL}
	{$divSLP}
{/render}
  </td>
  <td style="width:50%;"><h3>{t}Available products{/t}</h3>
{render acl=$localProductACL}
	{$divALP}
{/render}
  </td>
 </tr>
 <tr>
  <td colspan="2">
   <hr><br>
   {if $standalone}
    <h3>{t}Action{/t}</h3>
	<select name='opsi_action' size=1>
		<option>&nbsp;</option>
		{if $is_installed}
		<option value="install">{t}Reinstall{/t}</option>
		{else}
		<option value="install">{t}Install{/t}</option>
		{/if}
		<option value="wake">{t}Wake{/t}</option>
	</select>
{render acl=$triggerActionACL}
	<button type='submit' name='opsi_trigger_action'>{t}Execute{/t}</button>

{/render}
   {/if}
  </td>
 </tr>
</table> 
<hr>

{$netconfig}

<input type='hidden' name='opsiGeneric_posted' value='1'>
{/if}
