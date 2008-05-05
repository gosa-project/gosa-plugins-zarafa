
<h2>{t}Log view{/t}</h2>

{if $logs_available}
	<table>
		<tr>
			<td>{t}Creation time{/t}:&nbsp;</td>
			<td>
				<select name="selected_date" onChange="document.mainform.submit();">
				{foreach from=$logs.$mac item=stuff key=date}
					{if $date == $selected_date}
						<option value="{$date}" selected>{$stuff.REAL_DATE|date_format:"%d.%m.%Y %H:%m:%S"}</option>
					{else}
						<option value="{$date}">{$stuff.REAL_DATE|date_format:"%d.%m.%Y %H:%m:%S"}</option>
					{/if}
				{/foreach}
				</select>
			</td>
		</tr>
		<tr>
			<td>{t}Log file{/t}:&nbsp;</td>
			<td>
				<select name="selected_file" onChange="document.mainform.submit();">
				{foreach from=$logs.$mac.$selected_date.FILES item=filename}
					{if $filename == $selected_file}
						<option value="{$filename}" selected>{$filename}</option>
					{else}
						<option value="{$filename}">{$filename}</option>
					{/if}
				{/foreach}
				</select>
			</td>
		</tr>
	</table>

	<h2>{t}Log file{/t}</h2>
	<div style="width:100%;height:350px; overflow-y: scroll; background-color:#EEEEEE; border: solid 1px;">
		{$log_file}
	</div>
{else}
	<h2>No logs for this host</h2>
{/if}

<p class='seperator'></p>
<div style='text-align:right;width:100%; padding-right:5px; padding-top:5px;'>
	<input type='submit' name='abort_event_dialog' value='{msgPool type=okButton}'>
</div>
