{if $write_protect}
  {t}The text is write protected due to its encoding. Editing may break it!{/t}
  <br>
  <button type='submit' name='editAnyway' {if !$enabled} disabled {/if}>{t}Edit anyway{/t}</button>
{/if}
<textarea {if !$enabled} disabled {/if} {if $write_protect} disabled {/if} {if !$write_protect} name="{$postName}" {/if}
    style="width:100%;height:300px;" id="{$postName}"
    rows="20" cols="120">{$value}</textarea>
<div>
  <input type="file" name="ImportFile" {if !$enabled} disabled {/if}>&nbsp;
  <button type='submit' name='ImportUpload' {if !$enabled} disabled {/if}>{t}Import text{/t}</button>
</div>
