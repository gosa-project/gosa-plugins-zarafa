<h2>Opsi product properties</h2>

<table>
{foreach from=$cfg item=item key=key}
	<tr>
		<td>{$key}</td>
		<td><input type='input' name='value_{$key}' value="{$item}"></td>
	</tr>
{/foreach}
</table>
<p class="seperator">&nbsp;</p>
<div style='width:100%; text-align: right; padding:3px;'>
	<input type='submit' name='save_properties' value='{msgPool type='saveButton'}'>
	&nbsp;
	<input type='submit' name='cancel_properties' value='{msgPool type='cancelButton'}'>
</div>
