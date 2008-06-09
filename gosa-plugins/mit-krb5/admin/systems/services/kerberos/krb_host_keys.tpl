{if !$service_plugin}
<p class="seperator">&nbsp;</p>
{/if}
<h2><img class="center" alt="" src="images/lists/locked.png" align="middle">	{t}Kerberos keys{/t} </h2>
<table>
{foreach from=$keys key=ID item=data}
	<tr>
		<td>{$data.REALM}</td>
		<td>{$data.NAME}</td>
		<td>
		{if $data.USED}
			<img src='images/empty.png' class="center">
			<input type='image' class='center' name='recreate_{$ID}'
				alt='{t}Recreate key{/t}' title='{t}Recreate key{/t}'
				src='images/lists/reload.png'>
			<input type='image' class='center' name='remove_{$ID}'
				alt='{t}Remove key{/t}' title='{t}Remove key{/t}'
				src='images/lists/trash.png'>
		{else}
			<input type='image' class='center' name='create_{$ID}'
				alt='{t}Create key{/t}' title='{t}Create key{/t}'
				src='images/lists/new.png'>
			<img src='images/empty.png' class="center">
			<img src='images/empty.png' class="center">
		{/if}
		</td>
	</tr>
{/foreach}
</table>

{if $service_plugin}
<p class="seperator">&nbsp;</p>
{/if}
