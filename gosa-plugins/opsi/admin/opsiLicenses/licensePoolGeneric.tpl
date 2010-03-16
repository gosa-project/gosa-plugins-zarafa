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
    <td style='vertical-align:top;'>
        <!-- GENERIC -->
        <h3>{t}Generic{/t}</h3>
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
        <h3>{t}Licenses{/t}</h3>
        <table style='width:100%;'>
          <tr> 
            <td>
              {$licenses}
{render acl=$licensesACL}
              <button type='submit' name='addLicense'>{msgPool type=addButton}</button>

{/render}
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
        <h3>{t}Applications{/t}</h3>
        <table style='width:100%;'>
          <tr> 
            <td>
{render acl=$productIdsACL}
              <select name='productIds[]' multiple size="6" style="width:100%;">
                {html_options options=$productIds}
              </select><br>
{/render}
{render acl=$productIdsACL}
              <select name='availableProduct'>
                {html_options options=$availableProductIds}
              </select>
{/render}
{render acl=$productIdsACL}
              <button type='submit' name='addProduct'>{msgPool type=addButton}</button>

{/render}
{render acl=$productIdsACL}
              <button type='submit' name='removeProduct'>{msgPool type=delButton}</button>

{/render}
            </td>
          </tr>
        </table>

    </td>
    <td style="border-left: 1px solid #AAA; padding: 5px;vertical-align:top">
        <!-- SOFTWARE -->
        <h3>{t}Windows software IDs{/t}</h3>
        <table style='width:100%;'>
          <tr> 
            <td>
{render acl=$windowsSoftwareIdsACL}
              <select name='softwareIds[]' multiple size="6" style="width:100%;">
                {html_options options=$softwareIds}
              </select>
{/render}
<!--
{render acl=$windowsSoftwareIdsACL}
              <input type='text' name='newSoftwareId' value='' size=10>
{/render}
{render acl=$windowsSoftwareIdsACL}
              <input type='submit' name='addSoftware' value='{msgPool type='addButton'}'>
{/render}
{render acl=$windowsSoftwareIdsACL}
              <input type='submit' name='removeSoftware' value='{msgPool type='delButton'}'>
{/render}
-->
            </td>
          </tr>
        </table>

    </td>
  </tr>
</table>
<input name='opsiLicensePoolPosted' value='1' type='hidden'>
{/if}
