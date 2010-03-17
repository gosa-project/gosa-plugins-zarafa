<table style='width:100%; ' summary="">

<tr>
  <td style='width:600px'>

   <div class="contentboxh" style="height:20px;">
    <p class="contentboxh" style="font-size:12px">
     <b>{t}Select numbers to add{/t} {$hint}</b><br>
    </p>
   </div>
   <div class="contentboxb">
    <p class="contentboxb" style="border-top:1px solid #B0B0B0;background-color:#F8F8F8">
     <select style="width:100%; margin-top:4px; height:450px;" name="local_list[]" size="15" multiple>
    	{html_options options=$list}
     </select>
    </p>
   </div>
  </td>
  <td>

   <div class="contentboxh" style="height:20px;">
    <p class="contentboxh" style="font-size:12px">{image path="{$launchimage}" align="right"}
<b>{t}Filters{/t}</b></p>
   </div>
   <div class="contentboxb"  style="background-color:#F8F8F8">
     <table style='width:100%;background-color:#F8F8F8' summary="">

      {$alphabet}
     </table>
    <table style='background-color:#F8F8F8' summary="">

		<tr>
			<td>
				{image path="{$tree_image}" title="{t}Display numbers of department{/t}"}&nbsp;

				<select name="depselect" size="1" onChange="mainform.submit()" title="{t}Choose the department the search will be based on{/t}">
			      {html_options options=$deplist selected=$depselect}
			    </select>
			</td>
		</tr>
	</table>
    <table style='background-color:#F8F8F8' summary="">

		<tr>
			<td width="18">
				{image path="{$search_image}" title="{t}Display numbers matching{/t}"}

			</td>
    		<td>
				<input type='text' name='regex' maxlength='20' value='{$regex}' style="width:99%" title='{t}Regular expression for matching numbers{/t}' onChange="mainform.submit()">	</td>
		</tr>
	</table>
    <table style='background-color:#F8F8F8' summary="">

		<tr>
			<td width="18">
				{image path="{$usearch_image}" title="{t}Display numbers of user{/t}"}

			</td>
		    <td>
				<input type='text' name='fuser' style="width:99%" maxlength='20' value='{$fuser}' title='{t}User name of which numbers are shown{/t}' onChange="mainform.submit()">	</td>
		</tr>
	</table>
   {$apply}
   </div>
  </td>
</tr>
</table>

<hr>
<div class="plugin-actions">
  <button type='submit' name='add_locals_finish'>{msgPool type=addButton}</button>

  <button type='submit' name='add_locals_cancel'>{msgPool type=cancelButton}</button>

</div>
