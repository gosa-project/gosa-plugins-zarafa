<script src="TreeMenu.js" language="JavaScript" type="text/javascript"></script>

<table style='width:100%; ' summary="{t}Share settings{/t}">

<tr>
  <td style='width:600px'>

  <div class="contentboxh">
    <p class="contentboxh">
     {t}DFS Shares{/t} {$hint}
    </p>
  </div>
  <div class="contentboxb">
      {$dfshead}
  </div>
  <div style='height:4px;'></div>
  <div class="contentboxb" style="border-top:1px solid #B0B0B0;">
      {$tree}
    <input type=hidden name="edit_helper">
  </div>
  </td>
  <td>

   <div class="contentboxh">
    <p class="contentboxh">{image path="{$infoimage}" align="right"}{t}Information{/t}
</p>
   </div>
   <div class="contentboxb">
   <p class="contentboxb">
     {t}This menu allows you to create, delete and edit selected dfs shares. Having a large numbers of dfs shares, you might prefer the range selectors on top of the dfs share list.{/t}
   </p>
   </div>
   <br>
   <div class="contentboxh">
    <p class="contentboxh">{image path="{$launchimage}" align="right"}{t}Filters{/t}
</p>
   </div>
   <div class="contentboxb">
     <table style='width:100%;' summary="{t}Share settings{/t}">

      {$alphabet}
     </table>
     <table style='width:100%;' summary="{t}Share settings{/t}">

     <tr>
     <td>
     <LABEL for "regex">{image path="{$search_image}" title="{t}Display dfs shares matching{/t}"}
</LABEL>
     </td>
     <td width="99%">
     <input type='text' name='regex' maxlength='20' style='width:99%' value='{*$regex*}' id='filter'
     title='{t}Regular expression for matching dfs share names{/t}' onChange="mainform.submit()">
     </td>
     </tr>
     </table>
                            {$apply}
   </div>
  </td>
</tr>
</table summary="">

<input type="hidden" name="ignore">
