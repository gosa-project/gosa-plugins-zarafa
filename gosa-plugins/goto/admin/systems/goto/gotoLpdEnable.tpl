
<table style='width:100%;'>
	<tr>
		<td style='width:50%;'>
			<table>
				<tr>
					<td>{t}Device{/t}</td>
					<td>	
						<select name='device'>
							{html_options options=$devices selected=$device}
						</select>
					</td>
				</tr>
				<tr>
					<td>{t}Port{/t}</td>
					<td>
						<input type='text' name='port' value='{$port}'>
					</td>
				</tr>
				<tr>
					<td>{t}Options{/t}</td>
					<td>
						<input type='text' value='{$options}' name='options'>
					</td>
				</tr>
				<tr>
					<td>{t}Write only{/t}</td>
					<td>
						<input type='checkbox' name='writeonly' value='1' >
					</td>
				</tr>
			</table>
		</td>
		<td>
			<table>
				<tr>
					<td>{t}Speed{/t}</td>
					<td></td>
				</tr>
				<tr>
					<td>{t}Flow control{/t}</td>
					<td></td>
				</tr>
				<tr>
					<td>{t}Parity{/t}</td>
					<td></td>
				</tr>
				<tr>
					<td>{t}Bits{/t}</td>
					<td></td>
				</tr>
			</table>
		</td>
	</tr>
</table>

<p class='seperator'>
</p>
<div style='width:100%; text-align: right; padding:5px;'>
	<input type='submit' name='gotoLpdEnable_Ok' value='{msgPool type=okButton}'>&nbsp;
	<input type='submit' name='gotoLpdEnable_Close' value='{msgPool type=cancelButton}'>
</div>
