<table summary="" style="width:100%;">
 <tr>
  <td style='width:45%; '>

  <LABEL for="admins"> {t}Folder administrators{/t}</LABEL>
   <br>
   <select id="admins" style="width:380px; height:300px;" name="admins[]" size=15 multiple>
    {html_options options=$admins}
   </select>
  </td>

  <td style='width:10%; text-align:center;'>

   <button type='submit' name='add_users'>&larr;</button>

   <br>
   <br>
   <button type='submit' name='del_users'>&rarr;</button>

  </td>

  <td style='width:45%; '>

   <br>
   <select style="width:380px; height:275px;" name="users[]" size=15 multiple>
    {html_options options=$mailusers}
   </select>
   <br>
	<LABEL for="department">{t}Select a specific department{/t}</LABEL>:
   <select id="department" name="department" size=1 onChange="mainform.submit()">
    {html_options options=$departments selected=$department}
   </select>

   {if $javascript ne "true"}
   <button type='submit' name='goButton'>{t}Choose{/t}</button>

   {/if}
  </td>
 </tr>
</table>

<hr>
<div class="plugin-actions">
 <button type='submit' name='edit_admins_finish'>{msgPool type=applyButton}</button>

 <button type='submit' name='edit_admins_cancel'>{msgPool type=cancelButton}</button>

</div>

