{if !$init_successfull}
<br>
<b>{msgPool type=siError}</b><br>
{t}Check if the GOsa support daemon (gosa-si) is running.{/t}&nbsp;
<input type='submit' name='retry_init' value="{t}retry{/t}">
<br>
<br>
{else}

<!-- GENERIC -->
<h2>{t}Licenses used{/t}</h2>
{$licenseUses}

<h2>{t}Licenses reserved for this host{/t}</h2>
{$licenseReserved}

<input name='opsiLicenseUsagePosted' value='1' type='hidden'>
{/if}
