
{if $is_new}

<table style='width:100%;'>
	<tr>
		<td style='vertical-align:top'>{t}Send on:{/t}</td>
		<td>{$timestamp}</td>
	</tr>
	<tr>
		<td>
			{t}Subject{/t}
		</td>
		<td>
			<input type="text" value="{$subject}" name="subject">
		</td>
	</tr>
	<tr>
		<td colspan="2">
			{t}Text{/t}
		</td>
	</tr>
	<tr>
		<td colspan="2">
			<textarea style="height:200px;width:100%;" name="message">{$message}</textarea>
		</td>
	</tr>
</table>

{else}

<table style='width:100%;'>
	<tr>
		<td style='width:50%; vertical-align:top;'>
			<table>
				<tr>
					<td>{t}ID{/t}</td>
					<td>{$data.ID}</td>
				</tr>
				<tr>
					<td>{t}Status{/t}</td>
					<td>{$data.STATUS}</td>
				</tr>
				<tr>
					<td>{t}Result{/t}</td>
					<td>{$data.RESULT}</td>
				</tr>
				<tr>
					<td>{t}Target{/t}</td>
					<td>{$data.MACADDRESS}</td>
				</tr>
				<tr>
					<td style='vertical-align:top'>{t}Timestamp{/t}</td>
					<td>{$timestamp}</td>
				</tr>
			</table>
		</td>
	</tr>
</table>

{/if}
