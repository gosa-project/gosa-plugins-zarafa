<br>
<h3>{t}Attachment{/t}</h3>
	
<table summary="" width="100%">
	<tr>
		<td width="50%" style="vertical-align:top;">
			<table summary="" width="100%">
				<tr>
					<td  style="vertical-align:top;">
						{t}Name{/t}
					</td>
					<td>
						<input type="text" value="{$name}" name="name">
					</td>
				</tr>
				<tr>
					<td style="vertical-align:top;">
						{t}Comment{/t}
					</td>
					<td style="vertical-align:top;">
						<textarea name="comment">{$comment}</textarea>
					</td>
				</tr>
			</table>
		</td>
		<td  style="vertical-align:top;">
			<table summary="" width="100%">
				<tr>
					<td style="vertical-align:top;">
						{t}File{/t}
					</td>
					<td style="vertical-align:top;">
						<input type="file" value="" name="filename"><button type='submit' name='upload'>{t}Upload{/t}</button>

					</td>
				</tr>
				<tr>
					<td style="vertical-align:top;">
						{t}Status{/t}
					</td>
					<td style="vertical-align:top;">
						{$status}
					</td>
				</tr>
				<tr>
					<td style="vertical-align:top;">
						{t}Filename{/t}
					</td>
					<td style="vertical-align:top;">
						{$filename}
					</td>
				</tr>
				<tr>
					<td style="vertical-align:top;">
						{t}Mime-type{/t}
					</td>
					<td style="vertical-align:top;">
						{$mime}
					</td>
				</tr>
			</table>
		</td>
	</tr>
</table>


<hr>
<div align="right">
	<p>
		<button type='submit' name='SaveAttachment'>{msgPool type=saveButton}</button>

		<button type='submit' name='CancelAttachment'>{msgPool type=cancelButton}</button>

	</p>
</div>
<script language="JavaScript" type="text/javascript">
  <!-- // First input field on page
	focus_field('name');
  -->
</script>

