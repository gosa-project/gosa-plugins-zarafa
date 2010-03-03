<h3>{t}Mail settings{/t}</h3>
<table style='width:100%;' cellspacing=0>
	<tr>
		<td style='width:50%;vertical-align:top; border-right:1px solid #AAA'>
			<table style="width:100%;">
				<tr>
					<td><label for="mail">{t}Primary address{/t}</label>{$must}</td>
					<td><input type='text' id="mail" name="mail" size=35 maxlength=65 value="{$mail}"></td>
				</tr>
				<tr>
					<td colspan="2">
						<p style="margin-bottom:0px;">
							<b><label for="forwarder_list">{t}Forward messages to{/t}</label></b>
						</p>
					</td>
				</tr>
			</table>
		</td>
		<td style="vertical-align:bottom;border-left:1px solid #A0A0A0">
			<table style="width:100%;">
				<tr>
				 	<td style="vertical-align:top;">
			   			<h3>
							{image path="plugins/mail/images/alternatemail.png"}

							<label for="alternates_list">{t}Alternative addresses{/t}</label>
						</h3>
					</td>
				</tr>
			</table>
		</td>
	</tr>
	<tr>
		<td>
			<table style='width:100%;'>
				<tr>
					<td colspan="2">
					   	<select id="gosaMailForwardingAddress" style="width:100%; height:100px;" name="forwarder_list[]" size=15 multiple >
								{html_options values=$gosaMailForwardingAddress output=$gosaMailForwardingAddress}
								<option disabled>&nbsp;</option>
					   	</select>
						<br>
					    <input type='text' name="forward_address" size=20 align="middle" maxlength=65 	value="">
					    <button type='submit' name='add_forwarder'>{msgPool type=addButton}</button>&nbsp;

					    <button type='submit' name='add_local_forwarder'>{t}Add local{/t}</button>&nbsp;

					    <button type='submit' name='delete_forwarder'>{msgPool type=delButton}</button>

					</td>
				</tr>
			</table>
		</td>
		<td style="vertical-align:bottom;border-left:1px solid #A0A0A0">
			<table style='width:100%;'>
				<tr>
					<td colspan="2" >
					   	<select id="alternates_list" style="width:100%;height:100px;" name="alternates_list[]" size="15"
							 multiple title="{t}List of alternative mail addresses{/t}"> 
								{html_options values=$gosaMailAlternateAddress output=$gosaMailAlternateAddress}
								<option disabled>&nbsp;</option>
					   	</select>
					   	<br/>
					   	<input type='text' name="alternate_address" size="30" align="middle" maxlength="65" value="">
			   			<button type='submit' name='add_alternate'>{msgPool type=addButton}</button>

			   			<button type='submit' name='delete_alternate'>{msgPool type=delButton}</button>

			  		</td>
				</tr>
			</table>
		</td>		
	</tr>
</table>
<br>
<br>
