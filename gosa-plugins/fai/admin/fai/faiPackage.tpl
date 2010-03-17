<table width="100%" summary="">
	<tr>
		<td class='right-border'>

				<h3>{t}Generic{/t}</h3>
				<table summary="" cellspacing="4">
					<tr>
						<td>
							<LABEL for="cn">
							{t}Name{/t}
							</LABEL>
						</td>
						<td>
{render acl=$cnACL}
							<input type='text' value="{$cn}" size="45" disabled id="cn">
{/render}
						</td>
					</tr>
					<tr>
						<td>
							<LABEL for="description">
							{t}Description{/t}
							</LABEL>
						</td>
						<td>
{render acl=$descriptionACL}
							<input type='text' value="{$description}" size="45" name="description" id="description">
{/render}
						</td>
					</tr>
				</table>
		</td>
		<td width="50%" valign="top">
			<h3>{t}Repository{/t}</h3>
				<table summary="" cellspacing="4">
					<tr>
						<td>
							{t}Release{/t} : 
						</td>
						<td>
							{$release}
						</td>
					</tr>
					<tr>
						<td>
							{t}Section{/t} : 
						</td>
						<td>
							{$section}
						</td>
					</tr>
					<tr>
					    <td>
						  {t}Install method{/t} :
						</td>
						<td>
{render acl=$FAIinstallMethodACL}
							<select name="FAIinstallMethod" title='{t}Please select the installation method{/t}' size=1>
							  {html_options options=$FAIinstallMethods output=$FAIinstallMethod selected=$FAIinstallMethod}
							</select>
{/render}
						</td>
					</tr>
				</table>
		</td>
	</tr>
</table>
<hr>
<table width="99%" summary="">
	<tr>
		<td> 
			<h3>{t}Used packages{/t}</h3>
			{$divlist}
		</td>
	</tr>
{render acl=$FAIpackageACL}
	<tr>
		<td>
      			<input type="text" size="25" name="addPpkgsText" value="" />
			<button type='submit' name='AddManualpkg'>{msgPool type=addButton}</button>

			&nbsp;
			<button type='submit' name='Addpkg'>{t}Add from list{/t}</button>

		</td>
	</tr>
{/render}
</table>

<!-- Place cursor -->
<script language="JavaScript" type="text/javascript">
  <!-- // First input field on page
	focus_field('cn','description');
  -->
</script>

