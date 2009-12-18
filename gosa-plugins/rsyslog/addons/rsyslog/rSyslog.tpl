
<select name='selected_server' onChange='document.mainform.submit();'>
  {foreach from=$servers item=item key=key}
    <option value='{$key}' {if $key == $selected_server} selected {/if}>{$item.cn}</option>
  {/foreach}
</select>

<select name='selected_host' onChange='document.mainform.submit();'>
  {foreach from=$hosts item=item key=key}
    <option value='{$key}' {if $key == $selected_host} selected {/if}>{$item}</option>
  {/foreach}
</select>

<p class="separator">&nbsp;</p>
{if $result.status != 'ok'}
  <b>{t}Error{/t}: &nbsp;{$result.status}</b><br>
  {$result.error}<br>
{else}
  <b>{t}Results{/t}:</b>
  <table>
    {foreach from=$result.entries item=item key=key}
    <tr>
      <td>{$item.DeviceReportedTime}</td>
      <td>{$item.FromHost}</td>
      <td>{$item.Facility}</td>
      <td>{$item.Priority}</td>
      <td>{$item.Message}</td>
    </tr>
    {/foreach}
  </table>
  {$result.count} {t}entries matching filter{/t}
{/if}

