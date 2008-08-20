
<h2>Opsi host</h2>


<table style="width: 100%;">
 <tr>
  <td colspan="2">
   <table>
    <tr>
     <td>{t}Boot product{/t}</td>
     <td>
      <select name="opsi_netboot_product">
		{foreach from=$ANP item=item key=key}
			<option {if $key == $SNP} selected {/if} value="{$key}">{$key}</option>
		{/foreach}
      </select>
     </td>
    </tr>
   </table>
  <td>
 </tr>
 <tr>
  <td style="width:50%;"><h2>Installed products</h2>
	{$divSLP}
  </td>
  <td style="width:50%;"><h2>Available products</h2>
	{$divALP}
  </td>
 </tr>
</table> 
<input type='hidden' name='opsi_generic' value='1'>
