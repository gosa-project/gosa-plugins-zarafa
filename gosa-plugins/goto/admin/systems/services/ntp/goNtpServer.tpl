<br>
<h3>{t}Time server{/t}</h3>
<br>
<table summary="" style="width:100%">
<tr>
 <td>
{render acl=$goTimeSourceACL}
	<select style="width:100%;" id="goTimeEntry" name="goTimeSource[]" size=8 multiple>
		{html_options values=$goTimeSource output=$goTimeSource}
		<option disabled>&nbsp;</option>
	</select>
{/render}
<br>
{render acl=$goTimeSourceACL}
	<input type="text" name="NewNTPExport"  id="NewNTPExportId">
{/render}
{render acl=$goTimeSourceACL}
	<button type='submit' name='NewNTPAdd' id="NewNTPAddId">{msgPool type=addButton}</button>

{/render}
{render acl=$goTimeSourceACL}
	<button type='submit' name='DelNTPEnt' id="DelNTPEntId">{msgPool type=delButton}</button>

{/render}
</td>
</tr>
</table>

<hr>
<div style="width:100%; text-align:right;padding-top:10px;padding-bottom:3px;">
	<button type='submit' name='SaveService'>{msgPool type=saveButton}</button>

	&nbsp; 
	<button type='submit' name='CancelService'>{msgPool type=cancelButton}</button> 

</div>
