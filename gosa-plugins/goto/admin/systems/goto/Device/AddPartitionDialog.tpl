<h3>{t}Add Partition{/t}</h3>

<table>
    <tr>
        <td>{t}Mount point{/t}</td>
        <td>
            {if $fsType == "raid" || $fsType == "swap" || $fsType == "pv"}
                <input disabled type="text" name="mountPoint" value=" - ">
            {else}
                <input type="text" name="mountPoint" value="{$mountPoint}">
            {/if}
        </td>
    </tr>
    <tr>
        <td>{t}File system type{/t}</td>
        <td>
            <select name="fsType" onChange="document.mainform.submit();">
                {html_options options=$fsTypes selected=$fsType}
            </select>
        </td>
    </tr>
    <tr>
        <td>{t}Allowable drives{/t}</td>
        <td>
            {foreach from=$disks item=item key=key}
                <input type="checkbox" {if $disk_selected[$item]} checked {/if} 
                    name="disk_selected_{$item}">{$item}
            {/foreach}
        </td>
    </tr>
    <tr>
        <td>{t}Size{/t}</td>
        <td>
            <input name="size" value="{$size}">
        </td>
    </tr>
    <tr>
        <td><input type="checkbox" name="forcePrimary" {if $forcePrimary_selected} checked {/if}></td>
        <td>{t}Force to be primary partition{/t}</td>
    </tr>
    <tr>
        <td><input type="checkbox" name="encrypt" {if $encrypt_selected} checked {/if}></td>
        <td>{t}Encrypt{/t}</td>
    </tr>
</table>

<hr>

<h3>{t}Additional size options{/t}</h3>
<table>
    <tr>
        <td><input type="radio" name="size_options" value="0" 
                onClick="document.mainform.submit();"
                {if $size_options==0} checked {/if}></td>
        <td>{t}Fixed size{/t}</td>
    </tr>
    <tr>
        <td><input type="radio" name="size_options" value="1" 
                onClick="document.mainform.submit();"
                {if $size_options==1} checked {/if}></td>
        <td>{t}Fill all space up to{/t} 
            <input {if $size_options != 1} disabled {/if}
                    id="size_max_value"
                    type="text" value="{$size_max_value}">&nbsp;{t}MB{/t}
        </td>
    </tr>
    <tr>
        <td><input type="radio" name="size_options" value="2" 
                onClick="document.mainform.submit();"
                {if $size_options==2} checked {/if}></td>
        <td>{t}Fill to maximum allowable size{/t}</td>
    </tr>
</table>

<hr>
<div class="clear"></div>

<div class="plugin-actions">
  <button type='submit' name='save_partition_add'>{msgPool type=addButton}</button>
  <button type='submit' name='cancel_partition_add'>{msgPool type=cancelButton}</button>
</div>


