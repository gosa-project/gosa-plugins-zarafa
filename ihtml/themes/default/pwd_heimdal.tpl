
<table style="width:100%;">
	<tr>
		<td style="width:50%;vertical-align:top;">
			<h2>{t}Heimdal options{/t}</h2>
			<table>
				<tr>
					<td>
						<label for="unlimited_krb5MaxLife">{t}Max life{/t}</label>
					</td>
					<td>
						<input id="unlimited_krb5MaxLife" name="unlimited_krb5MaxLife" value="1"  type="checkbox" 
							{if $unlimited_krb5MaxLife} checked {/if} onClick="changeState('krb5MaxLife');">
						<input id="krb5MaxLife" type="text" name="krb5MaxLife" value="{$krb5MaxLife}" 
							{if $unlimited_krb5MaxLife} disabled {/if}>
					</td>
				</tr>
				<tr>
					<td>
						<label for="unlimited_krb5MaxRenew">{t}Max renew{/t}</label>
					</td>
					<td>
						<input id="unlimited_krb5MaxRenew" name="unlimited_krb5MaxRenew" value="1"  type="checkbox" 
							{if $unlimited_krb5MaxRenew} checked {/if} onClick="changeState('krb5MaxRenew');">
						<input id="krb5MaxRenew" type="text" name="krb5MaxRenew" value="{$krb5MaxRenew}" 
							{if $unlimited_krb5MaxRenew} disabled {/if}>
					</td>
				</tr>
				<tr>
					<td>
						<label for="unlimited_krb5ValidStart">{t}Valid start{/t}</label>
					</td>
					<td>
						<input id="unlimited_krb5ValidStart" name="unlimited_krb5ValidStart" value="1"  type="checkbox" 
							{if $unlimited_krb5ValidStart} checked {/if} onClick="changeState('krb5ValidStart');">
						<input id="krb5ValidStart" type="text" name="krb5ValidStart" value="{$krb5ValidStart}" 
							{if $unlimited_krb5ValidStart} disabled {/if}>
					</td>
				</tr>
				<tr>
					<td>
						<label for="unlimited_krb5ValidEnd">{t}Valid end{/t}</label>
					</td>
					<td>
						<input id="unlimited_krb5ValidEnd" name="unlimited_krb5ValidEnd" value="1"  type="checkbox"
							{if $unlimited_krb5ValidEnd} checked {/if} onClick="changeState('krb5ValidEnd');">
						<input id="krb5ValidEnd" type="text" name="krb5ValidEnd" value="{$krb5ValidEnd}" 
							{if $unlimited_krb5ValidEnd} disabled {/if}>
					</td>
				</tr>
				<tr>
					<td>
						<label for="unlimited_krb5PasswordEnd">{t}Password end{/t}</label>
					</td>
					<td>
						<input id="unlimited_krb5PasswordEnd" name="unlimited_krb5PasswordEnd" value="1"  type="checkbox"
							{if $unlimited_krb5PasswordEnd} checked {/if} onClick="changeState('krb5PasswordEnd');">
						<input id="krb5PasswordEnd" type="text" name="krb5PasswordEnd" value="{$krb5PasswordEnd}" 
							{if $unlimited_krb5PasswordEnd} disabled {/if}>
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
