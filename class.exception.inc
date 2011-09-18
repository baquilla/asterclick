<?php
/*	File		:	class.exception.inc
**	Author		:	Dr. Clue
**	Description	:
*/


if(method_exists(new Exception,"getPrevious"))
	{
	class	AsterClickBaseException extends Exception
		{
private		 $previous;
public		function __construct($message, $code = 0, Exception $previous = null)
			{
		        parent::__construct($message, $code);
			if (!is_null($previous))$this -> previous = $previous;
			}



		}	//	End	Class	AsterClickBaseException

	}else{

	class	AsterClickBaseException extends Exception
		{
private		 $previous;
public		function __construct($message, $code = 0, Exception $previous = null)
			{
		        parent::__construct($message, $code);
			if (!is_null($previous))$this -> previous = $previous;
			}
	
public		function getPrevious()	{        return $this -> previous;	}

		}	//	End	Class	AsterClickBaseException

	}


class SocketException extends AsterClickBaseException
	{
	//	Redefine the exception so message isn't optional
	public function __construct($message, $code = 0	, Exception	$previous = null)
		{
		$socket		=$code				;
		$code		=socket_last_error($socket)	;
		$message	.=((strlen($message)>0)?" - ":" ").socket_strerror($code)." - ";
		parent::__construct($message, $code	,		$previous);	// make sure everything is assigned properly
		}


	public function __toString()	// custom string representation of object
		{
		return __CLASS__ . ": [{$this->code}]: {$this->message}\n";
		}

	public function customFunction()
		{
	        echo "A custom function for socket_exception type exceptions\n";
		}
	}	//	End	class	socket_exception




?>
