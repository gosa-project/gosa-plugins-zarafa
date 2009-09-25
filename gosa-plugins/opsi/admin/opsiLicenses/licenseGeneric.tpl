{if !$init_successfull}
<br>
<b>{msgPool type=siError}</b><br>
{t}Check if the GOsa support daemon (gosa-si) is running.{/t}&nbsp;
<input type='submit' name='retry_init' value="{t}retry{/t}">
<br>
<br>
{else}

<h2>{t}License{/t}</h2>

<table style='width:100%'>
  <tr>
    <td style='width:50%; border-right: solid 1px #AAA; padding: 5px; vertical-align:top;'>
        
        <table>
          <tr>
            <td>
              {t}Name{/t}
            </td>
            <td>
              {if $initially_was_account}
                <input type='text' name='dummy12' disabled value='{$cn}'>
              {else}
                <input type='text' name='cn' value='{$cn}'>
              {/if}
            </td>
          </tr>
          <tr>
            <td>
              {t}Partner{/t}
            </td>
            <td>
              <input type='text' name='partner' value='{$partner}'>
            </td>
          </tr>
        </table>

    </td>
    <td style='padding: 5px; vertical-align:top;'>

        <table>
          <tr>
            <td>
              {t}Description{/t}
            </td>
            <td>
              <input type='text' name='description' value='{$description}'>
            </td>
          </tr>
        </table>

    </td>
  </tr>
  <tr>
    <td colspan="2"><p class='separator'>&nbsp;</p></td>
  </tr>
  <tr>
    <td style='border-right: solid 1px #AAA; padding: 5px; vertical-align:top;'>
    
        <table>
          <tr>
            <td>
              {t}Conclusion date{/t}
            </td>
            <td>
              <input type='text' name='conclusionDate' value='{$conclusionDate}'>
            </td>
          </tr>
          <tr>
            <td>
              {t}Expiration date{/t}
            </td>
            <td>
              <input type='text' name='expirationDate' value='{$expirationDate}'>
            </td>
          </tr>
        </table>
 
    </td> 
    <td style='border-right: solid 1px #AAA; padding: 5px; vertical-align:bottom;'>
   
        <table>
          <tr>
            <td>
              {t}Notification date{/t}
            </td>
            <td>
              <input type='text' name='notificationDate' value='{$notificationDate}'>
            </td>
          </tr>
        </table>
    </td> 
  </tr>
</table>

<p class='separator'>&nbsp;</p>

<h2>{t}License model{/t}</h2>

<table width="100%">
  <tr>
    <td style='width:50%;border-right: solid 1px #AAA; padding: 5px; vertical-align:top;'>
    
        <table>
          <tr>
            <td>
              {t}Model{/t}
            </td>
            <td>
              <select name='licenseModel'>
                {html_options options=$licenseModels values=$licenseModels selected=$licenseModel}
              </select>
            </td>
          </tr>
        </table>
 
    </td> 
  </tr>
</table>

<p class='separator'>&nbsp;</p>

<table width="100%">
  <tr>
    <td style='border-right: solid 1px #AAA; padding: 5px; vertical-align:top;'>
    
        <table>
          <tr>
            <td>
              {t}License key{/t}
            </td>
            <td>
              <input type='text' name='licenseKey' value='{$licenseKey}'>
            </td>
          </tr>
          <tr>
            <td>
              {t}Maximum installations{/t}
            </td>
            <td>
              <input type='text' name='maximumInstallations' value='{$maximumInstallations}'>
            </td>
          </tr>
          <tr>
            <td>
              {t}Reserved for Host{/t}
            </td>
            <td>
              <select name='boundToHost'>
                <option value="">{t}none{/t}</option>
                {html_options options=$hosts selected=$boundToHost}
              </select>
            </td>
          </tr>
        </table>
 
    </td> 
    <td style='border-right: solid 1px #AAA; padding: 5px; vertical-align:bottom;'>
   
        <table width="100%">
          <tr>
            <td colspan="2">
              <b>{t}Assigned to Host{/t}</b><br>
              <select name='usedByHost[]' multiple size=4 style='width:100%;'>
                {html_options options=$usedByHost}
              </select><br>
              <select name='selectedHostToAdd'>
                {html_options options=$notUsedHosts}
              </select>
              <input type="submit" name="addLicenseUsage" value="{msgPool type='addButton'}">
              <input type="submit" name="removeLicenseUsage" value="{msgPool type='delButton'}">
            </td>
          </tr>
        </table>
    </td> 
  </tr>
</table>
<input name='opsiLicensesPosted' value='1' type='hidden'>
{/if}
