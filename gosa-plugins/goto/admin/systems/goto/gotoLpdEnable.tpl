{if !$is_account}

{else}
<table style='width:100%;'>
	<tr>
		<td style='width:55%;'>
			<table>
<!--
				<tr>
					<td>{t}Device{/t}&nbsp;#</td>
					<td>
						<select name='DevID' onChange="document.mainform.submit();">
							{foreach from=$data key=key item=item}
	<option {if $key == $DevID} selected {/if} value='{$key}'>{$key+1}: {$item.s_Type} {$item.s_Device}</option>
							{/foreach}
						</select>
						{if $data_cnt >= 3}
						<input type="button" disabled name="dummy1" value="{msgPool type=addButton}">
						{else}
						<input type="submit" name="add_printer" value="{msgPool type=addButton}">
						{/if}
						<input type="submit" name="del_printer" value="{msgPool type=delButton}">
					</td>
				</tr>
-->
				<tr>
					<td>{t}Type{/t}</td>
					<td>	
						<select name='s_Type'  onChange="document.mainform.submit();">
							{html_options options=$a_Types selected=$s_Type}
						</select>
					</td>
				</tr>
				<tr>
					<td>{t}Device{/t}</td>
					<td>	
						<input type='text' name='s_Device' value='{$s_Device}'>
					</td>
				</tr>
				<tr>
					<td>{t}Port{/t}</td>
					<td>
						<input type='text' name='i_Port' value='{$i_Port}'>
					</td>
				</tr>
				<tr>
					<td>{t}Options{/t}</td>
					<td>
						<input type='text' name='s_Options' value='{$s_Options}'>
					</td>
				</tr>
				<tr>
					<td>{t}Write only{/t}</td>
					<td>
						<input {if $s_WriteOnly == "Y"} checked {/if} type='checkbox' name='s_WriteOnly' value='Y' >
					</td>
				</tr>
			</table>
		</td>
		<td>
{if $s_Type == "Serial"}
			<table>
				<tr>
					<td>{t}Bit rate{/t}</td>
					<td>
						<select name='s_Speed'>
							{html_options options=$a_Speeds selected=$s_Speed}
						</select>
					</td>
				</tr>
				<tr>
					<td>{t}Flow control{/t}</td>
					<td>
						<select name='s_FlowControl'>
							{html_options options=$a_FlowControl selected=$s_FlowControl}
						</select>
					</td>
				</tr>
				<tr>
					<td>{t}Parity{/t}</td>
					<td>
						<select name='s_Parity'>
							{html_options options=$a_Parities selected=$s_Parity}
						</select>
					</td>
				</tr>
				<tr>
					<td>{t}Bits{/t}</td>
					<td>
						<select name='i_Bit'>
							{html_options options=$a_Bits selected=$i_Bit}
						</select>
					</td>
				</tr>
			</table>
{/if}
		</td>
	</tr>
</table>
<input type='hidden' name="gotoLpdEnable_entry_posted" value="1">
<p class='seperator'>
</p>
{/if}
<div style='width:100%; text-align: right; padding:5px;'>
	<input type='submit' name='gotoLpdEnable_Ok' value='{msgPool type=okButton}'>&nbsp;
	<input type='submit' name='gotoLpdEnable_Close' value='{msgPool type=cancelButton}'>
</div>
