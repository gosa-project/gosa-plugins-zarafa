<table summary="" style="width:100%;">
  <tbody>
    <tr>
      <td style="width:50%;">
        <h3>&nbsp;{t}Generic{/t}</h3>
        <table>
          <tbody>
            <tr>
              <td>
                <label for="mail">{t}Primary address{/t}</label>
              </td>
              <td>
                <input disabled type='text' id="dummy" name="dummy" value="{$mail}">
              </td>
            </tr>
            <tr>
              <td>
                <label for="gosaMailServer">{t}Server{/t}</label>
              </td>
              <td>
                <input disabled type='text' id="dummy2" name="dummy2" value="{$gosaMailServer}">
              </td>
            </tr>
            <tr>
              <td>
                <label for="zarafaUserArchiveServers">{t}Archive server{/t}</label>
              </td>
              <td>
                <input type='text' id="zarafaUserArchiveServers" name="zarafaUserArchiveServers" value="{$zarafaUserArchiveServers}">
              </td>
            </tr>
          </tbody>
        </table>
      </td>
      <td class="left-border">&nbsp;</td>
      <td>
        <h3><label for="alternates_list"> {t}Alternative addresses{/t}</label></h3>
        {render acl=$gosaMailAlternateAddressACL}
        <select id="alternates_list" style="width:100%;height:100px;" name="dummy4"
        title="{t}List of alternative mail addresses{/t}" multiple>
        {html_options values=$gosaMailAlternateAddress output=$gosaMailAlternateAddress}
        <option disabled>&nbsp;</option>
        </select>
        {/render}
      </td>
    </tr>
    <tr>
      <td colspan="3">
        <hr/>
      </td>
    </tr>
    <tr>
      <td>
        <h3>&nbsp;{t}Zarafa specific settings{/t}</h3>
        {render acl=$zarafaAccountACL checked=$use_zarafaAccount}
        <input class="center" id='zarafaAccount'
        type="checkbox" name="zarafaAccount" value="1" {$zarafaAccount} 
        onclick="changeState('zarafaAdmin', 'zarafaSharedStoreOnly', 'zarafaHidden',
                             'zarafaResourceCapacity','zarafaSendAsPrivilege','zarafaResourceType', 'zarafaQuotaHard',
                             'zarafaQuotaSoft', 'zarafaQuotaWarn', 'zarafaQuotaOverride', 'add_local_sendas','delete_sendas');"
        title="{t}Enable Zarafa Account for this user{/t}"> {t}Enable Zarafa{/t}
        {/render}
        <br/>
        {render acl=$zarafaAdminACL checked=$use_zarafaAdmin}
        <input type="checkbox" name="zarafaAdmin" id="zarafaAdmin" value="1" {$zarafaAdmin} onclick="changeState('zarafaSharedStoreOnly')" {if !$zarafaAccount || $zarafaSharedStoreOnly} disabled {/if}
        title="{t}Decide if the current user is an Administrator{/t}"> {t}Zarafa Administrator{/t}</input>
        {/render}
        <br/>
        {render acl=$zarafaSharedStoreOnlyACL checked=$use_zarafaSharedStoreOnly}
        <input type="checkbox" name="zarafaSharedStoreOnly" id="zarafaSharedStoreOnly" value="1" {$zarafaSharedStoreOnly}  onclick="changeState('zarafaAdmin')" {if !$zarafaAccount || $zarafaAdmin} disabled {/if}
        title="{t}User is only a shared store{/t}"> {t}Zarafa shared store{/t}</input>
        {/render}
        <br/>
        {render acl=$zarafaHiddenACL checked=$use_zarafaHidden}
        <input id='zarafaHidden'
        type="checkbox" name="zarafaHidden" value="1" {$zarafaHidden} {if !$zarafaAccount} disabled {/if}
        title="{t}Hide from Zarafa addressbook{/t}"> {t}Hide from addressbook{/t}
        {/render}
        <br>
        {render acl=$zarafaQuotaOverrideACL checked=$use_zarafaQuotaOverride}
        <input type="checkbox" name="zarafaQuotaOverride" id="zarafaQuotaOverride" value="1" {$zarafaQuotaOverride} onclick="changeState('zarafaQuotaHard', 'zarafaQuotaSoft', 'zarafaQuotaWarn')" {if !$zarafaAccount} disabled {/if}
        title="{t}Override default quota settings{/t}"> {t}Override default quota{/t}</input>
        {/render}
        <br/>
      </td>
      <td class="left-border">&nbsp;</td>
      <td>
        <h3><label for="forwarder_list">{t}Forward messages to{/t}</label></h3>
        {render acl=$gosaMailForwardingAddressACL}
        <select id="gosaMailForwardingAddress" style="width:100%; height:100px;" name="dummy3" multiple>
        {html_options values=$gosaMailForwardingAddress output=$gosaMailForwardingAddress}
        <option disabled>&nbsp;</option>
        </select>
        {/render}
      </td>
    </tr>
    <tr>
      <td colspan="3">
        <hr/>
      </td>
    </tr>
    <tr>
      <td>
        <table>
          <tr>
            <td><label for="zarafaQuotaHard">{t}Hard quota size{/t}</label></td>
            <td>
              {render acl=$zarafaQuotaACL}
                <input type="text" id="zarafaQuotaHard" name="zarafaQuotaHard" {if !$zarafaAccount || !$zarafaQuotaOverride} disabled {/if} value="{$zarafaQuotaHard}"> MB
              {/render}
            </td>
          </tr>
          <tr>
            <td><label for="zarafaQuotaSoft">{t}Soft quota size{/t}</label></td>
            <td>
              {render acl=$zarafaQuotaACL}
              <input  type="text"  id="zarafaQuotaSoft" name="zarafaQuotaSoft" {if !$zarafaAccount || !$zarafaQuotaOverride} disabled {/if} value="{$zarafaQuotaSoft}"> MB
              {/render}
            </td>
          </tr>
          <tr>
            <td><label for="zarafaQuotaWarn">{t}Warn quota size{/t}</label></td>
            <td>
              {render acl=$zarafaQuotaACL}
                <input  type="text" id="zarafaQuotaWarn" name="zarafaQuotaWarn" {if !$zarafaAccount || !$zarafaQuotaOverride} disabled {/if} value="{$zarafaQuotaWarn}"> MB
              {/render}
            </td>
          </tr>
          <tr>
            <td><label for="zarafaResourceType">{t}Resource type{/t}</label></td>
            <td>
              {render acl=$zarafaResourceACL}
                <select size="1" id="zarafaResourceType" name="zarafaResourceType" {if !$zarafaAccount} disabled {/if}>
                  {html_options options=$zarafaResourceType_list selected=$zarafaResourceType}
                </select>
              {/render}
            </td>
          </tr>
          <tr>
            <td><label for="zarafaResourceCapacity">{t}Resource capacity{/t}</label></td>
            <td>
              {render acl=$zarafaResourceACL}
                <input type="text" id='zarafaResourceCapacity' {if !$zarafaAccount} disabled {/if}
                name="zarafaResourceCapacity" value="{$zarafaResourceCapacity}">
              {/render}
            </td>
          </tr>
        </table>
      </td>
      <td class="left-border">&nbsp;</td>
      <td>
        <h3><label for="sendas_list">{t}Send as privileges{/t}</label></h3>
        {render acl=$zarafaSendAsPrivilegeACL}
           <select {if $use_zarafaSendAsPrivilege} checked {/if} {if !$zarafaAccount} disabled {/if}
          id="zarafaSendAsPrivilege" style="width:100%; height:100px;" name="sendas_list[]" size=15 multiple>
          {html_options values=$zarafaSendAsPrivilege output=$zarafaSendAsPrivilege}
          <option disabled>&nbsp;</option>
           </select>
        {/render}
          <br>
        {render acl=$zarafaSendAsPrivilegeACL}
          <input id='add_local_sendas' type="submit" value="{t}Add local{/t}" name="add_local_sendas" {if !$zarafaAccount} disabled {/if}>&nbsp;
        {/render}
        {render acl=$zarafaSendAsPrivilegeACL}
          <input id='delete_sendas' type="submit" value="{msgPool type=delButton}" name="delete_sendas" {if !$zarafaAccount} disabled {/if}>
        {/render}
      </td>
    </tr>
    <tr>
      <td colspan="3">
        <hr/>
      </td>
    </tr>
    <tr>
      <td>
        <h2>{t}Features{/t}</h2>
        <table>
          <tr>
            <td>
              {t}POP3:{/t}
              <input type="radio" name="pop3" value="default" checked >{t}Default{/t}</input>
              <input type="radio" name="pop3" value="on" {if in_array("pop3", $zarafaEnabledFeatures)} checked {/if}>{t}On{/t}</input>
              <input type="radio" name="pop3" value="off" {if in_array("pop3", $zarafaDisabledFeatures)} checked {/if}>{t}Off{/t}</input>
            </td>
          </tr>
          <tr>
            <td>
              {t}IMAP:{/t}
              <input type="radio" name="imap" value="default" checked >{t}Default{/t}</input>
              <input type="radio" name="imap" value="on" {if in_array("imap", $zarafaEnabledFeatures)} checked {/if}>{t}On{/t}</input>
              <input type="radio" name="imap" value="off" {if in_array("imap", $zarafaDisabledFeatures)} checked {/if}>{t}Off{/t}</input>
            </td>
          </tr>
        </table>
      </td>
    </tr>
  </tbody>
</table>

<input type="hidden" name="zarafaTab" value="zarafaTab">
