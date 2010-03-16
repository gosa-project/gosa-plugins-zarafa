{if !$init_successfull}
<br>
<b>{msgPool type=siError}</b><br>
{t}Check if the GOsa support daemon (gosa-si) is running.{/t}&nbsp;
<button type='submit' name='retry_init'>{t}Retry{/t}</button>

<br>
<br>
{else}


<table width="100%">
  <tr>
    <td style='vertical-align:top;width: 50%; padding-right:5px; border-right: solid 1px #AAA; '>
        <h3>{t}Reserved for{/t}</h3>
{render acl=$boundToHostACL}
        {$licenseReserved}
{/render}
{render acl=$boundToHostACL}
        <select name='availableLicense'>
{/render}
          {html_options options=$availableLicenses}
        </select>
{render acl=$boundToHostACL}
        <button type='submit' name='addReservation'>{msgPool type=addButton}</button>

{/render}
    </td>
    <td style='vertical-align:top;'>
        <h3>{t}Licenses used{/t}</h3>
{render acl=$boundToHostACL}
        {$licenseUses}
{/render}
    </td>
  </tr>
</table>

<input name='opsiLicenseUsagePosted' value='1' type='hidden'>
{/if}
