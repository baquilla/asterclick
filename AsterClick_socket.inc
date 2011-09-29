<?php
/*	File		:	AsterClick_socket.inc
**	Author		:	Dr. Clue
**	Description	:	AsterClick Asterisk AMI functions
**			that interact with the Asterisk AMI sockets.
*/


require_once("include.socket.defines.php");



/*	Function	:	AMIsocket_read()
**	Parameters	:	None
**	Returns		:	None
**	Description	:	Checks for and collects bytes coming back from
**			requests to the Asterisk AMI interface. This function
**			checks for packet termination values to ensure it
**			has collected a full response from Asterisk over as many reads
**			as needed over time.
**
**			Once an entire packet has been recieved , same is passed off to
**			the result2Array() function which returns a name/value array
**			which in turn is passed onto the processAMIresponse() function.
**			Both of those functions as of this writeing are located here
**			in the FastAMI.php file.
*/
function AMIsocket_read()
	{
	global $sock,$szBufferIn,$szCRLF,$szCRLFterminator,$szPacketTerminator;
	global $iSockReadCounter;
	$iPacketTerminatorLen	=strlen($szPacketTerminator)	;
	$iPacketLen		=0				;
	$changed		=Array($sock); // An array of sockets to be examined (just one here)
	$selectResult		=@socket_select($changed,$write=NULL,$except=NULL,1,0);
	if($selectResult===FALSE)
		{
		dPrint("AMIsocket_read(FALSE)",iCare_dPrint_socketsAMI);
		}
//	while(0< socket_select($changed,$write=NULL,$except=NULL,2))
	$bHit	=FALSE;
	while(0< $selectResult)
		{
		$bHit	=TRUE	;
		if(count($changed)<1)break;
		dPrint("AMIsocket_read($selectResult)",iCare_dPrint_socketsAMI);
		foreach($changed as $socket)
			{
			$recvBufferIn	="";
			$iBytes		=@socket_recv($sock,$recvBufferIn,4096,0);
			if($iBytes>0)$szBufferIn.=$recvBufferIn;
			}// /foreach
		usleep(10);
		$changed	=Array($sock);
		$selectResult	=@socket_select($changed,$write=NULL,$except=NULL,0,10);
		if($selectResult===FALSE)
			{
			dPrint("AMIsocket_read(FALSE)",iCare_dPrint_socketsAMI);
			break;
			}
		}	//	End	while	socket selects readable
//	$szBufferIn=trim($szBufferIn);
	$iMaxLoop=20;
	while(!empty($szBufferIn)&&($iMaxLoop--)>0)//return;
		{
		$iPacketLen=0;
		if(($iPacketLen =strpos($szBufferIn,$szPacketTerminator))===FALSE)	return;
		$szPacket	=substr($szBufferIn,0,$iPacketLen+$iPacketTerminatorLen);
		$szBufferIn	=substr($szBufferIn,strlen($szPacket));
		$aResponse	=result2Array($szPacket);
		processAMIresponse($aResponse);
		$changed	= Array($sock);
		}// end buffer parse loop

	if($bHit===TRUE)
	dPrint("/AMIsocket_read()",iCare_dPrint_socketsAMI);

	}	//	End	Function	readAMIsocket()

/*	Function	:	AMIsocket_write()
**	Parameters	:	None
**	Returns		:	None
**	Description	:	Writes content to the socket connected to the
**				Asterisk AMI
*/
function AMIsocket_write()
	{
	global $sock;
	$oSHM		= new shm();
	if(count($oSHM->aStackCommands)<1) return;

	$selectResult	=socket_select($read=null,$write=Array($sock),$except=NULL,0,0);
	if($selectResult===FALSE)
		{
		dPrint("AMIsocket_write(FALSE)"				,iCare_dPrint_socketsAMI);
		return;
		}
	while(0< $selectResult )
		{
		dPrint("AMIsocket_write()"				,iCare_dPrint_socketsAMI);
		foreach($write as $socket)
			{
			$aStackCommands		=$oSHM->aStackCommands;
			$szSend			=array_shift($aStackCommands);
			$oSHM->aStackCommands	=$aStackCommands;

			dPrint("AMIsocket_write Sending... ".$szSend.")",iCare_dPrint_socketsAMI);
			socket_write($sock,$szSend,strlen($szSend));
			dPrint("/AMIsocket_write Sending... "		,iCare_dPrint_socketsAMI);
			}// /foreach
		dPrint("/AMIsocket_write"				,iCare_dPrint_socketsAMI);
		break;
		}// while
	}// End Function;

/*	Function	:	AMIsocket_check()
**	Parameters	:	None
**	Returns		:	None
**	Description	:	Checks Sockets for error
*/
function AMIsocket_check()
	{
	global $sock;
	$aErrors = Array($sock); // An array of sockets to be examined (just one here)
	if(count($aErrors)==0)return TRUE;

	$oResult =socket_select($read=NULL,$write=NULL,$aErrors,0,0);
	if($oResult===FALSE)
		{
		dPrint(__FILE__.":".__LINE__. "AMIsocket_check() FALSE socket_select ",iCare_dPrint_always);
		return;
		}
	if($oResult==0)return;

	while(0<$oResult )
		{
		if(count($aErrors)<1)break;
		foreach($aErrors as $socket)
			{
			$iError=	(($socket==NULL)		?
					socket_last_error()		:
					socket_last_error($socket)	);
			switch($iError)
				{
			case EADDRINUSE:break;
			default:
			dPrint(__FILE__.":".__LINE__.
				" Socket operation error ($iError) ".
				socket_strerror($iError).""		,
				iCare_dPrint_always			);

				}// end switch
			}// /foreach
		usleep(1);
		break;
		}// while
	}// End Function
/*	Function	:	AMIsocket_create()
**	Parameters	:	None
**	Returns		:	None
**	Description	:
*/
function AMIsocket_create()
	{
	global $iAMIport,$szAMIhost,$sock;

	dPrint("START AMISOCKETS",1);
	if(($sock=socket_create(AF_INET, SOCK_STREAM, SOL_TCP))===FALSE)
		{
		die(__FILE__.":".__LINE__."socket_create() failed\n");
		exit();
		}
	usleep(100000);
	if(socket_set_option(	$sock		,
				SOL_SOCKET	,
				SO_REUSEADDR	,
				1		)	===FALSE)
		{

		dPrint(sprintf(	"*** AMIsocket	socket_set_option()	FAILED(%s)!"	,
				socket_strerror(socket_last_error($sock))		),iCare_dPrint_always);
		die(__FILE__.":".__LINE__);

		}
	// socket_set_nonblock($sock) or die("socket_set_nonblock() failed");
//	dPrint(__FILE__.":".__LINE__," Connect Socket on port (".$iAMIport.")",1);
	usleep(200000);
	if(FALSE===@socket_connect($sock,$szAMIhost,$iAMIport))
		{
		dPrint(sprintf(	"*** AMIsocket	socket_connect(%s:%d)	FAILED(%s)!"	,
				$szAMIhost,$iAMIport					,
				socket_strerror(socket_last_error($sock))		),iCare_dPrint_always);


		exit();
		}else{
		dPrint(sprintf(	"*** AMIsocket	socket_connect(%s:%d)	OK."	,
				$szAMIhost,$iAMIport					),iCare_dPrint_always);
		}
	socket_set_nonblock($sock);
	}
/*	Function	:	AMIsocket_loop()
**	Parameters	:	None
**	Returns		:	None
**	Description	:	This function is called as a result of a forking
**			that occurs in the startAMIsocket_loop() function defined further below
**			in this file.
**
**			After initializing some shared memory variables and creating 
**			a PID text file , it enters into a perpetual loop of reading and
**			writing to the socket associate with the Asterisk AMI connection
**			until the PID file is removed as a result of a shell command
**			(FastAMI.php stop)
*/
function AMIsocket_loop()
	{
	$oSHM = new shm();
	$oSHM->bLoggedIn		=0			;
	$oSHM->aStackCommands		=Array()		;
	$oSHM->aStackBreadCrumbs	=Array()		;
	$oSHM->aAMIresponce		=Array("peers"=>Array());
	$oSHM->aPendingLogin		=Array()		;	// Stores AMI commands issued by HTML5
									// prior to AMI login for issuance
									// after we have reached a logged in state.
	$oSHM->bAuthenticated		=FALSE			;	// Flag indicating if we have been
									// authenticated by AMI yet
	// When events are generated and stuffed in the 
	// aMessages queue , the iEventSequence increments.
	// Later when the function vectorSHMmessages() in class.wSockets.inc
	// processes the event message, iEventSequenceRead is incremented.
	// This allows the code in FastAMI.php to cleanly remeove the messages.
	// The bInUpdate is used by both threads to avoid reading memory that is not 
	// there at the moment due to updates.
	$oSHM->aMessages		=Array()		;
	$oSHM->iEventSequence		=0			;
	$oSHM->iEventSequenceRead	=-1			;
	$oSHM->bInUpdate		=FALSE			;
	//Keep this web socket flag at the end of the initialization or
	//things are likely to get accessed before they are defined.
	$oSHM->bWebSocketLoop		=1			;
	// setup signal handlers
	AsterClick_signal_handler_setup();



	AMIsocket_create();



	$oPid= new pid();// Creates PID file to track process ID.
	//***
	//*** main AMI read/write loop
	//*** 

	dPrint(			"\n*** AMISocket	LOOP	BEGINS "		,iCare_dPrint_always)	;



	$oPid->setPIDfilename();


//	$oSHM->bWebSocketLoop		=1			;

//	while($oPid->isrunning())
	while($oSHM->bWebSocketLoop		==1&&$oPid->isrunning()==TRUE)
		{

		// Service Socket read/write
		AMIsocket_check();
		AMIsocket_read();	
		AMIsocket_check();
		AMIsocket_write();		usleep(1);

		}// /while TRUE
	$oSHM->bWebSocketLoop	=0;
	dPrint(			"\n*** AMISocket	LOOP	ENDS "		,iCare_dPrint_always)	;
//	echo "\nAMI Loop Ends\n";	
	}



/*	Function	:	wSocketLoop()
**	Parameters	:	None
**	Returns		:	None
**	Description	:	This function declares waits for the AMI server loop
**			to be initiated as indicated by that process creating a
**			PID text file and then declares an instance of the wSockets
**			class (defined in class.wSockets.inc) that handles communications
**			with HTML5 clients using WebSockets.
**			
**			This function is called as a result of a forking that occurs
**			in the startAMIsocket_loop() function further below.
*/
function wSocketLoop()
	{
	global $oSHM			;
	usleep(100000)			;
	dPrint("*** wSocket	LOOP	STARTING"	,iCare_dPrint_always);
	$oPid		=new pid();
	$iTriesStart	=50;
	while(	($iTriesStart--) >0				&&
		file_exists($oPid->getPIDfilename())===FALSE	&&
		$oSHM->bWebSocketLoop!=1			)
		{
		dPrint(sprintf("*** wSocket	LOOP	WAITING".'(%d) 	for AMIsocket',$iTriesStart),iCare_dPrint_always);
		usleep(100000);
		}
	$oWsockets = new wSockets();
	dPrint("*** wSocket		LOOP	ENDS"		,iCare_dPrint_always);
	}
/*	Function	:	startAMIsocket_loop()
**	Parameters	:	None
**	Returns		:	None
**	Description	:	Once the main program has forked away from the
**			shell prompt, we split via fork to start
**			two independent processes.
**
**			The parent of the fork calls the AMIsocket_loop() function
**			which provides the Asterisk facing socket loop for sending
**			and receiving the commands and responses associated 
**			with the Asterisk AMI interface.
**
**			The child of the fork calls the wSocketLoop function
**			which provides the HTML5 facing socket loop for
**			connecting to multiple HTML5 clients via HTML5 WebSockets.
**			That client facing server process handles the reception
**			of HTML5 command requests and the eventual return of results
**			of same as well as conveying relevant AsteriskAMI event data.
*/
function startAMIsocket_loop()
	{
	$oForkRun = new fork(Array(	"szfParent"	=>"AMIsocket_loop",
					"szfChild"	=>"wSocketLoop"));
	}



?>
