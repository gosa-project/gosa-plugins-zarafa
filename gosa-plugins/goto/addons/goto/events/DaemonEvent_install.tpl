
{if $is_new}

<table style='width:100%;'>
	<tr>
		<td style='width:50%; vertical-align:top;'>
			<table>
				<tr>
					<td style='vertical-align:top'>{t}Timestamp{/t}</td>
					<td>{$timestamp}</td>
				</tr>
			</table>
		</td>
		<td style='width:50%; vertical-align:top;'>
			<table style='width:100%;'>
				<tr>
					<td>
						{t}Target objects{/t}
						<br>
						{$target_list}
					</td>
				</tr>
			</table>
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
					<td>{t}Progress{/t}</td>
					<td>{$progress}</td>
				</tr>
				<tr>
					<td>{t}Status{/t}</td>
					<td>{$status}</td>
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
		<td style='width:50%; vertical-align:top;'>
			<table >
			</table>
		</td>
	</tr>
</table>

{/if}
