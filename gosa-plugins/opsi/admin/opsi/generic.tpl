
<h2><img src='plugins/opsi/images/client_generic.png' alt=' ' class='center'>&nbsp;{t}Opsi host{/t}</h2>

{if $init_failed}

<font style='color: #FF0000;'>{msgPool type=siError p=$message}</font>

<input type='submit' name='reinit' value="{t}Retry{/t}">

{else}

<table style="width: 100%;">
 <tr>
  <td>
   <table>
    {if $parent_mode}
    <tr>
     <td>{t}Name{/t}</td>
     <td><input style='width:300px;' type='text' name='hostId' value='{$hostId}'></td>
    </tr>
    <tr>
     <td>{t}MAC address{/t}</td>
     <td><input type='text' name="mac" value="{$mac}"></td>
	</tr>
	{else}
    <tr>
     <td>{t}Name{/t}</td>
     <td><input style='width:300px;' type='text' disabled value="{$hostId}"></td>
    </tr>
    <tr>
     <td>{t}MAC address{/t}</td>
     <td><input type='text' name="mac" value="{$mac}"></td>
    </tr>
    {/if}
    <tr>
     <td>{t}Boot product{/t}</td>
     <td>
      <select name="opsi_netboot_product">
		{foreach from=$ANP item=item key=key}
			<option {if $key == $SNP} selected {/if} value="{$key}">{$key}</option>
		{/foreach}
      </select>
     </td>
    </tr>
   </table>
  </td>
  <td style='vertical-align: top;'>
   <table>
    <tr>
     <td>{t}Description{/t}</td>
     <td><input type='text' name='description' value='{$description}'></td>
    </tr>
    <tr>
     <td>{t}Notes{/t}</td>
     <td><input type='text' name='note' value='{$note}'></td>
    </tr>
   </table>
  </td>
 </tr>
 <tr>
  <td colspan="2">
   <p class='seperator'>&nbsp;</p>
  </td>
 </tr>
 <tr>
  <td style="width:50%;"><h2><img class='center' src='plugins/opsi/images/product.png' 
		alt=' '>&nbsp;{t}Installed products{/t}</h2>
	{$divSLP}
  </td>
  <td style="width:50%;"><h2>{t}Available products{/t}</h2>
	{$divALP}
  </td>
 </tr>
 <tr>
  <td colspan="2">
   <p class='seperator'>&nbsp;</p><br>
   {if $parent_mode}
    <h2><img src='images/rocket.png' alt="" class="center">&nbsp;{t}Action{/t}</h2>
	<select name='opsi_action'>
		<option>&nbsp;</option>
		{if $is_installed}
		<option value="install">{t}Reinstall{/t}</option>
		{else}
		<option value="install">{t}Install{/t}</option>
		{/if}
	</select>
	<input type='submit' name='opsi_trigger_action' value="{t}Execute{/t}">
   {/if}
  </td>
 </tr>
</table> 
<input type='hidden' name='opsigeneric_posted' value='1'>
{/if}
