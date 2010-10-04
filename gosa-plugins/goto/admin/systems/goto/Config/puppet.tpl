{if $type == 'PuppetModule'}
    <table width="100%">
        <tr>
            <td style="width:50%; vertical-align: top;">
                <table>
                    <tr>
                        <td>{$nameName}</td>
                        <td>{$name}</td>
                    </tr>
                    <tr>
                        <td>{$descriptionName}</td>
                        <td>{$description}</td>
                    </tr>
                    <tr>
                        <td>{$versionName}</td>
                        <td>{$version}</td>
                    </tr>
                </table>
            </td>
            <td style="width:50%; vertical-align: top;">
                {$dependencyName}:<br>
                {$dependency}
            </td>
        </tr>
    </table>
    <hr>
    {$checkboxName}
    {$checkbox}

    <hr>
    {$comboboxName}
    {$combobox}

    <hr>
    {$textEditorName}<br>
    {$textEditor}

    <hr>
    {$fixedListName}<br>
    {$fixedList}
{/if}
{if $type == 'PuppetTemplate'}
    <table>
        <tr>
            <td>{$nameName}</td>
            <td>{$name}</td>
        </tr>
        <tr>
            <td>{$dataName}</td>
            <td>{$data}</td>
        </tr>
    </table>
    <input type='submit'>    
{/if}
