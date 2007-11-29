#!/usr/bin/php5 -q
<?php
$socket = socket_create(AF_INET,SOCK_STREAM,SOL_TCP);
$max_clients = 10;
$port = 10000;

socket_set_option($socket,SOL_SOCKET,SO_REUSEADDR,1);
socket_bind($socket,0,$port);
socket_listen($socket,$max_clients);

$clients = array('0' => array('socket' => $socket));

echo "\nServer startet on port : ".$port."
You may use telnet to connect to the server
";

while(TRUE) {
	$read[0] = $socket;
	for($i=1;$i<count($clients)+1;$i++) {
		if($clients[$i] != NULL) {
			$read[$i+1] = $clients[$i]['socket'];
		}
	}

	$ready = socket_select($read,$write=NULL,$except=NULL,0);

	if(in_array($socket,$read)) {
		for($i=1;$i<$max_clients+1;$i++) {
			if(!isset($clients[$i])) {
				$clients[$i]['socket'] = socket_accept($socket);
				socket_getpeername($clients[$i]['socket'],$ip);
				$clients[$i]['ipaddy'] = $ip;

				socket_write($clients[$i]['socket'],
"Welcome to GOsa Test Server 
============================
Type some text here:\n");

				echo("New client connected: " . $clients[$i]['ipaddy'] . " ");
				break;
			}
			elseif($i == $max_clients - 1) {
				echo("To many Clients connected! ");
			}
			if($ready < 1) {
				continue;
			}
		}
	}
	for($i=1;$i<$max_clients+1;$i++) {
		if(in_array($clients[$i]['socket'],$read)) {

			$data = @socket_read($clients[$i]['socket'],1024000, PHP_NORMAL_READ);

			if ($data === FALSE) {
				unset($clients[$i]);
				echo "Client disconnected! ";
				continue;
			}

			$data = trim($data);

			socket_write($clients[$i]['socket'],$data);

#		if(!empty($data)) {
#			for($j=1;$j<$max_clients+1;$j++) {
#				if(isset($clients[$j]['socket'])) {
#					if(($clients[$j]['socket'] != $clients[$i]['socket']) && ($clients[$j]['socket'] != $socket)) {
#						echo($clients[$i]['ipaddy'] . " is sending a message! ");
#						socket_write($clients[$j]['socket'],"[" . $clients[$i]['ipaddy'] . "] says: " . $data . " ");
#					}
#				}
#			}
#			break;
#		}
		}
	}
}
?> 
