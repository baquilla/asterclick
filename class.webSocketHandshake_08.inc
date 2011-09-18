<?php
require_once("include.dPrint.php");

/*	File		:	class.webSocketHandshake.inc
**	Author		:	Dr. Clue
**	CLASS		:	WebSocketHandshake_08
**	Parameter	:	String $szBuffer	- HTTP request to decode and produce response for
**	Description	:	Constructs a response to a handshake request as defined in draft-ietf-hybi-thewebsocketprotocol-00
**	URLs		:	The Spec	http://www.ietf.org/id/draft-ietf-hybi-thewebsocketprotocol-00.txt
**						http://tools.ietf.org/html/draft-ietf-hybi-thewebsocketprotocol-08
**						http://en.wikipedia.org/wiki/WebSocket
*/


class	WebSocketHandshake_08
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

/**
GET /chat HTTP/1.1
Host: server.example.com
Upgrade: websocket
Connection: Upgrade
Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==
Sec-WebSocket-Origin: http://example.com
Sec-WebSocket-Protocol: chat, superchat
Sec-WebSocket-Version: 8
**/
public function doHandshake(&$szBuffer)
		{
		dPrint("\nWebSocket Handshake Request\n".$szBuffer,iCare_dPrint_webSockets_handshake);

	        $szResource		= $szHost			= $szUpgrade			= $szConnection	=
		$szSec_WebSocket_Key	= $szSec_WebSocket_Origin	= $szSec_WebSocket_Protocol	= $szSec_WebSocket_Version="";

$head_size=strpos($szBuffer,"\r\n\r\n"			)	;
$head_size=($head_size===FALSE)?-666:($head_size+4	)	;
printf( "\n".'in handshake headsize=(%d) buffersize=(%d) ',$head_size,strlen($szBuffer));


		preg_match('#GET (.*?) HTTP#'			, $szBuffer, $match) && $szResource			= $match[1];
		preg_match("#Host: ([^\r\n]+)#"			, $szBuffer, $match) && $szHost				= $match[1];
		preg_match("#Upgrade: ([^\r\n]+)#"		, $szBuffer, $match) && $szUpgrade			= $match[1];
		preg_match("#Connection: ([^\r\n]+)#"		, $szBuffer, $match) && $szConnection			= $match[1];
		preg_match("#Sec-WebSocket-Key: ([^\r\n]+)#"	, $szBuffer, $match) && $szSec_WebSocket_Key		= $match[1];

//print "\nszResource = ".$szResource."\n";
//print "\nszHost = ".$szHost."\n";
//print "\nszConnection = ".$szConnection."\n";
//print print_r($match,TRUE);


	        preg_match("#Sec_WebSocket_Origin: (.*?)\r\n#"	, $szBuffer, $match) && $szSec_WebSocket_Origin		= $match[1];
	        preg_match("#Sec_WebSocket_Protocol: (.*?)\r\n#", $szBuffer, $match) && $szSec_WebSocket_Protocol	= $match[1];
	        preg_match("#Sec_WebSocket_Version: (.*?)\$#"	, $szBuffer, $match) && $szSec_WebSocket_Versoin	= $match[1];

		// This GUID is specifically related to version 08 of the webSocketthe 08 
		$GUID			= "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

//		dPrint("\nszSec_WebSocket_Key = ".$szSec_WebSocket_Key."\n",iCare_dPrint_webSockets);
		$ConcatKey		=$szSec_WebSocket_Key.$GUID;

//		dPrint("\nConcatKey = ".$ConcatKey."\n",iCare_dPrint_webSockets);
		$SHA1			=	sha1($ConcatKey,TRUE);

//		dPrint("\nSHA1 = ".bin2hex($SHA1)."\n",iCare_dPrint_webSockets);
		$szSec_WebSocket_Accept	=base64_encode($SHA1);


/*---------	The handshake from the server looks as follows:
        HTTP/1.1 101 Switching Protocols
        Upgrade: websocket
        Connection: Upgrade
        Sec-WebSocket-Accept: s3pPLMBiTxaQ9kYGzzhZRbK+xOo=
        Sec-WebSocket-Protocol: chat
*/

// Invalid UTF-8 sequence in header value

		$this->szHandshakeResult =	"HTTP/1.1 101 Switching Protocols\r\n"				.
						"Upgrade: WebSocket\r\n"					.
						"Connection: Upgrade\r\n"					.
						"Sec-WebSocket-Accept: {$szSec_WebSocket_Accept}\r\n"		;//.
//						"Sec-WebSocket-Protocol: {$szSec_WebSocket_Protocol}\r\n\r\n"	;
		$this->szHandshakeResult.="\r\n";


		dPrint( "\nWebSocket Handshake_08 RESPONSE\n".$this->szHandshakeResult,iCare_dPrint_webSockets_handshake);

//dumpframe($this->szHandshakeResult);

$szBuffer=substr($szBuffer,$head_size);


		return $this->szHandshakeResult;

		}// end function __construct()

	public function getHandShakeResult()
		{
		$this->szHandshakeResult ;
		}

	public function __toString()	{	return $this->szHandshakeResult;	}

}// class

/*	Function	:	WebSocketHandshake
**	Parameters	:	$szBuffer
**	Returns		:	String - Hanshake response.
**	Description	:	Utility function to invoke instance of the WebSocketHandshake_00 class and 
**				process a handshake request into a result and return same.
*/
function WebSocketHandshake($szBuffer)
	{
	$szResult =(string)new WebSocketHandshake_08($szBuffer);
	return $szResult;
	}


?>
