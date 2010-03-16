<h3>{t}Apache VHosts{/t}</h3>
<table summary="" width="100%">
<tr>
	<td style="width:100%;vertical-align:top;">
		{$VhostList}

		{render acl=$VirtualHostACL}
		<button type='submit' name='AddVhost'>{t}Add{/t}</button>

		{/render}
	</td>
</tr>
</table>
<script language="JavaScript" type="text/javascript">
  <!-- // First input field on page
  document.mainform.AddVhost.focus();
  -->
</script>
<input type="hidden" name="servapache" value="1">

<hr>
<br>
<div style="width:100%; text-align:right;">
    <button type='submit' name='SaveService'>{msgPool type=saveButton}</button>

    &nbsp;
    <button type='submit' name='CancelService'>{msgPool type=cancelButton}</button>

</div>

