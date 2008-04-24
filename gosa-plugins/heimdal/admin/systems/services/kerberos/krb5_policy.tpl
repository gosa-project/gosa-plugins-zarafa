<table>
	<tr>
		<td>{t}Policy name{/t}</td>
		<td><input type="text" name="name" value="{$name}"></td>
	</tr>
	<tr>
		<td>{t}Mask{/t}</td>
		<td><input type="text" name="MASK" value="{$MASK}"></td>
	</tr>
	<tr>
		<td>{t}Password minimum length{/t}</td>
		<td><input type="text" name="PW_MIN_LENGTH" value="{$PW_MIN_LENGTH}"></td>
	</tr>
	<tr>
		<td>{t}Password history{/t}</td>
		<td><input type="text" name="PW_HISTORY_NUM" value="{$PW_HISTORY_NUM}"></td>
	</tr>
	<tr>
		<td>{t}Password minimum lifetime{/t}</td>
		<td><input type="text" name="PW_MIN_LIFE" value="{$PW_MIN_LIFE}">{t}seconds{/t}</td>
	</tr>
	<tr>
		<td>{t}Password lifetime{/t}</td>
		<td><input type="text" name="PW_MAX_LIFE" value="{$PW_MAX_LIFE}">{t}seconds{/t}</td>
	</tr>
	<tr>
		<td>{t}Password min characters{/t}</td>
		<td><input type="text" name="PW_MIN_CLASSES" value="{$PW_MIN_CLASSES}"></td>
	</tr>
	<tr>
		<td colspan="2">{t}Number of principals referring to this policy{/t}:&nbsp;{$POLICY_REFCNT}</td>
	</tr>
</table>
<input type="hidden" name="Policy_Posted" value="1">
<p class="seperator">&nbsp;</p>

<div style="text-align:right; padding:4px;">
	<input type='submit' name="save_policy" value="{msgPool type=okButton}">
	&nbsp;
	<input type='submit' name="cancel_policy" value="{msgPool type=cancelButton}">
</div>
