<table width="100%" summary=''>
	<tr>
		<td width="50%" valign="top">
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
							<input type='text' value="{$cn}" size="45" maxlength="80" disabled id="cn">
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
							<input type='text' value="{$description}" size="45" maxlength="80" name="description" id="description">
{/render}
						</td>
					</tr>
				</table>
		</td>
	</tr>
</table>
<hr>
<table width="100%" summary=''>
	<tr>
		<td>
				<h3>
					{t}List of hook scripts{/t}
				</h3>
				<table width="100%" summary=''>
				<tr>
					<td>
						{$Entry_divlist}
{if $sub_object_is_addable}
						<input type="submit" name="AddSubObject"     value="{msgPool type=addButton}"	title="{msgPool type=addButton}">
{else}
						<input type="submit" name="AddSubObject"     value="{msgPool type=addButton}"	title="{msgPool type=addButton}" disabled>
{/if}
					</td>
				</tr>
				</table>
		</td>
	</tr>
</table>
<input type="hidden" value="1" name="FAIhook_posted">
<!-- Place cursor -->
<script language="JavaScript" type="text/javascript">
  <!-- // First input field on page
	focus_field('cn','description');
  -->
</script>

