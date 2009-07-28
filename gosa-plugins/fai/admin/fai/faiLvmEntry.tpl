<input type="hidden" name="LvmFrameSubmitted" value="1">

<h2>
  <img class="center" alt="" src="plugins/fai/images/fai_partitionTable.png" 
    align="middle" title="{t}Generic{/t}">&nbsp;{t}Device{/t}
</h2>
<table style='width:100%' summary="">
	<tr>
		<td style='width:50%;border-right:1px solid #909090;'>

      <table>
        <tr>
          <td>
            <LABEL for="DISKcn">{t}Name{/t}</LABEL>{$must}&nbsp;
          </td>
          <td>
{render acl=$DISKcnACL}
            <input value="{$DISKcn}" size="45" maxlength="80" name="DISKcn" id="DISKcn">
{/render}
		      </td>
        </tr>
        <tr>
		      <td>
            <LABEL for="fstabkey">{t}FSTAB key{/t}</LABEL>
          </td>
          <td>
{render acl=$DISKFAIdiskOptionACL}
            <select name="fstabkey" size="1">
               {html_options options=$fstabkeys selected=$fstabkey}
            </select>
{/render}
  		    </td>
        </tr>
      </table>
    </td>
		<td style='vertical-align:top;'>
      <table>
        <tr>
          <td>
            <LABEL for="DISKdescription">&nbsp;{t}Description{/t}</LABEL>&nbsp;
          </td>
          <td>
{render acl=$DISKdescriptionACL}
			      <input value="{$DISKdescription}" size="45" maxlength="80" 
              name="DISKdescription" id="DISKdescription">
{/render}
		      </td>
        </tr>
      </table>
    </td>
	</tr>
</table>
<br>
<p class="seperator">&nbsp;</p>
<h2>{t}Combined partitions{/t}</h2>

{$clist}

<br>
<p class="seperator">&nbsp;</p>
<br>

<div style="align:right;" align="right">
{if !$freeze}
	<input type="submit" name="SaveDisk" value="{msgPool type=saveButton}">
{/if}
<input type="submit" name="CancelDisk" value="{msgPool type=cancelButton}" >
</div>
<!-- Place cursor -->
<script language="JavaScript" type="text/javascript">
  <!-- // First input field on page
	focus_field('DISK_cn');
  -->
</script>

