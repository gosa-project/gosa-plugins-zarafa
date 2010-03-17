<table style='width:100%; ' summary="">

<tr>
  <td style='width:50%;'>

  <div class="contentboxh" style="height:20px;">
    <p class="contentboxh" style="font-size:12px;">
     {t}List of attachments{/t}
    </p>
  </div>
  <div class="contentboxb">
      {$attachmenthead}
  </div>
  <div style='height:4px;'></div>
  <div class="contentboxb" style="border-top:1px solid #B0B0B0;">
      {$attachments}
    <input type=hidden name="edit_helper">
  </div>
  </td>
  <td>

   <div class="contentboxh" style="border-bottom:1px solid #B0B0B0;height:20px;">
    <p class="contentboxh" style="font-size:12px;">{image path="{$infoimage}" align="right"}{t}Information{/t}
</p>
   </div>
   <div class="contentboxb" style="padding:5px;">
    {t}This dialog allow you to attach additional objects (like manuals, guides, etc.)  to your currently edited computer.{/t}
   </div>
   <br>
   <div class="contentboxh" style="height:20px;">
    <p class="contentboxh" style="font-size:12px;">{image path="{$launchimage}" align="right"}{t}Filters{/t}
</p>
   </div>
   <div class="contentboxb">
     <table style='width:100%;' summary="">

      {$alphabet}
     </table>
<table style='width:100%;' summary="">

<tr>
<td><LABEL for="regex">{image path="{$search_image}"}
</label></td>
<td width="99%">
<input type='text' style='width:99%' name='attachment_regex' maxlength='20' value='{$attachment_regex}' title='{t}Regular expression for matching attachment names{/t}' onChange="mainform.submit()">
</td>
</tr>
</table>
   {$apply}
   </div>
  </td>
</tr>
</table>

<input type="hidden" name="ignore">
<hr>
<div align="right">
	<p>
		<button type='submit' name='UseAttachment'>{t}Use{/t}</button>

		<button type='submit' name='AbortAttachment'>{msgPool type=cancelButton}</button>

	</p>
</div>
