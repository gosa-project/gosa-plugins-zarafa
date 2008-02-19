<table cellspacing="0" cellpadding="0">
	<tr>
		<td>{t}Jahr{/t}</td>
		<td>{t}Monat{/t}</td>
		<td>{t}Tag{/t}</td>
		<td>{t}Stunde{/t}</td>
		<td>{t}Minute{/t}</td>
		<td>{t}Sekunde{/t}</td>
	</tr>
	<tr>
		<td>
			<select name="time_year" onChange="document.mainform.submit();">
				{html_options values=$years options=$years selected=$time_year}
			</select>&nbsp;
		</td>
		<td>
			<select name="time_month" onChange="document.mainform.submit();">
				{html_options values=$months options=$months selected=$time_month}
			</select>&nbsp;
		</td>
		<td>
			<select name="time_day">
				{html_options values=$days options=$days selected=$time_day}
			</select>&nbsp;
		</td>
		<td>
			<select name="time_hour">
				{html_options values=$hours options=$hours selected=$time_hour}
			</select>&nbsp;
		</td>
		<td>
			<select name="time_minute">
				{html_options values=$minutes options=$minutes selected=$time_minute}
			</select>&nbsp;
		</td>
		<td>
			<select name="time_second">
				{html_options values=$seconds options=$seconds selected=$time_second}
			</select>
		</td>
	</tr>
</table>

