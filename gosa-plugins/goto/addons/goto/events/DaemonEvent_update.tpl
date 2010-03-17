
{if $is_new}

<table style='width:100%;'>
  <tr>
    <td style='width:50%; ' class='right-border'>

      <table>
        <tr>
          <td>
<b>{t}Schedule{/t}</b><br><br>
          {$timestamp}</td>
        </tr>
      </table>
    </td>
    <td style='width:50%; '>

      <table style='width:100%;'>
        <tr>
          <td>
            <b>{t}System list{/t}</b>
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
		<td style='width:50%; '>

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
					<td>{t}Timestamp{/t}
</td>
					<td>{$timestamp}</td>
				</tr>
			</table>
		</td>
		<td style='width:50%; '>

			<table >
			</table>
		</td>
	</tr>
</table>

{/if}
