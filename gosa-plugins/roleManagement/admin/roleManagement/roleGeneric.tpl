<table>
	<tr>
		<td>{t}cn{/t}:</td>
		<td>
			{render acl=$cnACL}
			 <input type='text' value='{$cn}' name='cn'>
			{/render}
		</td>
	</tr>
	<tr>
		<td>{t}description{/t}:</td>
		<td>
			{render acl=$descriptionACL}
			 <input type='text' value='{$description}' name='description'>
			{/render}
		</td>
	</tr>
  <tr>
    <td>
      <div style="height:10px;"></div>
      <label for="base">{t}Base{/t}</label>
    </td>
    <td>
      <div style="height:10px;"></div>
{render acl=$baseACL}
      <select id="base" size="1" name="base" title="{t}Choose subtree to place user in{/t}">
        {html_options options=$bases selected=$base_select}
      </select>
{/render}
{render acl=$baseACL disable_picture='images/lists/folder_grey.png'}
      <input type="image" name="chooseBase" src="images/lists/folder.png" class="center" 
        title="{t}Select a base{/t}">
{/render}
    </td>
  </tr>
	<tr>
		<td>{t}x121Address{/t}:</td>
		<td>
			{render acl=$x121AddressACL}
			 <input type='text' value='{$x121Address}' name='x121Address'>
			{/render}
		</td>
	</tr>
	<tr>
		<td>{t}telephoneNumber{/t}:</td>
		<td>
			{render acl=$telephoneNumberACL}
			 <input type='text' value='{$telephoneNumber}' name='telephoneNumber'>
			{/render}
		</td>
	</tr>
	<tr>
		<td>{t}facsimileTelephoneNumber{/t}:</td>
		<td>
			{render acl=$facsimileTelephoneNumberACL}
			 <input type='text' value='{$facsimileTelephoneNumber}' name='facsimileTelephoneNumber'>
			{/render}
		</td>
	</tr>
</table>
