<table>
	<tr>
		<td>{t}Mask{/t}</td>
		<td><input type="text" name="MASK" value="{$MASK}"></td>
	</tr>
	<tr>
		<td>{t}POLICY{/t}</td>
		<td><input type="text" name="POLICY" value="{$POLICY}"></td>
	</tr>
	<tr>
		<td>{t}REF_COUNT{/t}</td>
		<td><input type="text" name="REF_COUNT" value="{$REF_COUNT}"></td>
	</tr>
	<tr>
		<td>{t}PW_MIN_LENGTH{/t}</td>
		<td><input type="text" name="PW_MIN_LENGTH" value="{$PW_MIN_LENGTH}"></td>
	</tr>
	<tr>
		<td>{t}POLICY_REFCNT{/t}</td>
		<td><input type="text" name="POLICY_REFCNT" value="{$POLICY_REFCNT}"></td>
	</tr>
	<tr>
		<td>{t}PW_HISTORY_NUM{/t}</td>
		<td><input type="text" name="PW_HISTORY_NUM" value="{$PW_HISTORY_NUM}"></td>
	</tr>
	<tr>
		<td>{t}PW_MAX_LIFE{/t}</td>
		<td><input type="text" name="PW_MAX_LIFE" value="{$PW_MAX_LIFE}"></td>
	</tr>
	<tr>
		<td>{t}PW_MIN_CLASSES{/t}</td>
		<td><input type="text" name="PW_MIN_CLASSES" value="{$PW_MIN_CLASSES}"></td>
	</tr>
	<tr>
		<td>{t}PW_MIN_LIFE{/t}</td>
		<td><input type="text" name="PW_MIN_LIFE" value="{$PW_MIN_LIFE}"></td>
	</tr>
</table>
<input type="hidden" name="Policy_Posted" value="1">
<p class="seperator">&nbsp;</p>

<div style="text-align:right; padding:4px;">
	<input type='submit' name="save_policy" value="{msgPool type=okButton}">
	&nbsp;
	<input type='submit' name="cancel_policy" value="{msgPool type=cancelButton}">
</div>
