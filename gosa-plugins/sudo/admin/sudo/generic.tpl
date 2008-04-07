<table style="width: 90%;">
 <tr>
  <td style="vertical-align:top;width:50%">
   <h2>Sudo generic</h2>
   <table> 
    <tr>
     <td>
      {t}Name{/t}
     </td>
     <td>
      <input type="text" name="cn" value="{$cn}">
     </td>
    </tr>
    <tr>
     <td>
      {t}Description{/t}
     </td>
     <td>
      <input type="text" name="description" value="{$description}">
     </td>
    </tr>
   </table>
	</td>
	<td>
   <h2><img alt="" class="center" align="middle" src="images/closedlock.png" /> {t}System trust{/t}</h2>
    {t}Trust mode{/t}&nbsp;
    {render acl=$trustmodeACL}
        <select name="trustmode" id="trustmode" size=1
            onChange="changeSelectState('trustmode', 'wslist');
                      changeSelectState('trustmode', 'add_ws');
                      changeSelectState('trustmode', 'del_ws');">
          {html_options options=$trustmodes selected=$trustmode}
        </select>
    {/render}
    {render acl=$trustmodeACL}
       <select style="width:100%" id="wslist" name="workstation_list[]" size=5 multiple {$trusthide}>
        {html_options values=$workstations output=$workstations}
        {if $emptyArrAccess}
            <option disabled>&nbsp;</option>
        {/if}
       </select>
    {/render}
       <br>
    {render acl=$trustmodeACL}
       <input type="submit" id="add_ws" value="{msgPool type=addButton}" name="add_ws" {$trusthide}>&nbsp;
    {/render}
    {render acl=$trustmodeACL}
       <input type="submit" id="del_ws" value="{msgPool type=delButton}" name="delete_ws" {$trusthide}>
    {/render}

  </td>
 </tr> 
 <tr><td style="width:100%;"colspan="2"><p class="seperator">&nbsp;</p></td></tr>
 <tr>
  <td style="width:50%"><b>User / Groups</b>
   {$divlist_sudoUser}
   <input type='text' value='' name='new_sudoUser'><input type='submit' name='add_sudoUser' value='{msgPool type=addButton}'>
   <input type='submit' name='list_sudoUser' value='{t}Add from list{/t}'>
  </td>
  <td><b>Hosts</b>
   {$divlist_sudoHost}
   <input type='text' value='' name='new_sudoHost'><input type='submit' name='add_sudoHost' value='{msgPool type=addButton}'>
   <input type='submit' name='list_sudoHost' value='{t}Add from list{/t}'>
  </td>
 </tr> 
 <tr><td style="width:100%;"colspan="2"><p class="seperator">&nbsp;</p></td></tr>
 <tr>
  <td style="border-left: solid 1px #AAAAAA;"><b>Commands</b>
   {$divlist_sudoCommand}
   <input type='text' value='' name='new_sudoCommand'><input type='submit' name='add_sudoCommand' value='{msgPool type=addButton}'>
  </td>
  <td style="border-left: solid 1px #AAAAAA;"><b>Run as</b>
   {$divlist_sudoRunas}
   <input type='text' value='' name='new_sudoRunas'><input type='submit' name='add_sudoRunas' value='{msgPool type=addButton}'>
  </td>
 </tr>
</table>
