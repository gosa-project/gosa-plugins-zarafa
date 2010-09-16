<h3>{t}Registered device{/t}</h3>


<table width="100%">
    <tr>
        <td style='width:50%;'>
            <table>
                <tr>
                    <td><LABEL for='name'>{t}Name{/t}</LABEL></td>
                    <td> <input type="text" name="cn" value="{$cn}"></td>
                </tr>
                <tr>
                    <td><LABEL for='description'>{t}Description{/t}</LABEL></td>
                    <td> <input type="text" name="description" value="{$description}"></td>
                </tr>
                <tr>
                    <td><LABEL for='ou'>{t}Organizational unit{/t}</LABEL></td>
                    <td> <input type="text" name="ou" value="{$ou}"></td>
                </tr>
                <tr>
                    <td><LABEL for='o'>{t}Organizaton{/t}</LABEL></td>
                    <td> <input type="text" name="o" value="{$o}"></td>
                </tr>
                <tr>
                    <td><LABEL for='l'>{t}Location{/t}</LABEL></td>
                    <td> <input type="text" name="l" value="{$l}"></td>
                </tr>
            </table>
        </td>
        <td class='left-border'>
            <table>
                <tr>
                    <td><LABEL for='serialNumber'>{t}Serial number{/t}</LABEL></td>
                    <td> <input type="text" name="serialNumber" value="{$serialNumber}"></td>
                </tr>
                <tr>
                    <td><LABEL for='deviceUUID'>{t}Device UUID{/t}</LABEL></td>
                    <td> <input type="text" name="deviceUUID" value="{$deviceUUID}"></td>
                </tr>
                <tr>
                    <td><LABEL for='deviceStatus'>{t}Status{/t}</LABEL></td>
                    <td> <input type="text" name="deviceStatus" value="{$deviceStatus}"></td>
                </tr>
            </table>
            <hr>
            <table>
                <tr>
                    <td><LABEL for='seeAlso'>{t}See also{/t}</LABEL></td>
                    <td> <input type="text" name="seeAlso" value="{$seeAlso}"></td>
                </tr>
                <tr>
                    <td><LABEL for='owner'>{t}Owner{/t}</LABEL></td>
                    <td> <input type="text" name="owner" value="{$owner}"></td>
                </tr>
                <tr>
                    <td><LABEL for='manager'>{t}Manager{/t}</LABEL></td>
                    <td> <input type="text" name="manager" value="{$manager}"></td>
                </tr>
            </table>
        </td>
    </tr>
</table>

<hr>

<h3>{t}Network settings{/t}</h3>

<table>
    <tr>
        <td><LABEL for='ipHostNumber'>{t}IP-address{/t}</LABEL></td>
        <td> <input type="text" name="ipHostNumber" value="{$ipHostNumber}"></td>
    </tr>
    <tr>
        <td><LABEL for='macAddress'>{t}MAC-address{/t}</LABEL></td>
        <td> <input type="text" name="macAddress" value="{$macAddress}"></td>
    </tr>
</table>
