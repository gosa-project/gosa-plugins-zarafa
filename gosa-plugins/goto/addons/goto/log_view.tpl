
<h2>{t}Log view{/t}</h2>

{if $logs_available}
	<div style="width:100%;border: solid 1px #CCCCCC;">{$divlist}</div>
	<h2>{t}Log file{/t}</h2>
	<div style="width:100%;height:350px; overflow-y: scroll;border: solid 1px;">
		{$log_file}
	</div>
{else}
	<h2>No logs for this host</h2>
{/if}

<p class='seperator'></p>
<div style='text-align:right;width:100%; padding-right:5px; padding-top:5px;'>
	<input type='submit' name='abort_event_dialog' value='{msgPool type=okButton}'>
</div>
