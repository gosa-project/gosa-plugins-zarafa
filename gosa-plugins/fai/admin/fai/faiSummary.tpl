<table style='width:100%; ' summary="">


 <tr style="background-color: #E8E8E8; height:26px;font-weight:bold;">
  <td style="padding:5px;">{t}FAI object tree{/t}</td>
 </tr>

{if $readable}
 <tr>
	<td style='padding-left:5px;padding-top:5px;padding-bottom:12px;'>

		{image path="images/lists/reload.png" action="reloadList"}	

		{t}Reload class and release configuration from parent object.{/t}
	</td>
 </tr>
 <tr style="background-color: #E8E8E8; ">
	<td>
 {$objectList}
	</td>
 </tr>
{else}
<tr>
	<td style='padding:6px;'>
		{t}You are not allowed to view the fai summary.{/t}
	</td>
</tr>
{/if}
</table>

