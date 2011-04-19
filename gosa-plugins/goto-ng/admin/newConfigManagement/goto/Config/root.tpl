{if $type == 'Distribution'}
<table>
    <tr>
        <td>{$nameName}</td>
        <td>{$name}</td>
    </tr>
    <tr>
        <td>{$installation_typeName}</td>
        <td>{$installation_type}</td>
    </tr>
    <tr>
        <td>{$installation_methodName}</td>
        <td>{$installation_method}</td>
    </tr>
    <tr>
        <td>{$mirrorName}</td>
        <td>{$mirror}</td>
    </tr>
</table>
{else if $type == 'Release'}
<table>
    <tr>
        <td>{$nameName}</td>
        <td>{$name}</td>
    </tr>
</table>
{/if}
