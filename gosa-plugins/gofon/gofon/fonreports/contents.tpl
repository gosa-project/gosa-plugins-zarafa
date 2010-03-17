<div class="contentboxh">
 <p class="contentboxh">{image path="{$launchimage}" align="right"}{t}Filter{/t}
</p>
</div>
<div class="contentboxb">
 <p class="contentboxb" style="border-top:1px solid #B0B0B0; padding-top:5px;">
 {image path="{$search_image}"}&nbsp;{t}Search for{/t}

 <input type='text' name="search_for" size=25 maxlength=60 value="{$search_for}" title="{t}Enter user name to search for{/t}" onChange="mainform.submit()">
 {t}in{/t}
 <select size="1" name="search_base" title="{t}Select subtree to base search on{/t}" onChange="mainform.submit()">
  {html_options options=$bases selected=$base_select}
 </select>
 {t}on{/t}
 <select size="1" name="selected_server" title="{t}Select server to search on{/t}" onChange="mainform.submit()">
  {html_options options=$servers selected=$selected_server}
 </select>
 {t}during{/t}
 <select size="1" name="month" onChange="mainform.submit()">
  {html_options options=$months selected=$month_select}
 </select>
 {t}in{/t} 
 <select size="1" name="year" onChange="mainform.submit()">
  {html_options values=$years output=$years selected=$year_select}
 </select>
 &nbsp;
 <button type='submit' name='search'>{t}Search{/t}</button>

</p>
</div>

<br>

{if $search_result}
 <table style='width:100%; '>  <tr style="background-color: #E8E8E8; height:26px; font-weight:bold">
   <td><a href="main.php{$plug}&amp;sort=0">{t}Date{/t} {$mode0}</a></td>
   <td><a href="main.php{$plug}&amp;sort=1">{t}Source{/t} {$mode1}</a></td>
   <td><a href="main.php{$plug}&amp;sort=2">{t}Destination{/t} {$mode2}</a></td>	
   <td><a href="main.php{$plug}&amp;sort=3">{t}Channel{/t} {$mode3}</a></td>	
   <td><a href="main.php{$plug}&amp;sort=4">{t}Application{/t} {$mode4}</a></td>	
   <td><a href="main.php{$plug}&amp;sort=5">{t}Status{/t} {$mode5}</a></td>	
   <td><a href="main.php{$plug}&amp;sort=6">{t}Duration{/t} {$mode6}</a></td>	
  </tr>
  {$search_result}
 </table>

 <table style='width:100%; text-align:center;'>  <tr>
   <td>{$range_selector}</td>
  </tr>
 </table>

<hr>
{else}
  <b>{t}Search returned no results...{/t}</b>
{/if}

<!-- Place cursor -->
<script language="JavaScript" type="text/javascript">
  <!-- // First input field on page
	focus_field('search_for');
  -->
</script>
