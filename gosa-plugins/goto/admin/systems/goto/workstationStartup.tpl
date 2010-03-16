<table summary="" style="width:100%;">
 <tr>
  <td style="width:50%; vertical-align:top;">
<h3>{t}Boot parameters{/t}</h3>

   <table summary="" style="width:100%">
	{if $fai_activated && $si_active && !$si_fai_action_failed}
    <tr>
     <td><LABEL for="gotoBootKernel">{t}Boot kernel{/t}</LABEL></td>
     <td style="width:70%">
{render acl=$gotoBootKernelACL}
        <select id="gotoBootKernel" name="gotoBootKernel">
         {html_options options=$gotoBootKernels selected=$gotoBootKernel}
		</select>
{/render}
      </td>
    </tr>
	{/if}
    <tr>
     <td><LABEL for="gotoKernelParameters">{t}Custom options{/t}</LABEL></td>
     <td>
{render acl=$gotoKernelParametersACL}
	<input name="gotoKernelParameters" id="gotoKernelParameters" size=25 maxlength=500
                value="{$gotoKernelParameters}" title="{t}Enter any parameters that should be passed to the kernel as append line during bootup{/t}">
{/render}
     </td>
    </tr>
    <tr>
     <td colspan="2" style='vertical-align:top;padding-top:3px;'><LABEL for="gotoLdapServer">{t}LDAP server{/t}</LABEL>
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

	{if !$fai_activated}
			<h3>{t}FAI Object assignment disabled. You can't use this feature until FAI is activated.{/t}</h3>			
	{elseif !$si_active}
		<b>{t}GOsa support daemon not configured{/t}</b><br>
		{t}FAI settings cannot be modified{/t}
	{elseif $si_fai_action_failed}
		<b>{msgPool type=siError}</b><br>
		{t}Check if the GOsa support daemon (gosa-si) is running.{/t}
		<button type='submit' name='fai_si_retry'>{t}retry{/t}</button>

	{elseif $fai_activated}

		{if $FAIdebianMirror == "inherited"}

			<table>
				<tr>
					<td>
						<h3>{t}FAI server{/t}
						</h3>
					</td>
					<td>
						<h3>{t}Release{/t}
						</h3>
					</td>
				</tr>
				<tr>
					<td>
	{render acl=$FAIdebianMirrorACL}
						<select name="FAIdebianMirror" {$FAIdebianMirrorACL} onchange='document.mainform.submit()'>
							{foreach from=$FAIservers item=val key=key}
								{if $key == "inherited" || $key == "auto"} 
								<option value="{$key}" {if $FAIdebianMirror == $key} selected {/if}>{t}{$key}{/t}</option>
								{else}
								<option value="{$key}" {if $FAIdebianMirror == $key} selected {/if}>{$key}</option>
								{/if}
							{/foreach}
						</select>
	{/render}
					</td>
					<td>
						<select name="FAIrelease"  disabled>
						{html_options options=$InheritedFAIrelease output=$InheritedFAIrelease selected=$InheritedFAIrelease}
						</select>
					</td>
				</tr>
			</table>
			<h3>
				<img class="center" alt="" align="middle" src="plugins/goto/images/fai_settings.png">&nbsp;{t}Assigned FAI classes{/t}
			</h3>
	{render acl=$FAIclassACL}
			{$FAIScriptlist}	
	{/render}
		{else}

			<table>
				<tr>
					<td>
						<h3>{t}FAI server{/t}
						</h3>
					</td>
					<td>
						<h3>{t}Release{/t}
						</h3>
					</td>
				</tr>
				<tr>
					<td>
	{render acl=$FAIdebianMirrorACL}
						<select name="FAIdebianMirror" {$FAIdebianMirrorACL} onchange='document.mainform.submit()'>
							{foreach from=$FAIservers item=val key=key}
								{if $key == "inherited" || $key == "auto"} 
								<option value="{$key}" {if $FAIdebianMirror == $key} selected {/if}>{t}{$key}{/t}</option>
								{else}
								<option value="{$key}" {if $FAIdebianMirror == $key} selected {/if}>{$key}</option>
								{/if}
							{/foreach}
						</select>
	{/render}
	{if $javascript eq 'false'}
	{render acl=$FAIdebianMirrorACL}
		<button type='submit' name='refresh'>{t}set{/t}</button>

	{/render}
	{/if}
					</td>
					<td>
	{render acl=$FAIreleaseACL}
						<select name="FAIrelease"  onchange='document.mainform.submit()' {$FAIclassACL}>
							{foreach from=$FAIservers.$FAIdebianMirror item=val key=key}
								<option value="{$val}" {if $FAIrelease == $key} selected {/if}>{$val}</option>
							{/foreach}
						</select>
	{/render}
					</td>
				</tr>
			</table>
			<h3>
				<img class="center" alt="" align="middle" src="plugins/goto/images/fai_settings.png">&nbsp;{t}Assigned FAI classes{/t}
			</h3>
	{render acl=$FAIclassACL}
			{$FAIScriptlist}	
	{/render}

	{render acl=$FAIclassACL}
			<select name="FAIclassesSel">
				{foreach from=$FAIclasses item=val key=key}
					<option value="{$key}">{$key}&nbsp;[{$val}]</option>
				{/foreach}
			</select>	
	{/render}
	{render acl=$FAIclassACL}
			<button type='submit' name='AddClass'>{msgPool type=addButton}</button> 

	{/render}
	<!--		<input name="DelClass" value="{msgPool type=delButton}" type="submit"> -->
			{/if}

		{/if}
  		</td>
	</tr>
</table>
<hr>
<table summary="" style="width:100%;">
 <tr>
  <td style="width:50%; vertical-align:top; border-right:1px solid #B0B0B0">
   <h3>
    <img class="center" alt="" align="middle" src="plugins/goto/images/hardware.png"> {t}Kernel modules (format: name parameters){/t}
   </h3>
{render acl=$gotoModulesACL}
    <select style="width:100%; height:150px;" name="modules_list[]" size=15 multiple title="{t}Add additional modules to load on startup{/t}">
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

  <td style="vertical-align:top;">
        <h3><LABEL for="gotoShare">{t}Shares{/t}</LABEL></h3>
        <table summary="" style="width:100%">
                <tr>
                        <td>
{render acl=$gotoShareACL}
                        <select style="width:100%;height:150px;" name="gotoShare" multiple size=4 id="gotoShare">
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
                                <button type='submit' name='gotoShareDel' {if $gotoSharesCount == 0} disabled {/if}
>{t}Remove{/t}</button>

{/render}
                        </td>
                </tr>
        </table>
  </td>
 </tr>
</table>
<input name="WorkstationStarttabPosted" type="hidden" value="1">
<!-- Place cursor -->
<script language="JavaScript" type="text/javascript">
  <!-- // First input field on page
	focus_field('gotoLdapServer');
  -->
</script>
