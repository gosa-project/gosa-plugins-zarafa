<p class="seperator">&nbsp;</p>
<h2><img class="center" alt="" src="images/lists/locked.png" align="middle">	{t}Host key{/t}</h2>
<table style="">
{foreach from=$server_list item=item key=key}
	<tr>
		<td>{$item.REALM}</td>
		<td style="padding-left:50px;">
			{if $item.PRINCIPAL}
				<img src='images/empty.png' class="center">
				<input type='image' class='center' name='recreate_{$key}'
					src='images/lists/reload.png'>
				<input type='image' class='center' name='remove_{$key}'
					src='images/lists/trash.png'>
			{else}
				<input type='image' class='center' name='create_{$key}'
					src='images/lists/new.png'>
				<img src='images/empty.png' class="center">
				<img src='images/empty.png' class="center">
			{/if}
		</td>
	</tr>
{/foreach}
</table>
