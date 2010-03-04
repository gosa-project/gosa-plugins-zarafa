<h3>{t}LDAP service{/t}</h3>
{t}LDAP URI{/t}{$must} 
{render acl=$goLdapBaseACL}
<input type="text" size="80" value="{$goLdapBase}"  name="goLdapBase" id="goLdapBaseId">
{/render}

<p class='seperator'>&nbsp;</p>
<div style="width:100%; text-align:right;padding-top:10px;padding-bottom:3px;">
    <input type='submit' name='SaveService' value='{msgPool type=saveButton}'>
    &nbsp;
    <input type='submit' name='CancelService' value='{msgPool type=cancelButton}'>
</div>
<input type="hidden" name="goLdapServerPosted" value="1">
