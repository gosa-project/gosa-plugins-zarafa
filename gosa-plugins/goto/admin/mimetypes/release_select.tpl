<div class="contentboxh" style="border-bottom:1px solid #B0B0B0;">
 <p class="contentboxh">{image path="{$branchimage}" align="right"}{t}Branches{/t}
</p>
</div>
<div class="contentboxb">
 <table summary="" style="width:100%;">
  <tr>
   <td>
    {t}Current release{/t}&nbsp;
    <select name="mime_release" onChange="document.mainform.submit();" size=1>
        {html_options output=$mime_releases values=$mime_releases selected=$mime_release}
    </select>
   </td>
  </tr>
 </table>
</div>
<br>
