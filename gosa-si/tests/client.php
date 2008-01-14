#!/usr/bin/php5 -q
<?php

require_once("../../gosa-core/include/class_socketClient.inc");
error_reporting(E_ALL);

$sock = new Socket_Client("127.0.0.1","20082",TRUE,1);
#$sock = new Socket_Client("169.254.2.248","9999",TRUE,1);
$sock->setEncryptionKey("secret-gosa-password");

if($sock->connected()){
	/* Prepare a hunge bunch of data to be send */

# add
	#$data = "<xml><header>gosa_ping</header><source>10.89.1.155:20082</source><target>10.89.1.155:20080</target></xml>";
	#$data = "<xml> <header>job_ping</header> <source>10.89.1.155:20083</source><mac>00:1B:77:04:8A:6C</mac> <timestamp>19700101000000</timestamp> </xml>";
	#$data = "<xml> <header>job_sayHello</header> <source>10.89.1.155:20083</source><mac>00:1B:77:04:8A:6C</mac> <timestamp>20130102133900</timestamp> </xml>";
	#$data = "<xml> <header>job_ping</header> <source>10.89.1.155:20083</source><mac>00:1B:77:04:8A:6C</mac> <timestamp>20130102133900</timestamp> </xml>";

# delete
	#$data = "<xml> <header>gosa_delete_jobdb_entry</header> <where>headertag</where> <headertag>sayHello</headertag> </xml>";

# update  
	#$data = "<xml> <header>gosa_update_status_jobdb_entry</header> <where> <status>waiting</status> </where> <update> <status>processing</status> </update></xml>";
	#$data = "<xml> <header>gosa_update_status_jobdb_entry</header> <update> <status>waiting</status> </update></xml>";
	#$data = "<xml> <header>gosa_update_timestamp_jobdb_entry</header> <update> <timestamp>20130123456789</timestamp> </update></xml>";

# query
	$data = "<xml><header>gosa_query_jobdb</header><where>status</where><status>waiting</status></xml>";
	
# clear
	#$data = "<xml> <header>gosa_clear_jobdb</header> </xml>";

  $sock->write($data);

	$answer = $sock->read();	
  echo "$answer\n";
	$sock->close();	
}else{
	echo "... FAILED!\n";
}

?>
