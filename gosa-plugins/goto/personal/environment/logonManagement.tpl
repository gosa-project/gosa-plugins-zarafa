<h3>{image path="plugins/goto/images/logon_script.png"}&nbsp;{t}Logon script management{/t}
</h3>

    <input type="hidden" name="dialogissubmitted" value="1">

    <table {t}Logon script management{/t}" width="100%">
    	<tr>
			<td width="50%" style="vertical-align:top;border-right:1px solid #B0B0B0">
					<table {t}Logon script settings{/t}">
						<tr>
							<td><LABEL for="LogonName">{t}Script name{/t}</LABEL>
							</td>
							<td>
								<input type="text" size=20 value="{$LogonName}" name="LogonName" {$LogonNameACL} id="LogonName">
							</td>
						</tr>
						<tr>
							<td><LABEL for="LogonDescription">{t}Description{/t}</LABEL>
							</td>
							<td>
								<input type="text" size=40 value="{$LogonDescription}" name="LogonDescription" id="LogonDescription"> 
							</td>
						</tr>
						<tr>
							<td><LABEL for="LogonPriority">{t}Priority{/t}</LABEL>
							</td><td>
				            	<select name="LogonPriority" id="LogonPriority">
                					{html_options values=$LogonPriorityKeys output=$LogonPrioritys selected=$LogonPriority}
                				</select>
							</td>
						</tr>
					</table>
			</td>
			<td style="vertical-align:top">
					<table {t}Logon script flags{/t}">
						<tr>
							<td>
								<input type="checkbox" value="L" name="LogonLast" {$LogonLastCHK} id="LogonLast">
							<LABEL for="LogonLast">{t}Last script{/t}</LABEL>
							</td>
						</tr>
						<tr>
							<td>
								<input type="checkbox" value="O" name="LogonOverload" {$LogonOverloadCHK} id="LogonOverload">
								<LABEL for="LogonOverload">{t}Script can be replaced by user{/t}</LABEL>
							</td>
						</tr>
					</table>
			</td>
		</tr>
	</table>
	<p class="seperator">&nbsp;</p>
	<table width="100%" >
		<tr>
			<td colspan="2">
			<h3>{image path="plugins/goto/images/logon_script.png"}&nbsp;{t}Script{/t}
</h3>
					<table width="100%" {t}Logon script{/t}">
						<tr>
							<td>
								<textarea style="width:100%;height:400px;" name="LogonData">{$LogonData}</textarea>
							</td>
						</tr>
						<tr>
							<td>
								<input type="file" name="importFile" id="importFile">
								<button type='submit' name='StartImport'>{t}Import{/t}</button>

							</td>
						</tr>
					</table>
			</td>
		</tr>
	</table>	

	<p class="seperator">&nbsp;</p>
    <p align="right">
    <button type='submit' name='LogonSave'>{msgPool type=applyButton}</button>

    <button type='submit' name='LogonCancel'>{msgPool type=cancelButton}</button>

    </p>

<script language="JavaScript" type="text/javascript">
  <!-- // First input field on page
	focus_field('LogonName');
  -->
</script>

