<table style='width:100%; ' summary="{t}FAI profile entry{/t}">

<tr>
  <td style='width:600px'>

	<div class="contentboxh" style="height:20px">
    	<p class="contentboxh" style="font-size:12px">
     		{t}List of FAI objects (Fully Automatic Installation){/t}
    	</p>
  	</div>
  	<div class="contentboxb">
      	{$faihead}
  	</div>
  	<div style='height:4px;'></div>
	<div class="contentboxb" style="border-top:1px solid #B0B0B0;">
    	{$failist}
   		<input type=hidden name="edit_helper">
	</div>
  </td>
  <td>

   	<div class="contentboxh" style="height:20px;border-bottom:1px solid #B0B0B0">
    	<p class="contentboxh" style="font-size:12px">
			{image path="{$infoimage}" align="right"}{t}Information{/t}

		</p>
   	</div>
   	<div class="contentboxb">
   		<p class="contentboxb" style="border-color:#f0f0f0;">
     		{t}This menu allows you to select FAI class names and to add them to the currently edited profile.{/t}
   		</p>
   	</div>
   	<br>
   	<div class="contentboxh" style="height:20px">
    	<p class="contentboxh" style="font-size:12px">{image path="{$launchimage}" align="right"}{t}Filters{/t}
</p>
   	</div>
   	<div class="contentboxb">
     	<table style='width:100%;' summary="{t}Filter options{/t}">

      		{$alphabet}
     	</table>
	 	<table style='width:100%;' summary="{t}Filter options{/t}">

			<tr><td><input onClick='document.mainform.submit()' 
								type="checkbox" name="SShowTemplates" 	{$SShowTemplatesCHK}>{t}Show only classes with templates{/t}</td></tr>
			<tr><td><input onClick='document.mainform.submit()'
									type="checkbox" name="SShowScripts" 		{$SShowScriptsCHK}>{t}Show only classes with scripts{/t}</td></tr>
			<tr><td><input onClick='document.mainform.submit()'
									type="checkbox" name="SShowHooks" 		{$SShowHooksCHK}>{t}Show only classes with hooks{/t}</td></tr>
			<tr><td><input onClick='document.mainform.submit()'
									type="checkbox" name="SShowVariables" 	{$SShowVariablesCHK}>{t}Show only classes with variables{/t}</td></tr>
			<tr><td><input onClick='document.mainform.submit()'
									type="checkbox" name="SShowPackages" 	{$SShowPackagesCHK}>{t}Show only classes with packages{/t}</td></tr>
			<tr><td><input onClick='document.mainform.submit()'
								type="checkbox" name="SShowPartitions"	{$SShowPartitionsCHK}>{t}Show only classes with partitions{/t}</td></tr>
		 </table>
    	 <table style='width:100%;' summary="{t}Filter options{/t}">

     		<tr>
     			<td>
     					{image path="{$search_image}"}
							align=middle title='{t}Display objects matching{/t}'>
     			</td>
     			<td width="99%">
     					<input type='text' name='Sregex' maxlength='20' style='width:99%' value='{$Sregex}' id='filter' 
     						title='{t}Regular expression for matching object names{/t}' >
     			</td>
     		</tr>
     	</table>
		{$apply}
   		</div>
  	</td>
</tr>
</table>

<div align="right" style="align:right;">
	<button type='submit' name='SaveSubObject'>{msgPool type=applyButton}</button>

	<button type='submit' name='CancelSubObject'>{msgPool type=cancelButton}</button>

</div>
<!-- Place cursor -->
