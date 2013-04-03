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
        onclick="changeState('zarafaAdmin', 'zarafaSharedStoreOnly',
                             'zarafaResourceCapacity','zarafaSendAsPrivilege','zarafaResourceType', 'zarafaQuotaHard',
                             'zarafaQuotaSoft', 'zarafaQuotaWarn', 'zarafaQuotaOverride', 'add_local_sendas','delete_sendas');"
        title="{t}Enable Zarafa Account for this user{/t}"> {t}Enable Zarafa{/t}
        {/render}
        <br/>
        {render acl=$zarafaAdminACL checked=$use_zarafaAdmin}
        <input type="checkbox" name="zarafaAdmin" id="zarafaAdmin" value="1" {$zarafaAdmin} onChange="toogleAdminStore()"
        title="{t}Decide if the current user is an Administrator{/t}"> {t}Zarafa Administrator{/t}</input>
        {/render}
        <br/>
        {render acl=$zarafaSharedStoreOnlyACL checked=$use_zarafaSharedStoreOnly}
        <input type="checkbox" name="zarafaSharedStoreOnly" id="zarafaSharedStoreOnly" value="1" {$zarafaSharedStoreOnly}  onChange="toogleAdminStore()"
        title="{t}User is only a shared store{/t}"> {t}Zarafa shared store{/t}</input>
        {/render}
        <br/>
        {render acl=$zarafaHiddenACL checked=$use_zarafaHidden}
        <input id='zarafaHidden'
        type="checkbox" name="zarafaHidden" value="1" {$zarafaHidden}
        title="{t}Hide from Zarafa addressbook{/t}"> {t}Hide from addressbook{/t}
        {/render}
        <br>
        {render acl=$zarafaQuotaOverrideACL checked=$use_zarafaQuotaOverride}
        <input type="checkbox" name="zarafaQuotaOverride" id="zarafaQuotaOverride" value="1" {$zarafaQuotaOverride} onChange="toogleQuotaRules()" {if !$zarafaAccount} disabled {/if}
        title="{t}Override default quota settings{/t}"> {t}Override default quota{/t}</input>
        {/render}
        <br/>
        <input type="checkbox" name="zarafaContactAccount" id="zarafaContactAccount" value="1" {$zarafaContactAccount}
        title="{t}Use this account as zarafa contact{/t}" onChange="toogleContact()"> {t}Define as contact{/t}</input>
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
                <input type="text" id="zarafaQuotaHard" name="zarafaQuotaHard" {if !$zarafaQuotaOverride} disabled {/if} value="{$zarafaQuotaHard}"> MB
              {/render}
            </td>
          </tr>
          <tr>
            <td><label for="zarafaQuotaSoft">{t}Soft quota size{/t}</label></td>
            <td>
              {render acl=$zarafaQuotaACL}
              <input  type="text"  id="zarafaQuotaSoft" name="zarafaQuotaSoft" {if !$zarafaQuotaOverride} disabled {/if} value="{$zarafaQuotaSoft}"> MB
              {/render}
            </td>
          </tr>
          <tr>
            <td><label for="zarafaQuotaWarn">{t}Warn quota size{/t}</label></td>
            <td>
              {render acl=$zarafaQuotaACL}
                <input  type="text" id="zarafaQuotaWarn" name="zarafaQuotaWarn" {if !$zarafaQuotaOverride} disabled {/if} value="{$zarafaQuotaWarn}"> MB
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
          <br/>
        {render acl=$zarafaSendAsPrivilegeACL}
          <input id='add_local_sendas' type="submit" value="{t}Add local{/t}" name="add_local_sendas" {if !$zarafaAccount} disabled {/if} />&nbsp;
        {/render}
        {render acl=$zarafaSendAsPrivilegeACL}
          <input id='delete_sendas' type="submit" value="{msgPool type=delButton}" name="delete_sendas" {if !$zarafaAccount} disabled {/if} />
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
              <input type="radio" name="pop3" id="popA" value="default" checked >{t}Default{/t}</input>
              <input type="radio" name="pop3" id="popB"value="on" {if in_array("pop3", $zarafaEnabledFeatures)} checked {/if}>{t}On{/t}</input>
              <input type="radio" name="pop3" id="popC"value="off" {if in_array("pop3", $zarafaDisabledFeatures)} checked {/if}>{t}Off{/t}</input>
            </td>
          </tr>
          <tr>
            <td>
              {t}IMAP:{/t}
              <input type="radio" name="imap" id="imapA" value="default" checked >{t}Default{/t}</input>
              <input type="radio" name="imap" id="imapB" value="on" {if in_array("imap", $zarafaEnabledFeatures)} checked {/if}>{t}On{/t}</input>
              <input type="radio" name="imap" id="imapC" value="off" {if in_array("imap", $zarafaDisabledFeatures)} checked {/if}>{t}Off{/t}</input>
            </td>
          </tr>
        </table>
      </td>
      <td class="left-border">&nbsp;</td>
      <td>
        <label for="choosenArchiveServer">{t}Archive server{/t}</label>
        <select style="width: 20200" size="1" id="choosenArchiveServer" name="choosenArchiveServer" {if !$zarafaAccount} disabled {/if}>
          {html_options options=$availableArchiveServer selected=$choosenArchiveServerdentifier}
        </select>
        <input id='add_archive' type="submit" value="{t}Add archive server{/t}" name="add_archive" />
        <br/>
        <select id="archiveServerList" style="width:100%; height:100px;" name="archive_server_list[]" size=15 multiple>
          {html_options values=$zarafaUserArchiveServers output=$zarafaUserArchiveServers}
          <option disabled>&nbsp;</option>
        </select>
        <br/>
        <input id='delete_archive' type="submit" value="{msgPool type=delButton}" name="delete_archive"/>
      </td>
    </tr>
  </tbody>
</table>

<input type="hidden" name="zarafaTab" value="zarafaTab">

<script>
  var contactAccount_check = document.getElementById("zarafaContactAccount");
  var admin_check = document.getElementById("zarafaAdmin"); 
  var sharedStoreOnly_check = document.getElementById("zarafaSharedStoreOnly"); 
  var quotaOverride_check = document.getElementById("zarafaQuotaOverride"); 

  var resourceType_combo = document.getElementById("zarafaResourceType");
  var chooseArchiveServer_combo = document.getElementById("choosenArchiveServer");

  var resourceCapacity_line = document.getElementById("zarafaResourceCapacity");
  var quotaHard_line = document.getElementById("zarafaQuotaHard");
  var quotaSoft_line = document.getElementById("zarafaQuotaSoft"); 
  var quotaWarn_line = document.getElementById("zarafaQuotaWarn");

  var archiveServer_list = document.getElementById("archiveServerList");
  var mailForwardingAddresses_list = document.getElementById("gosaMailForwardingAddress");

  var deleteArchive_btn = document.getElementById("delete_archive");
  var addArchive_btn = document.getElementById("add_archive");

  var popA_rad = document.getElementById("popA");
  var popB_rad = document.getElementById("popB");
  var popC_rad = document.getElementById("popC");

  var imapA_rad = document.getElementById("imapA");
  var imapB_rad = document.getElementById("imapB");
  var imapC_rad = document.getElementById("imapC");


  /*
   * unused fields get deacivated 
   */
  function toogleContact(){
    if(contactAccount_check.checked){
      admin_check.disabled           = true; 
      sharedStoreOnly_check.disabled = true; 
      quotaOverride_check.disabled   = true; 

      resourceType_combo.disabled        = true;
      chooseArchiveServer_combo.disabled = true;

      resourceCapacity_line.disabled = true;
      quotaHard_line.disabled        = true;
      quotaSoft_line.disabled        = true;
      quotaWarn_line.disabled        = true;

      archiveServer_list.disabled           = true;
      mailForwardingAddresses_list.disabled = true;

      deleteArchive_btn.disabled = true;
      addArchive_btn.disabled    = true;

      popA.disabled = true;
      popB.disabled = true;
      popC.disabled = true;
      imapA.disabled = true;
      imapB.disabled = true;
      imapC.disabled = true;

    } else {
      if(!sharedStoreOnly_check.checked)admin_check.disabled = false; 
      if(!admin_check.checked)sharedStoreOnly_check.disabled = false; 

      quotaOverride_check.disabled   = false;

      resourceType_combo.disabled        = false;
      chooseArchiveServer_combo.disabled = false;

      resourceCapacity_line.disabled = false;

      if(!quotaOverride_check.disabled && quotaOverride_check.checked){        
        quotaHard_line.disabled        = false;
        quotaSoft_line.disabled        = false;
        quotaWarn_line.disabled        = false;
      }

      archiveServer_list.disabled           = false;
      mailForwardingAddresses_list.disabled = false;

      deleteArchive_btn.disabled = false;
      addArchive_btn.disabled    = false;

      popA.disabled = false;
      popB.disabled = false;
      popC.disabled = false;
      imapA.disabled = false;
      imapB.disabled = false;
      imapC.disabled = false;
    }
  }

  function toogleQuotaRules(){
    if(quotaOverride_check.checked && !quotaOverride_check.disabled){
      quotaHard_line.disabled = false;
      quotaSoft_line.disabled = false;
      quotaWarn_line.disabled = false;
    } else {
      quotaHard_line.disabled = true;
      quotaSoft_line.disabled = true;
      quotaWarn_line.disabled = true;
    }
  }

  function toogleAdminStore(){
    if(!contactAccount_check.checked){
      sharedStoreOnly_check.disabled = (admin_check.checked) ? true : false;
      admin_check.disabled = (sharedStoreOnly_check.checked) ? true : false;
    }
  }

  toogleContact();
  toogleQuotaRules();
  toogleAdminStore();
  
</script>

