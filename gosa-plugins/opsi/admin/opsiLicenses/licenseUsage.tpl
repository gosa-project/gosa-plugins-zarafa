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
    <td style='vertical-align:top;'>

        <!-- GENERIC -->
        <h2>{t}Generic{/t}</h2>
        <table>
          <tr> 
            <td>{t}Name{/t}</td>
            <td>
              TESTER
            </td>
          </tr>
        </table>

    </td>
  </tr>
</table>
<input name='opsiLicenseUsagePosted' value='1' type='hidden'>
{/if}
