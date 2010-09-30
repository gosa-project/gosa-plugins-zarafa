
<input type="text" name="rootPassword" value="">
<select name="passwordHash" size=1>
    {html_options options=$hashes selected=$hash}
</select>
<div class="plugin-action">
    <button name="setPassword">{msgPool type=okButton}</button>
    <button name="cancelPassword">{msgPool type=cancelButton}</button>
</div>
