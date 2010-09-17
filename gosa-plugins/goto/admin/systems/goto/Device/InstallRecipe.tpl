<table>
    <tr>
        <td><LABEL for="kickstartRootEnabled">{t}Root enabled{/t}</LABEL></td>
        <td>
            <input type='checkbox' value='1' {if $kickstartRootEnabled} checked {/if}
                onClick="changeState('kickstartRootPasswordHash');"
                name="kickstartRootEnabled" id="kickstartRootEnabled" 
               >
        </td>
    </tr>
    <tr>
        <td><LABEL for="kickstartRootPasswordHash">{t}Root password hash{/t}</LABEL></td>
        <td>
            <input type='text' name="kickstartRootPasswordHash" id="kickstartRootPasswordHash"
                {if !$kickstartRootEnabled} disabled {/if} value="{$kickstartRootPasswordHash}">
        </td>
    </tr>
    <tr>
        <td><LABEL for="member">{t}Member{/t}</LABEL></td>
        <td>
            {$memberList}
            <input type='text' name="member" id="member" value="">
            <button name='addMember' type='submit'>{msgPool type='addButton'}</button>
        </td>
    </tr>
    <tr>
        <td><LABEL for="kickstartTemplateDN">{t}Kickstart template{/t}</LABEL></td>
        <td>
            <input type='text' name="kickstartTemplateDN" id="kickstartTemplateDN" value="{$kickstartTemplateDN}">
        </td>
    </tr>
    <tr>
        <td><LABEL for="kickstartKeyboardlayout">{t}Keyboard layout{/t}</LABEL></td>
        <td>
            <input type='text' name="kickstartKeyboardlayout" id="kickstartKeyboardlayout" value="{$kickstartKeyboardlayout}">
        </td>
    </tr>
    <tr>
        <td><LABEL for="kickstartSystemLocale">{t}System locale{/t}</LABEL></td>
        <td>
            <input type='text' name="kickstartSystemLocale" id="kickstartSystemLocale" value="{$kickstartSystemLocale}">
        </td>
    </tr>
    <tr>
        <td><LABEL for="kickstartTimezone">{t}Timezone{/t}</LABEL></td>
        <td>
            <select size='1' name="kickstartTimezone" id="kickstartTimezone">
                {html_options values=$timezones options=$timezones selected=$kickstartTimezone}
            </select>
        </td>
    </tr>
    <tr>
        <td><LABEL for="kickstartTimeUTC">{t}UTC Time{/t}</LABEL></td>
        <td>
            <input type='text' name="kickstartTimeUTC" id="kickstartTimeUTC" value="{$kickstartTimeUTC}">
        </td>
    </tr>
    <tr>
        <td><LABEL for="kickstartNTPServer">{t}NTP server{/t}</LABEL></td>
        <td>
            <input type='text' name="kickstartNTPServer" id="kickstartNTPServer" value="{$kickstartNTPServer}">
        </td>
    </tr>
    <tr>
        <td><LABEL for="kickstartMirrorDN">{t}Mirror{/t}</LABEL></td>
        <td>
            <input type='text' name="kickstartMirrorDN" id="kickstartMirrorDN" value="{$kickstartMirrorDN}">
        </td>
    </tr>
    <tr>
        <td><LABEL for="kickstartKernelPackage">{t}Kernel package{/t}</LABEL></td>
        <td>
            <input type='text' name="kickstartKernelPackage" id="kickstartKernelPackage" value="{$kickstartKernelPackage}">
        </td>
    </tr>
    <tr>
        <td><LABEL for="kickstartPartitionTable">{t}Partition table{/t}</LABEL></td>
        <td>
            <input type='text' name="kickstartPartitionTable" id="kickstartPartitionTable" value="{$kickstartPartitionTable}">
        </td>
    </tr>
</table>

