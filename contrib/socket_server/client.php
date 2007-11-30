#!/usr/bin/php5 -q
<?php

require_once("../../include/class_socketClient.inc");
error_reporting(E_ALL);


echo "\n\nTry to connect";
$sock = new Socket_Client("localhost","10000",TRUE,1);
if($sock->connected()){
	echo "... successful\n";
	echo "|--Reading welcome message : \n";
	echo $sock->read();
	
	/* Prepare a hunge bunch of data to be send */
	$data = "a";
	for($i = 0 ; $i < (1024 * 4); $i++){
		$data .= "a";
	}
	echo "|--Sending ".strlen($data)."bytes of data to socket.\n";
	$sock->send($data);
	echo "|--Done!\n";
	$sock->read();	
	echo "|--".$sock->bytes_read()."bytes read.\n";
	echo "|--Sending 'exit' command to socket.\n";	
	$sock->send("exit");
	echo "|--Reading message:\n";
	echo $sock->read()."\n";	
	
	echo "|--Closing connection.\n";
	$sock->close();	
	echo "|--Done!\n";
	echo "|--End\n\n";
	
}else{
	echo "... FAILED!\n";
}

?>
