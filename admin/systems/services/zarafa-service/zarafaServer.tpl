<table summary="" style="width:100%; vertical-align:top; text-align:left;" cellpadding="0" border="0">
    <tr>
        <td>
            <h3>Zarafa server settings</h3>
            <br/>
            <input type="checkbox" name="zarafaAccount" id="zarafaAccount" value="1" {$zarafaAccount}
            title="{t}Make this server a Zarafa-server{/t}"> {t}Enable Zarafa service{/t}
            <br/>
            <input type="checkbox" name="zarafaHidden" id="zarafaHidden" value="1" {$zarafaHidden}
            title="{t}Hide server from address book. Administrators will see it anyway{/t}"> {t}Hide server from address book{/t}
            <br/>
            {assign var="publicStoreText"
            value="Enable public store on this server (On a multi - server configuration keep in mind only one node is allowed to enable this feature)"}
            <input type="checkbox" name="zarafaContainsPublic" id="zarafaContainsPublic" value="1" {$zarafaContainsPublic} title="{t}{$publicStoreText}{/t}"> {t}Enable public store{/t}
            <br/>
            <input type="checkbox" name="goZarafaArchiveServer" id="goZarafaAchriveServer" value="1" {$goZarafaArchiveServer}
            title="{t}Make this server a Zarafa-archive-server{/t}"> {t}Set archive server{/t}
        </td>
        <td class="left-border">&nbsp;</td>
        <td>
            <table>
                <tr>
                    <td>
                        HTTP Port:
                    </td>
                    <td>
                        <input type="text" id="zarafaHttpPort" name="zarafaHttpPort" value="{$zarafaHttpPort}">
                    </td>
                </tr>
                <tr>
                    <td>
                        SSL Port:
                    </td>
                    <td>
                        <input type="text" id="zarafaSslPort" name="zarafaSslPort" value="{$zarafaSslPort}">
                    </td>
                </tr>
                <tr>
                    <td>
                        Zarafa file path:
                    </td>
                    <td>
                        <input type="text" id="zarafaFilePath" name="zarafaFilePath" value="{$zarafaFilePath}">
                    </td>
                </tr>
            </table>
        </td>
    </tr>
    <tr>
        <td colspan="3">
            <hr/>
        </td>
    </tr>
</table>

<div class="plugin-actions">
    <button type='submit' name='SaveService'>{msgPool type=saveButton}</button>
    <button type='submit' name='CancelService'>{msgPool type=cancelButton}</button>
    <input type='hidden' name='zarafa-server-plugin' value='1'/>
</div>
