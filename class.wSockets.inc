<?php
/*	File		:	class.wSockets.inc
**	Author		:	Dr. Clue
**	Description	:	Support for HTML5 web sockets.
**			This class not only supports the HTML5 webSockets , but also
**			coordinates with AsterClick commands and of course deals with 
**			sending and receiving commands with that part of the AsterClick
**			server that directly communicates with the Asterisk AMI interface.
**
**
**	URLs		:	http://tools.ietf.org/html/draft-ietf-hybi-thewebsocketprotocol-10#section-1.3
**				http://www.asciitable.com/
**	NOTES		:
**				Main number Capital of The United States of America : 202-225-3121
*/

/**
***
***	CLASS	wSockets
***
**/
require_once("class.exception.inc"		);	//	Custom exception classes
require_once("class.pid.inc"			);	//	Tracks the PID numer , and manages PID file.
require_once("class.webSocketHandshake_08.inc"	);	//	Handshake to upgrade from HTTP to WebSockets
require_once("class.webSocketFrames_08.inc"	);	//	The message framing protocol
require_once("class.webSocketUser.inc"		);	//	User class for each connected socket.
require_once("class.msg_queues.inc"		);	//	Class for working with System V message ques.
require_once("class.nodes.inc"			);	//	XML Node creation / manipulation.
require_once("AsterClick_signal.inc"		);	//	Signal Handling
require_once("include.socket.defines.php"	);	//	Standard socket constants

		AsterClick_signal_handler_setup();

		$oMSGqueue	= new msgQueue();	//	The (AsterClick/WebSockets) code runs as it's own process
							//	while the (AsterClick/AsteriskAMI) code runs as it's own process.
							//	The message queues are the means by which command,control, and data
							//	are passed arround in the various AsterClick processes.
/*	CLASS		:	wSockets()
**	Description	:	The wSockets class implements a non forking multi client
**			server supporting HTML5 WebSockets. 
*/
class wSockets
	{
var	$master	=	NULL			;	//	The orignal server socket. Each time this master
						 	//	socket shows up as readable in a socket select,
							//	we socket_accept() a new incoming socket connection.

var	$debug	=	FALSE			;	//	Used to enable/disable the console() display method.

static	$sockets	=	Array()		;	//	Array of all sockets accept()'ed from $master
static	$users		=	Array()		;	//	Array of user Objects.
static	$buffers	=	Array()		;	//	Array of socket io buffers array(array($read,$write),...) 
							//	$buffers[$socket][0=read buffer | 1=write buffer]
static	$socketsHospice	=	Array()		;	//	WebSockets in the CLOSING state are placed in
							//	in the $socketHospice array to process the 
							//	WebSockets closing handshake.


/*	CONSTRUCTOR	:	wSockets()
**	Parameters	:	None
**	Returns		:	None
*	Description	:	Initializes the class and enters into a perpetual
**			loop to service HTML5 WebSocket connections until
**			the shared memory varable bWebSocketLoop is assigned
**			a value other than 1. This occurs in FastAMI.php
**			at the conclusion of the AMIserverloop() function.
*/
function wSockets()
	{
	$oSHM		=new shm()						;	//	Shared memory is used for keeping
											//	track of configuration and state
											//	information common to all the 
											//	AsterClick processes.

	global			$szWebSocketHost,	$iWebSocketPort		;	//	Host and port to run the webSockets as.
	$this->wSockets_start(	$szWebSocketHost,	$iWebSocketPort)	;	//	These can be adjusted on AsterClickServer.conf

	if($oSHM->bWebSocketLoop==0)return dPrint(	"\nWebSockets Loop Ends\n",iCare_dPrint_always);


	foreach($this->sockets as $key =>$value)	socket_set_nonblock($value);

/*****************************************
**					**
**	Main webSockets loop		**
**					**
*****************************************/
	dPrint(		"WebSockets Loop Begins\n",iCare_dPrint_always);

try	{

	while($oSHM->bWebSocketLoop	==1)
		{
pcntl_signal_dispatch();
		$SKread		=$this->sockets			;//	Check all sockets for read
		$SKwrite	=$this->getsocketsbypending()	;//	Find users socket with pending messages
		$SKexcept	=NULL				;//	If there is something amis it will
								 //	be stored here.

		$SKconnect	=Array()			;//	Sockets needing protocol connection.

		usleep(1000)					;//	Give a little breathing room to other processes

		wSockets::vectorQueuedMessages();		;//	Dispatch messages from SYSV to user queues
		$this->checkHospice();


/************************************************************************
**									*
**	Select Sockets needing Reading, Writing or Exception handling	*
**									*
************************************************************************/
		if(	(gettype($SKread	)!="array"||count($SKread	)==0)	&&
			(gettype($SKwrite	)!="array"||count($SKwrite	)==0)	)

{

}else{


		if(FALSE===socket_select(	$SKread			,//	If there are no sockets in need of service, loop.
						$SKwrite		,
						$SKexcept		,
						0,0			))continue;
}

/****************************************
**	Process Socket Exceptions.	*
****************************************/
		if(			$SKexcept	!=	NULL		)
		foreach(		$SKexcept	as	$socket		)dPrint("\n*** wSockets socket_select() exception ",iCare_dPrint_always)	;
/****************************************
**	Process Sockets needing reading	*
****************************************/
		foreach(		$SKread		as	$socket		)
			{
			if(		$socket		==	$this->master	)			//	If this is the server socket
				{
				print "\nAccept"				;
				if(($socket		=	socket_accept($this->master))===FALSE	)	continue;
				if(!is_resource(		$socket)				)
					{
					print "\nAccept->is_resource FALSE"	;
					continue				;
					}
				print "\nAccept->connect";
				$this->connect(			$socket)	;

				};//	Grab a socket from it.

			usleep(1000);								//	Allow other processes breathing room.
//				if((		$bufferLen	=@socket_recv(		$socket,$buffer,2048,0	))==FALSE)
				if((		$bufferLen	=@socket_recv(		$socket,$buffer,2048,0	))===FALSE)
					{
					switch((	$slError	= socket_last_error(	$socket			)))
						{
					case	SOCKET_EAGAIN		:	print "\nSOCKET_EAGAIN"		;	continue;
					case	SOCKET_ECONNRESET	:	print "\nSOCKET_ECONNRESET"	;
										$this->disconnect($socket)	;	continue;
					default				:
						print("\nsocket_recv->switch->default=($slError)");

						throw new SocketException("socket_recv",$socket);			continue;
						};	//	End	Switch
					}
				if($bufferLen		==0					)
					{
//					print("\nBUFFER_LEN == 0 , perhaps it is closed");
					continue;
					}

				print("\nbufferLen=($bufferLen) type=".gettype($bufferLen));

			if(($user		= wSockets::getuserbysocket($socket))	===NULL		)
				{$buffer="";	print "\n**** user object not found for socket"		;	continue;}

			if(preg_match('#^GET (.*?) HTTP#',$buffer,$match)===1)
				{
				$user->Qclear()						;	//	Clear the user's pending message queue
				$user->handshake	=false				;	//	Indicate the user needs to handshake
				$user->classHandshake	=new WebSocketHandshake_08()	;	//	Assign a WebSocket handshake class instance
				$user->classFraming	=new WebSocket_codec_frame()	;	//	Assign a WebSocket message framing instance.
				$this->dohandshake(	$user,$buffer)			;	//	Initiate a handshake.
			//	$buffer=""						;
				continue						;
				}
			$this->process(			$user,$buffer);
			continue;
			}// foreach
/************************************************
**	Proccess Sockets needing a write	*
************************************************/
		usleep(1000);
		if(	$SKwrite==NULL		)continue;
		foreach($SKwrite as $socket	)
			{
			if($socket			==	$this->master	)continue;
			$oUser				=	wSockets::getuserbysocket(	$socket)			;
			if(		!$oUser->Qempty()					)$this->send(	$socket,$oUser->Qshift())	;
			else print "\nWritable socket with nothing to write";
			}
/************************************************
**	Proccess WebSockets in CLOSING state	*
************************************************/
		$this->checkHospice();
		}// while TRUE

	}catch(SocketException	$oException){	print "\nSocketException	in main wSocket Loop ".print_r($oException,TRUE);
	}catch(Exception	$oException){	print "\nException		in main wSocket Loop ".print_r($oException,TRUE);
	}	//	End	Try	Catch

	foreach($this->sockets as $k=>$v)$this->disconnect($v);


	$this->console("\n**** WebSockets Loop ENDS\n",0	);
	$this->console("End Loop wSockets\n"			);
	}// end function wSockets constructor 

/*	Function	:	checkHospice()
**	Parameters	:	None
**	Returns		:	None
**	Description	:
*/
function checkHospice()
	{
	if(count(wSockets::$socketsHospice)==0)return;
	$iLimit=3;
	foreach(wSockets::$socketsHospice as $k=>$v)
		{
		if(	!is_resource($v)	)	{$this->array_remove_object(wSockets::$socketsHospice ,$v);continue;}
		else					{$this->disconnect($v);}
		if(--$iLimit<0		)break;
		}
	array_push(wSockets::$socketsHospice,array_shift(wSockets::$socketsHospice));//print_r($a);
	usleep(1000);								//	Allow other processes breathing room.
	}
/*	Function	:	connect()
**	Parameters	:	(Socket		)	$socket
**	Returns		:	None
**	Description	:	This bit creates a new entry in the user and socket
**			member arrays. the user is assigned an unique ID and the
**			socket is set to non-blocking.
**	NOTE		:	This does not perform the WebSocket handshake, but rather
**			just performs the bookkeeping for tracking the TCP socket and 
**			creating a new user object.
**
**			During the next read cycle for this socket, the handshaking should occur.
*/
function connect($socket)
	{
	if(!is_resource(		$socket))return	;	//	If $socket is not a socket type resource bail.
	$user				=	new User(Array("id"=>uniqid(),"socket"=>$socket));
	$user->id			=	uniqid()	;	//	An unique id created when the user makes their TCP/IP socket connection
	$user->socket			=	$socket		;	//	The socket the user is connected on.
	socket_set_nonblock(		$socket)		;	//	Tends to be much faster.
	array_push(wSockets::$users	,$user			);	//	Add new user to list.
	array_push($this->sockets	,$socket		);	//	Add new socket to list.
	wSockets::$buffers[$socket]	=	Array("",""	);	//	Add new I/O buffer set to list. (0=read buffer / 1=write buffer)

	$this->console($socket." CONNECTED!"			);	//	Announce th connection
	}// end function connect
/*	Function	:	array_remove_object()
**	Parameters	:	(Array		)		&$aArray
**				(Object		)		&$oObject
**	Returns		:	None
**	Description	:	Removes the oObject from the array if present.
**
*/
function array_remove_object(&$aArray,&$oObject)
	{
	if(!is_null($oObject)&&($indexObject	= array_search(		$oObject,	$aArray	))	>=0	)
		array_splice(	$aArray,$indexObject,1);
	}
/*	Function	:	disconnect()
**	Parameters	:	(Socket		)		$socket
**	Returns		:	None
**	Description	:	Disconnects a HTML5 socket client and removes
**			the related entry from the wSockets::$users array and .
*/
function disconnect($socket)
	{
	if(!isset(		wSockets::$socketsHospice[$socket])	)	//	add socket to hospice if not present
	if(is_resource($socket))wSockets::$socketsHospice[]=$socket	;

	//	MAME,KILL,DESTROY!!!!
	//	If there is no user orthe user's websocket state is CLOSE destroy session 
	if(is_null($user=wSockets::getuserbysocket(	$socket))||$user->wSocketState==WebSocket_endpoint::CLOSED)
		{
		if($user!==NULL)
		$this->array_remove_object(	wSockets::$users		,$user			);	//	remove user from array.
		$this->array_remove_object(	wSockets::$buffers		,$socket		);	//	remove socket buffers for ($socket).
		$this->array_remove_object(	wSockets::$socketsHospice	,$socket		);	//	remove socket Hodpice for ($socket).

		if(is_resource($socket))
			{
			socket_set_block(					$socket			);	//	Set blocking on so that buffers are flushed and such.
			socket_close(						$socket			);	//	Close the socket
			}

		$this->console(							$socket." DISCONNECTED!");	//	Announce disconnect of socket
		$this->array_remove_object(	$this->sockets			,$socket		);	//	Remove socket from socket array
		return	;	//	He's dead Jim....	If it's James T Kirk , why does the tombstome read "J.R.K."  ?
		}		//	Anyways , the socket and related references have been properly disposed of.



		$now	=	mktime();

		//	If the timeout start marker is not set , set it to the current time.
		if($user->closeHandshakeStarted	==0				)$user->closeHandshakeStarted	=$now			;

		//	If we are still showing an OPEN state , switch to a CLOSING state.
		if($user->wSocketState		==WebSocket_endpoint::OPEN	)$user->wSocketState		=WebSocket_endpoint::CLOSING	;

		//	If timeout for closing handshake has expired , proceed to closed state.
		if($now - $user->closeHandshakeStarted >30)
			{
			$user->wSocketState		=WebSocket_endpoint::CLOSED	;
			print "\n".__CLASS__."::".__FUNCTION__."() AsterClick CLOSING timeout DISCONNECT.";
			return;
			}
		//	If both parties have transacted close frames , proceed to closed state
		if($user->closeHandshakeByte==User::WSF_CLOSE_DISCONNECT)
			{
			$user->wSocketState		=WebSocket_endpoint::CLOSED	;
			print "\n".__CLASS__."::".__FUNCTION__."() AsterClick DISCONNECT.";
			return;
			}
		//	If we have not sent a close frame.
		if(!$user->closeHandshakeByte&User::WSF_CLOSE_SERVER)
			{
			print "\n".__CLASS__."::".__FUNCTION__."() AsterClick sending CLOSE frame.";
			$user->Qsend("AsterClick Closing Socket",WSF_close,1066)	;
			$user->closeHandshakeByte|=WSF_CLOSE_SERVER			;
			}

	}	//	End	Function	disconnect()
/*	Function	:	send()
**	Parameters	:	(Socket		)	$oUserSocket
**				(String		)	$szWebSocketFrame
**	Returns		:	None
**	Description	:	Writes a WebSocket frame to the client socket.
**				
*/
function send($oUserSocket,$szWebSocketFrame)
	{
	$iFrameLen	=	strlen				($szWebSocketFrame	);
	$bIsResource	=	is_resource			($oUserSocket		);
	$oUser		=	wSockets::getuserbysocket	($oUserSocket		);
	$bIsUser	=	!is_null			($oUser			);
	$iWrote		=0;

//	printf	("\n".'Attempting Send with (%d) bytes of data.  socket ready=(%s) valid user =(%s)'."\n"	,
//		$iFrameLen											,
//		(($bIsResource	)?"Yes":"No")									,
//		(($bIsUser	)?"Yes":"No")									);

//return;			//	129	130
//$szWebSocketFrame=chr(129).chr(2)."hi";
//dumpFrame($szWebSocketFrame);


	if($bIsResource&&$bIsUser)
	switch(	($iWrote	=socket_send(		$oUserSocket		,
							$szWebSocketFrame	,
							$iFrameLen		,
							0			)))
		{
	case	FALSE:
		switch(	($slError=socket_last_error(	$oUserSocket	))	)
			{
		case	SOCKET_EAGAIN		:	print "\n".__CLASS__."::".__FUNCTION__."() socket_send() SOCKET_EAGAIN"		;	return;
		case	SOCKET_ECONNRESET	:	print "\nSOCKET_ECONNRESET"	;
							$this->disconnect($oUserSocket)	;	break;	//return;
		case	SOCKET_EPIPE		:	print "\nSOCKET_EPIPE"		;
			$oUser->closeHandshakeByte=User::WSF_CLOSE_DISCONNECT;
			$this->disconnect($oUserSocket);
			break;// return;
		default				:
			print	"\nsend()->socket_send->switch->default=($slError)"	;
			throw new SocketException("socket_send",$oUserSocket)		;		
			return								;
			}	//	End	Switch
		$iWrote=0;
		}		//	End	Switch

	printf	("\n".'Wrote (%d) bytes of (%d) bytes of data.  valid socket/user resource (%s)/(%s)'."\n"	,
		$iWrote												,
		$iFrameLen											,
		(($bIsResource	)?"Yes":"No")									,
		(($bIsUser	)?"Yes":"No")									);
	} // end function send
/*	Function	:	process()
**	Parameters	:	(User		)	$user		- Instance of User class
**				(String		)	$socketBufferIn	- Content arriving in socket buffer.
**	Returns		:	None
**	Description	:	This is where received WebSocket client packets are
**				decoded and processed.
**				
*/
function process($user,&$socketBufferIn)
	{
//	$action		=$this->unwrap(Array("WebSocket_frame"=>$msg))		;	//	WebSock protocol unwrap request packet.
	$aMsg		=Array("websocket_frame"=>&$socketBufferIn);
printf( "\nProcess byte1=(%d) byte2=(%d) ",ord($socketBufferIn[0]),ord($socketBufferIn[1]));
	$iConsume	=strlen($socketBufferIn);
	print "\nprocess() ".strlen($socketBufferIn)." bytes, sending to unwrap()";

	$action		=$user->unwrap($aMsg)		;	//	WebSock protocol unwrap request packet.

	if(isset($aMsg["unwrap_exception"]))
		{
		switch($aMsg["unwrap_exception"])
			{
		case	UNWRAP_NEED_MORE_BYTES		:print "\nprocess() UNWRAP_NEED_MORE_BYTES"	;return;//break;
		case	UNWRAP_PEER_CLOSE_FRAME		:print "\nprocess() UNWRAP_PEER_CLOSE_FRAME"	;

//			$user->wSocketState		=WebSocket_endpoint::CLOSING			;
			$user->closeHandshakeByte	|=User::WSF_CLOSE_CLIENT			;
			$user->Qsend("Goodbye",WSF_close,1006);
			$socketBufferIn			=substr($socketBufferIn,$iConsume);
			$this->disconnect($user->socket);
			return ;

			}
		return			;
		}

	$socketBufferIn=substr($socketBufferIn,$iConsume);

	$iXMLdetect	=strpos($action,"<?")		;	//	Check for the XML lead in characters.
	if($iXMLdetect===FALSE)					//	If the content of the submitted content
		{						//	does not start with an "<?" we assume it is not XML, so bail.
		print "\nNon XML message ignored (".$action.")".print_r($aMsg,TRUE);
		return			;
		}
	$oNode		=new node()			;	//	Parse the XML request string into a Node object.
	$oNode->parseXML($action)			;

	$szORIGIN	="HTML5_".$user->id		;	//	Create an origin string indicating the HTML5 source and user.

	$aParameters	=Array("ORIGIN"=>$szORIGIN	);	//	Create the parameters array for the call to AMIaction

	foreach($oNode->attributes as	$key=>$value	)	//	Fill the Parameters array with entries from 
			$aParameters[	$key]=$value	;	//	the request XML element's attrbites.



	switch(strtolower($oNode->nodeName))			//	This is a hook for adding future custom processing
		{						//	of request nodes by means other than Asterisk itself.
	case	"asterclick"	:	$aParameters["AST_user"	]=$user;
					$aParameters["AST_xml"	]=$oNode;
					$this->ASTaction($aParameters);
		break;
	case	"asterplug"	:	dPrint("\nCUSTOM AsterPlug	Packet");break;
	case	"events"	:	dPrint("\nCUSTOM AsterEvents	Packet");break;
	case	"login"		:
		$szUsername		=$oNode->getAttribute("username"	);
		$szSecret		=$oNode->getAttribute("secret"		);
		$user->bAuthenticated	= FALSE;


		if(isset($this->aLoginList[$szUsername])		)
		if($this->aLoginList[$szUsername]["secret"]==$szSecret	)
		$user->bAuthenticated	= TRUE;

		$oNode			=new node(Array("nodeName"=>"Event"));
		$oNode->setAttribute("name","login");

		if(		$user->bAuthenticated == TRUE)		$oNode->appendChild(new node(Array("nodeName"=>"result","nodeValue"=>"accepted")));
		else							$oNode->appendChild(new node(Array("nodeName"=>"result","nodeValue"=>"rejected")));

		$user->QSend($oNode->renderXML());
		break;
	case	"logoff"	:	dPrint("\nCUSTOM AsterLogoff	Packet");
		$user->bAuthenticated = FALSE;
		break;
	case	"reload"	:	dPrint("\nCUSTOM AsterReload	Packet");break;
	default :
/*if(		$user->bAuthenticated = FALSE)
		{
		$oNode=new node(Array("nodeName"=>"Event"));
		$oNode->setAttribute("name","login");
		$oNode->appendChild(new node(Array("nodeName"=>"result","nodeValue"=>"required")));
		$user->QSend($oNode->renderXML());
		}*/
		AMIaction($oNode->nodeName,$aParameters		);
		}// end switch
	return;
	}// end function process
/*	Function	:	dohandshake()
**	Parameters	:	(User Class	)	$user
**				(String		)	$buffer
**	Returns		:	(Bool		)	Always TRUE
**	Description	:	This bit of stuff is supposed to take an ordinary
**				HTTP connection and upgrade it to a WebSocket connection.
**				
*/
function dohandshake(&$user,&$buffer)
	{
	$user->Qclear()									;	//	Clear outgoing user message queue.
	$upgrade		=$user->classHandshake->doHandShake($buffer		);	//	process user's webSocket handshake request
	socket_write(		 $user->socket,$upgrade,strlen($upgrade)		);	//	Send user the $upgrade response.
	$user->handshake	=true							;	//	Note the completetion of the handshake.
	$user->wSocketState	=WebSocket_endpoint::OPEN				;	//	Set connection state to OPEN
	return true;
	}// end function dohandshake

/*	Function	:	getusersbymessages()
**	Parameters	:	None
**	Returns		:	Array() - An array of user classes.
**	Description	: An array of user class instances that have pending return messages
**			in their outgoing queues. 
*/
function getusersbymessages()
	{
	$aFound						=Array()			;
	foreach(wSockets::$users as $user	)
		if(!$user->Qempty())	$aFound[]	=$user				;
	if(count(			$aFound)	==0		)return NULL	;
	return				$aFound						;
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
function		getsocketsbypending()
	{
	$aFound		=Array();
	foreach(wSockets::$users as	$user)		if(!			$user->Qempty())	$aFound[]=&$user->socket;
	return (count($aFound)==0)?NULL:$aFound;
	}
/*	Function	:	getuserbysocket()
**	Parameters	:	(Socket		)	$socket
**	Returns		:	(User		)	Returns the User associated with a socket or NULL;
**	Description	:	
*/
static	function	getuserbysocket($socket){	foreach(wSockets::$users as $user)if($user->socket	==$socket	)return $user;	return NULL	;	}
/*	Function	:	getuserbyid()
**	Parameters	:	(String		)	$id
**	Returns		:	
**	Description	:	
*/
static	function	getuserbyid($id)	{	foreach(wSockets::$users as $user)if($user->id		==$id		)return $user;	return NULL	;	}
/*	Function	:	getheaders()
**	Parameters	:	$req
**	Returns		:	Array() [Request,Host,Origin]
**	Description	:	Parses the initial HTTP style request from
**			a WebSockets client to acquire the Request, Host and Origin
**			values , which are then returned in an array.
*/
function getheaders($req)
	{
	$r=$h=$o=null;
	if(preg_match("/GET (.*) HTTP/"   ,$req,$match)){ $r=$match[1]; }
	if(preg_match("/Host: (.*)\r\n/"  ,$req,$match)){ $h=$match[1]; }
	if(preg_match("/Origin: (.*)\r\n/",$req,$match)){ $o=$match[1]; }
	return array($r,$h,$o);
	}// end function getheaders


/*	Function	:	vectorBroadcast()
**	Parameters	:	String			$szMessage		
**				node Class		$oNode			= NULL		
**	Returns		:	None
**	Description	:	This function is used to broadcast an element
**			to each each connected client.
*/
function vectorBroadcast(	$szMessage		,
				$oNode			= NULL		)
	{
	if($oNode == NULL) $oNode=	new node(Array(	"nodeName"	=>"event"			,
							"attributes"	=>Array("name"=>"unnamed")	));
	switch(strtolower($oNode->getAttribute("name")))
		{
	case	"_amiauthenticate"		:
	case	"_amiauthenticateAccepted"	:dPrint("\nEvent broadcast for (".$oNode->getAttribute("name").") SKIPPED");break;
	default					:dPrint("\nEvent broadcast for (".$oNode->getAttribute("name").")");
//DCDCDC

//print "\n".__FUNCTION__."()  just pinging";

//dumpFrame($szMessage,__FUNCTION__."()");


						foreach(wSockets::$users as $user)$user->QSend($szMessage,WSF_text);
//						foreach(wSockets::$users as $user)$user->QSend("Hi boys and girls",WSF_text);



		break;
		}
	}
/*	Function	:	vectorSystemMessage()
**	Parameters	:	(Array		)	$aMessage
**	Returns		:	None
**	Description	:
*/
var $aLoginList		=Array();
var $aBuildLoginList	=Array();
function vectorSystemMessage($aMessage)
	{
	switch($aMessage["Event"])
		{
	case "LoginListStarts"		:	$this->aBuildLoginList=Array();			break;
	case "LoginListEntry"		:	$szExtension	=$aMessage["__Extension"];
						if(!isset(	$this->aBuildLoginList[$szExtension]))
								$this->aBuildLoginList[$szExtension]=Array();
						foreach($aMessage as $key=>$value)
						switch($key)
							{
						case	"Event"		:
						case	"__groupID"	:
						case	"__userID"	:
						case	"__groupID"	:
						case	"__Extension"	:continue;
						default			:$this->aBuildLoginList[$szExtension][$key]=$value;
							}	//	End	Switch
						break;
	case "LoginListComplete"	:	$this->aLoginList	=Array();
						foreach($this->aBuildLoginList as $key=>$value)	$this->aLoginList[$key]=$value;
						$aBuildLoginList=Array();			
						break;
		}	//	End	Switch
	}
/*	Function	:	vectorQueuedMessages()
**	Parameters	:	None
**	Returns		:	None
**	Description	:	This function is called from the HTML5 webSockets
**			main server loop in the wSockets() method/constructor
**			to distribute System V queued messages to clients 
*/
function vectorQueuedMessages()
	{
	global $oMSGqueue				;//	Global system V message queue
	$iMaxTries		=10			;//	Maximum number of messages to process in a single pass.
	$iMessageCount		=$oMSGqueue->msg_count();//	Number of active messages in the queue
	if($iMessageCount	<1)return		;//	If there are no messages , bail.
	$lStartTime		=mktime()		;//	Record time we started reading the queue
	$this->console("\nvectorQueuedMessages has ($iMessageCount) entries",9);
	if($iMessageCount	>$iMaxTries)		 //	If there are more messages than we want, limit 	
		$iMessageCount	=$iMaxTries		;//	the number of messages to read.
							
	for($iTries=$iMessageCount;$iTries>0;$iTries--)
		{
		$aMessage	=$oMSGqueue->msg_receive();
		if(!is_array($aMessage))	{	$this->console("\nvectorQueuedMessages EMPTY",9);	return;	}
		$szUserID	=$aMessage["__userID"		];
		$szEvent	=$aMessage["Event"		];
		$oUser		=$this->getuserbyid($szUserID	);

		$oNode		= new node(Array("nodeName"=>"event"));
		$oNode->setAttribute("name",strtolower($szEvent));

		if($aMessage["__groupID"	]=="SYSTEM")
			{
			$this->vectorSystemMessage($aMessage);	continue;
			}

		unset($aMessage["__userID"	]);
		unset($aMessage["__groupID"	]);
		unset($aMessage["Event"		]);

		$oNode->loadFromArray($aMessage);
		if(	$oUser			!=NULL		&&
			$oUser->bAuthenticated	!==TRUE		)
			{		$oNode->setAttribute("authentication","false");continue;}
		$szXML		=	$oNode->renderXML();

		if($oUser			!=NULL		)	$oUser->QSend($szXML);
		else							$this->vectorBroadcast($szXML,$oNode);
		}// end for;
	$lNowTime		=mktime()		;
	$lDifTime		=$lNowTime-$lStartTime	;
	$this->console("\nVectorQueuedMessages StartTime=".$lStartTime." Now=".$lNowTime." Diff=".$lDifTime,9);
	return;
	}// end function
/*	Function	:	getPIDfilename()
**	Parameters	:	None
**	Returns		:	String - The name of the PID file used to store the process ID.
**	Description	:	This is simply the name of the running PHP script with the ".php"
**				extension replaced with a ".pid" extension.
*/
function getPIDfilename()
	{
	global $argv		;
	$this->szPIDfile	=implode(".pid",explode(".php",$argv[0]));
	return $this->szPIDfile;
	}
/*	Function	:	isrunning()
**	Parameters	:	None
**	Returns		:	(PID || FALSE)	On success returns PID, otherwise returns FALSE
**	Description	:
*/
function isrunning()
	{
	global $argv									;
	$szPIDfile	=$this->getPIDfilename()					;
	$szPIDstart	=""								;

	if(file_exists($szPIDfile)===FALSE	)return	FALSE				;	//	If NO $szPIDFile FILE
	$szPIDstart	=file_get_contents($szPIDfile)					;	//	else Load PID information

	if(empty($szPIDstart	)		)return	FALSE				;	//	isrunning NO PID found 
	$system_result	=$this->shell_send_command("ps -A | grep $szPIDstart ")		;	//	Checking process list for $szPIDstart)

	if(empty($system_result	)		)return	FALSE				;	//	PID $szPIDstart not running

	$aSystem_result	=explode(" ",$system_result)					;	//	get the second field of the commands output
	$szPIDstart	=$aSystem_result[1]						;	//	which should be th process id.

	return $szPIDstart								;	//	PID found and running return same.
	}
/*	Function	:	wSockets_complain()
**	Parameters	:	(Socket		)	$oResource	=NULL
**				(String		)	$szCommand	=""
**	Returns		:	(String		)
**	Description	:
*/
function wSockets_complain($oResource=NULL,$szCommand="")
	{
	$iSLerror=	(($oResource==NULL		)	?
			socket_last_error(		)	:
			socket_last_error($oResource	)	);
	switch($iSLerror)
		{
		case EADDRINUSE	:return;
		default		:
		dPrint(	"Unvectored Socket Error (".$iSLerror.") "
			.socket_strerror($iSLerror)."\n"		,iCare_dPrint_webSockets);

		}// end switch
	return "Socket $szCommand failed ($iError) ". socket_strerror($iSLerror)."";
	}
/*	Function	:	wSockets_start()
**	Parameters	:	(String		)	$address
**				(Int		)	$port
**	Returns		:	(Socket		)	Returns the master listening socket
**	Description	:	
**
*/
function wSockets_start($address,$port) 
	{
	$oSHM				=new shm()				;	//	Interprocess shared memory object
	$this->master			=@socket_create(	AF_INET		,	//	We are using TCP as machines might not be local
								SOCK_STREAM	,	//	Selecting Reliable Sequenced communications
								SOL_TCP		)	//
					or die( $this->wSockets_complain(null,"socket_create()"));

	if(socket_set_option(		$this->master	,
					SOL_SOCKET	,
					SO_REUSEADDR	,
					1		)	===FALSE)
		{
		$this->wSockets_complain(null,"set_option()")	;
		$oSHM->bWebSocketLoop	=0			;	//	Setting the bWebSocketLoop to 0 will cause the main execution loop to exit
		return $this->master				;	//	This is the main listening socket for webSockets from which all connections
		}							//	Are accepted.


	if(@socket_bind(	$this->master		,
				$address		,
				$port			)===FALSE)
		{
		print <<<EOL

***			=================================================
***			==   CONFIGURATION ERROR - UNABLE TO PROCEED	=
***			=================================================
*** wSockets_start()	Unable to bind socket to port ($port). 
***			Please verify that this port is not currently
***			in use by another application, or specify another
***			port in AsterClickServer.conf 
***
EOL;
		$oSHM->bWebSocketLoop=0;
		$this->wSockets_complain(null,__FILE__.":".__LINE__." set_bind()");
		return $this->master;
		}

	socket_listen(		$this->master,20				)                                or die("socket_listen() failed");

	echo "Server Started : ".date('Y-m-d H:i:s')	."\n"				;
	echo "Master socket  : ".$this->master		."\n"				;
	echo "Listening on   : ".$address		." port ".$port."\n\n"		;

	$this->sockets = array($this->master);
	return $this->master;
	}// end function wSockets_start


/*	Function	:	ASTaction()
**	Parameters	:	$aParameters
**	Returns		:	None
**	Description	:
*/
function ASTaction($aParameters)
	{
	$oXML = $aParameters["AST_xml"];
	$oUser= $aParameters["AST_user"];
	dPrint("\nCUSTOM AsterClick Packet\n".$oXML->renderXML(),9);

	foreach($oXML->childNodes as $key=>$value)
		{
		$oXMLout = new node(Array("nodeName"=>"event","attributes"=>Array("name"=>"asterclick_".$value->nodeName)));
		$oXMLout->appendChild($value);
		switch(strtolower($value->nodeName))
			{
		case "setattribute"	:
			dPrint("\nSetAttribute ".$value->attributes["name"]."=".$value->attributes["value"]);
			$oUser->attrs[$value->attributes["name"]]=$value->attributes["value"];
			break;			
		case "getattribute"	:
			$value->setAttribute("value",$oUser->attrs[$value->attributes["name"]]);
			dPrint("\nGetAttribute ".$value->attributes["name"]);
			break;
		case "chatsend"		://wSockets::$users
			$szFromLabel	=$oUser->id;
			$szFrom		=$oUser->id;
			if(isset($oUser->attrs["Ext"	]))$szFrom	=$oUser->attrs["Ext"	];
			if(isset($oUser->attrs["Label"	]))$szFromLabel	=$oUser->attrs["Label"	];
			dPrint("\nchatSend ".$value->renderXML(),9);
	//		print_r($value);
	//		print "chatsend nodevalue = ".$value->nodeValue;
			$timeStamp		=mktime();
			$timeStampJavascript	=$timeStamp*1000;
			$oXMLchat=new node(Array(	"nodeName"	=>"event"					,
							"nodeValue"	=>$value->nodeValue,
							"attributes"	=>Array(	"name"=>"chatline"	,
											"from"=>$szFrom		,
											"label"=>$szFromLabel	,
											"timestamp"=> $timeStampJavascript,
											"date"=>date("m/d/y")	,
											"time"=>date("H:i:s") )	));
			
			dPrint("chatsendOut ".$oXMLchat->renderXML(),9);

			foreach(wSockets::$users as $key=>$uvalue)		$uvalue->Qsend($oXMLchat->renderXML());				
			break;
		default:
			dPrint("Child(".$value->nodeName.")");
			}// end switch
		$oUser->Qsend($oXMLout->renderXML());
		}// end
	}// End Func
/*	Function	:	say()
**	Parameters	:	(String		)	$msg=""
**	Returns		:	(None		)
**	Description	:	
*/
function	say($msg="")	{	dPrint($msg);	}

/*	Function	:	console()
**	Parameters	:	(String		)	$msg=""
**	Returns		:	
**	Description	:	
*/
function console($msg="")	{	if($this->debug){dPrint($msg);}		}
/*	Function	:	server_main()
**	Parameters	:	(Int		)	$argc
**				(Array		)	$argv
**	Returns		:	None
**	Description	:	Processes command-line or pseudo command line arguments
**			and starts,stops,restarts the daemon or provides help text.
*/
function server_main($argc,$argv)
	{
	$this->szPIDstart	=$this->isrunning()	;				//	Get the process id

	$bGiveHelp		=FALSE			;
	$bStart			=FALSE			;
	$bStop			=FALSE			;

	$szCommand		="help"			;				//	Default command

	if(	$argc<2								||
		in_array($argv[1],Array("-help","-h","---help"))===TRUE		)
		$argv[1]	="---help"		;

	switch($argv[1])
		{
	case	"start"		:
	case	"stop"		:
	case	"restart"	:	$szCommand		=$argv[1];
				if(	$szCommand		!="start"	)$bStop		=TRUE;
				if(	$szCommand		!="stop"	)$bStart	=TRUE;
				if(	$this->szPIDstart	!==FALSE	&&
					($bStart===TRUE&&$bStop===FALSE)	)
					{
					print "\n${argv[0]}  already running $this->szPIDstart \n";
					$bGiveHelp	=TRUE		;
					$bStart		=FALSE		;
					}
				break;
	case	"status"	:	exit(0);
	case	"-geninitd"	:	$this->geninitd();exit(0);
	case	"-h"		:case	"-help"		:case	"---help"	:	default			:
					$bGiveHelp=TRUE;
		}	//	End	Switch


	$this->console(		$this->server_banner())	;

	if($bGiveHelp	)	$this->server_help()	;
	if($bStop	)	$this->server_stop()	;
	if($bStart	)	$this->server_start()	;
	}	//	End	Method	server_main()
/*	Function	:	server_setup()
**	Parameters	:	None
**	Returns		:	None
**	Description	:	Called by server start just prior to any forking
**		to allow any global resources to be configured.
*/
function server_setup()
	{
	$this->console("::server_Setup ",9);
	}
/*	Function	:	server_start()
**	Parameters	:	None
**	Returns		:	None
**	Description	:	Starts the PHP daemon , sets the PID file , forks away from the
**		console starting thread , and kills the original console thread
*/
function server_start()
	{
	usleep(500);

	$this->szPIDstart	=$this->isrunning()		;	//	Get PID (Process Id) or FALSE;
	$this->server_setup()					;
	$pid		= pcntl_fork()				;
	if	($pid	== -1)	{	$this->console("\n::start_server fork failure!\n");	exit();}	//	fork failure
	elseif	($pid)		{								exit();}	//	close the parent
	else			{										//	child becomes our daemon
				$this->console("\n::server_start Starting..."			);
				posix_setsid(							);		//	Make the current process a session leader
				chdir('/'							);	
				umask(0								);		//	ajust the PHP user mask
				$newpid			=posix_getpid()				;		//	Note the current process Id
				$this->console("\n::server_start Running with pid $newpid \n"	);
				file_put_contents(	$this->szPIDfilepath,$newpid, LOCK_EX	);		//	Store the PID in our lock file.
				$this->server_loop(	$this->host, $this->port		);		//	Begin main WebSocket server loop
				return $newpid;
				};
	exit()							;
	}
/*	Function	:	server_stop()
**	Parameters	:	None
**	Returns		:	None
**	Description	:
*/
function server_stop()
	{
	if((	$this->szPIDstart	=	$this->isrunning())!==FALSE)			//	Get Current orocess Id or FALSE
		{
		$this->console("\n::server_stop Stopping Run For $this->szPIDstart ...");
		if(is_resource(	$this->sock)	&&	$this->sock)
				{
				fflush(			$this->sock)			;
				socket_set_block(	$this->sock)			;	//	Setting blocking before close helps tidy things up.
				$this->socket_close(	$this->sock)			;	//	Close the socket
				}
			dPrint( $this->shell_send_command("kill $this->szPIDstart"),20)	;	//	Kill ourselves
			sleep(2)							;
		}
	if(file_exists($this->szPIDfilepath))unlink(	$this->szPIDfilepath)		;	//	Delete the PID lock file.
	$this->console("\n::server_stop Stopped\n")					;	//	That's all folks!!
	}
/*	Function	:	server_banner()
**	Parameters	:	None
**	Returns		:	None
**	Description	:	Returns a banner string.
*/
function server_banner()
	{
	global $argv;
	return <<<EOT

${argv[0]} daemon program  
====================================================
EOT;
	}
}// end wSockets class.


?>
