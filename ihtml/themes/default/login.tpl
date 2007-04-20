<body style="background-color:white;background-image:none;">
{$php_errors}
<div>
        <div class='setup_header'>
                <div style="float:left;"><img src='themes/default/images/go_logo.png' class='center' alt='GOsa'></div>
                <div style="padding-top:8px;text-align:right;height:28px">{$version}</div>
        </div>
        <div class='setup_menu'>
                <b>{t}GOsa login screen{/t}</b>
        </div>
</div>

{* GOsa login - smarty template *}
{$php_errors}

<!-- Spacer for some browsers -->
<div class='gosaLoginSpacer'></div>

<div style='float:left; width:25%;'>&nbsp;</div>
<div style='float:left; width:50%; border:1px solid #AAAAAA;'>

	<div style='border-bottom:1px dashed #AAAAAA'>
		<div style='padding:3px;'>
		<p class="center" style="margin:0px 0px 0px 5px;padding:0px;font-size:24px;font-weight:bold;">
			<img class="center" src='{$password_img}' align="middle" alt="*">&nbsp;{t}Login{/t}
		</p>
		</div>
	</div>
	<div>

	    <div style='padding:12px;text-align:center;'>
		{t}Please use your username and your password to log into the site administration system.{/t}
	    </div>
  
	    <form action='index.php' method='post' name='mainform' onSubmit='js_check(this);return true;'>

	    	<input id='focus' name='focus' type='image' src='images/empty.png' style='width:0px; height:0px;' >
		<div style='text-align:center; padding:10px;'>	
		<img class='center' align='middle' src='{$personal_img}' alt='{t}Directory{/t}' title='{t}Directory{/t}'>&nbsp;
		<input type='text' name='username' maxlength='25' value='{$username}'
			 title='{t}Username{/t}' onFocus="nextfield= 'password';">
		<br>
		<br>
		<img class='center' align='middle' src='{$password_img}' alt='{t}Directory{/t}' title='{t}Directory{/t}'>&nbsp;
		<input type='password' name='password' maxlength='25' value=''
			 title='{t}Password{/t}' onFocus="nextfield= 'login';">
		</div>	

		<div style='text-align:center; padding:15px;'>
		        <img class='center' align='middle' src='{$directory_img}' alt='{t}Directory{/t}' title='{t}Directory{/t}'>&nbsp;
			<select name='server'  title='{t}Directory{/t}'>
				{html_options options=$server_options selected=$server_id}
			</select>
		</div>
	    </form>

	    <!-- Display error message on demand -->
	    <p class='gosaLoginWarning'> {$message} </p>
	    <!-- check, if cookies are enabled -->
	    <p class='gosaLoginWarning'>
	     <script language="JavaScript" type="text/javascript">
		<!--
		    document.cookie = "gosatest=empty;path=/";
		    if (document.cookie.indexOf( "gosatest=") > -1 )
			document.cookie = "gosatest=empty;path=/;expires=Thu, 01-Jan-1970 00:00:01 GMT";
		    else
			document.write("{$cookies}");
		-->
	     </script>
	    </p>
	</div>
	<div style='border-top:1px dashed #AAAAAA; text-align:right; padding:5px;'>
		  <input type='submit' name='login' value='{t}Sign in{/t}'
			 title='{t}Click here to log in{/t}'>
		<input type='hidden' name='javascript' value='false'/>		
	</div>

</div>

<div style="clear:both"></div>

<!-- Place cursor in username field -->
<script language="JavaScript" type="text/javascript">
  <!-- // First input field on page
  nextfield= "{$nextfield}";
  document.mainform.{$nextfield}.focus();
  -->
</script>

<!-- Spacer for some browsers -->
<div class='gosaLoginSpacer'></div>
</body>
