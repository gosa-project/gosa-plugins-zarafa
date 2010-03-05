{if !$pg}
  <h3>{t}Open-Xchange Account{/t} - {t}disabled, no Postgresql support detected. Or the specified database can't be reached{/t}</h3>
{else}
  <h3>
<input type="checkbox" name="oxchange" value="B" 
	{$oxchangeState} {$oxchangeAccountACL} 
	onCLick="	
	{if $OXAppointmentDays_W} 
		changeState('OXAppointmentDays');
	{/if}
	{if $OXTaskDays_W} 
		changeState('OXTaskDays');
	{/if}
	{if $OXTimeZone_W} 
		changeState('OXTimeZone'); 
	{/if}
	">
{t}Open-Xchange account{/t}</h3>


<table summary="{t}Open-Xchange configuration{/t}" style="width:100%; vertical-align:top; text-align:left;" cellpadding=0 border=0>

 <!-- Headline container -->
 <tr>
   <td style="width:50%; vertical-align:top;">
     <table style="margin-left:4px;" summary="{t}Open-Xchange configuration{/t}">
       <tr>
         <td colspan=2 style="vertical-align:top;">
           <b>{t}Remember{/t}</b>
         </td>
       </tr>
       <tr>
         <td><LABEL for="OXAppointmentDays">{t}Appointment Days{/t}</LABEL></td>
	 <td>

{render acl=$OXAppointmentDaysACL}	
<input type='text' name="OXAppointmentDays" id="OXAppointmentDays" size=7 maxlength=7 value="{$OXAppointmentDays}" {$oxState} >
{/render}
	 {t}days{/t}</td>
       </tr>
       <tr>
         <td><LABEL for="OXTaskDays">{t}Task Days{/t}</LABEL></td>
	 <td>

{render acl=$OXTaskDaysACL}	
<input type='text' name="OXTaskDays" id="OXTaskDays" size=7 maxlength=7 value="{$OXTaskDays}" {$oxState} >
{/render}

	 {t}days{/t}
	</td>
       </tr>
     </table>
   </td>
   <td rowspan=2 style="border-left:1px solid #A0A0A0">
     &nbsp;
   </td>
   <td style="vertical-align:top;">
     <table summary="{t}Open-Xchange configuration{/t}">
       <tr>
         <td colspan=2 style="vertical-align:top;">
           <b>{t}User Information{/t}</b>
         </td>
       </tr>
       <tr>
         <td><LABEL for="OXTimeZone">{t}User Timezone{/t}</LABEL></td>
	 <td>

{render acl=$OXTimeZoneACL}	
<select size="1" name="OXTimeZone" id="OXTimeZone" {$oxState} > 
 {html_options values=$timezones output=$timezones selected=$OXTimeZone}
 </select>
{/render}

	 </td>
       </tr>
       <tr>
         <td></td>
	 <td></td>
       </tr>
     </table>
   </td>
 </tr>
</table>
{/if}
