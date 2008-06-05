{if !$is_service_key}
<p class="seperator">&nbsp;</p>
{/if}
<h2><img class="center" alt="" src="images/lists/locked.png" align="middle">	{t}Kerberos keys{/t}</h2>

{t}Reload key list{/t} &nbsp;<input type='image' name='reload_krb_keys' src='images/lists/reload.png' alt='{t}Reload{/t}' class='center'>


<br>
<br>
<b>{t}Realms{/t}</b>: &nbsp;
<br>
<table style="">
{foreach from=$server_list item=item key=key}
	<tr>
		<td style="padding-right:100px;">{$item.REALM}</td>
		<td>
			{if $item.PRESENT}
				<img src='images/empty.png' class="center">
				<input type='image' class='center' name='recreate_{$key}'
					alt='{t}Recreate key{/t}' title='{t}Recreate key{/t}'
					src='images/crossref.png'>
				<input type='image' class='center' name='remove_{$key}'
					alt='{t}Remove key{/t}' title='{t}Remove key{/t}'
					src='images/lists/trash.png'>
			{else}
				<input type='image' class='center' name='create_{$key}'
					alt='{t}Create key{/t}' title='{t}Create key{/t}'
					src='images/lists/new.png'>
				<img src='images/empty.png' class="center">
				<img src='images/empty.png' class="center">
			{/if}
		</td>
	</tr>
	<tr>
		<td><i>{t}Kerberos keys for this realm{/t}:</i></td>
		<td>{$item.USED}</td>
	</tr>
{/foreach}
</table>
{if $is_service_key}
<p class="seperator">&nbsp;</p>
{/if}
