<h3>{t}Generic{/t}</h3>


<table summary="{t}Labeled uri definitions{/t}" style='width:100%'>
 <tr>
  <td>{t}Base{/t}</td>
  <td></td>
  <td>{t}Attribute{/t}</td>
  <td>{t}Filter{/t}</td>
  <td></td>
 </tr>
{foreach item=item key=key from=$labeledURIparsed}
 <tr>
  <td>
    <input type='text' value='{$item.base}'>
  </td>
  <td>
    <select name='scope' size='1'>
     {html_options options=$scopes selected=$item.scope}
    </select>
  </td>
  <td><input type='text' name='attr_{$key}' value='{$item.attr}'></td>
  <td style='width:50%;'><input name='filter_{$key}' type='text' style='width:98%;' value='{$item.filter}'></td>
  <td><button name='delUri_{$key}'>{msgPool type='delButton'}</button></td>
 </tr>
{/foreach}
 <tr>
  <td colspan="4"></td>
  <td><button name='addUri'>{msgPool type='addButton'}</button></td>
 </tr>
</table>
