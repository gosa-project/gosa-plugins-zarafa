{if !$init_successfull}
<br>
<b>{msgPool type=siError}</b><br>
{t}Check if the GOsa support daemon (gosa-si) is running.{/t}&nbsp;
<input type='submit' name='retry_init' value="{t}retry{/t}">
<br>
<br>
{else}


<table width="100%">
  <tr>
    <td style='vertical-align:top;width: 50%; padding: 5px; border-right: solid 1px #888888; '>
        <h2>{t}Licenses used{/t}</h2>
        {$licenseUses}
    </td>
    <td style='vertical-align:top;'>
        <h2>{t}Licenses reserved for this host{/t}</h2>
        {$licenseReserved}
        <select name='availableLicense'>
          {html_options options=$availableLicenses}
        </select>
        <input type='submit' name='addReservation' value='{msgPool type=addButton}'>
    </td>
  </tr>
</table>

<input name='opsiLicenseUsagePosted' value='1' type='hidden'>
{/if}
