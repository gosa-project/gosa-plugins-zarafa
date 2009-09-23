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
    <td>
        <!-- GENERIC -->
        <h2>{t}Generic{/t}</h2>
        <table>
          <tr> 
            <td>{t}Name{/t}</td>
            <td>
{render acl=$cnACL}
              <input type='text' value='{$cn}' name='cn'>
{/render}
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
  </tr>
  <tr> 
    <td>
        <!-- LICENSES -->
        <h2>{t}Licenses{/t}</h2>
        <table>
          <tr> 
            <td></td>
          </tr>
        </table>

    </td>
  </tr>
  <tr> 
    <td>
        <!-- APPLICATIONS -->
        <h2>{t}Applications{/t}</h2>
        <table>
          <tr> 
            <td></td>
          </tr>
        </table>

    </td>
  </tr>
  <tr> 
    <td>
        <!-- SOFTWARE -->
        <h2>{t}Windows software IDs{/t}</h2>
        <table>
          <tr> 
            <td></td>
          </tr>
        </table>

    </td>
  </tr>
</table>

{/if}
