
<h3>{t}Installation type{/t}</h3>
<table style='width:100%;'>
    <tr>
        <td style='width:50%; vertical-align: top;'>
            <h3>{t}Bootstrap settings{/t}</h3>
    
            <table>
                <tr>
                    <td><LABEL for="kickstartMirrorDN">{t}Mirror{/t}</LABEL></td>
                    <td>
                        <input type='text' name="kickstartMirrorDN" id="kickstartMirrorDN" value="{$kickstartMirrorDN}">
                    </td>
                </tr>
                <tr>
                    <td><LABEL for="kickstartTemplateDN">{t}Kickstart template{/t}</LABEL></td>
                    <td>
                        <input type='text' name="kickstartTemplateDN" id="kickstartTemplateDN" value="{$kickstartTemplateDN}">
                    </td>
                </tr>
                <tr>
                    <td><LABEL for="kickstartKernelPackage">{t}Kernel package{/t}</LABEL></td>
                    <td>
                        <input type='text' name="kickstartKernelPackage" id="kickstartKernelPackage" value="{$kickstartKernelPackage}">
                    </td>
                </tr>
            </table>
        </td>
        <td style='width:50%; vertical-align: top;padding-left:5px;' class='left-border'>
            <h3>{t}Login{/t}</h3>

            <table>
                <tr>
                    <td>
                        <input type='checkbox' value='1' {if $kickstartRootEnabled} checked {/if}
                            onClick="changeState('setKickstartRootPasswordHash');"
                            name="kickstartRootEnabled" id="kickstartRootEnabled" 
                           >
                        <LABEL for="kickstartRootEnabled">{t}Use root-user{/t}</LABEL>&nbsp;
                        <button name='setKickstartRootPasswordHash'
                            {if !$kickstartRootEnabled} disabled {/if}
                            id="setKickstartRootPasswordHash">{t}Set password{/t}</button>
                    </td>
                </tr>
            </table>
        </td>
    </tr>
    <tr>
        <td colspan=2><hr></td>
    </tr>
    <tr>
        <td style='width:50%; vertical-align: top;'>
            <h3>{t}Locale{/t}</h3>

            <table>
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
            </table>
        </td>
        <td style='width:50%; vertical-align: top;padding-left:5px;' class='left-border'>
            <h3>{t}Time{/t}</h3>

            <table>
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
                    <td><LABEL for="kickstartTimezone">{t}Timezone{/t}</LABEL></td>
                    <td>
                        <select size='1' name="kickstartTimezone" id="kickstartTimezone">
                            {html_options values=$timezones options=$timezones selected=$kickstartTimezone}
                        </select>
                    </td>
                </tr>
            </table>
        </td>
    </tr>
</table>

<hr>
<h3>{t}Partition table{/t}</h3>
<table>
    <tr>
        <td><LABEL for="kickstartPartitionTable">{t}Partition table{/t}</LABEL></td>
        <td>
            <input type='text' name="kickstartPartitionTable" id="kickstartPartitionTable" value="{$kickstartPartitionTable}">
        </td>
    </tr>
</table>

<input type='hidden' name='InstallRecipePosted' value=1>
