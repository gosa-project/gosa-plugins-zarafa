<h3>{t}OPSI product properties{/t}</h3>


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
		<td>
{render acl=$ACL}
			{if isset($item.VALUE_CNT)}
				<select name="value_{$key}" style='width:180px;'>
				{foreach from=$item.VALUE key=k item=i}
					<option {if $item.CURRENT == $i} selected {/if} value="{$i}">{$i}</option>
				{/foreach}
				</select>
			{else}
				<input type='input' name='value_{$key}' value="{$item.CURRENT}" style='width:280px;'>
			{/if}
{/render}
		</td>
	</tr>
{/foreach}
</table>

{/if}
<hr>
<div style='width:100%; text-align: right; padding:3px;'>
	<button type='submit' name='save_properties'>{msgPool type=saveButton}</button>

	&nbsp;
	<button type='submit' name='cancel_properties'>{msgPool type=cancelButton}</button>

</div>
