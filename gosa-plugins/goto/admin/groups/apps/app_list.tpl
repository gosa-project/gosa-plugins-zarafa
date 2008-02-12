<table style="width:100%;">
	{if $enableReleaseManagement}
	<tr>
		<td>
			{t}Release{/t}
			<select name="FAIrelease" onChange="document.mainform.submit();">
			{foreach from=$releases item=item key=key}
				{if $item.found}
				<option value="{$key}" {if $key == $FAIrelease} selected {/if}>{$item.name} </option>
				{else}
				<option style="color: #999999;" value="{$key}" {if $key == $FAIrelease} selected {/if}>{$item.name} </option>
				{/if}
			{/foreach}
			</select>
		</td>
	</tr>
	{/if}
	<tr>
		<td style="width:50%; vertical-align:top;">
		<div style="height:290px; overflow:auto; 
					border-top: solid 2px #999999;
					border-left: solid 2px #999999;
					padding:5px; background-color: #EEEEEE;">
			
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
			<td style='width:22px; padding-top:5px;padding-bottom:5px;'>
				{if $item.ICON != ""}
					<div style="height:20px;width:20px; overflow:hidden">
					<img class="center" src='?plug={$plug_id}&amp;send={$item.UNIQID}' alt='{t}Folder{/t}'>
					</div>
				{else}
					<div style="height:20px;width:20px; overflow:hidden">
					<img class="center" src='images/folder.png' alt='{t}Folder{/t}'>
					</div>
				{/if}
			</td>
			<td>
				<b>{$item.NAME}</b>
			</td>
			<td style='width:100px;text-align:right'>
				<input title="{t}Move up{/t}" 	class="center" type='image' 
					name='up_{$item.UNIQID}' src='images/move_object_up.png'>
				<input title="{t}Move down{/t}" class="center" type='image' 
					name='down_{$item.UNIQID}' src='images/move_object_down.png'>
				<input title="{t}Remove{/t}" 	class="center" type='image' 
					name='del_{$item.UNIQID}' src='images/edittrash.png'>
				<input title="{t}Edit{/t}" 	 	class="center" type='image' 
					name='app_entry_edit{$item.UNIQID}' src='images/edit.png'>
			</td>
		</tr>
	{elseif $item.TYPE == "ENTRY"}
		<tr>
			<td style='width:22px; padding-top:5px;padding-bottom:5px;'>
				<div style="width:20px; overflow:hidden; text-align:center;">
					<img src='images/select_application.png' alt='{t}Entry{/t}' class="center">
				</div>
			</td>
			<td>
				{$item.NAME} {$item.INFO}
			</td>
			<td style='width:100px;text-align:right'>
				<input title="{t}Move up{/t}" 	class="center" type='image' 
					name='up_{$item.UNIQID}' src='images/move_object_up.png'>
				<input title="{t}Move down{/t}" class="center" type='image' 
					name='down_{$item.UNIQID}' src='images/move_object_down.png'>
				<input title="{t}Remove{/t}" 	class="center" type='image' 
					name='del_{$item.UNIQID}' src='images/edittrash.png'>
				<input title="{t}Edit{/t}" 	 	class="center" type='image' 
					name='app_entry_edit{$item.UNIQID}' src='images/edit.png'>
			</td>
		</tr>
	{/if}
{/foreach}
</table>
		</div>
			<input type="text" name="menu_folder_name" value="">
			<select name="menu_folder">
			{foreach from=$folders item=item key=key}
				<option value="{$key}">{$item}</option>
			{/foreach}
			</select>
			<input type="submit" name="add_menu_to_folder" value="{t}Add{/t}" title="{t}Add selected applications to this folder.{/t}">
		</td>
		<td style="vertical-align:top">
			{$app_list}
			<select name="folder">
			{foreach from=$folders item=item key=key}
				<option value="{$key}">{$item}</option>
			{/foreach}
			</select>
			<input type="submit" name="add_to_folder" value="{t}Add{/t}" title="{t}Add selected applications to this folder.{/t}">
		</td>
	</tr>
</table>
	
