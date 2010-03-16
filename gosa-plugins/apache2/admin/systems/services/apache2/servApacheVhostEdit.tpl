<h3>{t}Generic{/t}</h3>
<table summary="" width="100%">
	<tr>
		<td style="width:50%;vertical-align:top;border-right:1px	solid	#b0b0b0;">
			<table summary="">
				<tr>
					<td>{t}Virtual host name{/t}{$must}
					</td>
					<td><input type="text" name="apacheServerName" value="{$apacheServerName}" {if $NotNew} disabled {/if}>
					</td>
				</tr>
				<tr>
					<td>{t}Document root{/t}{$must}
					</td>
					<td><input type="text" name="apacheDocumentRoot" value="{$apacheDocumentRoot}">
					</td>
				</tr>
				<tr>
					<td>{t}Admin mail address{/t}{$must}
					</td>
					<td><input type="text" name="apacheServerAdmin" value="{$apacheServerAdmin}">
					</td>
				</tr>
			</table>
		</td>
		<td style="vertical-align:top;">
			<table summary="" width="100%">
				<tr>
					<td style="vertical-align:top;width:100%;border-right:1px	solid	#b0b0b0;">
						<h3>{t}Server Alias{/t}</h3>
						<table width="100%">	
							<tr>
								<td>
									{$apacheServerAlias}
								</td>
							</tr>
							<tr>
								<td>
									<table width="100%">
										<tr>
											<td  style="vertical-align:top;width:30%;">
												<h3>{t}URL Alias{/t}</h3>
											</td>
											<td>
												<h3>{t}Directory Path{/t}</h3>
											</td>
										</tr>
										<tr>
											<td style="vertical-align:top;width:30%;">
												<input type="text" 		name="StrSAAlias" value="">
											</td>
											<td>
												<input type="text" 		name="StrSADir" value="">
												<button type='submit' name='AddSARecord'>{t}Add{/t}</button>

											</td>
										</tr>
									</table>
								</td>
							</tr>
						</table>
					</td>
				</tr>
			</table>
		</td>
	</tr>
</table>

<hr>
<br>

<table summary="" width="100%">
	<tr>
		<td style="vertical-align:top;width:50%;border-right:1px	solid	#b0b0b0;">
			<h3>{t}Script Alias{/t}</h3>
			<table width="100%">	
				<tr>
					<td>
						{$apacheScriptAlias}
					</td>
				</tr>
				<tr>
					<td>
						<table width="100%">
							<tr>
								<td  style="vertical-align:top;width:30%;">
									<h3>{t}Alias Directory{/t}</h3>
								</td>
								<td>
									<h3>{t}Script Directory{/t}</h3>
								</td>
							</tr>
							<tr>
								<td style="vertical-align:top;width:30%;">
									<input type="text" 		name="StrSCAlias" value="">
								</td>
								<td>
									<input type="text" 		name="StrSCDir" value="">
									<button type='submit' name='AddSCRecord'>{t}Add{/t}</button>

								</td>
							</tr>
						</table>
					</td>
				</tr>
			</table>
		</td>
		<td style="vertical-align:top;">
		</td>
	</tr>
</table>
<div style="text-align:right;" align="right">
	<p>
		<button type='submit' name='SaveVhostChanges'>{t}Save{/t}</button>

		<button type='submit' name='CancelVhostChanges'>{t}Cancel{/t}</button>

	</p>
</div>
<script language="JavaScript" type="text/javascript">
  <!-- // First input field on page
  document.mainform.apacheServerName.focus();
  -->
</script>
