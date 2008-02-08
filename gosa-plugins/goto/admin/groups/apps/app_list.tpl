<table style="width:100%;">
	<tr>
		<td>
			<select name="FAIrelease" onChange="document.mainform.submit();">
			{foreach from=$releases item=item key=key}
				<option value="{$key}" {if $key == $FAIrelease} selected {/if}>{$item.name}</option>
			{/foreach}
			</select>
		</td>
	</tr>
	<tr>
		<td style="width:50%; vertical-align:top;">
			
<table style='width:100%' cellpadding=0 cellspacing=0>
{foreach from=$entries item=item key=key}
	{if $item.TYPE == "OPEN"}
		<tr>
			<td colspan=3 style="background-color: #BBBBBB;height:1px"></td>
		</tr>
		<tr>
			<td style='padding-left:20px;' colspan=3>
				<table style='width:100%;' cellpadding=0 cellspacing=0>

	{elseif $item.TYPE == "CLOSE"}
				</table>
			</td>
		</tr>
		<tr>
			<td colspan=3 style="background-color: #BBBBBB;height:1px"></td>
		</tr>
	{elseif $item.TYPE == "RELEASE"}
		<tr>
			<td style='width:20px; padding-top:5px;padding-bottom:5px;'>
				<img src='images/fai_small.png' alt='{t}Release{/t}'>
			</td>
			<td>
				{$item.NAME}
			</td>
			<td style='width:100px;text-align:right'>
			</td>
		</tr>
	{elseif $item.TYPE == "FOLDER"}
		<tr>
			<td style='width:20px; padding-top:5px;padding-bottom:5px;'>
				<img src='images/folder.png' alt='{t}Folder{/t}'>
			</td>
			<td>
				{$item.NAME}
			</td>
			<td style='width:100px;text-align:right'>
				<input title="{t}Move up{/t}" 	class="center" type='image' 
					name='up_{$item.UNIQID}' src='images/move_object_up.png'>
				<input title="{t}Move down{/t}" class="center" type='image' 
					name='down_{$item.UNIQID}' src='images/move_object_down.png'>
				<input title="{t}Remove{/t}" 	class="center" type='image' 
					name='del_{$item.UNIQID}' src='images/edittrash.png'>
				<input title="{t}Edit{/t}" 	 	class="center" type='image' 
					name='edit_{$item.UNIQID}' src='images/edit.png'>
			</td>
		</tr>
	{elseif $item.TYPE == "ENTRY"}
		<tr>
			<td style='width:20px; padding-top:5px;padding-bottom:5px;'>
				<img src='images/select_application.png' alt='{t}Entry{/t}'>
			</td>
			<td>
				{$item.NAME}
			</td>
			<td style='width:100px;text-align:right'>
				<input title="{t}Move up{/t}" 	class="center" type='image' 
					name='up_{$item.UNIQID}' src='images/move_object_up.png'>
				<input title="{t}Move down{/t}" class="center" type='image' 
					name='down_{$item.UNIQID}' src='images/move_object_down.png'>
				<input title="{t}Remove{/t}" 	class="center" type='image' 
					name='del_{$item.UNIQID}' src='images/edittrash.png'>
				<input title="{t}Edit{/t}" 	 	class="center" type='image' 
					name='edit_{$item.UNIQID}' src='images/edit.png'>
			</td>
		</tr>
	{/if}
{/foreach}
</table>
		</td>
		<td style="vertical-align:top">
			{$app_list}
		</td>
	</tr>
</table>
	
