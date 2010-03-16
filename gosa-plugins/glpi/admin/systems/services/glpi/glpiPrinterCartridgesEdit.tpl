<table summary="" title="" style="width: 100%;">
	<tr>
		<td>
			<h3>{t}Generic{/t}</h3>
			<table summary="" title="" width="100%" style="vertical-align:top;">	
				<tr>
					<td>{t}Name{/t}
					</td>
					<td>
						<input type="text" name="name" value="{$name}">
					</td>
				</tr>
				<tr>
					<td>{t}Reference{/t}
					</td>
					<td>
						<input type="text" name="ref" value="{$ref}">
					</td>
				</tr>
			</table>
			<hr>
			<h3>{t}Comments{/t}</h3>
			<table summary="" title="" width="100%" style="vertical-align:top;">	
				<tr>
					<td>{t}Comment{/t}
					</td>
					<td>
						<textarea name="comments" style="width:100%;">{$comments}</textarea>	
					</td>
				</tr>
			</table>
		</td>
		<td style="vertical-align:top;border-left: 1px solid rgb(160, 160, 160);padding-right: 5px;">
			<h3>{t}Generic{/t}</h3>
            <table summary="" title="" width="100%" style="vertical-align:top;">
                <tr>
                    <td>{t}Type{/t}
                    </td>
                    <td>
						<select name="type" >
                            {html_options values=$typeKeys output=$types selected=$type}
                        </select>
                        <button type='submit' name='edit_type_cartridge'>{t}edit{/t}</button>

                    </td>
	    		</tr>
                <tr>
                    <td>{t}Manufacturer{/t}
                    </td>
                    <td>
						<select name="FK_glpi_enterprise">
                            {html_options values=$ManufacturerKeys output=$Manufacturers selected=$FK_glpi_enterprise}
                        </select>
                        <button type='submit' name='edit_manufacturer_cartridges'>{t}edit{/t}</button>

                    </td>
	    		</tr>
				<tr>
                    <td>{t}Technical responsible{/t}&nbsp;
                    </td>
                    <td>
                        <i>{$tech_num}&nbsp; </i>&nbsp;
                        <button type='submit' name='SelectCartridgeTechPerson'>{t}Choose{/t}</button>

                    </td>
                </tr>
			</table>
			<hr>
			{$PrinterTypeMatrix}
		</td>
	</tr>
</table>

<hr>
<div style="text-align:right;">
	<p>
		<button type='submit' name='SaveCartridge'>{msgPool type=saveButton}</button>&nbsp;

		<button type='submit' name='CancelCartridge'>{msgPool type=cancelButton}</button>
<br>
	</p>
</div>
<script language="JavaScript" type="text/javascript">
  <!-- // First input field on page
	focus_field('name');
  -->
</script>

