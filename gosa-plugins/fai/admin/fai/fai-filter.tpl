<div class="contentboxh">
 <p class="contentboxh">
  <img src="images/launch.png" align="right" alt="[F]">{t}Filter{/t}
 </p>
</div>

<div class="contentboxb">

<div style="border-top:1px solid #AAAAAA"></div>

 <table summary="" style="width:100%;">
  <tr>
   <td>
     <LABEL for='RELEASE'>{t}Release{/t}</LABEL>&nbsp;{$RELEASE}<br>
   </td>
  </tr>
 </table>
<div style="border-top:1px solid #AAAAAA"></div>

{$PROFILE}   {t}Show profiles{/t}
<br>{$TEMPLATE}  {t}Show templates{/t}
<br>{$SCRIPT}    {t}Show scripts{/t}
<br>{$HOOK}      {t}Show hooks{/t}
<br>{$VARIABLE}  {t}Show variables{/t}
<br>{$PACKAGE}   {t}Show packages{/t}
<br>{$PARTITION} {t}Show partition{/t}

 <table summary="" style="width:100%;border-top:1px solid #B0B0B0;">
  <tr>
   <td>
    <label for="NAME">
     <img src="images/lists/search.png" align=middle>&nbsp;Name
    </label>
   </td>
   <td>
    {$NAME}
   </td>
  </tr>
 </table>

 <table summary=""  width="100%"  style="background:#EEEEEE;border-top:1px solid #B0B0B0;">
  <tr>
   <td width="100%" align="right">
    {$APPLY}
   </td>
  </tr>
 </table>
</div>
