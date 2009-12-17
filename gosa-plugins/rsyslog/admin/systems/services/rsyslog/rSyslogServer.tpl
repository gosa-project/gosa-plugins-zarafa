<h2><img class="center" alt="" align="middle" src="images/rightarrow.png" /> {t}rSyslog logging database{/t}</h2>
<table summary="">
    <tr>
     <td>{t}Database{/t}{$must}</td>
     <td>
{render acl=$rSyslogDatabaseACL}
 	<input name="rSyslogDatabase" id="rSyslogDatabase" size=30 maxlength=60 value="{$rSyslogDatabase}">
{/render}
     </td>
    </tr>
    <tr>
     <td>{t}Database user{/t}{$must}</td>
     <td>
{render acl=$rSyslogUserACL}
	<input name="rSyslogUser" id="rSyslogUser" size=30 maxlength=60 value="{$rSyslogUser}">
{/render}
     </td>
    </tr>
    <tr>
     <td>{t}Password{/t}{$must}</td>
     <td>
{render acl=$rSyslogPasswordACL}
 	<input type="password" name="rSyslogPassword" id="rSyslogPassword" size=30 maxlength=60 value="{$rSyslogPassword}">
{/render}
     </td>
    </tr>
   </table>

<p class='seperator'>&nbsp;</p>
<div style="width:100%; text-align:right;padding-top:10px;padding-bottom:3px;">
    <input type='submit' name='SaveService' value='{msgPool type=saveButton}'>
    &nbsp;
    <input type='submit' name='CancelService' value='{msgPool type=cancelButton}'>
</div>
<input type="hidden" name="rSyslogServerPosted" value="1">
