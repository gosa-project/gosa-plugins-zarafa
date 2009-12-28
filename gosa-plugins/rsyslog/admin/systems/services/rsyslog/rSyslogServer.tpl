<h2><img class="center" alt="" align="middle" src="images/rightarrow.png" /> {t}rSyslog logging database{/t}</h2>
<table summary="">
    <tr>
     <td>{t}Database{/t}{$must}</td>
     <td>
{render acl=$goLogDBACL}
 	<input name="goLogDB" id="goLogDB" size=30 maxlength=60 value="{$goLogDB}">
{/render}
     </td>
    </tr>
    <tr>
     <td>{t}Database user{/t}{$must}</td>
     <td>
{render acl=$goLogDBUserACL}
	<input name="goLogDBUser" id="goLogDBUser" size=30 maxlength=60 value="{$goLogDBUser}">
{/render}
     </td>
    </tr>
    <tr>
     <td>{t}Password{/t}{$must}</td>
     <td>
{render acl=$goLogDBPasswordACL}
 	<input type="password" name="goLogDBPassword" id="goLogDBPassword" size=30 maxlength=60 value="{$goLogDBPassword}">
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
