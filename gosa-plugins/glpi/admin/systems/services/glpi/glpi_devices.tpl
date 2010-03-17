{if $device_type=="monitor"}
	<h3>{t}Add/Edit monitor{/t}</h3>
	<hr>
	<br>
	<table summary="" width="100%">
		<tr>
			<td style='padding-right:5px;' class='right-border'>

				<table summary="" width="100%">
					<tr>
						<td>{t}Name{/t}
						</td>	
						<td>	
							<input type='text' name="name" value="{$name}">
						</td>
					</tr>
					<tr>
						<td>{t}Comments{/t}

						</td>	
						<td>	
							<textarea name="comments">{$comments}</textarea>
						</td>
					</tr>
					<tr>
						<td>{t}Manufacturer{/t}
						</td>	
						<td>	
							<select name="FK_glpi_enterprise" size=1>
								{html_options values=$FK_glpi_enterpriseKeys output=$FK_glpi_enterprises selected=$FK_glpi_enterprise}
							</select>
						</td>
					</tr>
					<tr>
						<td>{t}Monitor size{/t}

						</td>	
						<td>	
							<input type='text' name="size" value="{$size}"> {t}Inch{/t}
						</td>
					</tr>
				</table>
			</td>
			<td>
				<table summary="" width="100%">
  					<tr>
                        <td>{t}Integrated microphone{/t}
                        </td>
                        <td>
                            <input type="radio" name="flags_micro" value="1" {if $flags_micro == "1"}checked {/if}>{t}Yes{/t}
                            <input type="radio" name="flags_micro" value="0" {if ($flags_micro == "0")||($flags_micro=="")}checked {/if}>{t}No{/t}
                        </td>
                    </tr>
                    <tr>
                        <td>{t}Integrated speakers{/t}
                        </td>
                        <td>
                            <input type="radio" name="flags_speaker" value="1" {if $flags_speaker == "1"}checked {/if}>{t}Yes{/t}
                            <input type="radio" name="flags_speaker" value="0" {if ($flags_speaker == "0")||($flags_speaker=="")}checked {/if}>{t}No{/t}
                        </td>
                    </tr>
                    <tr>
                        <td>{t}Sub-D{/t}
                        </td>
                        <td>
                            <input type="radio" name="flags_subd" value="1" {if $flags_subd == "1"}checked {/if}>{t}Yes{/t}
                            <input type="radio" name="flags_subd" value="0" {if ($flags_subd == "0")||($flags_subd=="")}checked {/if}>{t}No{/t}
                        </td>
                    </tr>
                    <tr>
                        <td>{t}BNC{/t}
                        </td>
                        <td>
                            <input type="radio" name="flags_bnc" value="1" {if $flags_bnc == "1"}checked {/if}>{t}Yes{/t}
                            <input type="radio" name="flags_bnc" value="0" {if ($flags_bnc == "0")||($flags_bnc=="")}checked {/if}>{t}No{/t}
                        </td>
                    </tr>
					<tr>
						<td>{t}Serial number{/t}

						</td>	
						<td>	
							<input type='text' name="serial" value="{$serial}">
						</td>
					</tr>
					<tr>
						<td>{t}Additional serial number{/t}

						</td>	
						<td>	
							<input type='text' name="otherserial" value="{$otherserial}">
						</td>
					</tr>
				</table>
			</td>
		</tr>
	</table>


{elseif $device_type=="pci"}

        <h3>{t}Add/Edit other device{/t}</h3>
        <hr>
        <br>
	<table summary="" width="100%">
		<tr>
			<td style='padding-right:5px;' class='right-border'>

				<table summary="" width="100%">
					<tr>
						<td>{t}Name{/t}
						</td>	
						<td>	
							<input type='text' name="designation" value="{$designation}">
						</td>
					</tr>
					<tr>
						<td>{t}Comment{/t}

						</td>	
						<td>	
							<textarea name="comment">{$comment}</textarea>
						</td>
					</tr>
				</table>
			</td>
			<td>
				<table summary="" width="100%">
					<tr>
						<td>{t}Manufacturer{/t}
						</td>	
						<td>	
							<select name="FK_glpi_enterprise" size=1>
								{html_options values=$FK_glpi_enterpriseKeys output=$FK_glpi_enterprises selected=$FK_glpi_enterprise}
							</select>
						</td>
					</tr>
				</table>
			</td>
		</tr>
	</table>

{elseif $device_type=="power"}

        <h3>{t}Add/Edit power supply{/t}</h3>
        <hr>
        <br>
	<table summary="" width="100%">
		<tr>
			<td style='padding-right:5px;' class='right-border'>

				<table summary="" width="100%">
					<tr>
						<td>{t}Name{/t}
						</td>	
						<td>	
							<input type='text' name="designation" value="{$designation}">
						</td>
					</tr>
					<tr>
						<td>{t}Comment{/t}

						</td>	
						<td>	
							<textarea name="comment">{$comment}</textarea>
						</td>
					</tr>
				</table>
			</td>
			<td>
				<table summary="" width="100%">
					<tr>
						<td>{t}Manufacturer{/t}
						</td>	
						<td>	
							<select name="FK_glpi_enterprise" size=1>
								{html_options values=$FK_glpi_enterpriseKeys output=$FK_glpi_enterprises selected=$FK_glpi_enterprise}
							</select>
						</td>
					</tr>
					<tr>
						<td>{t}Atx{/t}
						</td>	
						<td>	
							<input type="radio" name="atx" value="Y" {if ($atx == "Y")||($atx=="")}checked {/if}>{t}Yes{/t}
							<input type="radio" name="atx" value="N" {if $atx == "N"}checked {/if}>{t}No{/t}
						</td>
					</tr>
					<tr>
						<td>{t}Power{/t}
						</td>	
						<td>	
							<input type='text' name="power" value="{$power}">
						</td>
					</tr>
				</table>
			</td>
		</tr>
	</table>
{elseif $device_type=="gfxcard"}

        <h3>{t}Add/Edit graphic card{/t}</h3>
        <hr>
        <br>
	<table summary="" width="100%">
		<tr>
			<td style='padding-right:5px;' class='right-border'>

				<table summary="" width="100%">
					<tr>
						<td>{t}Name{/t}
						</td>	
						<td>	
							<input type='text' name="designation" value="{$designation}">
						</td>
					</tr>
					<tr>
						<td>{t}Comment{/t}

						</td>	
						<td>	
							<textarea name="comment">{$comment}</textarea>
						</td>
					</tr>
				</table>
			</td>
			<td>
				<table summary="" width="100%">
					<tr>
						<td>{t}Manufacturer{/t}
						</td>	
						<td>	
							<select name="FK_glpi_enterprise" size=1>
								{html_options values=$FK_glpi_enterpriseKeys output=$FK_glpi_enterprises selected=$FK_glpi_enterprise}
							</select>
						</td>
					</tr>
					<tr>
						<td>{t}Interface{/t}
						</td>	
						<td>	
							<select name="interface" size=1>
								{html_options values=$GFXInterfaceKeys output=$GFXInterfaces selected=$interface}
							</select>
						</td>
					</tr>
					<tr>
						<td>{t}Ram{/t}
						</td>	
						<td>	
							<input type='text' name="ram" value="{$ram}">
						</td>
					</tr>
				</table>
			</td>
		</tr>
	</table>
{elseif $device_type=="control"}

        <h3>{t}Add/Edit controller{/t}</h3>
        <hr>
        <br>
	<table summary="" width="100%">
		<tr>
			<td style='padding-right:5px;' class='right-border'>

				<table summary="" width="100%">
					<tr>
						<td>{t}Name{/t}
						</td>	
						<td>	
							<input type='text' name="designation" value="{$designation}">
						</td>
					</tr>
					<tr>
						<td>{t}Comment{/t}

						</td>	
						<td>	
							<textarea name="comment">{$comment}</textarea>
						</td>
					</tr>
				</table>
			</td>
			<td>
				<table summary="" width="100%">
					<tr>
						<td>{t}Manufacturer{/t}
						</td>	
						<td>	
							<select name="FK_glpi_enterprise" size=1>
								{html_options values=$FK_glpi_enterpriseKeys output=$FK_glpi_enterprises selected=$FK_glpi_enterprise}
							</select>
						</td>
					</tr>
					<tr>
						<td>{t}Interface{/t}
						</td>	
						<td>	
							<select name="interface" size=1>
								{html_options values=$HDDInterfaceKeys output=$HDDInterfaces selected=$interface}
							</select>
						</td>
					</tr>
					<tr>
						<td>{t}Size{/t}
						</td>	
						<td>	
							<input type="radio" name="raid" value="Y" {if ($raid == "Y")||($raid=="")}checked {/if}>{t}Yes{/t}
							<input type="radio" name="raid" value="N" {if $raid == "N"}checked {/if}>{t}No{/t}
						</td>
					</tr>
				</table>
			</td>
		</tr>
	</table>

{elseif $device_type=="drive"}

        <h3>{t}Add/Edit drive{/t}</h3>
        <hr>
        <br>
	<table summary="" width="100%">
		<tr>
			<td style='padding-right:5px;' class='right-border'>

				<table summary="" width="100%">
					<tr>
						<td>{t}Name{/t}
						</td>	
						<td>	
							<input type='text' name="designation" value="{$designation}">
						</td>
					</tr>
					<tr>
						<td>{t}Comment{/t}

						</td>	
						<td>	
							<textarea name="comment">{$comment}</textarea>
						</td>
					</tr>
				</table>
			</td>
			<td>
				<table summary="" width="100%">
					<tr>
						<td>{t}Manufacturer{/t}
						</td>	
						<td>	
							<select name="FK_glpi_enterprise" size=1>
								{html_options values=$FK_glpi_enterpriseKeys output=$FK_glpi_enterprises selected=$FK_glpi_enterprise}
							</select>
						</td>
					</tr>
					<tr>
						<td>{t}Speed{/t}
						</td>	
						<td>	
							<input type="text" name="speed" value="{$speed}">
						</td>
					</tr>
					<tr>
						<td>{t}Interface{/t}
						</td>	
						<td>	
							<select name="interface" size=1>
								{html_options values=$HDDInterfaceKeys output=$HDDInterfaces selected=$interface}
							</select>
						</td>
					</tr>
					<tr>
						<td>{t}Writeable{/t}
						</td>	
						<td>	
							<input type="radio" name="is_writer" value="Y" {if ($is_writer == "Y")||($is_writer=="")}checked {/if}>{t}Yes{/t}
							<input type="radio" name="is_writer" value="N" {if $is_writer == "N"}checked {/if}>{t}No{/t}
						</td>
					</tr>
				</table>
			</td>
		</tr>
	</table>

{elseif $device_type=="hdd"}
        <h3>{t}Add/Edit harddisk{/t}</h3>
        <hr>
        <br>
	<table summary="" width="100%">
		<tr>
			<td style='padding-right:5px;' class='right-border'>

				<table summary="" width="100%">
					<tr>
						<td>{t}Name{/t}
						</td>	
						<td>	
							<input type='text' name="designation" value="{$designation}">
						</td>
					</tr>
					<tr>
						<td>{t}Comment{/t}

						</td>	
						<td>	
							<textarea name="comment">{$comment}</textarea>
						</td>
					</tr>
				</table>
			</td>
			<td>
				<table summary="" width="100%">
					<tr>
						<td>{t}Manufacturer{/t}
						</td>	
						<td>	
							<select name="FK_glpi_enterprise" size=1>
								{html_options values=$FK_glpi_enterpriseKeys output=$FK_glpi_enterprises selected=$FK_glpi_enterprise}
							</select>
						</td>
					</tr>
					<tr>
						<td>{t}Rpm{/t}
						</td>	
						<td>	
							<input type="text" name="rpm" value="{$rpm}">
						</td>
					</tr>
					<tr>
						<td>{t}Cache{/t}
						</td>	
						<td>	
							<input type="text" name="cache" value="{$cache}">
						</td>
					</tr>
					<tr>
						<td>{t}Size{/t}
						</td>	
						<td>	
							<input type="text" name="specif_default" value="{$specif_default}">
						</td>
					</tr>
					<tr>
						<td>{t}Type{/t}
						</td>	
						<td>	
							<select name="interface" size=1>
								{html_options values=$HDDInterfaceKeys output=$HDDInterfaces selected=$interface}
							</select>
						</td>
					</tr>
					
				</table>
			</td>
		</tr>
	</table>

{elseif $device_type=="ram"}

        <h3>{t}Add/Edit memory{/t}</h3>
        <hr>
        <br>
	<table summary="" width="100%">
		<tr>
			<td style='padding-right:5px;' class='right-border'>

				<table summary="" width="100%">
					<tr>
						<td>{t}Name{/t}
						</td>	
						<td>	
							<input type='text' name="designation" value="{$designation}">
						</td>
					</tr>
					<tr>
						<td>{t}Comment{/t}

						</td>	
						<td>	
							<textarea name="comment">{$comment}</textarea>
						</td>
					</tr>
				</table>
			</td>
			<td>
				<table summary="" width="100%">
					<tr>
						<td>{t}Manufacturer{/t}
						</td>	
						<td>	
							<select name="FK_glpi_enterprise" size=1>
								{html_options values=$FK_glpi_enterpriseKeys output=$FK_glpi_enterprises selected=$FK_glpi_enterprise}
							</select>
						</td>
					</tr>
					<tr>
						<td>{t}Frequenz{/t}
						</td>	
						<td>	
							<input type="text" name="frequence" value="{$frequence}">
						</td>
					</tr>
					<tr>
						<td>{t}Size{/t}
						</td>	
						<td>	
							<input type="text" name="specif_default" value="{$specif_default}">
						</td>
					</tr>
					<tr>
						<td>{t}Type{/t}
						</td>	
						<td>	
							<select name="type" size=1>
								{html_options values=$RAMtypeKeys output=$RAMtypes selected=$type}
							</select>
						</td>
					</tr>
					
				</table>
			</td>
		</tr>
	</table>

{elseif $device_type=="sndcard"}
        <h3>{t}Add/Edit sound card{/t}</h3>
        <hr>
        <br>
	<table summary="" width="100%">
		<tr>
			<td style='padding-right:5px;' class='right-border'>

				<table summary="" width="100%">
					<tr>
						<td>{t}Name{/t}
						</td>	
						<td>	
							<input type='text' name="designation" value="{$designation}">
						</td>
					</tr>
					<tr>
						<td>{t}Comment{/t}

						</td>	
						<td>	
							<textarea name="comment">{$comment}</textarea>
						</td>
					</tr>
				</table>
			</td>
			<td>
				<table summary="" width="100%">
					<tr>
						<td>{t}Manufacturer{/t}
						</td>	
						<td>	
							<select name="FK_glpi_enterprise" size=1>
								{html_options values=$FK_glpi_enterpriseKeys output=$FK_glpi_enterprises selected=$FK_glpi_enterprise}
							</select>
						</td>
					</tr>
					<tr>
						<td>{t}Type{/t}
						</td>	
						<td>	
							<input type="text" name="type" value="{$type}">
						</td>
					</tr>
				</table>
			</td>
		</tr>
	</table>
{elseif $device_type=="iface"}
    <h3>{t}Add/Edit network interface{/t}</h3>
    <hr>
    <br>

    <table summary="" width="100%">
        <tr>
            <td style='padding-right:5px;' class='right-border'>

                <table summary="" width="100%">
                    <tr>
                        <td>{t}Name{/t}
                        </td>
                        <td>
                            <input type='text' name="designation" value="{$designation}">
                        </td>
                    </tr>
                    <tr>
                        <td>{t}Comment{/t}

                        </td>
                        <td>
                            <textarea name="comment">{$comment}</textarea>
                        </td>
                    </tr>
                </table>
            </td>
            <td>
                <table summary="" width="100%">
                    <tr>
                        <td>{t}Manufacturer{/t}
                        </td>
                        <td>
                            <select name="FK_glpi_enterprise" size=1>
                                {html_options values=$FK_glpi_enterpriseKeys output=$FK_glpi_enterprises selected=$FK_glpi_enterprise}
                            </select>
                        </td>
                    </tr>
                    <tr>
                        <td>{t}MAC address{/t}
                        </td>
                        <td>
                            <input type="text" name="specif_default" value="{$specif_default}">
                        </td>
                    </tr>
                    <tr>
                        <td>{t}Bandwidth{/t}
                        </td>
                        <td>
                            <input type="text" name="bandwidth" value="{$bandwidth}">
                        </td>
                    </tr>
                </table>
            </td>
        </tr>
    </table>
{elseif $device_type=="processor"}
    <h3>{t}Add/Edit processor{/t}</h3>
    <hr>
    <br>
    <table summary="" width="100%">
        <tr>
            <td style='padding-right:5px;' class='right-border'>

                <table summary="" width="100%">
                    <tr>
                        <td>{t}Name{/t}
                        </td>
                        <td>
                            <input type='text' name="designation" value="{$designation}">
                        </td>
                    </tr>
                    <tr>
                        <td>{t}Comment{/t}

                        </td>
                        <td>
                            <textarea name="comment">{$comment}</textarea>
                        </td>
                    </tr>
                </table>
            </td>
            <td>
                <table summary="" width="100%">
                    <tr>
                        <td>{t}Manufacturer{/t}
                        </td>
                        <td>
                            <select name="FK_glpi_enterprise" size=1>
                                {html_options values=$FK_glpi_enterpriseKeys output=$FK_glpi_enterprises selected=$FK_glpi_enterprise}
                            </select>
                        </td>
                    </tr>
                    <tr>
                        <td>{t}Frequence{/t}
                        </td>
                        <td>
                            <input type="text" name="frequence" value="{$frequence}">
                        </td>
                    </tr>
                    <tr>
                        <td>{t}Default frequence{/t}
                        </td>
                        <td>
                            <input type="text" name="specif_default" value="{$specif_default}">
                        </td>
                    </tr>
                </table>
            </td>
        </tr>
    </table>

{elseif $device_type=="moboard"}
        <h3>{t}Add/Edit motherboard{/t}</h3>
        <hr>
        <br>
	<table summary="" width="100%">
		<tr>
			<td style='padding-right:5px;' class='right-border'>

				<table summary="" width="100%">
					<tr>
						<td>{t}Name{/t}
						</td>	
						<td>	
							<input type='text' name="designation" value="{$designation}">
						</td>
					</tr>
					<tr>
						<td>{t}Comment{/t}

						</td>	
						<td>	
							<textarea name="comment">{$comment}</textarea>
						</td>
					</tr>
				</table>
			</td>
			<td>
				<table summary="" width="100%">
					<tr>
						<td>{t}Manufacturer{/t}
						</td>	
						<td>	
							<select name="FK_glpi_enterprise" size=1>
								{html_options values=$FK_glpi_enterpriseKeys output=$FK_glpi_enterprises selected=$FK_glpi_enterprise}
							</select>
						</td>
					</tr>
					<tr>
						<td>{t}Chipset{/t}
						</td>	
						<td>	
							<input type="text" name="chipset" value="{$chipset}">
						</td>
					</tr>
				</table>
			</td>
		</tr>
	</table>
{elseif $device_type=="case"}
        <h3>{t}Add/Edit computer case{/t}</h3>
        <hr>
        <br>
	<table summary="" width="100%">
		<tr>
			<td style='padding-right:5px;' class='right-border'>

				<table summary="" width="100%">
					<tr>
						<td>{t}Name{/t}
						</td>	
						<td>	
							<input type='text' name="designation" value="{$designation}">
						</td>
					</tr>
					<tr>
						<td>{t}Comment{/t}

						</td>	
						<td>	
							<textarea name="comment">{$comment}</textarea>
						</td>
					</tr>
				</table>
			</td>
			<td>
				<table summary="" width="100%">
					<tr>
						<td>{t}Manufacturer{/t}
						</td>	
						<td>	
							<select name="FK_glpi_enterprise" size=1>
								{html_options values=$FK_glpi_enterpriseKeys output=$FK_glpi_enterprises selected=$FK_glpi_enterprise}
							</select>
						</td>
					</tr>
					<tr>
						<td>{t}format{/t}

						</td>	
						<td>	
							<select name="format" size=1>
								{html_options values=$formatKeys output=$formats selected=$format}
							</select>
						</td>
					</tr>
				</table>
			</td>
		</tr>
	</table>
{/if}


<hr>
<div align="right">
	<p>
		<button type='submit' name='SaveDeviceChanges'>{msgPool type=saveButton}</button>

		<button type='submit' name='AbortDeviceChanges'>{msgPool type=cancelButton}</button>

	</p> 
</div>
