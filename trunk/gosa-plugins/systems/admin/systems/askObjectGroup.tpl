<div style="font-size: 18px;">
	{$headline}
</div>
<br>
<p class="seperator">
{t}This dialog gives you the possibility to select an optional bundle of predefined settings to be inherited.{/t}
<br>
<br>
</p>
<br>
<table summary="" style='width:100%'>
 <tr>
  <td style='width:49%'>
   <table summary="">
    <tr>
     <td>
        <h1>{t}Select object group{/t}</h1>
     </td>
   </tr>
   <tr>
     <td>
      {t}Choose an object group as template{/t}&nbsp;
      <select name="SelectedOgroup" title="{t}Select object group{/t}" style="width:120px;">
      {html_options options=$ogroups}
      
     </td>
    </tr>
  </table>
  <p class="seperator">
  <br>
  </p>
  <p style="text-align:right">
  <input type="submit" name="edit_continue" value="Fortsetzen">&nbsp;<input type="submit" name="edit_cancel" value="Abbrechen">
  </p>

