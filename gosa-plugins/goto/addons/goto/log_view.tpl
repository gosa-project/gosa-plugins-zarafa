{if !$ACL}

	<h3>{msgPool type=permView}</h3>

{else}
	{if $logs_available}
	<h3>{t}Available logs{/t}</h3>

		<div style="width:99%;border: solid 1px #CCCCCC;">{$divlist}</div>
	  <br>
	  <hr>
		<h3>{t}Selected log{/t}: {$selected_log}</h3>
		<div style="width:99%;height:350px;padding:3px;background-color:white; overflow-y: scroll;border: solid 1px;">
			{$log_file}
		</div>
	{else}
		<h3>{t}No logs for this host available!{/t}</h3>
	{/if}
{/if}

{if $standalone}
<br>
<input type="hidden" name="ignore" value="1">
<hr>
<div style='text-align:right;width:99%; padding-right:5px; padding-top:5px;'>
	<input type='submit' name='abort_event_dialog' value='{msgPool type=backButton}'>
</div>
<br>
{/if}
