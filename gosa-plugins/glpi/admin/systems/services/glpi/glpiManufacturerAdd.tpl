<h3>{t}Add/Edit manufacturer{/t}</h3>
<table summary="" width="100%">
	<tr>
		<td width="50%">
			<table summary=""> 
				<tr>
					<td>{t}Name{/t}
					</td>
					<td>
						<input type="text" name="name" value="{$name}">
					</td>
				</tr>
				<tr>
					<td>{t}Website{/t}
					</td>
					<td>
						<input type="text" name="website" value="{$website}">
					</td>
				</tr>
				<tr>
					<td>{t}Address{/t}

					</td>
					<td>
						<textarea  name="address">{$address}</textarea>
					</td>
				</tr>
			</table>
		</td>
		<td>
			<table summary="" >
				<tr>
					<td>{t}Phone number{/t}
					</td>
					<td>
						<input type="text" name="phonenumber" value="{$phonenumber}">
					</td>
				</tr>
				<tr>
					<td>{t}Fax{/t}
					</td>
					<td>
						<input type="text" name="fax" value="{$fax}">
					</td>
				</tr>
				<tr>
					<td>{t}Email{/t}
					</td>
					<td>
						<input type="text" name="email" value="{$email}">
					</td>
				</tr>
				<tr>
					<td>{t}Comments{/t}

					</td>
					<td>
						<textarea name="comments">{$comments}</textarea>
					</td>
				</tr>
			</table>
		</td>
	</tr>
</table>













<br>
<hr>
<div align="right">
<p>
	<button type='submit' name='save_manufacturer'>{msgPool type=saveButton}</button>

	<button type='submit' name='close_manufacturer'>{t}Close{/t}</button>

</p>
</div>
<script language="JavaScript" type="text/javascript">
  <!-- // First input field on page
		focus_field('name');
  -->
</script>

