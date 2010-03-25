<div class="contentboxh">
 <p class="contentboxh">{image path="images/launch.png" align="right"}{t}Filter{/t}
</p>
</div>
<div class="contentboxb">
 
<table style='padding:0px;' class='contentboxb' summary="{t}DAK queue{/t}"> 

	<tr>
		<td width="33%">
			{image path="plugins/systems/images/server.png"}

				&nbsp;
			<LABEL FOR="host">{t}Repositories{/t}</LABEL>
			<select name="selected_Repository" onChange='document.mainform.submit();' size=1>
			{foreach from=$Repositories item=item key=key}
				<option {if $key == $selected_Repository} selected {/if} 
					value='{$key}'>{$item.SERVER} - {$item.RELEASE}</option>
			{/foreach}
			</select>
   		</td>
	</tr>
</table>
