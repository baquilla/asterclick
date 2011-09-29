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
require_once("class.circularBuffer.inc"		);	//	A circular Ring buffer class.


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
var	$master		=	NULL		;	//	The orignal server socket. Each time this master
						 	//	socket shows up as readable in a socket select,
							//	we socket_accept() a new incoming socket connection.

var	$debug		=	FALSE		;	//	Used to enable/disable the console() display method.

static	$sockets	=	Array()		;	//	Array of all sockets accept()'ed from $master
//static	$users		=	Array()		;	//	Array of user Objects.
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

	if($oSHM->bWebSocketLoop==0)return dPrint(	"\n***wSocket LOOP	Loop Ends\n",iCare_dPrint_always);


	foreach($this->sockets as $key =>$value)	socket_set_nonblock($value);

/*****************************************
**					**
**	Main webSockets loop		**
**					**
*****************************************/
	dPrint(		"*** wSocket	LOOP	BEGINS",iCare_dPrint_always);

try	{

	while($oSHM->bWebSocketLoop	==1)
		{
		pcntl_signal_dispatch();
		$SKread		=$this->sockets			;//	Check all sockets for read
		$SKwrite	=User::getsocketsbypending()	;//	Find users socket with pending messages
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
		foreach(		$SKexcept	as	$socket		)
			{
			dPrint("\n*** wSockets socket_select() exception ",iCare_dPrint_always)	;
			}

/****************************************
**	Process Sockets needing reading	*
****************************************/
		foreach(		$SKread		as	$socket		)
			{
			if(		$socket		==	$this->master	)	//	If this is the server socket, it's readability
				{$this->connect(	$socket);continue;	};	//	indicates there is a new connection to Accept.

			usleep(1000);							//	Allow other processes breathing room.

			if((		$bufferLen	=@socket_recv(		$socket,$buffer,2048,0	))===FALSE)
				{
				switch((	$slError	= socket_last_error(	$socket			)))
					{
				case	SOCKET_EAGAIN		:	dPrint("\n*** wSockets socket_recv->switch->SOCKET_SOCKET_EAGAIN"	,iCare_dPrint_sockets)	;continue;
				case	SOCKET_ECONNRESET	:	dPrint("\n*** wSockets socket_recv->switch->SOCKET_ECONNRESET"		,iCare_dPrint_sockets)	;
									$this->disconnect($socket)						;continue;
				default				:	dPrint("\n*** wSockets socket_recv->switch->default=($slError)"		,iCare_dPrint_sockets)	;
									throw new SocketException("socket_recv",$socket)			;continue;
					};	//	End	Switch
				}

			if($bufferLen		==0					)
				{
//				print("\nBUFFER_LEN == 0 , perhaps it is closed");
				continue;
				}

			if(($user		= User::getuserbysocket($socket))	===NULL		)
				{
				dPrint("\n*** wSockets SKreading User::getuserbysocket() returned NULL disconnecting",iCare_dPrint_sockets)	;
				$this->disconnect();
				continue;
				}

			if(preg_match('#^GET (.*?) HTTP#',$buffer,$match)===1)
				{
				if($user->dohandshake(	$buffer)===FALSE)
					{
					dPrint(	"\n*** wSocket	PROTOCOL	HANDSHAKE	FAILS disconnecting"	,iCare_dPrint_sockets)	;
					$this->disconnect($socket);
					}	//	Initiate a handshake.
				dPrint(		"\n*** wSocket	PROTOCOL	OPEN"					,iCare_dPrint_sockets)	;
				continue;
				}

			if($user->wSocketState!==WebSocket_endpoint::OPEN)
				{
				dPrint(		"\n*** wSocket	PROTOCOL	(warning)	webSocket State (".$user->wSocketState.")",iCare_dPrint_sockets)	;
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
			if($socket							==	$this->master	)continue;
			if(($oUser	=	User::getuserbysocket(	$socket))	==NULL			){$this->disconnect($socket);continue;}	;
			if(		!$oUser->Qempty()							){$this->send(	$socket,$oUser->Qshift())	;	}
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

	dPrint(	sprintf	(	"\n*** wSocket	LOOP	ENDS")	,iCare_dPrint_sockets)	;	//	Announce disconnect of socket

//	$this->console("\n**** WebSockets Loop ENDS\n",0	);
	}// end function wSockets constructor 

/*	Function	:	checkHospice()
**	Parameters	:	None
**	Returns		:	None
**	Description	:	When a WebSocket connection enters the process of the closing handshake
**				it is placed in a hospice collection that is checked and processed periodically
**				to achieve an orderly shutdown of that connection.
*/
function checkHospice()
	{
	if(count(wSockets::$socketsHospice)==0)return;
	$iLimit=3;
	foreach(wSockets::$socketsHospice as $k=>$v)
		{
		if(	!is_resource($v)	)	{array_remove_object(wSockets::$socketsHospice ,$v);continue;}
		else					{$this->disconnect($v);}
		if(--$iLimit<0		)break;
		}
	array_push(wSockets::$socketsHospice,array_shift(wSockets::$socketsHospice));//print_r($a);
	usleep(1000);								//	Allow other processes breathing room.
	}

/*	Function	:	connect()
**	Parameters	:	(Socket		)	$socket
**	Returns		:	(Boolean	)	TRUE	=	Success
**							FAKSE	=	Error
**	Description	:	This bit creates a new entry in the user and socket
**			member arrays. the user is assigned an unique ID and the
**			socket is set to non-blocking.
**	NOTE		:	This does not perform the WebSocket handshake, but rather
**			just performs accept for the new connection and the bookkeeping for tracking the TCP
**			socket and creating a new user object.
**
**			During the next read cycle for this socket, the handshaking should occur.
*/
function connect($socket)
	{
	dPrint(			"\n*** wSocket	socket_accept() "		,iCare_dPrint_sockets)	;
	if(($socket		=	socket_accept($this->master))===FALSE	)				return	FALSE	;
	if(!is_resource(		$socket)				)
		{	dPrint(	"\n*** wSocket	socket_accept(BAD SOCKET) "	,iCare_dPrint_sockets)	;	return	FALSE ;
		}else	dPrint(	"\n*** wSocket	socket_accept(OK) "		,iCare_dPrint_sockets)	;


	$user				=	new User(Array("id"=>uniqid(),"socket"=>$socket));

	socket_set_nonblock(			$socket				);	//	Tends to be much faster.
	array_push($this->sockets	,	$socket				);	//	Add new socket to list.
	wSockets::$buffers[			$socket]	=Array("",""	);	//	Add new I/O buffer set to list. (0=read buffer / 1=write buffer)

	dPrint(			"\n*** wSocket	SOCKET		CONNECTED"	,iCare_dPrint_sockets)	;
	dPrint(			"\n*** wSocket	PROTOCOL	CONNECTING"	,iCare_dPrint_sockets)	;
	}// end function connect

/*	Function	:	disconnect()
**	Parameters	:	(Socket		)		$socket
**	Returns		:	None
**	Description	:	Disconnects a HTML5 socket client and removes
**			the related entry from the User::$users array and .
*/
function disconnect($socket)
	{
	if(!isset(		wSockets::$socketsHospice[$socket])	)	//	add socket to hospice if not present
	if(is_resource($socket))wSockets::$socketsHospice[]=$socket	;

	//	MAME,KILL,DESTROY!!!!
	//	If there is no user or the user's websocket state is CLOSE destroy session 
	if(is_null($user=User::getuserbysocket(	$socket))||$user->wSocketState==WebSocket_endpoint::CLOSED)
		{
		if($user!==NULL)
		array_remove_object(	User::$users			,$user			);	//	remove user from array.
		array_remove_object(	wSockets::$buffers		,$socket		);	//	remove socket buffers for ($socket).
		array_remove_object(	wSockets::$socketsHospice	,$socket		);	//	remove socket Hodpice for ($socket).

		if(is_resource($socket))
			{
			if(@socket_shutdown(	$socket, 1	))	// Sutdown writing	Remote host yet can read
				{
				usleep(			500		);//wait remote host
				@socket_shutdown(	$socket, 0	);//close reading
				}
			socket_set_block(					$socket			);	//	Set blocking on so that buffers are flushed and such.
			socket_close(						$socket			);	//	Close the socket
			}


		dPrint(	sprintf	(	"\n*** wSocket	DISCONNECTED socket(%s)",$socket)	,iCare_dPrint_sockets)	;	//	Announce disconnect of socket

//		$this->console(							$socket." DISCONNECTED!");



		array_remove_object(	$this->sockets			,$socket		);	//	Remove socket from socket array
		return	;	//	He's dead Jim....	If it's James T Kirk , why does the tombstome read "J.R.K."  ?
		}		//	Anyways , the socket and related references have been properly disposed of.



		$now	=	mktime();

		//	If the timeout start marker is not set , set it to the current time.
		if($user->closeHandshakeStarted		==0				)$user->closeHandshakeStarted	=$now			;

		//	If we are still showing an OPEN state , switch to a CLOSING state.
		if($user->wSocketState			==WebSocket_endpoint::OPEN	)$user->wSocketState		=WebSocket_endpoint::CLOSING	;

		//	If timeout for closing handshake has expired , proceed to closed state.
		if($now - $user->closeHandshakeStarted	>30				)
			{
			$user->wSocketState		=WebSocket_endpoint::CLOSED				;
			dPrint("\n*** wSockets	PROTOCOL	TIMEOUT	CLOSING",iCare_dPrint_sockets)		;	return		;
			}
		//	If both parties have transacted close frames , proceed to closed state
		if($user->closeHandshakeByte		==User::WSF_CLOSE_DISCONNECT	)
			{
			$user->wSocketState		=WebSocket_endpoint::CLOSED				;
			dPrint("\n*** wSockets	PROTOCOL	CLOSED",iCare_dPrint_sockets)			;	return		;
			}
		//	If we have not sent a close frame.
		if(!$user->closeHandshakeByte&User::WSF_CLOSE_SERVER			)
			{
			dPrint("\n*** wSockets	PROTOCOL	CLOSING frame sent",iCare_dPrint_sockets)	;
			$user->Qsend("AsterClick Closing Socket",WSF_close,1066)				;
			$user->closeHandshakeByte|=WSF_CLOSE_SERVER						;	return		;
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
	$oUser		=	User::getuserbysocket	($oUserSocket		);
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
	switch(	($iWrote	=@socket_send(		$oUserSocket		,
							$szWebSocketFrame	,
							$iFrameLen		,
							0			)))
		{
	case	FALSE:
		switch(	($slError=socket_last_error(	$oUserSocket	))	)
			{
		case	SOCKET_EAGAIN		:	dPrint(	sprintf	("\n*** wSocket socket_send() SOCKET_EAGAIN"	),iCare_dPrint_sockets)	;return	;
		case	SOCKET_ECONNRESET	:	dPrint(	sprintf	("\n*** wSocket socket_send() SOCKET_ECONNRESET"),iCare_dPrint_sockets)	;
							$oUser->closeHandshakeByte=User::WSF_CLOSE_DISCONNECT					;
							$this->disconnect($oUserSocket)								;break	;
		case	SOCKET_EPIPE		:	dPrint(	sprintf	("\n*** wSocket socket_send() SOCKET_EPIPE"	),iCare_dPrint_sockets)	;
							$oUser->closeHandshakeByte=User::WSF_CLOSE_DISCONNECT					;
							$this->disconnect($oUserSocket)								;break	;
		default				:	dPrint(	sprintf	("\n*** wSocket socket_send() error(%d : %s)"	,
									$slError,socket_strerror($slError)		),iCare_dPrint_sockets)	;
							throw new SocketException("socket_send",$oUserSocket)					;return	;
			}	//	End	Switch
		$iWrote=0;
		}		//	End	Switch


	dPrint(	sprintf	("\n*** wSocket		".'socket_write(%d of %d sent). %s %s'."\n"		,
		$iWrote												,
		$iFrameLen											,
		(($bIsResource	)?""	:"NO socket"	)						,
		(($bIsUser	)?""	:"NO user"	)						)	,iCare_dPrint_sockets)	;
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
	$iBufferConsume	=strlen($socketBufferIn);
	$iBufferLen	=strlen($socketBufferIn);

	dPrint(	sprintf	(			"*** wSocket PROTOCOL	UNWRAP(%d) bytes"	,
						$iBufferLen						)	,iCare_dPrint_sockets)	;

	$action		=$user->unwrap($aMsg)		;	//	WebSock protocol unwrap request packet.

	if(isset($aMsg["unwrap_exception"]))
		{
		switch($aMsg["unwrap_exception"])
			{
		case	UNWRAP_NEED_MORE_BYTES		:
			dPrint(	sprintf	(	"*** wSocket PROTOCOL	UNWRAP(UNWRAP_NEED_MORE_BYTES)"	)	,iCare_dPrint_sockets)	;
			return;//break;
		case	UNWRAP_PEER_CLOSE_FRAME		:
			dPrint(	sprintf	(	"*** wSocket PROTOCOL	UNWRAP(UNWRAP_PEER_CLOSE_FRAME)"	)	,iCare_dPrint_sockets)	;
			$user->closeHandshakeByte	|=User::WSF_CLOSE_CLIENT			;
			$user->Qclear();
			$user->Qsend("1000: Normal Closure : Goodbye. AsterClick acknowledges your close request.",WSF_close,1000);
//			$socketBufferIn			=substr($socketBufferIn,$iBufferConsume);
			$this->disconnect($user->socket);
//			return ;
			}
//		return			;
		}

	//	Remove that portion of the buffer consummed by the parsed frame.
	$socketBufferIn=substr($socketBufferIn,$iBufferConsume);

	//	REQUIRE	client Payload_Data to be in the form of an XML object or FAIL the connection to close.
	$iXMLdetect	=strpos($action,"<?")		;	//	Check for the XML lead in characters.
	if($iXMLdetect===FALSE)					//	If the content of the submitted content
		{						//	does not start with an "<?" we assume it is not XML, so bail.
		print "\n".__CLASS__."::".__FUNCTION__."() Non XML message ignored (".$action.")".print_r($aMsg,TRUE);
		$user->Qsend("1003: Unsupported Data : AsterClick encountered a Non XML Message.",WSF_close,1003);
		$this->disconnect($user->socket);
		return			;
		}

	if($user->wSocketState		!=WebSocket_endpoint::OPEN	)
		{
		print "\n".__CLASS__."::".__FUNCTION__."() User wSocketState is ({$user->wSocketState}) and no longer OPEN, XML command ('{$oNode->nodeName}') ignored.";
		$this->disconnect($user->socket);
		return ;
		}

	$oNodeRequest		=new node()			;	//	Parse the XML request string into a Node object.
	$oNodeRequest->parseXML($action)			;

	$szORIGIN		="HTML5_".$user->id		;	//	Create an origin string indicating the HTML5 source and user.

	$aParameters		=Array("ORIGIN"=>$szORIGIN	);	//	Create the parameters array for the call to AMIaction

	foreach($oNodeRequest->attributes as	$key=>$value	)	//	Fill the Parameters array with entries from 
			$aParameters[	$key]=$value	;	//	the request XML element's attrbites.

print "\n".__CLASS__."::".__FUNCTION__."() XML node name ='{$oNodeRequest->nodeName}' ";

	switch(strtolower($oNodeRequest->nodeName))			//	This is a hook for adding future custom processing
		{						//	of request nodes by means other than Asterisk itself.
	case	"asterclick"	:	$aParameters["AST_user"	]		=$user		;
					$aParameters["AST_xml"	]		=$oNodeRequest		;
					$this->ASTaction($aParameters		)		;		break;
	case	"asterplug"	:	dPrint("\nCUSTOM AsterPlug	Packet"	)		;		break;
	case	"events"	:	dPrint("\nCUSTOM AsterEvents	Packet"	)		;		break;
	case	"login"		:	$szUsername		=$oNodeRequest->getAttribute(	"username"	)	;
					$szSecret		=$oNodeRequest->getAttribute(	"secret"	)	;
					$user->bAuthenticated	=			FALSE			;

			if(isset(	$this->aLoginList[$szUsername]		)		&&
					$this->aLoginList[$szUsername]["secret"]==$szSecret	)	$user->bAuthenticated	= TRUE;

			$oNodeResult					=new node(Array("nodeName"	=>"Event"	));$oNodeResult->setAttribute("name","login");
			$oNodeResult->appendChild(			 new node(Array("nodeName"	=>"result	",
											"nodeValue"	=>(($user->bAuthenticated === TRUE)?"accepted":"rejected"))	));
			$user->QSend($oNodeResult->renderXML(),WSF_text)						;break;
	case	"logoff"	:		dPrint("\nCUSTOM AsterLogoff	Packet");

			$oNodeResult					=new node(Array("nodeName"	=>"Event"	));$oNodeResult->setAttribute("name","logout");
			$oNodeResult->appendChild(			 new node(Array("nodeName"	=>"result"	,
											"nodeValue"	=>"GoodBye"	)));

			$user->QSend($oNodeResult->renderXML(),WSF_close)						;break;
	case	"reload"	:	dPrint("\nCUSTOM AsterReload	Packet");break;
	default :
		if(		$user->bAuthenticated === FALSE)
				{
				$oNodeResult=new node(Array(	"nodeName"	=>	"Event"));
				$oNodeResult->setAttribute(	"name"		,	"login");
				$oNodeResult->appendChild(new node(Array("nodeName"=>"result","nodeValue"=>"required")));
				$user->QSend($oNodeResult->renderXML());
				
				}else{

				print "\n".__CLASS__."::".__FUNCTION__."() XML node name ='{$oNodeRequest->nodeName}'  being sent to AMIaction() with ";
				print "[".print_r($aParameters,TRUE)."]";
				AMIaction($oNodeRequest->nodeName,$aParameters		);
				}
		}// end switch
print "\n".__CLASS__."::".__FUNCTION__."() /".__FUNCTION__."() XML node name ='{$oNodeRequest->nodeName}' ";
	return;
	}// end function process

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
						foreach(User::$users as $user)$user->QSend($szMessage,WSF_text);
		break;
		}
	}
/*	Function	:	vectorSystemMessage()
**	Parameters	:	(Array		)	$aMessage
**	Returns		:	None
**	Description	:	Sorts out AMI results for internal AsterClick system requests
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

	dPrint(	sprintf	(			"*** wSocket AMI	EVENTS(%d)"	,
						$iMessageCount						)	,iCare_dPrint_sockets)	;

	if($iMessageCount	>$iMaxTries)		 //	If there are more messages than we want, limit 	
		$iMessageCount	=$iMaxTries		;//	the number of messages to read.
							
	for($iTries=$iMessageCount;$iTries>0;$iTries--)
		{
		$aMessage	=$oMSGqueue->msg_receive();
		if(!is_array($aMessage))
			{
			dPrint(	sprintf	(	"*** wSocket AMI	EVENT(EMPTY oMSGqueue)!"	)	,iCare_dPrint_sockets)	;
			continue;
			}
		$szUserID	=$aMessage["__userID"		];
		$szEvent	=$aMessage["Event"		];
		$oUser		=User::getuserbyid($szUserID	);

		$oNodeEvent		= new node(Array("nodeName"=>"event"));
		$oNodeEvent->setAttribute("name",strtolower($szEvent));

		dPrint(	sprintf	(		"*** wSocket AMI	EVENT(User(%s) Group(%s))"	,
						$aMessage["__userID"	]				,
						$aMessage["__groupID"	]				)	,iCare_dPrint_sockets)	;

		if($aMessage["__groupID"	]=="SYSTEM")
			{
			$this->vectorSystemMessage($aMessage);			continue;
			}
		//	Drop	internal associative keys
		unset($aMessage["__userID"	]);	unset($aMessage["__groupID"	]);	unset($aMessage["Event"		]);
		$oNodeEvent->loadFromArray($aMessage);


		if(	$oUser			!=NULL		&&
			$oUser-> bAuthenticated	!==TRUE		)
			{

//				$bAuthenticated

			
		dPrint(	sprintf	(		"*** wSocket	AMI	EVENT(User(%s) Not Authenticated)! user details...\n%s"	,
						$oUser->id							,
						print_r($oUser,TRUE))	,iCare_dPrint_sockets)	;
			$oNodeEvent->setAttribute("authentication","false");
			continue;
			}


		$szXML		=	$oNodeEvent->renderXML();

		if($oUser			!=NULL		)
			{

		dPrint(	sprintf	(		"*** wSocket	AMI	EVENT(send to %s)\n%s"	,
						$oUser->id					,
						$szXML						)	,iCare_dPrint_sockets)	;
			$oUser->QSend(		$szXML		);
			}else{

		dPrint(	sprintf	(		"*** wSocket	AMI	EVENT(send to %s)\n%s"	,
						"BROADCAST"					,
						$szXML						)	,iCare_dPrint_sockets)	;
			$this->vectorBroadcast(	$szXML,$oNodeEvent	);
			}
		}// end for;
	$lNowTime		=mktime()		;
	$lDifTime		=$lNowTime-$lStartTime	;
/*
	dPrint(	sprintf	(			"*** wSocket vectorQueued() Start=%d Now=%d timed=%d",
						$lStartTime								,
						$lNowTime								,
						$lDifTime							)	,iCare_dPrint_sockets)	;
*/
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

	dPrint("*** wSocket	socket_listen()	STARTED	".date('Y-m-d H:i:s')				,iCare_dPrint_always);
	dPrint("*** wSocket	socket_listen()	".sprintf('HOST	%s'."\t".' PORT %d',$address,$port)	,iCare_dPrint_always);
	dPrint("*** wSocket	socket_listen()	SOCKET	".$this->master					,iCare_dPrint_always);

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
		case "chatsend"		://User::$users
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
			foreach(User::$users as $key=>$uvalue)		$uvalue->Qsend($oXMLchat->renderXML());				
			break;
		default:
			dPrint("Child(".$value->nodeName.")");
			}// end switch
		$oUser->Qsend($oXMLout->renderXML());
		}// end
	}// End Func

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

	dPrint(	sprintf	(	"\n*** wSocket	BANNER\n%s",
				$this->server_banner()				)	,iCare_dPrint_sockets)	;

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
	dPrint(	sprintf	(	"\n*** wSocket	SERVER SETUP"	)	,iCare_dPrint_sockets)	;
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
	if	($pid	== -1)	{

	dPrint(	sprintf	(	"\n*** wSocket	SERVER	pcntl_fork() FAILS"	)	,iCare_dPrint_sockets)	;
	exit();


	}	//	fork failure
	elseif	($pid)		{								exit();}	//	close the parent
	else			{										//	child becomes our daemon
				dPrint(	sprintf	(	"\n*** wSocket	SERVER	Starting"	)		,iCare_dPrint_sockets)	;
				posix_setsid(							);		//	Make the current process a session leader
				chdir('/'							);	
				umask(0								);		//	ajust the PHP user mask
				$newpid			=posix_getpid()				;		//	Note the current process Id
				dPrint(	sprintf	(	"\n*** wSocket	SERVER	RUNNING PID(%d)",$newpid)	,iCare_dPrint_sockets)	;
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
		dPrint(	sprintf	(	"\n*** wSocket	SERVER	Ending run for %s ",$this->szPIDstart)	,iCare_dPrint_sockets)	;
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



	dPrint(	sprintf	(	"\n*** wSocket	SERVER	Stopped")	,iCare_dPrint_sockets)	;//	That's all folks!!
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
