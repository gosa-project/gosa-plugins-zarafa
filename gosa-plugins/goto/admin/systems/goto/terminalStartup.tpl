<h3>{t}Boot parameters{/t}</h3>
<table summary="" style="width:100%;">
 <tr>
  <td style="width:50%; vertical-align:top;">
	<table>
    <tr>
     <td colspan="2" style='vertical-align:top;padding-top:3px;width:100%'><LABEL for="gotoLdapServer">{t}LDAP server{/t}</LABEL>
{render acl=$gotoLdapServerACL}
{if $member_of_ogroup}
(<input type='checkbox' name='gotoLdap_inherit' {if $gotoLdap_inherit} checked {/if} value="1"
      onClick="document.mainform.submit();" class='center'>
&nbsp;{t}inherit from group{/t})
{if !$JS}
      <input type='image' src="images/lists/reload.png" alt='{t}Reload{/t}' class='center'>
{/if}
{/if}
{/render}
{render acl=$gotoLdapServerACL_inherit}
        {$gotoLdapServers}
{/render}
{render acl=$gotoLdapServerACL_inherit}
      <select name='ldap_server_to_add' id='ldap_server_to_add'>
        {html_options options=$gotoLdapServerList}
    </select>
{/render}
{render acl=$gotoLdapServerACL_inherit}
      <button type='submit' name='add_ldap_server' id="add_ldap_server">{msgPool type=addButton}</button>

{/render}
     </td>
    </tr>
	</table>	
  </td>
  <td style="border-left:1px solid #A0A0A0">
     &nbsp;
  </td>
  <td style="vertical-align:top;">

   <table summary="" style="width:100%">
    <tr>
     <td style="width:30%"><LABEL for="gotoBootKernel">{t}Boot kernel{/t}</LABEL></td>
     <td>
{render acl=$gotoBootKernelACL}
	<select id="gotoBootKernel" name="gotoBootKernel">
	{html_options options=$gotoBootKernels  selected=$gotoBootKernel}
	<option disabled>&nbsp;</option>
	</select>
{/render}
     </td>
    </tr>
    <tr>
     <td><LABEL for="gotoKernelParameters">{t}Custom options{/t}</LABEL></td>
     <td>
{render acl=$gotoKernelParametersACL}
      <input name="gotoKernelParameters" id="gotoKernelParameters"  size=25 maxlength=500
                value="{$gotoKernelParameters}" title="{t}Enter any parameters that should be passed to the kernel as append line during bootup{/t}"></td>
{/render}
    </tr>
   </table>

  </td>
 </tr>
</table>

<table summary="" style="width:100%;">
 <tr><td colspan=2>
  <hr>
  </td></tr>

 <tr>
  <td style="width:50%; vertical-align:top;">
   <h3>
    <img class="center" alt="" align="middle" src="plugins/goto/images/hardware.png"> {t}Kernel modules (format: name parameters){/t}
   </h3>
{render acl=$gotoModulesACL}
    <select style="width:100%;" name="modules_list[]" size=15 multiple title="{t}Add additional modules to load on startup{/t}">
     {html_options values=$gotoModules output=$gotoModules}
	 <option disabled>&nbsp;</option>
    </select>
{/render}
    <br>
{render acl=$gotoModulesACL}
    <input type='text' name="module" size=30 align=middle maxlength=30>
{/render}
{render acl=$gotoModulesACL}
    <button type='submit' name='add_module'>{msgPool type=addButton}</button>&nbsp;

{/render}
{render acl=$gotoModulesACL}
    <button type='submit' name='delete_module'>{msgPool type=delButton}</button>

{/render}
  </td>

  <td style="padding-left:10px;border-left:1px solid #A0A0A0;vertical-align:top">
        <h3><LABEL for="gotoShare">{t}Shares{/t}</LABEL></h3>
        <table summary="" style="width:100%">
                <tr>
                        <td>
{render acl=$gotoShareACL}
                        <select style="width:100%;" name="gotoShare" multiple size=15 id="gotoShare">
        			{html_options values=$gotoShareKeys output=$gotoShares}
			        <option disabled>&nbsp;</option>
                        </select>
{/render}
                                <br>
{render acl=$gotoShareACL}
        	                <select name="gotoShareSelection">
				        {html_options values=$gotoShareSelectionKeys output=$gotoShareSelections}
				        <option disabled>&nbsp;</option>
                                </select>
{/render}
{render acl=$gotoShareACL}
                                <input type="text" size=15 name="gotoShareMountPoint" value="{t}Mountpoint{/t}">
{/render}
{render acl=$gotoShareACL}
                                <button type='submit' name='gotoShareAdd'>{msgPool type=addButton}</button>

{/render}
{render acl=$gotoShareACL}
                                <button type='submit' name='gotoShareDel'>{t}Remove{/t}</button>

{/render}
                        </td>
                </tr>
        </table>
  </td>
 </tr>
</table>
<input type='hidden' name='TerminalStarttabPosted' value="1">

<!-- Place cursor -->
<script language="JavaScript" type="text/javascript">
  <!-- // First input field on page
	focus_field('gotoLdapServer');
  -->
</script>
