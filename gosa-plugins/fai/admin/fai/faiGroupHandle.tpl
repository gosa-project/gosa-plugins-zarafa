
{if $mode == "remove"}

<h2>{t}Remove entry{/t}</h2>
<br>
{t}Select the entries you want to remove.{/t}
<br>
<br>
<table>
{foreach from=$FAI_group item=item key=key}
	<tr>
		<td>
			{if $item.freezed}
				<img src="images/lists/locked.png" class='center'>
			{else}
				<input type='checkbox' name='{$mode}_{$key}'
    	     		{if $item.selected} checked {/if}>
			{/if}
		</td>
		<td>
			<img src='{$types.$key.IMG}' alt='{$types.$key.KZL}' title='{$types.$key.NAME}'
				class='center'>
		</td>
		<td style='width:150px;'>{$types.$key.NAME}</td>
		<td style='width:80px;'>{if $item.freezed}<i>({t}Freezed{/t})</i>{/if}</td>
		<td>{$item.description}</td>
	</tr>
{/foreach}
</table>

{elseif $mode == "edit"}

<h2>{t}Edit entry{/t}</h2>
<br>
{t}Select the entry you want to edit.{/t}
<br>
<br>
<table>
{foreach from=$FAI_group item=item key=key}
	<tr>
		<td>
	        <input type='radio' name='{$mode}_selected' value='{$key}'
               {if $item.selected} checked {/if}>
		</td>
		<td>
			<img src='{$types.$key.IMG}' alt='{$types.$key.KZL}' title='{$types.$key.NAME}'
				class='center'>
		</td>
		<td style='width:150px;'>{$types.$key.NAME}</td>
		<td>{$item.description}
		</td>
	</tr>
{/foreach}
</table>

{elseif $mode == "copy"}

<h2>{t}Copy entries{/t}</h2>
<br>
{t}Select the entry you want to copy.{/t}
<br>
<br>
<table>
{foreach from=$FAI_group item=item key=key}
	<tr>
		<td>
			<input type='checkbox' name='{$mode}_{$key}'
    	   		{if $item.selected} checked {/if}>
		</td>
		<td>
			<img src='{$types.$key.IMG}' alt='{$types.$key.KZL}' title='{$types.$key.NAME}'
				class='center'>
		</td>
		<td style='width:150px;'>{$types.$key.NAME}</td>
		<td>{$item.description}
		</td>
	</tr>
{/foreach}
</table>

{elseif $mode == "cut"}

<h2>{t}Cut entries{/t}</h2>
<br>
{t}Select the entry you want to cut.{/t}
<br>
<br>
<table>
{foreach from=$FAI_group item=item key=key}
	<tr>
		<td>
			{if $item.freezed}
				<img src="images/lists/locked.png" class='center'>
			{else}
				<input type='checkbox' name='{$mode}_{$key}'
    	     		{if $item.selected} checked {/if}>
			{/if}
		</td>
		<td>
			<img src='{$types.$key.IMG}' alt='{$types.$key.KZL}' title='{$types.$key.NAME}'
				class='center'>
		</td>
		<td style='width:150px;'>{$types.$key.NAME}</td>
		<td>{$item.description}
		</td>
	</tr>
{/foreach}
</table>
{/if}
<br>
<br>
<input type='hidden' value='faiGroupHandle' name='faiGroupHandle'>
<p class='seperator'></div>
<div style='text-align:right; padding:5px'>
	<input type='submit' value='{msgPool type=applyButton}' name='faiGroupHandle_apply'>
	&nbsp;
	<input type='submit' value='{msgPool type=cancelButton}' name='faiGroupHandle_cancel'>
</div>
