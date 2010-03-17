<table summary="" style="width:100%;">
	<tr>
		<td style='width:50%;'>

			<!--Upper left-->	
			
			<h3>{t}Generic{/t}</h3>
			<table summary="" cellpadding=5>
				<tr>
					<td width="150">{t}Printer type{/t}
					</td>
					<td>
{render acl=$typeACL}
						<select name="type"  size=1>
							{html_options values=$PrinterTypeKeys output=$PrinterTypes selected=$type}
						</select>
{/render}
{render acl=$typeACL}
						<button type='submit' name='edit_type'>{t}edit{/t}</button>	

{/render}
					</td>
				</tr>
				<tr>
					<td>{t}Manufacturer{/t}
					</td>
					<td>
{render acl=$FKglpienterpriseACL}
						<select name="FK_glpi_enterprise"  size=1>
							{html_options values=$ManufacturerKeys output=$Manufacturers selected=$FK_glpi_enterprise}
						</select>	
{/render}
{render acl=$FKglpienterpriseACL}
						<button type='submit' name='edit_manufacturer'>{t}edit{/t}</button>	

{/render}
					</td>
				</tr>
			</table>
		</td>
		<td style='padding-right: 5px;' class='left-border'>

			<h3>{t}Supported interfaces{/t}</h3>
			<table summary="" width="100%">
				<tr>
					<td width="20">
{render acl=$flags_serialACL}
						<input type="checkbox" name="flags_serial" {if $flags_serial=="1"} checked {/if} value="1" >
{/render}
					</td>
					<td>
						{t}Serial{/t}
					</td>
				</tr>
				<tr>
					<td width="20">
{render acl=$flags_parACL}
						<input type="checkbox" name="flags_par" {if $flags_par=="1"} checked {/if} value="1" >
{/render}
					</td>
					<td>
						{t}Parallel{/t}
					</td>
				</tr>
				<tr>
					<td width="20">
{render acl=$flags_usbACL}
						<input type="checkbox" name="flags_usb" {if $flags_usb=="1"} checked {/if} value="1" >
{/render}
					</td>
					<td>
						{t}USB{/t}
					</td>
				</tr>
			</table>
			<input name="glpiPrinterFlagsPosted" value="1" type="hidden">
		</td>
	</tr>
</table>
<p class="seperator" >&nbsp;</p>
<table summary="" style="width:100%;">
	<tr>
		<td style='width:50%;'>

			<h3>{t}Contacts{/t}</h3>
			<table summary="" cellpadding=5>
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
			</table>
		</td>
		<td style='padding-right: 5px;' class='left-border'>

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
<p class="seperator" >&nbsp;</p>
<table summary="" style="width:100%;">
	<tr>
		<td style='width:50%;'>

			<h3>{t}Information{/t}</h3>
			<table summary="" width="100%">
				<tr>
					<td width="100%">
{render acl=$commentsACL}
						<textarea name="comments" style="width:100%;height:100px;" >{$comments}</textarea>
{/render}
					</td>
				</tr>
			</table>
		</td>
		<td style='padding-right: 5px;' class='left-border'>

			<h3>{t}Installed cartridges{/t}</h3>
			<table summary="" width="100%">	
				<tr>
					<td>
{render acl=$CartridgesACL}
						<select name="Cartridges[]"  style="height:100px;width:100%;" multiple size=1>
							{html_options values=$CartridgeKeys output=$Cartridges}
						</select>
{/render}
{render acl=$CartridgesACL}
						<button type='submit' name='AddCartridge'>{msgPool type=addButton}</button>	

{/render}
{render acl=$CartridgesACL}
						<button type='submit' name='RemoveCartridge'>{t}Remove{/t}</button>	

{/render}
					</td>
				</tr>
			</table>
		</td>
	</tr>
</table>
