<h2>{t}Opsi product properties{/t}</h2>


{if $cfg_count == 0}
<br>
<b>{t}This OPSI product has no options.{/t}</b>
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
{if $cfg_count != 0}
	<input type='submit' name='save_properties' value='{msgPool type='saveButton'}'>
	&nbsp;
{/if}
	<input type='submit' name='cancel_properties' value='{msgPool type='cancelButton'}'>
</div>
