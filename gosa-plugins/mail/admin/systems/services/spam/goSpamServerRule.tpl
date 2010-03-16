<h3>Edit spam rule</h3>

<table style='width:100%;'>
 <tr>
  <td width='70'>
   {t}Name{/t}
  </td>
  <td>
   <input type='text' name='name' value='{$name}' >
  </td>
 </tr>
 <tr>
  <td colspan=2>
   {t}Rule{/t}
   <textarea name='rule' style='width:100%;'>{$rule}</textarea>
  </td>
 </tr>
</table>
<hr>
<div style="width:100%; text-align:right;padding-top:10px;padding-bottom:3px;">
    <button type='submit' name='SaveRule'>{msgPool type=saveButton}</button>

    &nbsp;
    <button type='submit' name='CancelRule'>{msgPool type=cancelButton}</button>

</div>

