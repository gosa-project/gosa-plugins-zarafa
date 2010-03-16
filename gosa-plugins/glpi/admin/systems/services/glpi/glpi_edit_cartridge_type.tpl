<br>
     <select name="select_type_cartridge" size="12" style="width:100%">
                            {html_options values=$PrinterTypeKeys output=$PrinterTypes}
     </select><br>
	 <input type='text' name="cartridge_type_string">
	 <button type='submit' name='add_cartridge_type'>{msgPool type=addButton}</button>

	 <button type='submit' name='rename_cartridge_type'>{t}Rename{/t}</button>

	 <button type='submit' name='del_cartridge_type'>{msgPool type=delButton}</button>


<hr>
<div align="right">
<p>
<button type='submit' name='close_edit_type_cartridge'>{t}Close{/t}</button>

</p>
</div>
<script language="JavaScript" type="text/javascript">
  <!-- // First input field on page
	focus_field('cartridge_type_string');
  -->
</script>

