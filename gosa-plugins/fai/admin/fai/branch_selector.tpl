 <div class="contentboxh" style="border-bottom:1px solid #B0B0B0;">
    <p class="contentboxh">{image path="{$branchimage}" align="right"}{t}Releases{/t}
</p>
   </div>
   <div class="contentboxb">
        <table style='width:100%;' summary="">

     <tr>
      <td>
    {t}Current release{/t}&nbsp;
    <select name="fai_release" onChange="document.mainform.submit();" size=1>
        {html_options output=$fai_releases values=$fai_releases selected=$fai_release}
    </select>
      </td>
     </tr>
        </table>
        <table summary="" style="width:100%;">
     <tr>
      <td>
    {if $allow_create}
        {image path="plugins/fai/images/branch_small.png" action="branch_branch"}

        <a href="?plug={$plug_id}&act=branch_branch">{t}Create release{/t}</a>
        <br>
        {image path="plugins/fai/images/freeze.png" action="freeze_branch"}

		<a href="?plug={$plug_id}&act=freeze_branch">{t}Create read-only release{/t}</a>
    {else}
        {image path="plugins/fai/images/branch_small_grey.png"}

        {t}Create release{/t}
        <br>
        {image path="plugins/fai/images/freeze_grey.png"}

		{t}Create read-only release{/t}
    {/if}

    {if $fai_release != $fai_base && $allow_remove}
    <br>
        {image path="images/lists/trash.png" action="remove_branch"}

	    <a href="?plug={$plug_id}&act=remove_branch">{t}Delete current release{/t}</a>
    {/if}
      </td>
     </tr>
   </table>
   </div>
<br>

