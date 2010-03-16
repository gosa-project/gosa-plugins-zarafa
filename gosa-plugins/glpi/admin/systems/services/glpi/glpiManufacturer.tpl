<h3>{t}Manage manufacturers{/t}</h3>
<select name="manufacturer" style="width:100%;" size="12">
  {html_options values=$ManuKeys output=$Manus}
</select>
<br>
<button type='submit' name='add_manu'>{msgPool type=addButton}</button>

<button type='submit' name='edit_manu'>{t}Edit{/t}</button>

<button type='submit' name='remove_manu'>{t}Remove{/t}</button>


<hr>
<div align="right">
<p>
	<button type='submit' name='close_edit_manufacturer'>{t}Close{/t}</button>

</p>
</div>
