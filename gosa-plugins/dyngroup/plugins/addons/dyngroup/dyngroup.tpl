<table summary="" style="width:100%; vertical-align:top; text-align:left;" cellpadding="0" border="0">
 <tr>
  <td style="width:100%; vertical-align:top;">
   <h2><img class="center" alt="" align="middle" src="images/rightarrow.png" />&nbsp;{t}Generic{/t}</h2>
   <table summary="">
    <tr>
     <td><label for="{$memberURLAttributeLabel}">{t}{$memberURLAttributeLabel}{/t}</label></td>
     <td>
       {foreach item=line from=$memberURLAttributeValue}
       <input size=70 name="{$memberURLAttributeLabel}" value="{$line}">
       {/foreach}
     </td>
    </tr>
   </table>
  </td>
 </tr>
</table>
