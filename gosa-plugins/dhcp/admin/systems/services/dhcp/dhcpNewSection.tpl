<div style="font-size: 18px;">
	{t}Create new DHCP section{/t}
</div>
<br>
<p class="seperator">
<b>{t}Please choose one of the following DHCP section types.{/t}</b>
</p>
<br>
{t}Section{/t}&nbsp;
<select size="1" id="section" name="section" title="{t}Choose section type to create{/t}">
  {html_options options=$sections}
</select>

<hr>
<div class="plugin-actions">
  <input type=submit name="create_section" value="{t}Create{/t}">
  <input type=submit name="cancel_section" value="{msgPool type=cancelButton}">
</div>
	
<!-- Place cursor -->
<script language="JavaScript" type="text/javascript">
	<!--
	focus_field('section');
	-->
</script>
