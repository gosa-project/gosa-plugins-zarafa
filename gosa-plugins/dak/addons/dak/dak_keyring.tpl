<div class="contentboxh">
 <p class="contentboxh">{image path="images/launch.png" align="right"}{t}Filter{/t}
</p>
</div>
<div class="contentboxb">
 
<table style='padding:0px;' class='contentboxb' summary="{t}DAK key ring{/t}">

	<tr>
		<td width="50%">
			{image path="plugins/systems/images/server.png"}

				&nbsp;
			<LABEL FOR="host">{t}Server{/t}</LABEL>
			<select name="selected_Server" size=1>
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
