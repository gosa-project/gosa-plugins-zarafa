<div style="font-size:18px;">
  <img alt="" src="images/warning.png" align=top>&nbsp;{t}Warning{/t}
</div>
<p>
 {$intro}
 {t}This may be used by several users/groups. Please double check if your really want to do this since there is no way for GOsa to get your data back.{/t}
</p>
<p>
 {t}So - if you're sure - press 'Delete' to continue or 'Cancel' to abort.{/t}
</p>

<p class="plugbottom">
	{if $multiple}
		<input type=submit name="delete_multiple_device_confirm" value="{msgPool type=delButton}">
		&nbsp;
		<input type=submit name="delete_multiple_device_cancel" value="{msgPool type=cancelButton}">
	{else}
		<input type=submit name="delete_device_confirm" value="{msgPool type=delButton}">
		&nbsp;
		<input type=submit name="delete_cancel" value="{msgPool type=cancelButton}">
	{/if}
</p>

