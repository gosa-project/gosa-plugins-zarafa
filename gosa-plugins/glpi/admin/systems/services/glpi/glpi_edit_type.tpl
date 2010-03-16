<h3>{t}Manage System-types{/t}</h3>
{if $Method == "edit"}

     <select name="select_type" size="12" style="width:100%">
                            {html_options values=$SystemTypeKeys output=$SystemTypes}
     </select><br>
	 <input type='text' name="type_string">
	 <button type='submit' name='add_type'>{msgPool type=addButton}</button>

	 <button type='submit' name='rename_type'>{t}Rename{/t}</button>

	 <button type='submit' name='del_type'>{msgPool type=delButton}</button>


	<hr>
	<div align="right">
	<p>
	<button type='submit' name='close_edit_type'>{t}Close{/t}</button>

	</p>
	</div>
	<script language="JavaScript" type="text/javascript">
	  <!-- // First input field on page
		focus_field('type_string');
	  -->
	</script>
{else}
{t}Please enter a new name{/t}&nbsp;<input type='text' name="string" value="{$string}">
<hr>
	<p>
		<div align="right" style="text-align: right;">
			<button type='submit' name='Rename_type_OK'>{t}Rename{/t}</button>

			<button type='submit' name='Rename_Cancel'>{msgPool type=cancelButton}</button>

		</div>
	</p>
	<script language="JavaScript" type="text/javascript">
	<!-- // First input field on page
		focus_field('string');
	-->
	</script>
{/if}
