<?php
/*	File		:	class.wSocketUsers.inc
**	Author		:	Dr. Clue
**	Description	:
*/

/**
***
***	CLASS	User
***
**/
class User
	{
  	var	$id			=-1				;	//	Session unique identifier.

	var	$socket			=NULL				;	//	This would be the socket a user connects with.
	var	$handshake		=FALSE				;	//	Has the user completed the WebSocket handshake.
	var	$wSocketState		=WebSocket_endpoint::CONNECTING	;

	const	WSF_CLOSE_NOT_STARTED	=	0;
	const	WSF_CLOSE_CLIENT	=	1;
	const	WSF_CLOSE_SERVER	=	2;
	const	WSF_CLOSE_DISCONNECT	=	3;

	var	$closeHandshakeByte	=0				;	//	0	=No close started
										//	bit 1	=Client close frame received
										//	bit 2	=Server close frame sent
	var	$closeHandshakeStarted	=0				;	//	Used to provide a close timeout.


	var	$classHandshake		=NULL				;	//	Class instance for WebSocket handshake versuib
	var	$classFraming		=NULL				;	//	Class instance for wrapping/unwrapping WebSocket messages

	var	$szOutput_encoding	='US-ASCII'			;	//	Output encoding standard to use.
	var	$aMessages		=Array()			;	//	Messages queued to send on the user's socket

	var	$TIMESTAMP_QUEUED	=-1				;	//	Initialize the message timestamp to way back.
	var	$attrs			=Array()			;	//	Custom attributes to associate with the user.
	var	$bAuthenticated		=FALSE				;	//	Has the user submitted a valid login 

/*	CONSTRUCTOR	:	User()	
**	Parameters	:	Array() - $aArgs
**	Returns		:	None
**	Description	:	
**				
*/
function	User($aArgs=Array())
	{
	$this->szOutput_encoding			=iconv_get_encoding("output_encoding")	;
	foreach($aArgs as $key=>$value)$this->$key	=$value					;
	$this->Qclear()										;
	}
/*	Function	:	wrap()
**	Parameters	:	(Array		)	$oParams
**					(Boolean	)	"FIN"		[TRUE	]	Is this the final frame in this message.
**					(Boolean	)	"RSV1"		[FALSE	]	Reserved for use with custom protocol extensions.
**					(Boolean	)	"RSV2"		[FALSE	]	Is this the final frame in this message.
**					(Boolean	)	"RSV3"		[FALSE	]	Is this the final frame in this message.
**					(Number		)	"opcode"	[0x01	]	Type of Data / Control frames
**
**							//	Data Frames		//	Control	Frames
**							Continuation	= 0x00		Close		= 0x08
**							Text		= 0x01		Ping		= 0x09
**							Binary		= 0x02		Pong		= 0x0A
**							Reserved3	= 0x03		ReservedB	= 0x0B		
**							Reserved4	= 0x04		ReservedC	= 0x0C
**							Reserved5	= 0x05		ReservedD	= 0x0D
**							Reserved6	= 0x06		ReservedE	= 0x0E
**							Reserved7	= 0x07		ReservedF	= 0x0F
**												
**					(Boolean	)	"MASK"		[FALSE	]	Employ a data mask to avoid mis-interpretation 
**												by other software intermediaries. It is only a WebSocket
**
**					(Buffer		)	Payload_Data	[NULL	]	Whatever data your sending.
**
**	Returns		:	(WebSocketFrame_08	)	The constructed frame.
**	Description	:	
**
*/
function	wrap(&$oParams)
	{
	if($this->classFraming==NULL			)return FALSE	;
	return $this->classFraming->wrap(		$oParams)		;
//	print "\nUser->wrap oParams=".print_r($oParams,TRUE);
//	return TRUE							;//$oParams[WebSocketFrame];
	}
/*	Function	:	unwrap()
**	Parameters	:	(Array		)	$oParams
**					(Boolean	)	"FIN"		[TRUE	]	Is this the final frame in this message.
**					(Boolean	)	"RSV1"		[FALSE	]	Reserved for use with custom protocol extensions.
**					(Boolean	)	"RSV2"		[FALSE	]	Is this the final frame in this message.
**					(Boolean	)	"RSV3"		[FALSE	]	Is this the final frame in this message.
**					(Number		)	"opcode"	[0x01	]	Type of Data / Control frames
**
**							//	Data Frames		//	Control	Frames
**							Continuation	= 0x00		Close		= 0x08
**							Text		= 0x01		Ping		= 0x09
**							Binary		= 0x02		Pong		= 0x0A
**							Reserved3	= 0x03		ReservedB	= 0x0B		
**							Reserved4	= 0x04		ReservedC	= 0x0C
**							Reserved5	= 0x05		ReservedD	= 0x0D
**							Reserved6	= 0x06		ReservedE	= 0x0E
**							Reserved7	= 0x07		ReservedF	= 0x0F
**												
**					(Boolean	)	"MASK"		[FALSE	]	Employ a data mask to avoid mis-interpretation 
**												by other software intermediaries. It is only a WebSocket
**
**					(Buffer		)	Payload_Data	[NULL	]	Whatever data your sending.
**
**	Returns		:	(WebSocketFrame_08	)	The constructed frame.
**	Description	:	
**
*/
function	unwrap(&$oParams)	{	return $this->classFraming->unwrap(	$oParams);	}
function	close($code,$reason)
	{

	}
/*	Function	:	Qclear()
**	Parameters	:	None
**	Returns		:	None
**	Description	:	Removes all messsages from the user's outgoing message queue.
*/
function	Qclear()	{	$this->aMessages	=Array();						}
/*	Function	:	Qempty()
**	Parameters	:	None
**	Returns		:	Boolean	
**	Description	:	Indicates if the user's outgoing message queue is empty.
*/
function	Qempty()	{	return (empty($this->aMessages)	||	count($this->aMessages)==0	);	}
/*	Function	:	Qshift()
**	Parameters	:	None
**	Returns		:	(String|NULL)	
**	Description	:	Pulls the next message to be sent from the queue and returns 
**			it. (or NULL if the queue is empty)
*/
function	Qshift()	{	return ($this->Qempty())?NULL:array_shift($this->aMessages);			}
/*	Function	:	Qunshift()
**	Parameters	:	(String		)	$szMessage
**	Returns		:	None
**	Description	:	Places a message at the front of the queue.
*/
function	Qunshift()	{	return ($this->Qempty())?NULL:array_shift($this->aMessages);			}
/*	Function	:	QSend()
**	Parameters	:	(String		)	$bytesMessage
**				(Integer	)	$opcode		Indicates the type of frame to send.
**				(Integer	)	$code		Used in WSF_close frames to indicate
**									the type of close reason.
**	Returns		:	None
**	Description	:	Places the message in the webSocket user's message queue
**				which the main loop will later send to the browser.
*/
function	QSend($szMessage="",$opcode=WSF_text,$code=1006)
	{
//trace_call();
	$this->TIMESTAMP_QUEUED	=mktime();
	$oOut=NULL;

	switch($this->wSocketState)
		{
	case	WebSocket_endpoint::OPEN	:				break				;
	case	WebSocket_endpoint::CLOSED	:				return				;
	case	WebSocket_endpoint::CLOSING	:if($opcode==WSF_close)break;	return				;
		}	//	End	Switch()


	switch($opcode)
		{
	case	WSF_text	:	$oOut	=new WebSocket_frame_text(					$szMessage)	;break;
	case	WSF_close	:	$oOut	=new WebSocket_frame_close(Array("code"=>$code,"message"=>	$szMessage))	;
					$user->closeHandshakeByte|=User::WSF_CLOSE_SERVER					;break;
	default			:print "\nUser->QSend() unimplemented opcodce ($opcode)"					;break;
		}

	if($oOut===NULL)return ;

//	dumpFrame($oOut(),"QSend()");
	$this->aMessages[]=$oOut();
	}
}// end class user


?>
