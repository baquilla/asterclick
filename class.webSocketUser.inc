<?php
/*	File		:	class.wSocketUsers.inc
**	Author		:	Dr. Clue
**	Description	:	
*/


require_once("class.circularBuffer.inc"		);	//	A circular Ring buffer class.

/**
***
***	CLASS	User
***
**/
class User
	{
	static	$users		=	Array()		;	//	Array of user Objects.



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
	array_push(User::$users	,$this			);	//	Add (this) new user to list.
	$this->szOutput_encoding			=iconv_get_encoding("output_encoding")	;
	foreach($aArgs as $key=>$value)$this->$key	=$value					;
	$this->Qclear()										;
	}



/*	Function	:	dohandshake()
**	Parameters	:	(User Class	)	$user
**				(String		)	$buffer
**	Returns		:	(Bool		)	Always TRUE
**	Description	:	This bit of stuff is supposed to take an ordinary
**				HTTP connection and upgrade it to a WebSocket connection.
**				
*/
function dohandshake(&$buffer)
	{
	$this->Qclear()						;	//	Clear the user's pending message queue
	$this->handshake	=false				;	//	Indicate the user needs to handshake
	$this->classHandshake	=new WebSocketHandshake_08()	;	//	Assign a WebSocket handshake class instance
	$this->classFraming	=new WebSocket_codec_frame()	;	//	Assign a WebSocket message framing instance.
//	$this->Qclear()									;	//	Clear outgoing user message queue.
	$upgrade		=$this->classHandshake->doHandShake($buffer		);	//	process user's webSocket handshake request
	socket_write(		 $this->socket,$upgrade,strlen($upgrade)		);	//	Send user the $upgrade response.

	if($this->classHandshake->iResultCode!=101)
		{
		return false;
		}
	$this->handshake	=true							;	//	Note the completetion of the handshake.
	$this->wSocketState	=WebSocket_endpoint::OPEN				;	//	Set connection state to OPEN
	return true;
	}// end function dohandshake




/*	Function	:	getuserbyid()
**	Parameters	:	(String		)	$id
**	Returns		:	
**	Description	:	
*/
static	function	getuserbyid($id)	{
	foreach(User::$users as $user)if($user->id		==$id		)return $user;	return NULL	;
	}

/*	Function	:	getuserbysocket()
**	Parameters	:	(Socket		)	$socket
**	Returns		:	(User		)	Returns the User associated with a socket or NULL;
**	Description	:	
*/
static	function	getuserbysocket($socket)
	{
	foreach(User::$users as $user)
		if($user->socket	==$socket	)
			return $user;	return NULL	;
	}

/*	Function	:	getsocketsbypending()
**	Parameters	:	None
**	Returns		:	Array() - An array of user classes <OR> NULL if there are none.
**	Description	: An array of user sockets for user class instances that have
**			pending return messages in their user queue.
**			This function is called by the loop
**			in the wSockets constructor when checking for output destined 
**			to a connected HTML5 client.
*/
static	function		getsocketsbypending()
	{
	$aFound		=Array();
	foreach(User::$users as	$user	)	if(!			$user->Qempty())	$aFound[]=&$user->socket;
	return (count($aFound)==0)?NULL:$aFound;
	}

/*	Function	:	getusersbymessages()
**	Parameters	:	None
**	Returns		:	Array() - An array of user classes.
**	Description	: An array of user class instances that have pending return messages
**			in their outgoing queues. 
*/
/*static	function		getusersbymessages()
	{
	$aFound		=Array();
	foreach(User::$users as $user	)	if(!$user->Qempty())	$aFound[]	=$user				;
	if(count(			$aFound)	==0		)return NULL	;
	return				$aFound						;
	}
*/


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
	case	WSF_close	:	$this->Qclear();
					$oOut	=new WebSocket_frame_close(Array("code"=>$code,"message"=>	$szMessage))	;
					$this->closeHandshakeByte|=User::WSF_CLOSE_SERVER					;break;
	default			:print "\nUser->QSend() unimplemented opcodce ($opcode)"					;break;
		}

	if($oOut===NULL)return ;

//	dumpFrame($oOut(),"QSend()");
	$this->aMessages[]=$oOut();
	}
}// end class user


?>
