<h2><img src='plugins/opsi/images/product.png' class='center' alt=''>&nbsp;{t}Opsi product properties{/t}</h2>


{if $cfg_count == 0}
<br>
<b>{t}This product has no options.{/t}</b>
<br>
<br>

{else}

<table>
{foreach from=$cfg item=item key=key}
	<tr>
		<td>{$key}</td>
		<td><input type='input' name='value_{$key}' value="{$item}"></td>
	</tr>
{/foreach}
</table>

{/if}
<p class="seperator">&nbsp;</p>
<div style='width:100%; text-align: right; padding:3px;'>
	<input type='submit' name='save_properties' value='{msgPool type='saveButton'}'>
	&nbsp;
	<input type='submit' name='cancel_properties' value='{msgPool type='cancelButton'}'>
</div>
