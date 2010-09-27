<h3>{t}Device Config{/t}</h3>

{t}Add a new sub-module to the current{/t}
<select name='subModule'>
    {html_options output=$subModule values=$subModule}
</select>
<button name='addSubModule'>{msgPool type='addButton'}</button>

{$navigator}

<hr>

<h3>{$containerName}</h3>
{$containerDescription}

{$template}

