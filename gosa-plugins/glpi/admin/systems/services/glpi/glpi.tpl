<table summary="" style="width:100%;">
	<tr>
		<td style="width:50%;vertical-align:top;" >
			<!--Upper left-->	
			
			<h3>{t}Generic{/t}</h3>
			<table summary="" cellpadding=5>
				<tr>
					<td >{t}System type{/t}
					</td>
					<td>
{render acl=$typeACL}
						<select name="type">
							{html_options values=$SystemTypeKeys output=$SystemTypes selected=$type}
						</select>
{/render}
{render acl=$typeACL}
						<button type='submit' name='edit_type'>{t}edit{/t}</button>	

{/render}
					</td>
				</tr>
				<tr>
					<td>{t}Operating system{/t}
					</td>
					<td>
{render acl=$osACL}
						<select name="os">
							{html_options values=$OSKeys output=$OSs selected=$os}
						</select>	
{/render}
{render acl=$osACL}
						<button type='submit' name='edit_os'>{t}edit{/t}</button>	

{/render}
					</td>
				</tr>
				<tr>
					<td>{t}Manufacturer{/t}
					</td>
					<td>
{render acl=$FK_glpi_enterpriseACL}
						<select name="FK_glpi_enterprise">
							{html_options values=$ManufacturerKeys output=$Manufacturers selected=$FK_glpi_enterprise}
						</select>	
{/render}
{render acl=$FK_glpi_enterpriseACL}
						<button type='submit' name='edit_manufacturer'>{t}edit{/t}</button>	

{/render}
					</td>
				</tr>
			<!--</table>
			<hr>
			<h3>{t}Contacts{/t}</h3>
			<table summary="" width="100%">-->
				<tr>
					<td>
						{t}Contact person{/t}
					</td>
					<td>
						<i>{$contact_num}&nbsp; </i>&nbsp;
{render acl=$contact_numACL}
						<button type='submit' name='SelectContactPerson'>{t}Edit{/t}</button>

{/render}
					</td>
				</tr>
				<tr>
					<td>{t}Technical responsible{/t}&nbsp;
					</td>
					<td>
						<i>{$tech_num}&nbsp; </i>&nbsp;
{render acl=$tech_numACL}
						<button type='submit' name='SelectTechPerson'>{t}Edit{/t}</button>

{/render}
					</td>
				</tr>
			</table>
		</td>
		<td style="border-left: 1px solid rgb(160, 160, 160); vertical-align: top; padding-right: 5px;">
			<!--Upper right-->
			<h3>{t}Comment{/t}</h3>
			<table summary="" width="100%">
				<tr>
					<td valign="top" colspan="2">
{render acl=$commentsACL}
						<textarea name="comments" style="width:100%;height:180px;">{$comments}</textarea>
{/render}
					</td>
				</tr>
			</table>
		</td>
	</tr>
</table>
<hr>
<table summary="" style="width:100%;">
	<tr>
		<td style="width:50%;">
			<h3>{t}Installed devices{/t}</h3>
			<table summary="" width="100%">	
				<tr>
					<td>
{render acl=$DevicesACL}
						<select name="InstalledDevices" style="height:130px;width:100%;" multiple>
							{html_options values=$InstalledDeviceKeys output=$InstalledDevices}
						</select>
{/render}
{render acl=$DevicesACL}
						<button type='submit' name='AddDevice'>{t}Edit{/t}</button>	

{/render}
					</td>
				</tr>
			</table>
<!--			<hr>
			<input type="submit" value="{t}Trading{/t}" name="Trading">	
			<input type="submit" value="{t}Software{/t}" name="Software">	
			<input type="submit" value="{t}Contracts{/t}" name="Contracts">	-->
		</td>
		<td style="border-left: 1px solid rgb(160, 160, 160); vertical-align: top; padding-right: 5px;">
			<h3>{t}Attachments{/t}</h3>
			<table summary="" width="100%">	
				<tr>
					<td>
{render acl=$AttachmentsACL}
						{$AttachmentsDiv}
{/render}
{render acl=$AttachmentsACL}
						<button type='submit' name='AddAttachment'>{msgPool type=addButton}</button>	

{/render}
					</td>
				</tr>
			</table>
		</td>
	</tr>
</table>
<input type="hidden" name="glpi_tpl_posted" value="1">
<script language="JavaScript" type="text/javascript">
  <!-- // First input field on page
	focus_field('type');
  -->
</script><p style="text-align:right">
