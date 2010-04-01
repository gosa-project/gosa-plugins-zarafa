<div id="mainlist">
 <div class="mainlist-header">
  <p>{t}System logs{/t}</p>

  <div class="mainlist-nav">
   <table summary="{t}Filter{/t}" style="width: 100%;"
      id="t_scrolltable" cellpadding="0" cellspacing="0">
    <tr>
     <td>{t}Server{/t}
      <select size="1" name="selected_server" title="{t}Select server to search on{/t}" onChange="mainform.submit()">
       {html_options options=$servers selected=$selected_server}
      </select>
     </td>
     <td>{t}Date{/t}
      <select size="1" name="month" onChange="mainform.submit()">
       {html_options options=$months selected=$month_select}
      </select>
      <select size="1" name="year" onChange="mainform.submit()">
       {html_options values=$years output=$years selected=$year_select}
      </select>
     </td>
     <td>{t}Search for{/t}
      <input type='text' name="search_for" size=25 maxlength=60 
        value="{$search_for}" title="{t}Enter user name to search for{/t}" 
        onChange="mainform.submit()">
     </td>
     <td>
      <button type='submit' name='search'>{t}Search{/t}</button>
     </td>
    </tr>
   </table>

  </div>
 </div>
</div>

<br>

{if $search_result}

 <div class="listContainer" id="d_scrollbody" style="min-height: 475px; height: 444px;">
  <table summary="{t}Phone reports{/t}" style="width:100%;" cellpadding="0" cellspacing="0">
   <thead class="fixedListHeader listHeaderFormat">
    <tr>
     <td class='listheader'><a href="main.php{$plug}&amp;sort=0">{t}Date{/t} {$mode0}</a></td>
     <td class='listheader'><a href="main.php{$plug}&amp;sort=1">{t}Source{/t} {$mode1}</a></td>
     <td class='listheader'><a href="main.php{$plug}&amp;sort=2">{t}Destination{/t} {$mode2}</a></td>	
     <td class='listheader'><a href="main.php{$plug}&amp;sort=3">{t}Channel{/t} {$mode3}</a></td>	
     <td class='listheader'><a href="main.php{$plug}&amp;sort=4">{t}Application{/t} {$mode4}</a></td>	
     <td class='listheader'><a href="main.php{$plug}&amp;sort=5">{t}Status{/t} {$mode5}</a></td>	
     <td class='listheader'><a href="main.php{$plug}&amp;sort=6">{t}Duration{/t} {$mode6}</a></td>	
    </tr>
   </thead>
   <tbody class="listScrollContent listBodyFormat" id="t_nscrollbody">
 {$search_result}
     <tr>
      <td class="list0">&nbsp;</td>
      <td class="list0">&nbsp;</td>
      <td class="list0">&nbsp;</td>
      <td class="list0">&nbsp;</td>
      <td class="list0">&nbsp;</td>
      <td class="list0">&nbsp;</td>
     </tr>
   </tbody>
  </table>

 </div>
 <div class="nlistFooter">
  <div style='width:100%;'>
   {$range_selector}
  </div>
 </div>

{else}
 <hr>
 <b>{t}Search returned no results...{/t}</b>
{/if}

<!-- Place cursor -->
<script language="JavaScript" type="text/javascript">
  <!-- // First input field on page
	focus_field('search_for');
  -->
</script>
