<?php
// require_once("include.dPrint.php");

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
	private $aSec_WebSocket_Versions	= Array(
0	=>FALSE	,1	=>FALSE	,2	=>FALSE	,3	=>FALSE	,4	=>FALSE	,5	=>FALSE	,
6	=>FALSE	,7	=>TRUE	,8	=>TRUE	,9	=>FALSE	,10	=>FALSE	,11	=>FALSE	,
12	=>FALSE	,13	=>FALSE	,14	=>FALSE	,15	=>FALSE	);
	private	$szHandshakeResult		=""	;
	public $iResultCode			=500	;	//	Sever Error (Simply because if I don't set it , it would be )
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

public	function	checkVersion($szSec_WebSocket_Version)
	{
	if(array_key_exists(			$szSec_WebSocket_Version,	$this->aSec_WebSocket_Versions)		)
	if($this->aSec_WebSocket_Versions[	$szSec_WebSocket_Version]===TRUE	)return TRUE;




	$szSec_WebSocket_Versions_supperted=implode(',',$this->aSec_WebSocket_Versions);

$this->iResultCode		=400	;
$this->szHandshakeResult	=<<<EOL
HTTP/1.1 400 Bad Request
Sec-WebSocket-Version: $szSec_WebSocket_Versions_supperted
EOL;
	return FALSE;
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
		dPrint("\n*** wSocket PROTOCOL Handshake Request\n".$szBuffer,iCare_dPrint_webSockets_handshake);

	        $szResource		= $szHost			= $szUpgrade			= $szConnection			=
		$szSec_WebSocket_Key	= $szSec_WebSocket_Origin	= $szSec_WebSocket_Protocol	= $szSec_WebSocket_Version	="";

		$head_size		=strpos($szBuffer,"\r\n\r\n"			)	;
		$head_size		=($head_size===FALSE)?-666:($head_size+4	)	;


//		dPrint("\n*** wSocket PROTOCOL Handshake Request\n".$szBuffer,iCare_dPrint_webSockets_handshake);

		dPrint(sprintf( "\n*** ".'PROTOCOL Handshake headsize=(%d) buffersize=(%d) ',$head_size,strlen($szBuffer)),iCare_dPrint_webSockets_handshake);

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
	        preg_match("#Sec-WebSocket-Version: (.*?)\r\n#"	, $szBuffer, $match) && $szSec_WebSocket_Version	= $match[1];

		dPrint("\nWebSocket Version requested\n".$szSec_WebSocket_Version,iCare_dPrint_webSockets_handshake);


		if(!$this->checkVersion($szSec_WebSocket_Version))
			{
		dPrint("\nWebSocket Version REJECTED\n".$szSec_WebSocket_Version,iCare_dPrint_webSockets_handshake);
			return $this->szHandshakeResult;
			}else{
		dPrint("\nWebSocket Version ACCEPTED\n".$szSec_WebSocket_Version,iCare_dPrint_webSockets_handshake);
			}




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
$this->iResultCode =101;
		$this->szHandshakeResult =	"HTTP/1.1 101 Switching Protocols\r\n"				.
						"Upgrade: WebSocket\r\n"					.
						"Connection: Upgrade\r\n"					.
						"Sec-WebSocket-Accept: {$szSec_WebSocket_Accept}\r\n"		;//.
//						"Sec-WebSocket-Protocol: {$szSec_WebSocket_Protocol}\r\n\r\n"	;
		$this->szHandshakeResult.="\r\n";


		dPrint( "\nWebSocket Handshake_08 RESPONSE\n".$this->szHandshakeResult,iCare_dPrint_webSockets_handshake);

//dumpframe($this->szHandshakeResult);
$szBuffer=substr($szBuffer,$head_size);

		$this->iResultCode=101;
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
