
<div id="mainlist">
 <div class="mainlist-header">
  <p>{t}System logs{/t}
  </p>
  <div class="mainlist-nav">
   <table summary="{t}Filter{/t}" style="width: 100%;"      id="t_scrolltable" cellpadding="0" cellspacing="0">
    <tr>
     <td>{t}Search{/t}
      <select id="object_type" style='width:220px' name="object_type" onChange="mainform.submit()" title="{t}Choose the object that will be searched in{/t}" size=1>
       {html_options options=$objlist selected=$object_type}<option disabled>&nbsp;</option>
      </select>
      <input id="search_for" type='text' name='search_for' maxlength='20' value='{$search_for}' title='{t}Search string{/t}' onChange="mainform.submit()">
      <button name="apply" type='submit'>{t}Filter anwenden{/t}</button>
     </td>
     <td class='left-border'>
      <input type="checkbox" name="organizational" value="1" {$organizational} onClick="mainform.submit()" title="{t}Select to see regular users{/t}">{t}Show organizational entries{/t}
     </td>
    </tr>
    <tr>
     <td>{t}Display results for department{/t}
      <select name="search_base" style='width:220px' onChange="mainform.submit()"             title="{t}Choose the department the search will be based on{/t}" size=1>
       {html_options options=$deplist selected=$depselect}<option disabled>&nbsp;</option>
      </select>
     </td>
     <td class='left-border'>
      <input type="checkbox" name="global" value="1" {$global} onClick="mainform.submit()" title="{t}Select to see users in addressbook{/t}">{t}Show addressbook entries{/t}
     </td>
    </tr>
   </table>
  </div>
 </div>
 <div class="mainlist-header">
  <p>{t}Actions{/t}
  </p>
  <div class="mainlist-nav" style='padding: 3px;'>
   
   {if $internal_createable}
    {image path='images/lists/element.png[new]'}
    <a href="main.php{$plug}&amp;global=add" style="text-align:center;vertical-align:middle;">{t}Add entry{/t}</a>
    
   {/if}
   
   {if $internal eq 0}
    
    {if $internal_editable}
     {image path='images/lists/edit.png'}
     <a href="main.php{$plug}&amp;global=edit">{t}Edit entry{/t}</a>
     
    {/if}
    
    {if $internal_removeable}
     {image path='images/lists/trash.png'}
     <a href="main.php{$plug}&amp;global=remove" style="vertical-align:middle;">{t}Remove entry{/t}</a>
     
    {/if}
    
   {/if}
  </div>
 </div>
 <div class="listContainer">

  <table width='100%' summary="{t}Entry list{/t}" cellpadding="0" cellspacing="0">
   <thead class="fixedListHeader listHeaderFormat">
   <tr>
    <td class="listheader">{t}Name{/t}
    </td>
    <td class="listheader">{t}Phone{/t}
    </td>
    <td class="listheader">{t}Fax{/t}
    </td>
    <td class="listheader">{t}Mobile{/t}
    </td>
    <td class="listheader">{t}Private{/t}
    </td>
    <td class="listheader">{t}Contact{/t}
    </td>
   </tr>
  </thead>
   <tbody {if $show_info eq 1} style='height:50px;' {else} style='height:250px; width:100%;' {/if}
      class="listScrollContent listBodyFormat">
    {$search_result}
   </tbody>
  </table>
 </div>
 <div class="nlistFooter"  style='width:100%; text-align: center;'>
  {$range_selector}
 </div>
 
 {if $show_info eq 1}
  {include file=$address_info}
 {/if}
</div>
<br>
<script language="JavaScript" type="text/javascript"><!-- // First input field on page	  focus_field('search_for');  --></script>
