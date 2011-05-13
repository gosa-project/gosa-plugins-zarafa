<h3>{t}Device partition{/t}</h3>

{if $error}

    <p>
    {$errorMsg}
    </p>
    <button type='submit' name='retry'>{t}Retry{/t}</button>

{else}

    {$list}

    <button name="create_partition">{msgPool type=addButton}</button>

    <hr>

    <div class="clear"></div>
    <div class="plugin-actions">
      <button type='submit' name='partition_finish'>{msgPool type=applyButton}</button>
      <button type='submit' name='partition_cancel'>{msgPool type=cancelButton}</button>
    </div>
{/if}
