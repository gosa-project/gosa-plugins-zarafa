<h3>{t}Manage System-types{/t}</h3>
{if $Method == "edit"}

     <select name="select_type" size="12" style="width:100%">
                            {html_options values=$SystemTypeKeys output=$SystemTypes}
     </select><br>
	 <input type='text' name="type_string">
	 <input type="submit" name="add_type" 		value="{msgPool type=addButton}" >
	 <input type="submit" name="rename_type" 	value="{t}Rename{/t}" >
	 <input type="submit" name="del_type" 		value="{msgPool type=delButton}" >

	<hr>
	<div align="right">
	<p>
	<input name="close_edit_type" value="{t}Close{/t}" type="submit">
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
			<input type="submit" name="Rename_type_OK" 		value="{t}Rename{/t}" >
			<input type="submit" name="Rename_Cancel" 	value="{msgPool type=cancelButton}" >
		</div>
	</p>
	<script language="JavaScript" type="text/javascript">
	<!-- // First input field on page
		focus_field('string');
	-->
	</script>
{/if}
