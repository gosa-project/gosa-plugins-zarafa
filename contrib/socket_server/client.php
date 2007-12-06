#!/usr/bin/php5 -q
<?php

require_once("../../include/class_socketClient.inc");
error_reporting(E_ALL);

$sock = new Socket_Client("10.89.1.182","10000",TRUE,1);
$sock->setEncryptionKey("ferdinand_frost");

if($sock->connected()){
	/* Prepare a hunge bunch of data to be send */
	$data = "Hallo Andi. Alles wird toll.";
	$sock->write($data);
	echo $sock->read();	
	$sock->close();	
}else{
	echo "... FAILED!\n";
}

?>
