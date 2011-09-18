<?php
require_once("include.dPrint.php");

/*	File		:	class.webSocketHandshake.inc
**	Author		:	Dr. Clue
**	CLASS		:	WebSocketHandshake_00
**	Parameter	:	String $szBuffer	- HTTP request to decode and produce response for
**	Description	:	Constructs a response to a handshake request as defined in draft-ietf-hybi-thewebsocketprotocol-00
**	URLs		:	The Spec	http://www.ietf.org/id/draft-ietf-hybi-thewebsocketprotocol-00.txt
*/



class	WebSocketHandshake_00
	{
	private	$szHandshakeResult	="";
/*	Function	:	__construct()
**	Parameters	:	$szBuffer
**	Returns		:	None
**	Description	:	Parses the contents of $szBuffer , which contains the HTTP websocket client 
**				request for upgrading to a websocket connection. This function
**				after parsing the request , produces a suitable response and makes it available
**				via the __toString() method.
*/
	public function __construct()
		{

		}
	public function doHandshake($szBuffer)
		{
		dPrint("\nWebSocket Handshake Request\n".$szBuffer,iCare_dPrint_webSockets);

	        $szResource = $szHost = $szOrigin = $szKey1 = $szKey2 = $szProtocol = $eightByteCode	= "";//null;
	        preg_match('#GET (.*?) HTTP#'			, $szBuffer, $match) && $szResource	= $match[1];
	        preg_match("#Host: (.*?)\r\n#"			, $szBuffer, $match) && $szHost		= $match[1];
	        preg_match("#Origin: (.*?)\r\n#"		, $szBuffer, $match) && $szOrigin	= $match[1];
	        preg_match("#Sec-WebSocket-Key1: (.*?)\r\n#"	, $szBuffer, $match) && $szKey1		= $match[1];
	        preg_match("#Sec-WebSocket-Key2: (.*?)\r\n#"	, $szBuffer, $match) && $szKey2		= $match[1];
	        preg_match("#Sec-WebSocket-Protocol: (.*?)\r\n#", $szBuffer, $match) && $szProtocol	= $match[1];
	        preg_match("#\r\n(.*?)\$#"			, $szBuffer, $match) && $eightByteCode	= $match[1];
		$this->szHandshakeResult =	"HTTP/1.1 101 WebSocket Protocol Handshake\r\n".
"Connection: Upgrade\r\n".
"Upgrade: WebSocket\r\n".
"Sec-WebSocket-Location: ws://{$szHost}{$szResource}\r\n".
"Sec-WebSocket-Origin: {$szOrigin}\r\n".
($szProtocol ? "Sec-WebSocket-Protocol: {$szProtocol}\r\n\r\n" : "\r\n"). // blank line before payload
$this->createHandshakeAnswerKey($szKey1, $szKey2, $eightByteCode	);
dPrint( "\nWebSocket Handshake_00 RESPONSE\n".$this->szHandshakeResult,iCare_dPrint_webSockets);
return $this->szHandshakeResult;
/*
print <<<EOL
webSocketHandshake
==================
Resource	$szResource
Host		$szHost
Origin		$szOrigin
Key1		$szKey1
Key2		$szKey2
Protocol	$szProtocol

EOL;
*/
		}// end function __construct()

	public function getHandShakeResult()
		{
		$this->szHandshakeResult ;
		}

	public function __toString()	{	return $this->szHandshakeResult;	}
/*	Function	:	decodeKeyToInt32()
**	Parameters	:	$szKey
**	Returns		:	int32
**	Description	:	Collects the digits in the key into a number and divides by the number of spaces
*/
	private function decodeKeyToInt32($szKey)
		{
		$int32Result=	preg_match_all('#[0-9]#', $szKey, $aNumber	) &&		//1.)	Collect the digits in the key
				preg_match_all('# #'	, $szKey, $aSpace	) ?		//2.)	Collect the spaces in the key
				implode('', $aNumber[0]) / count($aSpace[0]	) :   ''  ;	//3.)	Consolidate the digits ($numbers) into an actual number and
												//	divide by the number of spaces found in the key.
		return $int32Result;
		}									
/*	Function	:	createHandshakeAnswerKey()
**	Parameters	:	$szKey1
**				$szKey2
**				$eightByteCode
**	Returns		:	String - 16 byte answerkey.
**	Description	:
*/
	private function createHandshakeAnswerKey($szKey1, $szKey2, $eightByteCode)
		{
		$szResult= md5(	pack('N', $this->decodeKeyToInt32($szKey1))		.
				pack('N', $this->decodeKeyToInt32($szKey2))		.
				$eightByteCode						,
				true						        );
		return $szResult;
		}
}// class

/*	Function	:	WebSocketHandshake
**	Parameters	:	$szBuffer
**	Returns		:	String - Hanshake response.
**	Description	:	Utility function to invoke instance of the WebSocketHandshake_00 class and 
**				process a handshake request into a result and return same.
*/
function WebSocketHandshake($szBuffer)
	{
	$szResult =(string)new WebSocketHandshake_00($szBuffer);
	return $szResult;
	}


?>


