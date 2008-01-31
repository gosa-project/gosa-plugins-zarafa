<table style='width:400px' cellpadding=0 cellspacing=0>
{foreach from=$entries item=item key=key}
	{if $item.TYPE == "OPEN"}
		<tr>
			<td colspan=2 style="background-color: #BBBBBB;height:1px"></td>
		</tr>
		<tr>
			<td style='padding-left:20px;' colspan=2>
				<table style='width:100%;' cellpadding=0 cellspacing=0>

	{elseif $item.TYPE == "CLOSE"}
				</table>
			</td>
		</tr>
		<tr>
			<td colspan=2 style="background-color: #BBBBBB;height:1px"></td>
		</tr>
	{elseif $item.TYPE == "RELEASE"}
		<tr>
			<td style='width:20px; padding-top:5px;padding-bottom:5px;'>
				<img src='images/fai_small.png' alt='{t}Release{/t}'>
			</td>
			<td>
				{$item.NAME}
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
		</tr>
	{elseif $item.TYPE == "ENTRY"}
		<tr>
			<td style='width:20px; padding-top:5px;padding-bottom:5px;'>
				<img src='images/select_application.png' alt='{t}Entry{/t}'>
			</td>
			<td>
				{$item.NAME}
			</td>
		</tr>
	{/if}
{/foreach}
</table>
