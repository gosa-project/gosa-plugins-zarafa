<input type="hidden" name="mailedit" value="1">
<table summary="" style="width:100%; vertical-align:top; text-align:left;" cellpadding=0 border=0>

 <!-- Headline container -->
 <tr>
  <td style="width:50%; border-right:1px solid #A0A0A0; vertical-align:top">
   <h2><img class="center" alt="" align="middle" src="images/rightarrow.png"> {t}Generic{/t}</h2>
   <table summary="">
    <tr>
     <td><LABEL for="mail">{t}Primary address{/t}</LABEL>{$must}</td>
     <td>
{if $multiple_support}
	<input id="dummy1" name="dummy1" size="25" maxlength="65" 
		value="{t}Multiple edit{/t}" disabled>
{else}
	{render acl=$mailACL}
		<input id="mail" name="mail" size="25" maxlength="65" value="{$mail}" title="{t}Primary mail address for this shared folder{/t}">
	{/render}
{/if}
     </td>
    </tr>
    <tr>
     <td><LABEL for="gosaMailServer">{t}Server{/t}</LABEL></td>
     <td>
{render acl=$gosaMailServerACL checkbox=$multiple_support checked=$use_gosaMailServer}
      <select size="1" name="gosaMailServer" title="{t}Select mail server to place user on{/t}">
       {html_options values=$mailServers output=$mailServers selected=$gosaMailServer}
	   <option disabled>&nbsp;</option>
      </select>
{/render}
     </td>
    </tr>
    <tr>
     <td>
      <br>
     	<LABEL for="gosaMailQuota"> {t}Quota usage{/t}</LABEL>
     </td>
     <td>
      <br>
{if $quotadefined eq "true"}
	{$quotausage}
{else}
      	{t}not defined{/t}
{/if}
     </td>
    </tr>
    <tr>
     <td>{t}Quota size{/t}</td>
     <td>
{render acl=$gosaMailQuotaACL  checkbox=$multiple_support checked=$use_gosaMailQuota}
	<input id="gosaMailQuota" name="gosaMailQuota" size="6" align=middle maxlength="30" value="{$gosaMailQuota}">
{/render}
	 {t}MB{/t}
	</td>
</tr>
	{if $kolab}
	<tr>
		<td>
			{t}Folder type{/t}
		</td>
		<td>
		
		{if $multiple_support}
			<input type='checkbox' name='use_kolabFolderType' id='use_kolabFolderType' class="center"
				{if $use_kolabFolderType} checked {/if}
				onClick="changeState('kolabFolderTypeType');changeState('kolabFolderTypeSubType');"			
			>
			<select id="kolabFolderTypeType" name="kolabFolderTypeType" onChange="document.mainform.submit();"
				{if !$use_kolabFolderType} disabled {/if}>
				{html_options options=$kolabFolderTypeTypes selected=$kolabFolderTypeType}
			</select>
			<select id="kolabFolderTypeSubType" name="kolabFolderTypeSubType" onChange="document.mainform.submit();"
				{if !$use_kolabFolderType} disabled {/if}>
				{html_options options=$kolabFolderTypeSubTypes selected=$kolabFolderTypeSubType}
			</select>
		{else}
			<select id="kolabFolderTypeType" name="kolabFolderTypeType" onChange="document.mainform.submit();">
				{html_options options=$kolabFolderTypeTypes selected=$kolabFolderTypeType}
			</select>
			<select id="kolabFolderTypeSubType" name="kolabFolderTypeSubType" onChange="document.mainform.submit();">
				{html_options options=$kolabFolderTypeSubTypes selected=$kolabFolderTypeSubType}
			</select>
		{/if}
			{if !$JS}
				<input type='image' src='images/lists/reload.png' class='center' alt='{t}Reload{/t}'>
			{/if}
		</td>
	</tr>
	{/if}
   </table>
     
  </td>

{if $multiple_support}

{else}

  <td style="vertical-align:top;padding-left:2px;">

	

   <h2><img class="center" alt="" align="middle" src="plugins/mail/images/alternatemail.png"> {t}Alternative addresses{/t}</h2>

{render acl=$gosaMailAlternateAddressACL}
   <select style="width:100%;" name="alternates_list[]" size=10 multiple title="{t}List of alternative mail addresses{/t}">
    {html_options values=$gosaMailAlternateAddress output=$gosaMailAlternateAddress}
	<option disabled>&nbsp;</option>
   </select>
{/render}

   <br>

{render acl=$gosaMailAlternateAddressACL}
   <input name="alternate_address" size="30" align=middle maxlength="60" value="">
{/render}

{render acl=$gosaMailAlternateAddressACL}
   <input type=submit value="{msgPool type=addButton}" name="add_alternate">&nbsp;
{/render}

{render acl=$gosaMailAlternateAddressACL}
   <input type=submit value="{msgPool type=delButton}" name="delete_alternate">
{/render}

  </td>
{/if}
 </tr>

</table>
<p class="seperator">&nbsp;</p>

<table summary="" style="width:100%; vertical-align:top; text-align:left;" cellpadding=4 border=0>
 <tr>
  <td style="vertical-align:top;width:50%; border-right:1px solid #A0A0A0">
   <h2><img class="center" alt="" align="middle" src="plugins/mail/images/shared_folder.png"> {t}IMAP shared folders{/t}</h2>
{if $multiple_support}

	<input class="center" type='checkbox' name="use_acl" value="1" {if $use_acl} checked {/if} 
		onClick="toggle('acl_div');">{t}Set shared folder permissions{/t}
	{if $use_acl}
		<div style="visibility:visible;" id="acl_div">
	{else}
		<div style="visibility:hidden;" id="acl_div">
	{/if}
{/if}      
   <table summary="" cellpadding=0 border=0>
    <tr>
     <td><LABEL for="default_permissions">{t}Default permission{/t}</LABEL></td>
     <td>
{render acl=$aclACL}
      <select size="1" id="default_permissions" name="default_permissions">
       {html_options options=$perms selected=$default_permissions}
	   <option disabled>&nbsp;</option>
      </select>
{/render}
     </td>
    </tr>
    <tr>
     <td><LABEL for="member_permissions">{t}Member permission{/t}</LABEL></td>
     <td>
{render acl=$aclACL}
      <select id="member_permissions" size="1" name="member_permissions">
       {html_options options=$perms selected=$member_permissions}
      </select>
{/render}
     </td>
    </tr>
	{$plusattributes}
   </table>
{if $multiple_support}
	</div>
{/if}

<p class="seperator">&nbsp;</p>

<h2><img class="center" alt="" align="middle" src="images/false.png" />&nbsp;{t}Advanced mail options{/t}</h2> 
<table summary="" style="width:100%; vertical-align:top; text-align:left;" cellpadding="2" border="0">
 <tr>
  <td>
{render acl=$gosaMailDeliveryModeIACL}
   <input type="checkbox" name="only_local" value="1" {$only_local} 
	title="{t}Select if user can only send and receive inside his own domain{/t}">
{/render}
	{t}User is only allowed to send and receive local mails{/t}
  </td> 
 </tr>
 </table> 

  </td>
  <td style="vertical-align:top;width:50%">
   <h2>
	<img class="center" alt="" align="middle" src="plugins/mail/images/envelope.png">
	{t}Forward messages to non group members{/t}
   </h2>

{render acl=$gosaMailForwardingAddressACL}
   <select style="width:100%;" name="forwarder_list[]" size=10 multiple>

	{if $multiple_support}

		{foreach from=$Forward_all item=item key=key}
			<option value="{$item}">{$item}&nbsp;({t}Used in all groups{/t})</option>
		{/foreach}
		{foreach from=$Forward_some item=item key=key}
			<option value="{$item}" style='color: #888888; background: #DDDDDD;background-color: #DDDDDD;'>{$item}&nbsp;({t}Not used in all groups{/t})</option>
		{/foreach}
	{else}
    {html_options values=$gosaMailForwardingAddress output=$gosaMailForwardingAddress}
	<option disabled>&nbsp;</option>
	{/if}
   </select>
{/render}

   <br>

{render acl=$gosaMailForwardingAddressACL}
   <input name="forward_address" size=20 align=middle maxlength=65 value="">
{/render}
{render acl=$gosaMailForwardingAddressACL}
   <input type=submit value="{msgPool type=addButton}" name="add_forwarder">&nbsp;
{/render}
{render acl=$gosaMailForwardingAddressACL}
   <input type=submit value="{t}Add local{/t}" name="add_local_forwarder">&nbsp;
{/render}
{render acl=$gosaMailForwardingAddressACL}
   <input type=submit value="{msgPool type=delButton}" name="delete_forwarder">
{/render}

  </td>
 </tr>
</table>

<!-- Place cursor -->
<script language="JavaScript" type="text/javascript">
  <!-- // First input field on page
	focus_field('mail');
  -->
</script>
{if $multiple_support}
	<input type="hidden" name="multiple_mail_group_posted" value="1">
{/if}
