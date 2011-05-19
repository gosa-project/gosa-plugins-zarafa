
<table width="100%">
    <tr>
        <td style="width:50%">
            <h3>{t}Installation type{/t}</h3>
            <table>
                <tr>
                    <td><LABEL for="installTemplate">{t}Template{/t}</LABEL></td>
                    <td>
                        {render acl=$installTemplateACL}
                        <select name="installTemplate" size=1 onChange="document.mainform.submit();">
                            {html_options options=$installTemplateList 
                            selected=$installTemplate}
                        </select>
                        {/render}
                    </td>
                </tr>
                <tr>
                    <td><LABEL for="installRelease">{t}Release{/t}</LABEL></td>
                    <td>
                        {render acl=$installReleaseACL}
                        <select name="installRelease" size=1 onChange="document.mainform.submit();">
                            {html_options options=$installReleaseList 
                            selected=$installRelease}
                        </select>
                        {/render}
                    </td>
                </tr>
                <tr>
                    <td><LABEL for="installConfigManagement">{t}Config management{/t}</LABEL></td>
                    <td>
                        {render acl=$installConfigManagementACL}
                        <select name="installConfigManagement" size=1 onChange='document.mainform.submit();'>
                            {html_options options=$installConfigManagementList 
                            selected=$installConfigManagement}
                        </select>
                        {/render}
                    </td>
                </tr>
            </table>
        </td>
        <td style='width:50%; vertical-align: top;padding-left:5px;' class='left-border'>
            <h3>{t}Partition table{/t}</h3>
            <table>
                <tr>
                    <td><LABEL for="installPartitionTable">{t}Partition table{/t}</LABEL></td>
                    <td>
                        {render acl=$installPartitionTableACL}
                        <input type='submit' name="edit_installPartitionTable" id="edit_installPartitionTable" 
                            value="{t}Edit partition table{/t}">
                        {/render}
                    </td>
                </tr>
            </table>
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
                    <td><LABEL for="installMirror">{t}Mirror{/t}</LABEL></td>
                    <td>
                        {render acl=$installMirrorACL}
                        <select name="installMirror" size=1>
                            {html_options options=$installMirrorList 
                            selected=$installMirror}
                        </select>
                        {/render}
                    </td>
                </tr>
                <tr>
                    <td><LABEL for="installKernelPackage">{t}Kernel package{/t}</LABEL></td>
                    <td>
                        {render acl=$installKernelPackageACL}
                        <select name="installKernelPackage" size=1>
                            {html_options values=$installKernelPackageList output=$installKernelPackageList 
                            selected=$installKernelPackage}
                        </select>
                        {/render}
                    </td>
                </tr>
            </table>
        </td>
        <td style='width:50%; vertical-align: top;padding-left:5px;' class='left-border'>
            <h3>{t}Login{/t}</h3>

            <table>
                <tr>
                    <td>
                        {render acl=$installRootEnabledACL}
                        <input type='checkbox' value='1' {if $installRootEnabled} checked {/if}
                            onClick="changeState('setPasswordHash');"
                            name="installRootEnabled" id="installRootEnabled" 
                           >
                        {/render}
                        <LABEL for="installRootEnabled">{t}Use root user{/t}</LABEL>&nbsp;
                        {render acl=$installRootPasswordHashACL}
                        <button name='setPasswordHash'
                            {if !$installRootEnabled} disabled {/if}
                            id="setPasswordHash">{t}Set password{/t}</button>
                        {/render}
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
                    <td><LABEL for="installKeyboardlayout">{t}Keyboard layout{/t}</LABEL></td>
                    <td>
                        {render acl=$installKeyboardlayoutACL}
                        <select name="installKeyboardlayout" size=1>
                            {html_options options=$installKeyboardlayoutList
                            selected=$installKeyboardlayout}
                        </select>
                        {/render}
                    </td>
                </tr>
                <tr>
                    <td><LABEL for="installSystemLocale">{t}System locale{/t}</LABEL></td>
                    <td>
                        {render acl=$installSystemLocaleACL}
                        <select name="installSystemLocale" size=1>
                            {html_options options=$installSystemLocaleList 
                            selected=$installSystemLocale}
                        </select>
                        {/render}
                    </td>
                </tr>
            </table>
        </td>
        <td style='width:50%; vertical-align: top;padding-left:5px;' class='left-border'>
            <h3>{t}Time{/t}</h3>

            <table>
                <tr>
                    <td>
                        {render acl=$installTimeUTCACL}
                        <input type='checkbox' name="installTimeUTC" id="installTimeUTC" 
                        {if $installTimeUTC} checked {/if}>
                        <LABEL for="installTimeUTC">{t}Use UTC{/t}</LABEL>
                        {/render}
                    </td>
                </tr>
                <tr>
                    <td><LABEL for="installTimezone">{t}Timezone{/t}</LABEL></td>
                    <td>
                        {render acl=$installTimezoneACL}
                        <select size='1' name="installTimezone" id="installTimezone">
                            {html_options output=$timezones values=$timezones 
                            selected=$installTimezone}
                        </select>
                        {/render}
                    </td>
                </tr>
                <tr>    
                    <td colspan=2>
                        {t}NTP server{/t}
                        {render acl=$installNTPServerACL}
                            {$installNTPServerList}
                        {/render}
                        {render acl=$installNTPServerACL}
                            <input type='text' name="installNTPServer_Input">
                        {/render}
                        {render acl=$installNTPServerACL}
                            <button name='installNTPServer_Add'>{msgPool type=addButton}</button>
                        {/render}
                    </td>
                </tr>
            </table>
        </td>
    </tr>
</table>

<input type='hidden' name='InstallRecipePosted' value=1>
