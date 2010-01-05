<div class="contentboxh">
 <p class="contentboxh">
  <img src="images/launch.png" align="right" alt="[F]">{t}Filter{/t}
 </p>
</div>

<div class="contentboxb">

<div style="border-top:1px solid #AAAAAA"></div>

<div style='padding:4px;'>
  <LABEL for='RELEASE'>{t}Release{/t}</LABEL>&nbsp;{$RELEASE}<br>
</div>

<div style="border-top:1px solid #AAAAAA"></div>

<div style='padding:4px;'>
  <input class="center" type="image" name="branch_branch" src="plugins/fai/images/branch_small.png">
  <a href="?plug={$plug}&act=branch_branch">{t}Create release{/t}</a>
  <br>
  <input class="center" type="image" name="freeze_branch" src="plugins/fai/images/freeze.png">
  <a href="?plug={$plug}&act=freeze_branch">{t}Create read-only release{/t}</a>
  <br>
  <input class="center" type="image" name="remove_branch" src="images/lists/trash.png">
  <a href="?plug={$plug}&act=remove_branch">{t}Delete current release{/t}</a>
</div>
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
