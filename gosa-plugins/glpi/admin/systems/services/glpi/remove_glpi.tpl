<div style="font-size:18px;">
<img alt="" src="images/warning.png" align=top>&nbsp;{t}Warning{/t}
</div>
<p>
 {$warning}
 {t}Please double check if your really want to do this since there is no way for GOsa to get your data back.{/t}
</p>

<p>
 {t}Best thing to do before performing this action would be to save the current contents of your MySql database in a file. So - if you've done so - press 'Delete' to continue or 'Cancel' to abort.{/t}
</p>

<hr>
<div class="plugin-actions">
  <input type=submit name="delete_glpi_confirm" value="{msgPool type=delButton}">
  <input type=submit name="delete_cancel" value="{msgPool type=cancelButton}">
</div>
