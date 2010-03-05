<h3>{t}GLPI database information{/t}</h3>
  <table summary="">
    <tr>
     <td>{t}Logging DB user{/t}{$must}</td>
     <td>
{render acl=$goGlpiAdminACL}
      <input type='text' name="goGlpiAdmin" id="goGlpiAdmin" size=30 maxlength=60 value="{$goGlpiAdmin}">
{/render}
     </td>
    </tr>
    <tr>
     <td>{t}Password{/t}</td>
     <td>
{render acl=$goGlpiPasswordACL}
      <input type="password" name="goGlpiPassword" id="goGlpiPassword" size=30 maxlength=60 value="{$goGlpiPassword}">
{/render}
     </td>
    </tr>
    <tr>
     <td>{t}Database{/t}{$must}</td>
     <td>
{render acl=$goGlpiDatabaseACL}
      <input type='text' name="goGlpiDatabase" id="goGlpiDatabase" size=30 maxlength=60 value="{$goGlpiDatabase}">
{/render}
     </td>
    </tr>
   </table>


<hr>
<div style="width:100%; text-align:right;padding-top:10px;padding-bottom:3px;">
    <input type='submit' name='SaveService' value='{msgPool type=saveButton}'>
    &nbsp;
    <input type='submit' name='CancelService' value='{msgPool type=cancelButton}'>
</div>
<input type="hidden" name="goGlpiServer_posted" value="1">
