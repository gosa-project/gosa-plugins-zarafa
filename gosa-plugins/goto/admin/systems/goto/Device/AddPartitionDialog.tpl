{if $error}

    <p>
    {$errorMsg}
    </p>
    <button type='submit' name='retry'>{t}Retry{/t}</button>

    <hr>
    <div class="clear"></div>

    <div class="plugin-actions">
      <button type='submit' name='cancel_partition_add'>{msgPool type=cancelButton}</button>
    </div>

{else}

    <h3>{t}Type{/t}</h3>

    <input {if $selected_type==0} checked {/if} onClick="document.mainform.submit();"
            type="radio" value="0" name="selected_type">{t}Disk{/t}<br>
    <input {if $selected_type==1} checked {/if} onClick="document.mainform.submit();"
            {if !count($disks)} disabled {/if}
            type="radio" value="1" name="selected_type">{t}Physical partition{/t}<br>
    <input  {if count($freeRaidPartitions) < 1} disabled {/if}
            {if $selected_type==2} checked {/if} onClick="document.mainform.submit();"
            type="radio" value="2" name="selected_type">{t}Raid device{/t}<br>
    <input  {if !count($freeLvmPartitions)} disabled {/if}
            {if $selected_type==3} checked {/if} onClick="document.mainform.submit();"
            type="radio" value="3" name="selected_type">{t}LVM Volume group{/t}<br>
    <input {if $selected_type==4} checked {/if} onClick="document.mainform.submit();"
            {if !count($volumeGroupList)} disabled {/if}
            type="radio" value="4" name="selected_type">{t}LVM Volume{/t}<br>

    <hr>

    {if $selected_type==4}
        
        <h3>{t}LVM Volume{/t}</h3>
        <table>
            <tr>
                <td>{t}Volume name{/t}</td>
                <td>
                    <input type="text" name="v_name" value="{$v_name}">
                </td>
            </tr>
            <tr>
                <td>{t}Volume group{/t}</td>
                <td>
                    <select name="v_group">
                        {foreach from=$volumeGroupList item=item}
                            <option value="{$item}"
                                {if $item==$v_group} selected {/if}
                                >{$item} {if isset($deviceUsage.vg[$item])} - ({$deviceUsage.vg[$item].size - $deviceUsage.vg[$item].usage} {t}MB{/t} {t}free{/t}){/if}</option>
                        {/foreach}
                    </select>
                </td>
            </tr>
            <tr>
                <td>{t}Mount point{/t}</td>
                <td>
                    {if $v_fsType == "swap"}
                        <input disabled type="text" name="v_mountPoint" value=" - ">
                    {else}
                        <input type="text" name="v_mountPoint" value="{$v_mountPoint}">
                    {/if}
                </td>
            </tr>
            <tr>
                <td>{t}File system type{/t}</td>
                <td>
                    <select name="v_fsType" onChange="document.mainform.submit();">
                        {html_options options=$fsTypes selected=$v_fsType}
                    </select>
                </td>
            </tr>
            <tr>
                <td>{t}Size{/t}</td>
                <td>
                    <input name="v_size" value="{$v_size}">
                </td>
            </tr>
            <tr>
                <td>{t}Encrypt{/t}</td>
                <td><input type="checkbox" name="v_encrypt" {if $v_encrypt_selected} checked {/if}></td>
            </tr>
        </table>

    {elseif $selected_type==3}
        
        <h3>{t}LVM Volume group{/t}</h3>
        <table>
            <tr>
                <td>{t}Volume group name{/t}</td>
                <td>
                    <input type="text" name="vg_name" value="{$vg_name}">
                </td>
            </tr>
            <tr>
                <td>{t}Use LVM partitions{/t}</td>
                <td>
                    {foreach from=$freeLvmPartitions item=item key=key}
                        <input type="checkbox" name="vg_partition_{$key}" 
                            {if in_array($item, $vg_partitions)} checked {/if}>&nbsp;{$item}
                            {if isset($deviceUsage.part[$item])} &nbsp;&nbsp; {$deviceUsage.part[$item].size} {t}MB{/t}
                            {elseif isset($deviceUsage.raid[$item])} &nbsp;&nbsp; {$deviceUsage.raid[$item].size} {t}MB{/t}{/if}    
                            <br>
                    {/foreach}
                </td>
            </tr>
        </table>

    {elseif $selected_type==2}
        <h3>{t}Add raid device{/t}</h3>

        <table>
            <tr>
                <td>{t}Mount point{/t}</td>
                <td>
                    {if $r_fsType == "swap" || $r_fsType == "pv"}
                        <input disabled type="text" name="r_mountPoint" value=" - ">
                    {else}
                        <input type="text" name="r_mountPoint" value="{$r_mountPoint}">
                    {/if}
                </td>
            </tr>
            <tr>
                <td>{t}File system type{/t}</td>
                <td>
                    <select name="r_fsType" onChange="document.mainform.submit();">
                        {html_options options=$fsTypes selected=$r_fsType}
                    </select>
                </td>
            </tr>
            <tr>
                <td>{t}Raid level{/t}</td>
                <td>
                    <select name="r_raidLevel">
                        {html_options options=$raidLevelList selected=$r_raidLevel}
                    </select>
                </td>
            </tr>
            <tr>
                <td>{t}Use raid partitions{/t}</td>
                <td>
                    {foreach from=$freeRaidPartitions item=item key=key}
                        <input type="checkbox" name="r_partition_{$key}" 
                            {if in_array($item, $r_partitions)} checked {/if}>&nbsp;{$item}
                            {if isset($deviceUsage.part[$item])} &nbsp;&nbsp;{$deviceUsage.part[$item].size} {t}MB{/t}
                            {elseif isset($deviceUsage.raid[$item])} &nbsp;&nbsp;{$deviceUsage.raid[$item].size} {t}MB{/t}{/if}    
                            <br>
                    {/foreach}
                </td>
            </tr>
            <tr>
                <td>{t}Number of spares{/t}</td>
                <td>
                    <input type="text" value="{$r_spares}" name="r_spares">
                </td>
            </tr>
            <tr>
                <td>{t}Encrypt{/t}</td>
                <td><input type="checkbox" name="r_encrypt" {if $r_encrypt_selected} checked {/if}></td>
            </tr>
        </table>

    {elseif $selected_type==1}

        <table width="100%">
            <tr>
                <td style="width:50%">
                    <h3>{t}Add Partition{/t}</h3>
                    <table>
                        <tr>
                            <td>{t}Mount point{/t}</td>
                            <td>
                                {if $p_fsType == "raid" || $p_fsType == "swap" || $p_fsType == "pv"}
                                    <input disabled type="text" name="p_mountPoint" value=" - ">
                                {else}
                                    <input type="text" name="p_mountPoint" value="{$p_mountPoint}">
                                {/if}
                            </td>
                        </tr>
                        <tr>
                            <td>{t}File system type{/t}</td>
                            <td>
                                <select name="p_fsType" onChange="document.mainform.submit();">
                                    {html_options options=$fsTypes selected=$p_fsType}
                                </select>
                            </td>
                        </tr>
                        <tr>
                            <td>{t}Allowable drives{/t}</td>
                            <td>
                                <select name="p_used_disk" onChange="document.mainform.submit();">
                                {foreach from=$disks item=disk}
                                    <option value="{$disk}"
                                        {if $disk==$p_used_disk} selected {/if}
                                        >{$disk} {if isset($deviceUsage.disk[$p_used_disk])} - ({$deviceUsage.disk[$p_used_disk].size - $deviceUsage.disk[$p_used_disk].usage} {t}MB{/t} {t}free{/t}){/if}</option>
                                {/foreach}
                                </select>
                            </td>
                        </tr>
                        <tr>
                            <td>{t}Size{/t}</td>
                            <td>
                                <input name="p_size" value="{$p_size}">
                            </td>
                        </tr>
                        <tr>
                            <td>{t}Force to be primary partition{/t}</td>
                            <td><input type="checkbox" name="p_forcePrimary" {if $p_forcePrimary_selected} checked {/if}></td>
                        </tr>
                        <tr>
                            <td>{t}Bootable{/t}</td>
                            <td><input type="checkbox" name="p_bootable" {if $p_bootable_selected} checked {/if}></td>
                        </tr>
                        <tr>
                            <td>{t}Encrypt{/t}</td>
                            {if $p_fsType == "raid" || $p_fsType == "swap" || $p_fsType == "pv"}
                                <td><input disabled type="checkbox" name="p_encrypt"></td>
                            {else}
                                <td><input type="checkbox" name="p_encrypt" {if $p_encrypt_selected} checked {/if}></td>
                            {/if}
                        </tr>
                        <tr>
                            <td>{t}Format{/t}</td>
                            {if $p_fsType == "raid" || $p_fsType == "swap" || $p_fsType == "pv"}
                                <td><input disabled type="checkbox" name="p_format"></td>
                            {else}
                                <td><input type="checkbox" name="p_format" {if $p_format_selected} checked {/if}></td>
                            {/if}
                        </tr>
                    </table>
                </td>
                <td class="left-border"></td>
                <td>

                    <h3>{t}Additional size options{/t}</h3>
                    <table>
                        <tr>
                            <td><input type="radio" name="p_size_options" value="0" 
                                    onClick="document.mainform.submit();"
                                    {if $p_size_options==0} checked {/if}></td>
                            <td>{t}Fixed size{/t}</td>
                        </tr>
                        <tr>
                            <td><input type="radio" name="p_size_options" value="2" 
                                    onClick="document.mainform.submit();"
                                    {if $p_size_options==2} checked {/if}></td>
                            <td>{t}Fill to maximum allowable size{/t}</td>
                        </tr>
                        <tr>
                            <td><input type="radio" name="p_size_options" value="1" 
                                    onClick="document.mainform.submit();"
                                    {if $p_size_options==1} checked {/if}></td>
                            <td>{t}Fill all space up to{/t} 
                                <input {if $p_size_options != 1} disabled {/if}
                                        id="p_size_max_value"
                                        name="p_size_max_value"
                                        type="text" value="{$p_size_max_value}">&nbsp;{t}MB{/t}
                            </td>
                        </tr>
                    </table>
                </td>
            </tr>
        </table>
    {elseif $selected_type==0}
        <h3>{t}Add disk{/t}</h3>
        <table>
            <tr>
                <td>{t}Disk name{/t}</td>
                <td>
                    <input type="text" name="d_name" value="{$d_name}">
                </td>
            </tr>
        </table>
    {/if}

    <hr>
    <div class="clear"></div>

    <div class="plugin-actions">
      <button type='submit' name='save_partition_add'>{msgPool type=addButton}</button>
      <button type='submit' name='cancel_partition_add'>{msgPool type=cancelButton}</button>
    </div>

{/if}
