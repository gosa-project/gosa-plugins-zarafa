<table style='width:100%; border:1px solid #B0B0B0;' summary="">

	<tr style="background-color: #E8E8E8; height:26px;font-weight:bold;">
		<td align=left width="100%">
			<LABEL for="storage_base">
				{$storage_info}
			</LABEL>
			<select id="storage_base" name="storage_base" size="1" title="{t}Choose the department to store entry in{/t}">
				{html_options options=$deplist selected=$storage_base}
			</select>
		</td>
		<td>
			<a href="{$url}">
				X
			</a>
		</td>
	</tr>
	<tr style="background-color: white">
		<td colspan=2>
			<table summary="" width="100%" cellspacing=2 cellpadding=4>
				<tr>
					<td style='width:50%; background-color: #F0F0F0'>

						<h3>
							{image path="{$personal_image}"}

								&nbsp;{t}Personal{/t}
						</h3>
 						<table summary="" width="100%">
  							<tr>
   								<td>
									<LABEL for="sn">
										{t}Last name{/t}{$must}
									</LABEL>, 
									<LABEL for="givenName">
										{t}First name{/t}{$must}
									</LABEL>
								</td>
   								<td>
{render acl=$snACL}	
									<input type='text' id="sn" name="sn" size=10 maxlength=60 value="{$info_sn}">, 
{/render}
{render acl=$givenNameACL}
									<input type='text' id="givenName" name="givenName" size=10 maxlength=60 value="{$info_givenName}">
{/render}
								<td>
  							</tr>
  							<tr>
   								<td>
									<LABEL for="initials">
										{t}Initials{/t}
									</LABEL>
								</td>
								<td>
{render acl=$initialsACL}
									<input type='text' id="initials" name="initials" size=5 maxlength=20 value="{$info_initials}">
{/render}
								</td>
  							</tr>
  							<tr>
   								<td>
									<LABEL for="title">
										{t}Personal title{/t}
									</LABEL>
								</td>
								<td>
{render acl=$titleACL}
									<input type='text' id="title" name="title" size=10 maxlength=20 value="{$info_title}">
{/render}
								</td>
  							</tr>
 						</table>
					</td>
					<td style='width:50%; background-color:#E8E8E8'>

 						<h3>
							{image path="{$home_image}"}

							&nbsp;{t}Private{/t}
						</h3>
						<table summary="" width="100%">
						  	<tr>
						   		<td>
									<LABEL for="homePostalAddress">
										{t}Address{/t}
									</LABEL>
								<br>
								<br>
								</td>
						   		<td>

{render acl=$homePostalAddressACL}
									<textarea id="homePostalAddress" name="homePostalAddress" rows=1 cols=20>{$info_homePostalAddress}</textarea>
{/render}
								</td>
						  	</tr>
						  	<tr>
						   		<td>
									<LABEL for="homePhone">
										{t}Phone{/t}
									</LABEL>
								</td>
								<td>
{render acl=$homePhoneACL}
									<input type='text' id="homePhone" name="homePhone" size=15 maxlength=60 value="{$info_homePhone}">
{/render}
								</td>
						  	</tr>
						  	<tr>
						   		<td>
									<LABEL for="mobile">
										{t}Mobile{/t}
									</LABEL>
								</td>
								<td>
{render acl=$mobileACL}
									<input type='text' id="mobile" name="mobile" size=15 maxlength=60 value="{$info_mobile}">
{/render}
								</td>
						  	</tr>
						  	<tr>
						   		<td>
									<LABEL for="mail">
										{t}Email{/t}
									</LABEL>
								</td>
								<td>
{render acl=$mailACL}
									<input type='text' id="mail" name="mail" size=15 maxlength=60 value="{$info_mail}">
{/render}
								</td>
						  	</tr>
						 </table>
					</td>
				</tr>
				<tr>
					<td style='width:100%; background-color: #E4E4E4' colspan="2">

 						<h3>
							{image path="{$company_image}"}

							&nbsp;{t}Organizational{/t}
						</h3>
 						<table summary="" width="100%">
							<tr>
						   		<td>
									<table summary="" width="100%">
									 	<tr>
									  		<td>
												<LABEL for="">	
													{t}Company{/t}
												</LABEL>
											</td>
											<td>
{render acl=$oACL}
												<input type='text' id="o" name="o" size=15 maxlength=60 value="{$info_o}">
{/render}
											</td>
									 	</tr>
									 	<tr>
									  		<td>
												<LABEL for="">
													{t}Department{/t}
												</LABEL>
											</td>
											<td>
{render acl=$ouACL}
												<input type='text' id="ou" name="ou" size=15 maxlength=60 value="{$info_ou}">
{/render}
											</td>
									 	</tr>
									 	<tr>
									  		<td>
												<LABEL for="">
													{t}City{/t}
												</LABEL>
											</td>
											<td>
{render acl=$lACL}
												<input type='text' id="l" name="l" size=15 maxlength=60 value="{$info_l}">
{/render}
											</td>
									 	</tr>
									 	<tr>
									  		<td>
												<LABEL for="">
													{t}Postal code{/t}
												</LABEL>
											</td>
											<td>
{render acl=$postalCodeACL}
												<input type='text' id="postalCode" name="postalCode" size=15 maxlength=60 value="{$info_postalCode}">
{/render}
											</td>
									 	</tr>
									 	<tr>
									  		<td>
												<LABEL for="">
													{t}Country{/t}
												</LABEL>
											</td>
											<td>
{render acl=$stACL}
												<input type='text' id="st" name="st" size=15 maxlength=60 value="{$info_st}">
{/render}
											</td>
									 	</tr>
									</table>
								   	</td>
								   	<td>

									<table summary="" width="100%">
										<tr>
											<td>
												<LABEL for="">
													{t}Address{/t}
												</LABEL>
												<br>
												<br>
											</td>
											<td>

{render acl=$postalAddressACL}
												<textarea id="postalAddress" name="postalAddress" rows=1 cols=20 >{$info_postalAddress}</textarea>
{/render}
											</td>
										</tr>
										<tr>
										  	<td>
												<LABEL for="">
													{t}Phone{/t}	
												</LABEL>
											</td>
											<td>
{render acl=$telephoneNumberACL}
												<input type='text' id="telephoneNumber" name="telephoneNumber" size=15 maxlength=60 value="{$info_telephoneNumber}">
{/render}
											</td>
										</tr>
										<tr>
										  	<td>
												<LABEL for="">
													{t}FAX{/t}
												</LABEL>
											</td>
											<td>
{render acl=$facsimileTelephoneNumberACL}
												<input id="facsimileTelephoneNumber" name="facsimileTelephoneNumber"
													size=15 maxlength=60 value="{$info_facsimileTelephoneNumber}">
{/render}
											</td>
										</tr>
										<tr>
											<td>
												<LABEL for="">
													{t}Pager{/t}
												</LABEL>
											</td>
											<td>
{render acl=$pagerACL}
												<input type='text' id="pager" name="pager" size=15 maxlength=60 value="{$info_pager}">
{/render}
											</td>
										</tr>
									</table>
								</td>
							</tr>
						</table>
					</td>
				</tr>
			</table>

			<p align=right>
			<button type='submit' name='save'>{msgPool type=saveButton}</button>

			<button type='submit' name='cancel'>{msgPool type=cancelButton}</button>

			</p>

		</td>
	</tr>
</table>

<!--
// vim:tabstop=2:expandtab:shiftwidth=2:filetype=php:syntax:ruler:
-->
