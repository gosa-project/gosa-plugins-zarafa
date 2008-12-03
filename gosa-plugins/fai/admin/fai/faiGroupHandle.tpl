



<table>
{foreach from=$FAI_group item=item key=key}
	<tr>
		<td>
			{if $mode == "remove"}
				<input type='checkbox' name='{$mode}_{$key}'
         			{if $item.selected} checked {/if}>
			{/if}
		</td>
		<td>
			<img src='{$types.$key.IMG}' alt='{$types.$key.KZL}' title='{$types.$key.NAME}'
				class='center'>
		</td>
		<td>{$types.$key.NAME}</td>
		<td>{$item.description}
		</td>
	</tr>
{/foreach}
</table>
<input type='hidden' value='faiGroupHandle' name='faiGroupHandle'>

<input type='submit'>

<p class='seperator'></div>
<div style='text-align:right; padding:5px'>
	<input type='submit' value='{msgPool type=applyButton}' name='faiGroupHandle_apply'>
	&nbsp;
	<input type='submit' value='{msgPool type=cancelButton}' name='faiGroupHandle_cancel'>
</div>
