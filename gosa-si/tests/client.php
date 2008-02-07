#!/usr/bin/php5 -q
<?php

require_once("../../gosa-core/include/class_socketClient.inc");
error_reporting(E_ALL);

$zahl= 1;

for($count = 1; $count <= $zahl; $count++)
{

$sock = new Socket_Client("127.0.0.1","20081",TRUE,1);
$sock->setEncryptionKey("secret-gosa-password");

if($sock->connected()){
	/* Prepare a hunge bunch of data to be send */

# add
#$data = "<xml> <header>job_ping</header> <source>10.89.1.155:20083</source><mac>00:1B:77:04:8A:6C</mac> <timestamp>19700101000001</timestamp> </xml>";
#$data = "<xml> <header>job_sayHello</header> <source>10.89.1.155:20083</source><mac>00:1B:77:04:8A:6C</mac> <timestamp>20130102133900</timestamp> </xml>";
#$data = "<xml> <header>job_ping</header> <source>10.89.1.155:20083</source> <target>10.89.1.155:20081</target><mac>00:1B:77:04:8A:6C</mac> <timestamp>20130102133900</timestamp> </xml>";

# delete
#$data = "<xml> <header>gosa_delete_jobdb_entry</header><where><clause><phrase><id>3</id></phrase></clause></where></xml>";

# update  
#$data = "<xml> <header>gosa_update_status_jobdb_entry</header> <where><clause><phrase> <status>waiting</status></phrase></clause> </where> <update><status>processing</status> <result>update</result></update></xml>";

# query
#$data = "<xml><header>gosa_query_jobdb</header><where><clause><connector>and</connector><phrase><operator>gt</operator><ROWID>0</ROWID></phrase><phrase><operator>le</operator><ROWID>5</ROWID></phrase></clause></where></xml>";
#$data= "<xml><header>gosa_query_jobdb</header><where><clause><phrase><headertag>ping</headertag></phrase></clause></where><limit><from>0</from><to>3</to></limit></xml>";
#$data= "<xml><header>gosa_query_jobdb</header><where><clause><phrase><headertag>ping</headertag></phrase></clause></where><limit><from>0</from><to>5</to></limit><orderby>timestamp</orderby></xml>";
#$data= "<xml><header>gosa_query_jobdb</header></xml>";

# count
#$data = "<xml> <header>gosa_count_jobdb</header></xml>";

# clear
#$data = "<xml> <header>gosa_clear_jobdb</header> </xml>";

# set gosa-si-client to 'activated'
#$data = "<xml> <header>gosa_set_activated_for_installation</header> <target>10.89.1.31:20083</target> <source>127.0.0.1:20081</source> </xml>";

#$data = "<xml> <header>gosa_detect_hardware</header> <target>10.89.1.31:20083</target> <source>10.89.1.31:20081</source> </xml>";
#$data = "<xml> <header>gosa_reboot</header> <target>10.89.1.31:20083</target> <source>10.89.1.31:20081</source> </xml>";
#$data = "<xml> <header>gosa_reinstall</header> <target>10.89.1.31:20083</target> <source>10.89.1.31:20081</source> </xml>";
#$data = "<xml> <header>gosa_softupdate</header> <target>10.89.1.31:20083</target> <source>10.89.1.31:20081</source> </xml>";
#$data = "<xml> <header>gosa_halt</header> <target>10.89.1.31:20083</target> <source>10.89.1.31:20081</source> </xml>";
#$data = "<xml> <header>gosa_new_key_for_client</header> <target>00:01:6c:9d:b9:fa</target> <source>10.89.1.31:20081</source> </xml>";
$data = "<xml> <header>gosa_new_key_for_client</header> <target>00:01:6c:9d:b9:fb</target> <source>10.89.1.31:20081</source> </xml>";


    $sock->write($data);
    $answer = "nothing";
	$answer = $sock->read();
    
    echo "$count: $answer\n";
	$sock->close();	
}else{
	echo "... FAILED!\n";
}
}
?>
