#!/usr/bin/php5 -q
<?php

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

class Socket_Client
{
	private $host 	= "";
	private $port 	= "";
	private $timeout= "";
	private $errno	= "";
	private $errstr	= "";
	private $b_data_send = FALSE;
	private $handle	= NULL;
	private $bytes_read = 0;

	public function __construct($host, $port, $connect = TRUE,$timeout = 3){
		$this->host 	= $host;
		$this->port 	= $port;
		$this->timeout 	= $timeout;
		if($connect){
			$this->connect();
		}
	}

	public function connected()
	{
		return($this->handle == TRUE);
	}

	public function connect()
	{
		$this->handle = @fsockopen($this->host, $this->port, $this->errno, $this->errstr, $this->timeout);
		if(!$this->handle){
			$this->handle = NULL;
			echo $this->errstr;
		}else{
			$this->b_data_send = TRUE;
		}
	}

	public function write($data){
		return($this->send($data));
	}

	public function send($data)
	{
		if($this->handle){
			$data = trim($data);
			fputs($this->handle,$data."\n");
			$this->b_data_send = TRUE;
			return(TRUE);
		}else{
			return(FALSE);
		}
	}

	public function close()
	{
		if($this->handle){
			fclose($this->handle);
		}
	}
	
	private function _read()
	{
		$str = FALSE;
		if($this->handle){

			/* Check if there is something to read for us */
			$read = array("0"=>$this->handle);	
			$num = stream_select($read,$write=NULL,$accept=NULL,$this->timeout);
				
			/* Read data if necessary */
			while($num && $this->b_data_send){
				$str.= fread($this->handle, 1024000);
				$read = array("0"=>$this->handle);	
				$num = stream_select($read,$write=NULL,$accept=NULL,$this->timeout);
			}
			$this->bytes_read = strlen($str);
			$this->b_data_send = FALSE;
		}
		return($str);
	}

	public function read()
	{
		return($this->_read());
	}

	public function bytes_read()
	{
		return($this->bytes_read);
	}
}


?>
