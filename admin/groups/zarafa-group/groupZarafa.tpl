<input type="hidden" name='zarafaedit' value='1'>
<table summary="" style="width:100%; vertical-align:top; text-align:left;" cellpadding="0" border="0">
  <tr>
    <td style="width:50%;">
      <h3>{t}Zarafa Settings{/t}</h3>
        <input class="center" id='zarafaAccount'
        type=checkbox name="zarafaAccount" {$zarafaAccount}
        title="{t}Enable Zarafa Account for this group{/t}"> {t}Enable Zarafa{/t}
      <br>   
      {render acl=$zarafaSecurityGroupACL}
        <input class="center" id='zarafaSecurityGroup'
        type=checkbox name="zarafaSecurityGroup" {$zarafaSecurityGroup}
        title="{t}Make this group a Zarafa Securitygroup{/t}"> {t}Zarafa Securitygroup{/t}
      {/render}
         <br>
      {render acl=$zarafaHiddenACL}
        <input class="center" id='zarafaHidden' 
        type=checkbox name="zarafaHidden" {$zarafaHidden}
        title="{t}Hide from Zarafa addressbook{/t}"> {t}Hide from addressbook{/t}
      {/render}
    </td>
    <td class="left-border">&nbsp;</td>
    <td>
      <h3><label for="alternates_list"> {t}Alternative addresses{/t}</label></h3>
      {render acl=$gosaMailAlternateAddressACL}
      <select id="alternates_list" name="dummy1" style="width:100%;height:100px;" multiple
      title="{t}List of alternative mail addresses{/t}">
      {html_options values=$gosaMailAlternateAddress output=$gosaMailAlternateAddress}
      <option disabled>&nbsp;</option>
      {/render}
      </select>
    </td>
  </tr>
  <tr>
    <td colspan="3">
      <hr/>
    </td>
  </tr>
  <tr>
    <td>
      <h3>{t}Send as privileges{/t}</h3>
      {render acl=$zarafaSendAsPrivilegeACL}
         <select id="zarafaSendAsPrivilege" name="sendas_list[]" style="width:100%;height:100px;" multiple>
           {html_options values=$zarafaSendAsPrivilege output=$zarafaSendAsPrivilege}
           <option disabled>&nbsp;</option>
         </select>
      {/render}
      <br/>
      {render acl=$zarafaSendAsPrivilegeACL}
        <input id='add_local_sendas' type="submit" value="{t}Add local{/t}" name="add_local_sendas" >&nbsp;
      {/render}
      {render acl=$zarafaSendAsPrivilegeACL}
        <input id='delete_sendas' type="submit" value="{msgPool type=delButton}" name="delete_sendas" >
      {/render}
    </td>
    <td class="left-border">&nbsp;</td>
    <td>
      <h3><label for="forwarding_list"> {t}Forwarding addresses{/t}</label></h3>
      {render acl=$gosaMailAlternateAddressACL}
      <select id="forwarding_list" name="dummyForwarding" style="width:100%;height:100px;" multiple
      title="{t}List of forwarding mail addresses{/t}">
      {html_options values=$gosaMailForwardingAddress output=$gosaMailForwardingAddress}
      <option disabled>&nbsp;</option>
      {/render}
      </select>
    </td>
  </tr>
  <tr>
    <td colspan="3">
      <hr/>
    </td>
  </tr>
  <tr>
    <td>
      <h3>{t}Zarafa group members{/t}</h3>
      {$memberList}
      {render acl=$zarafaSendAsPrivilegeACL}
        <input id='add_zarafa_member' type="submit" value="{t}Add{/t}" name="add_zarafa_member" >&nbsp;
      {/render}
    </td>
    <td class="left-border">&nbsp;</td>
  </tr>
</table>

<!-- Place cursor -->
<script language="JavaScript" type="text/javascript">
  <!-- // First input field on page
	focus_field('mail');
  -->
    function changeStates()
    {
      if($('zarafaSecurityGroup').checked) {
        $("gosaMailForwardingAddress", "gosaMailAlternateAddress").invoke("disable");
      }
    }

    changeStates();

</script>
