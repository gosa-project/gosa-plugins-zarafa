#!/usr/bin/php5 -q
<?php

require_once("../../gosa-core/include/class_socketClient.inc");
error_reporting(E_ALL);

$sock = new Socket_Client("10.89.1.155","20082",TRUE,1);
#$sock = new Socket_Client("169.254.2.248","9999",TRUE,1);
$sock->setEncryptionKey("secret-gosa-password");

if($sock->connected()){
	/* Prepare a hunge bunch of data to be send */
	#$data = "<xml><header>gosa_ping</header><source>10.89.1.155:20082</source><target></target><mac>11:22:33:44:55</mac></xml>";
	#$data = "<xml> <header>job_ping</header> <source>10.89.1.155:20083</source><mac>00:1B:77:04:8A:6C</mac> <timestamp>19700101000000</timestamp> </xml>";
	$data = "<xml> <header>job_ping</header> <source>10.89.1.155:20083</source><mac>00:1B:77:04:8A:6C</mac> <timestamp>20080102133900</timestamp> </xml>";
	$sock->write($data);
  
  #$sock->setEncryptionKey("ferdinand_frost");

	$answer = $sock->read();	
  echo "$answer\n";
	$sock->close();	
}else{
	echo "... FAILED!\n";
}

?>
