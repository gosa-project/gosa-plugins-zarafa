<h2>{t}Import jobs{/t}</h2>

<table>
	<tr>	
		<td>
			{t}Select the semicolon seperated list to import{/t}
		</td>
		<td>
			<input type='file' name='file' value="{t}Browse{/t}">
			<input type='submit' name='import' value='{t}Upload{/t}'>
		</td>
	</tr>
	{if  $count}
	<tr>
		<td>{t}Start import{/t}</td>
		<td>
			<input type='submit' name='start_import' value='{t}Import{/t}'>
		</td>
	</tr>
	{/if}
</table>

	{if  $count}
		<p class="seperator">&nbsp;</p>
		<br>
		<br>
		<div style='width:100%; height:300px; overflow: scroll;'>
		<table cellpadding="3" cellspacing="0" style='width:100%; background-color: #CCCCCC; border: solid 1px #CCCCCC;'>
			<tr>
				<td><b>{t}Mac{/t}</b></td>
				<td><b>{t}Event{/t}</b></td>
				<td><b>{t}Object group{/t}</b></td>
				<td><b>{t}Base{/t}</b></td>
				<td><b>{t}FQDN{/t}</b></td>
				<td><b>{t}IP{/t}</b></td>
				<td><b>{t}DHCP{/t}</b></td>
			</tr>
		{foreach from=$info item=item key=key}
			{if $item.ERROR}
				<tr style='background-color: #F0BBBB;'>
					<td>{$item.MAC}</td>
					<td>{$item.HEADER}</td>
					<td>{$item.OGROUP}</td>
					<td>{$item.BASE}</td>
					<td>{$item.FQDN}</td>
					<td>{$item.IP}</td>
					<td>{$item.DHCP}</td>
				</tr>	
				<tr style='background-color: #F0BBBB;'>
					<td colspan="7"><b>{$item.ERROR}</b></td>
				</tr>
			{else}
				{if ($key % 2)}
					<tr class="rowxp0"> 
				{else}
					<tr class="rowxp1"> 
				{/if}

					<td>{$item.MAC}</td>
					<td style='border-left: solid 1px #BBBBBB;'>{$item.HEADER}</td>
					<td style='border-left: solid 1px #BBBBBB;'>{$item.OGROUP}</td>
					<td style='border-left: solid 1px #BBBBBB;'>{$item.BASE}</td>
					<td style='border-left: solid 1px #BBBBBB;'>{$item.FQDN}</td>
					<td style='border-left: solid 1px #BBBBBB;'>{$item.IP}</td>
					<td style='border-left: solid 1px #BBBBBB;'>{$item.DHCP}</td>
				</tr>
			{/if}
		{/foreach}
		</table>
		</div>
	{/if}
<br>
<p class='seperator'></p>
<div style='text-align:right;width:99%; padding-right:5px; padding-top:5px;'>
	<input type='submit' name='import_abort' value='{msgPool type=okButton}'>
</div>
<br>
