{if $type == 'Distribution'}
<table>
    <tr>
        <td>{$nameName}</td>
        <td>{$name}</td>
    </tr>
    <tr>
        <td>{$typeName}</td>
        <td>{$type}</td>
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
