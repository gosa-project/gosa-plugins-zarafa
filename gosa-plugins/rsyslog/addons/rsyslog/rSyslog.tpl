
<table width="100%">
  <tr>
    <td>{t}Server{/t}:</td>
    <td>   
      <select name='selected_server' onChange='document.mainform.submit();'>
        {foreach from=$servers item=item key=key}
          <option value='{$key}' {if $key == $selected_server} selected {/if}>{$item.cn}</option>
        {/foreach}
      </select>
    </td>
    <td>{t}Host{/t}:</td>
    <td>   
      <select name='selected_host' onChange='document.mainform.submit();'>
        {foreach from=$hosts item=item key=key}
          <option value='{$key}' {if $key == $selected_host} selected {/if}>{$item}</option>
        {/foreach}
      </select>
    </td>
    <td>{t}From{/t}:</td>
    <td>
      <input type="text" id="startTime" name="startTime" class="date" style='width:100px' value="{$startTime}">
      <script type="text/javascript">
        {literal}
        var datepicker  = new DatePicker({ relative : 'startTime', language : '{/literal}{$lang}{literal}', keepFieldEmpty : true,
           enableCloseEffect : false, enableShowEffect : false });
        {/literal}
      </script> 
    </td>
    <td>{t}to{/t}:</td>
    <td>
      <input type="text" id="stopTime" name="stopTime" class="date" style='width:100px' value="{$stopTime}">
      <script type="text/javascript">
        {literal}
        var datepicker  = new DatePicker({ relative : 'stopTime', language : '{/literal}{$lang}{literal}', keepFieldEmpty : true,
           enableCloseEffect : false, enableShowEffect : false });
        {/literal}
      </script> 
    </td>
    <td><input type='submit' name='search' value="{t}Search{/t}">
  </tr>
</table>

<p class="separator">&nbsp;</p>
{if $result.status != 'ok'}
  <b>{t}Error{/t}: &nbsp;{$result.status}</b><br>
  {$result.error}<br>
{else}

  <br>
  <table width="100%">
    <tr>
      <td>
        <b>
          <a href='?plug={$plug_id}&amp;sort_value=DeviceReportedTime'>{t}Received{/t}
            {if $sort_value=="DeviceReportedTime"}{if $sort_type=="DESC"}{$downimg}{else}{$upimg}{/if}{/if}
          </a>
        </b>
      </td>
      <td>
        <b>
          <a href='?plug={$plug_id}&amp;sort_value=FromHost'>{t}Host{/t}
            {if $sort_value=="FromHost"}{if $sort_type=="DESC"}{$downimg}{else}{$upimg}{/if}{/if}
          </a>
        </b>
      </td>
      <td>
        <b>
          <a href='?plug={$plug_id}&amp;sort_value=Facility'>{t}Facility{/t}
            {if $sort_value=="Facility"}{if $sort_type=="DESC"}{$downimg}{else}{$upimg}{/if}{/if}
          </a>
        </b>
      </td>
      <td>
        <b>
          <a href='?plug={$plug_id}&amp;sort_value=Priority'>{t}Priority{/t}
            {if $sort_value=="Priority"}{if $sort_type=="DESC"}{$downimg}{else}{$upimg}{/if}{/if}
          </a>
        </b>
      </td>
      <td>
        <b>
          <a href='?plug={$plug_id}&amp;sort_value=Message'>{t}Message{/t}
            {if $sort_value=="Message"}{if $sort_type=="DESC"}{$downimg}{else}{$upimg}{/if}{/if}
          </a>
        </b>
      </td>
    </tr>
    <tr><td colspan="5"><p class="separator">&nbsp;</p></td></tr>
    {foreach from=$result.entries item=item key=key}
    <tr>
      <td title='{$item.DeviceReportedTime}' style='width:120px'>
        {$item.DeviceReportedTime}
      </td>
      <td title='{$item.FromHost}'>
        {$item.FromHost}
      </td>
      <td title='{$item.Facility}'>
        {$item.Facility}
      </td>
      <td title='{$item.Priority}'>
        {$item.Priority}
      </td>
      <td title='{$item.Message}' style="width:600px">
        <div style='overflow:hidden; width:600px'><nobr>{$item.Message}</nobr></div>
      </td>
    </tr>
    {/foreach}
  </table>
  {if !$result.count == 0}
  <p class="separator">&nbsp;</p>
  {/if}
  <div style='width:40%;float:left;'>{$result.count} {t}entries matching filter{/t}</div>
  <div style='width:80px;float:right;'>
    <select name='limit' onChange='document.mainform.submit();'>
      {html_options options=$limits selected=$limit}
    </select>
  </div>
  <div style='width:300px;float:left;'>{$page_sel}</div>
{/if}

