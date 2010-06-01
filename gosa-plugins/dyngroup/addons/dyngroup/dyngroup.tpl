<h3>{t}Generic{/t}</h3>
<label for="{$memberURLAttributeLabel}">{t}{$memberURLAttributeLabel}{/t}</label>
{foreach item=line from=$memberURLAttributeValue}
 <input size=70 name="{$memberURLAttributeLabel}" value="{$line}">
{/foreach}
