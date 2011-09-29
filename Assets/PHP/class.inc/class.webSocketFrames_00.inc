<?php
/*	File		:	webSocketFrames_00.inc
**	Author		:	Dr. Clue
**	Description	:	Well, the simplicity of earlier message wrappings has given way to
**			It is my hope to in separating the wrap/unwrap functionality to this separate
**			framing class allow a greater degree of flexibility in supporting past, current and future 
**			gyrations of the WebSocket protocol as it evolves.
**	URLS		:	http://tools.ietf.org/html/draft-ietf-hybi-thewebsocketprotocol-00#section-1.3
*/

class	WebSocketFrame_08
			{
var	$buffer		=""					;

	/*	Function	:	__construct()
	**	Parameters	:	
	**	Returns		:	
	**	Description	:	
	**
	*/
	function	__construct()
		{

		}//	End	__construct()
	/*	Function	:	wrap()
	**	Parameters	:	$msg=""
	**	Returns		:	
	**	Description	:	
	**
	*/
	function	wrap($msg="")
		{
		return chr(0).$msg.chr(255);
		}
	/*	Function	:	unwrap()
	**	Parameters	:	$msg=""
	**	Returns		:	
	**	Description	:	
	**
	*/
	function	unwrap($msg=chr(0)."".chr(255))
		{
		return substr($msg,1,strlen($msg)-2);
		}
	}//	End	Class	WebSocketFrame

?>
