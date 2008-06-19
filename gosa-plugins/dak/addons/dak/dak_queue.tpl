<div class="contentboxh">
 <p class="contentboxh"><img src="images/launch.png" align="right" alt="[F]">{t}Filter{/t}</p>
</div>
<div class="contentboxb">
 
<table summary="" width="100%" class="contentboxb" style="border-top:1px solid #B0B0B0; padding:0px;">
	<tr>
		<td width="33%">
			<img class="center" alt="" align="middle" border=0 src="images/server.png">
				&nbsp;
			<LABEL FOR="host">{t}Repositories{/t}</LABEL>
			<select name="selected_Repository" onChange='document.mainform.submit();'>
			{foreach from=$Repositories item=item key=key}
				<option {if $key == $selected_Repository} selected {/if} 
					value='{$key}'>{$item.SERVER} - {$item.RELEASE}</option>
			{/foreach}
			</select>
   		</td>
	</tr>
</table>
