<div class="contentboxh">
 <p class="contentboxh"><img src="images/launch.png" align="right" alt="[F]">{t}Filter{/t}</p>
</div>
<div class="contentboxb">
 
<table summary="" width="100%" class="contentboxb" style="border-top:1px solid #B0B0B0; padding:0px;">
	<tr>
		<td width="50%">
			<img class="center" alt="" align="middle" border=0 src="plugins/systems/images/server.png">
				&nbsp;
			<LABEL FOR="host">{t}Server{/t}</LABEL>
			<select name="selected_Server">
			{foreach from=$Servers item=item key=key}
				<option {if $key == $selected_Server} selected {/if} 
					value='{$key}'>{$item.SERVER}</option>
			{/foreach}
			</select>
			<button type='submit' name='search'>{t}Search{/t}</button>

   		</td>
		<td width="50%">
			<input type='FILE' name='import'>
			<button type='submit' name='import_key'>{t}Import{/t}</button>

   		</td> 
	</tr>
</table>
</div>

<br>
<div style='border-top: solid 1px #BBBBBB;'>
<div class="contentboxb">
{$list}
</div>
</div>
