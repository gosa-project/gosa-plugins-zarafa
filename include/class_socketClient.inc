<?php

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
	private $error = "";

	private $encrypt 	= FALSE;
	private $crypt_key 	= "";

	private $iv;

	public function __construct($host, $port, $connect = TRUE,$timeout = 3){
		$this->host 	= $host;
		$this->port 	= $port;
		$this->timeout 	= $timeout;
		if($connect){
			$this->connect();
		}
	}

	public function SetEncryptionKey($data)
	{
		if(!function_exists("mcrypt_get_iv_size")){
			$this->error = _("The mcrypt module was not found. Please install php5-mcrypt.") ;	
			return(FALSE);
		}else{
			$this->encrypt 	= TRUE;
			$this->crypt_key= $data; 
			$iv_size  = mcrypt_get_iv_size(MCRYPT_RIJNDAEL_256, MCRYPT_MODE_ECB);
			$this->iv = mcrypt_create_iv($iv_size, MCRYPT_RAND);
			return(TRUE);
		}
	}

	private function encrypt($data)
	{
		/* Encrypt data */
		if($this->encrypt){
			$data = mcrypt_encrypt (MCRYPT_RIJNDAEL_256, $this->crypt_key, $data, MCRYPT_MODE_ECB, $this->iv); 
		}
		return(base64_encode($data));
	}

	private function decrypt($data)
	{
		$data = base64_decode($data);	
		/* decrypt data */
		if($this->encrypt){
			$data = mcrypt_decrypt (MCRYPT_RIJNDAEL_256, $this->crypt_key, $data, MCRYPT_MODE_ECB, $this->iv);
			$data = rtrim($data,"\x00");
		}
		return($data);
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
			$this->error = $this->errstr;
		}else{
			$this->b_data_send = TRUE;
		}
	}

	public function get_error()
	{
		return($this->error);	
	}

	public function write($data){
		return($this->send($data));
	}

	public function send($data)
	{
		if($this->handle){
			$data = $this->encrypt($data);
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
			$num = @stream_select($read,$write=NULL,$accept=NULL,$this->timeout);
			$str = "";		
		
			/* Read data if necessary */
			while($num && $this->b_data_send){
				$str.= fread($this->handle, 1024000);
				$read = array("0"=>$this->handle);	
				$num = @stream_select($read,$write=NULL,$accept=NULL,$this->timeout);
			}
			$this->bytes_read = strlen($str);
			$this->b_data_send = FALSE;
			$str = $this->decrypt($str);
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
