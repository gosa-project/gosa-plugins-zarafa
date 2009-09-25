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
              {if $initially_was_account}
                <input type='text' value='{$cn}' disabled>
              {else}
{render acl=$cnACL}
              <input type='text' value='{$cn}' name='cn'>
{/render}
              {/if}
            </td>
          </tr>
          <tr> 
            <td>{t}Description{/t}</td>
            <td>
{render acl=$descriptionACL}
              <input type='text' value='{$description}' name='description'>
{/render}
            </td>
          </tr>
        </table>

    </td>
    <td style='width:50%; border-left: 1px solid #AAA;padding: 5px;'>
        <!-- LICENSES -->
        <h2>{t}Licenses{/t}</h2>
        <table style='width:100%;'>
          <tr> 
            <td>
              {$licenses}
              <input type='submit' name='addLicense' value='{msgPool type=addButton}'>
            </td>
          </tr>
        </table>

    </td>
  </tr>
  <tr> 
    <td colspan="2">
      <p class='separator'>&nbsp;</p>
    </td>
  </tr>
  <tr>
    <td style='width:50%'>
        <!-- APPLICATIONS -->
        <h2>{t}Applications{/t}</h2>
        <table style='width:100%;'>
          <tr> 
            <td>
              <select name='productIds[]' multiple size="6" style="width:100%;">
                {html_options options=$productIds}
              </select><br>
              <select name='availableProduct'>
                {html_options options=$availableProductIds}
              </select>
              <input type='submit' name='addProduct' value='{msgPool type='addButton'}'>
              <input type='submit' name='removeProduct' value='{msgPool type='delButton'}'>
            </td>
          </tr>
        </table>

    </td>
    <td style="border-left: 1px solid #AAA; padding: 5px;">
        <!-- SOFTWARE -->
        <h2>{t}Windows software IDs{/t}</h2>
        <table style='width:100%;'>
          <tr> 
            <td>
              <select name='softwareIds[]' multiple size="6" style="width:100%;">
                {html_options options=$softwareIds}
              </select>
              <input type='text' name='newSoftwareId' value='' size=10>
              <input type='submit' name='addSoftware' value='{msgPool type='addButton'}'>
              <input type='submit' name='removeSoftware' value='{msgPool type='delButton'}'>
            </td>
          </tr>
        </table>

    </td>
  </tr>
</table>
<input name='opsiLicensePoolPosted' value='1' type='hidden'>
{/if}
