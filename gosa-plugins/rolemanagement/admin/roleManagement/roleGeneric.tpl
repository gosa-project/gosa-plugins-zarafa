<table style='width:100%;' summary="{t}Edit role{/t}">
  <tr>
    <td style='width:50%; padding-right:10px;' class='right-border'>


      <h3>
        {t}Generic{/t}
      </h3>

      <table style='width:100%;' summary="{t}Generic settings{/t}">
        <tr>
          <td>{t}Name{/t}{$must}</td>
          <td>
            {render acl=$cnACL}
             <input type='text' value='{$cn}' name='cn'>
            {/render}
          </td>
        </tr>
        <tr>
          <td>{t}Description{/t}</td>
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
            {$base}
      {/render}
          </td>
        </tr>
        <tr>
          <td colspan="2"><hr><br></td>
        </tr>
        <tr>
          <td>{t}Phone number{/t}</td>
          <td>
            {render acl=$telephoneNumberACL}
             <input type='text' value='{$telephoneNumber}' name='telephoneNumber'>
            {/render}
          </td>
        </tr>
        <tr>
          <td>{t}Fax number{/t}</td>
          <td>
            {render acl=$facsimileTelephoneNumberACL}
             <input type='text' value='{$facsimileTelephoneNumber}' name='facsimileTelephoneNumber'>
            {/render}
          </td>
        </tr>
      </table>

    </td>
    <td style='padding-left:10px;'>

      <h3>
        {t}Occupants{/t}
      </h3>

{render acl=$roleOccupantACL}
      <select style="width:100%; height:450px;" id="members" name="members[]" size="15" multiple>
        {$members}
      </select>
{/render}
      <br>
{render acl=$roleOccupantACL}
      <button type='submit' name='edit_membership'>{msgPool type=addButton}</button>&nbsp;

{/render}
{render acl=$roleOccupantACL}
      <button type='submit' name='delete_membership'>{msgPool type=delButton}</button>

{/render}
    </td>
  </tr>
</table>  
