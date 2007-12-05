
<table style="width:100%;">
	<tr>
		<td style="width:50%;vertical-align:top;">
			<h2>{t}Heimdal options{/t}</h2>
			<i>{t}Use empty values for infinite{/t}</i>
			<table>
				<tr>
					<td>
						<label for="krb5MaxLife">{t}Ticket max life{/t}</label>
					</td>
					<td>
						<input id="krb5MaxLife" type="text" name="krb5MaxLife" value="{$krb5MaxLife}"> 
					</td>
				</tr>
				<tr>
					<td>
						<label for="krb5MaxRenew">{t}Ticket max renew{/t}</label>
					</td>
					<td>
						<input id="krb5MaxRenew" type="text" name="krb5MaxRenew" value="{$krb5MaxRenew}">
					</td>
				</tr>
				<tr>
					<td>
						<label for="krb5ValidStart">{t}Valid ticket start time{/t}</label>
					</td>
					<td>
						<input id="krb5ValidStart" type="text" name="krb5ValidStart" value="{$krb5ValidStart}">
					</td>
				</tr>
				<tr>
					<td>
						<label for="krb5ValidEnd">{t}Valid ticket end time{/t}</label>
					</td>
					<td>
						<input id="krb5ValidEnd" type="text" name="krb5ValidEnd" value="{$krb5ValidEnd}">
					</td>
				</tr>
				<tr>
					<td>
						<label for="krb5PasswordEnd">{t}Password end{/t}</label>
					</td>
					<td>
						<input id="krb5PasswordEnd" type="text" name="krb5PasswordEnd" value="{$krb5PasswordEnd}">
					</td>
				</tr>
			</table>
		</td>	
		<td>
			<h2>Flags</h2>
			<table>
				<tr>
					<td style="width:120px;">
<input {if $krb5KDCFlags_0} checked {/if} class="center" name="krb5KDCFlags_0" value="1" type="checkbox">initial<br>
<input {if $krb5KDCFlags_1} checked {/if} class="center" name="krb5KDCFlags_1" value="1" type="checkbox">forwardable<br>
<input {if $krb5KDCFlags_2} checked {/if} class="center" name="krb5KDCFlags_2" value="1" type="checkbox">proxiable<br>
<input {if $krb5KDCFlags_3} checked {/if} class="center" name="krb5KDCFlags_3" value="1" type="checkbox">renewable<br>
<input {if $krb5KDCFlags_4} checked {/if} class="center" name="krb5KDCFlags_4" value="1" type="checkbox">postdate<br>
<input {if $krb5KDCFlags_5} checked {/if} class="center" name="krb5KDCFlags_5" value="1" type="checkbox">server<br>
<input {if $krb5KDCFlags_6} checked {/if} class="center" name="krb5KDCFlags_6" value="1" type="checkbox">client<br>
					</td>
					<td>
<input {if $krb5KDCFlags_7} checked {/if} class="center" name="krb5KDCFlags_7" value="1" type="checkbox">invalid<br>
<input {if $krb5KDCFlags_8} checked {/if} class="center" name="krb5KDCFlags_8" value="1" type="checkbox">require-preauth<br>
<input {if $krb5KDCFlags_9} checked {/if} class="center" name="krb5KDCFlags_9" value="1" type="checkbox">change-pw<br>
<input {if $krb5KDCFlags_10} checked {/if} class="center" name="krb5KDCFlags_10" value="1" type="checkbox">require-hwauth<br>
<input {if $krb5KDCFlags_11} checked {/if} class="center" name="krb5KDCFlags_11" value="1" type="checkbox">ok-as-delegate<br>
<input {if $krb5KDCFlags_12} checked {/if} class="center" name="krb5KDCFlags_12" value="1" type="checkbox">user-to-user<br>
<input {if $krb5KDCFlags_13} checked {/if} class="center" name="krb5KDCFlags_13" value="1" type="checkbox">immutable<br>
 	 				</td>
				</tr>
			</table>
		</td>
	</tr>
</table>
<input type="hidden" name="pwd_heimdal_posted" value="1">
<p class="seperator"></p>
<p style="text-align:right;">
	<input type="submit" name="pw_save" value="{t}Save{/t}">
	&nbsp;
	<input type="submit" name="pw_abort" value="{t}Cancel{/t}">
</p>
