#!/usr/bin/php5 -q
<?php

error_reporting(E_ALL);

//IP to bind to, use 0 for all 
$bind_ip = 0;

// Port to bind  
$bind_port = 10000;

// Max clients 
$max_clients = 3;

// Rijndal encrypt key 
$enable_encryption = TRUE;
$encrypt_key = "Hallo hier bin ich.";


/* Create Socket - 
 *  AF_INET means IPv4 Socket 
 *  SOCK_STREAM means Fullduplex TCP
 *  SOL_TCP - Protocol type TCP
 */
$socket = socket_create(AF_INET,SOCK_STREAM,SOL_TCP);

/* Enable reuse of local address */
socket_set_option($socket,SOL_SOCKET,SO_REUSEADDR,1);

/* Bind to socket */
socket_bind($socket,$bind_ip,$bind_port);
socket_listen($socket,$max_clients);

$clients = array('0' => array('socket' => $socket));

echo "\nServer startet on port : ".$bind_port."
You may use telnet to connect to the server
";

/* Accept connections till server is killed */
while(TRUE) {

	/* Create an array of sockets to read from */
	$read[0] = $socket;
	for($i=1;$i<count($clients)+1;$i++) {
		if(isset($clients[$i] ) && $clients[$i] != NULL) {
			$read[$i+1] = $clients[$i]['socket'];
		}
	}

	/* Check each socket listed in array $read for readable data.
     * We must do this to prevent the server from freezing if the socket is blocked.
	 * All sockets that are readable will remain in the array, all blocked sockets will be removed.  
     */
	$ready = socket_select($read,$write=NULL,$except=NULL,0);

    /* Handle incoming connections / Incoming data
     */
	if(in_array($socket,$read)) {

		/* Check each client slot for a new connection */
		for($i=1;$i<$max_clients+1;$i++) {
		
			/* Accept new connections */
			if(!isset($clients[$i])) {
				$clients[$i]['socket'] = socket_accept($socket);
				socket_getpeername($clients[$i]['socket'],$ip);
				$clients[$i]['ipaddy'] = $ip;

				socket_write($clients[$i]['socket'],encrypt(
"Welcome to GOsa Test Server 
============================
Type some text here:",$encrypt_key)."\n");

				echo("New client connected: " . $clients[$i]['ipaddy'] . " \n");
				break;
			}
			elseif($i == $max_clients - 1) {
				echo("To many Clients connected! \n");
			}
			if($ready < 1) {
				continue;
			}
		}
	}

	/* Check if there is data to read from the client sockets 
     */
	for($i=1;$i<$max_clients+1;$i++) {

		/* Check if socket has send data to the server 
         */
		if(isset($clients[$i]) && in_array($clients[$i]['socket'],$read)) {

			/* Read socket data */
			$data = @socket_read($clients[$i]['socket'],1024000, PHP_NORMAL_READ);

			/* Client disconnected */
			if ($data === FALSE) {
				unset($clients[$i]);
				echo "Client disconnected! \n";
				continue;
			}

			$data = trim(decrypt($data,$encrypt_key));
			echo "Client (".$clients[$i]['ipaddy'].") send : ".substr($data,0,30)."... \n";
	
			if($data == "exit"){
				/* Close conenction */
				socket_write($clients[$i]['socket'],encrypt("Bye Bye!",$encrypt_key));
				@socket_close($clients[$i]);
				echo "Client disconnected! bye bye!".$clients[$i]['ipaddy']."\n";
			}else{
				/* Send some data back to the client */
				$data = encrypt(strrev($data),$encrypt_key);
				socket_write($clients[$i]['socket'],$data);
			}
		}
	}
}



function encrypt($data,$key)
{
	global $enable_encryption;
	/* Encrypt data */
	if($enable_encryption){
		$iv_size = mcrypt_get_iv_size(MCRYPT_RIJNDAEL_256, MCRYPT_MODE_ECB);
		$iv = mcrypt_create_iv($iv_size, MCRYPT_RAND);
		$data = mcrypt_encrypt (MCRYPT_RIJNDAEL_256, $key, $data, MCRYPT_MODE_ECB, $iv);
	}
	return(base64_encode($data));
}

function decrypt($data,$key)
{
	global $enable_encryption;
	$data = base64_decode($data);

	/* Decrypt data */
	if($enable_encryption){
		$iv_size = mcrypt_get_iv_size(MCRYPT_RIJNDAEL_256, MCRYPT_MODE_ECB);
		$iv = mcrypt_create_iv($iv_size, MCRYPT_RAND);
		$data = mcrypt_decrypt(MCRYPT_RIJNDAEL_256, $key, $data, MCRYPT_MODE_ECB, $iv);
		$data = rtrim($data,"\x00");
	}
	return($data);
}


?> 
