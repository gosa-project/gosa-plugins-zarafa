
<h3>{t}Installation type{/t}</h3>
<table>
    <tr>
        <td><LABEL for="__missingBoostrap">{t}Bootstrap method{/t}</LABEL></td>
        <td>
            <select name="__missingBoostrap" size=1>
                {html_options options=$__missingBoostrapList 
                selected=$__missingBoostrap}
            </select>
        </td>
    </tr>
    <tr>
        <td><LABEL for="__missingConfigManagement">{t}Config management{/t}</LABEL></td>
        <td>
            <select name="__missingConfigManagement" size=1>
                {html_options options=$__missingConfigManagementList 
                selected=$__missingConfigManagement}
            </select>
        </td>
    </tr>
</table>
<hr>
<table style='width:100%;'>
    <tr>
        <td style='width:50%; vertical-align: top;'>
            <h3>{t}Bootstrap settings{/t}</h3>
    
            <table>
                <tr>
                    <td><LABEL for="kickstartMirrorDN">{t}Mirror{/t}</LABEL></td>
                    <td>
                        <select name="kickstartMirrorDN" size=1>
                            {html_options options=$kickstartMirrorDNList 
                            selected=$kickstartMirrorDN}
                        </select>
                    </td>
                </tr>
                <tr>
                    <td><LABEL for="kickstartTemplateDN">{t}Kickstart template{/t}</LABEL></td>
                    <td>
                        <select name="kickstartTemplateDN" size=1>
                            {html_options options=$kickstartTemplateDNList 
                            selected=$kickstartTemplateDN}
                        </select>
                    </td>
                </tr>
                <tr>
                    <td><LABEL for="kickstartKernelPackage">{t}Kernel package{/t}</LABEL></td>
                    <td>
                        <select name="kickstartKernelPackage" size=1>
                            {html_options values=$kickstartKernelPackageList output=$kickstartKernelPackageList 
                            selected=$kickstartKernelPackage}
                        </select>
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
                        <select name="kickstartKeyboardlayout" size=1>
                            {html_options values=$kickstartKeyboardlayoutList output=$kickstartKeyboardlayoutList 
                            selected=$kickstartKeyboardlayout}
                        </select>
                    </td>
                </tr>
                <tr>
                    <td><LABEL for="kickstartSystemLocale">{t}System locale{/t}</LABEL></td>
                    <td>
                        <select name="kickstartSystemLocale" size=1>
                            {html_options values=$kickstartSystemLocaleList output=$kickstartSystemLocaleList 
                            selected=$kickstartSystemLocale}
                        </select>
                    </td>
                </tr>
            </table>
        </td>
        <td style='width:50%; vertical-align: top;padding-left:5px;' class='left-border'>
            <h3>{t}Time{/t}</h3>

            <table>
                <tr>
                    <td>
                        <input type='checkbox' name="kickstartTimeUTC" id="kickstartTimeUTC" 
                        {if $kickstartTimeUTC} checked {/if}>
                        <LABEL for="kickstartTimeUTC">{t}Use UTC{/t}</LABEL>
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
                    <td colspan=2>
                        {t}NTP server{/t}
                        {$kickstartNTPServerList}
                        <input type='text' name="kickstartNTPServer_Input">
                        <button name='kickstartNTPServer_Add'>{msgPool type=addButton}</button>
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
