#!/usr/bin/php -q
<?php  /*  >php -q server.php  */
/*	File		:	AsterClick.php
**	Author		:	Dr. Clue
**	Description	:	Provides a connection to the Asterisk AMI (Asterisk Manager Interface )
**				event and control Interface.
**
**						szAMIusername	- asterisk etc/asterisk/manager.conf [username].
**						szAMIsecret	- asterisk etc/asterisk/manager.conf secret.
**						$iAMIport	- port to run the AMI event client server on
**
**	NOTES		:	The project has 4266 lines of code including comments as of August 1, 2010
**			
In /etc/asterisk/manager.conf you must make the following settings to receive events.

[szAMIusername]
secret=szAMIsecret
read = all,system,call,log,verbose,command,agent,user,config
write = all,system,call,log,verbose,command,agent,user,config

**	COMMANDS	:
**
    [Park		] => Park a channel (Priv: call,all)
    [ParkedCalls	] => List parked calls (Priv: <none>)
    [ShowDialPlan	] => List dialplan (Priv: config,reporting,all)
    [ModuleCheck	] => Check if module is loaded (Priv: system,all)
    [ModuleLoad		] => Module management (Priv: system,all)
    [CoreShowChannels	] => List currently active channels (Priv: system,reporting,all)
    [Reload		] => Send a reload event (Priv: system,config,all)
    [CoreStatus		] => Show PBX core status variables (Priv: system,reporting,all)
    [CoreSettings	] => Show PBX core settings (version etc) (Priv: system,reporting,all)
    [UserEvent		] => Send an arbitrary event (Priv: user,all)
    [UpdateConfig	] => Update basic configuration (Priv: config,all)
    [SendText		] => Send text message to channel (Priv: call,all)
    [ListCommands	] => List available manager commands (Priv: <none>)
    [MailboxCount	] => Check Mailbox Message Count (Priv: call,reporting,all)
    [MailboxStatus	] => Check Mailbox (Priv: call,reporting,all)
    [AbsoluteTimeout	] => Set Absolute Timeout (Priv: system,call,all)
    [ExtensionState	] => Check Extension Status (Priv: call,reporting,all)
    [Command		] => Execute Asterisk CLI Command (Priv: command,all)
    [Originate		] => Originate Call (Priv: originate,all)
    [Atxfer		] => Attended transfer (Priv: call,all)
    [Redirect		] => Redirect (transfer) a call (Priv: call,all)
    [ListCategories	] => List categories in configuration file (Priv: config,all)
    [CreateConfig	] => Creates an empty file in the configuration directory (Priv: config,all)
    [Status		] => Lists channel status (Priv: system,call,reporting,all)
    [GetConfigJSON	] => Retrieve configuration (JSON format) (Priv: system,config,all)
    [GetConfig		] => Retrieve configuration (Priv: system,config,all)
    [Getvar		] => Gets a Channel Variable (Priv: call,reporting,all)
    [Setvar		] => Set Channel Variable (Priv: call,all)
    [Ping		] => Keepalive command (Priv: <none>)
    [Hangup		] => Hangup Channel (Priv: system,call,all)
    [Challenge		] => Generate Challenge for MD5 Auth (Priv: <none>)
    [Login		] => Login Manager (Priv: <none>)
    [Logoff		] => Logoff Manager (Priv: <none>)
    [Events		] => Control Event Flow (Priv: <none>)
**
**
**
**
**
**	URLS		:
http://www.voip-info.org/wiki/view/Asterisk+manager+API
http://www.voip-info.org/wiki/view/asterisk+manager+events
**
*/

//***
//*** Configure environment
//***

/*
iconv_set_encoding("input_encoding"	, "ISO-8859-1");	//	Set the character encoding to "iso-8859-1"
iconv_set_encoding("internal_encoding"	, "ISO-8859-1");	//	Set the character encoding to "iso-8859-1"
iconv_set_encoding("output_encoding"	, "ISO-8859-1");	//	Set the character encoding to "iso-8859-1"
*/
/*
iconv_set_encoding("input_encoding"	, "UTF-8");	//	Set the character encoding to "UTF-8"
iconv_set_encoding("internal_encoding"	, "UTF-8");	//	Set the character encoding to "UTF-8"
iconv_set_encoding("output_encoding"	, "UTF-8");	//	Set the character encoding to "UTF-8"
*/
iconv_set_encoding("input_encoding"	, "US-ASCII");	//	Set the character encoding to "UTF-8"
iconv_set_encoding("internal_encoding"	, "US-ASCII");	//	Set the character encoding to "UTF-8"
iconv_set_encoding("output_encoding"	, "US-ASCII");	//	Set the character encoding to "UTF-8"

error_reporting(	E_ALL	);	// We like error messages , bring em on.
set_time_limit(		0	);	// We don't want a time limit.
ob_implicit_flush(		);	// Clear the output buffer.


//	Recording starting TTY state.
$bIsTTYconnection	=posix_isatty(	STDOUT)		;
$szTTYname		=posix_ttyname(	STDOUT)||NULL	;


//***
//*** Declare Global variables
//***
$szCRLF			=chr(13).chr(10)	;	// Carriage return + line feed 
$szCRLFterminator	= $szCRLF.$szCRLF	;	// Shorthand for \r\n\r\n
$szPacketTerminator	=$szCRLF		;	// Asterisk packet terminator
							// for incoming responses from
							// the Asterisk AMI interface.
							// After the intitail connection
							// string from the Asterisk AMI interface
							// this value is adjusted from
							// \r\n to \r\n\r\n

$szBufferIn		=""			;	// Gobal read buffer.
$aStackResults		=Array()		;	// Incoming results from AMI

$sock			=NULL			;	// This is the socket connection
							// to the Asterisk AMI interface.
$iSockReadCounter	=0			;	// Used in debugging activites
							// within AMIsocket_read()

$lStartTime		=mktime()		;	// These are debugging variables
$lEndTime		=$lStartTime		;	// used in performance testing
							// within the AMIaction() and 
							// processAMIresponse() functions.


$szAsterClickVersion	="0.0.0.0.D.R15"	;	// YY.MM.DD.([D]ev|[A]lpha|[B]eta|[P]rod).[R]elease# Where # is a sequential number.

$szArgv			=""			;	// Place to put argv[1] command
if(isset($argv[1]))$szArgv=$argv[1]		;	// If argv[1] exists record it.






require_once("Assets/PHP/include.dPrint.php");

//***
//*** Load configuration
//***
if(file_exists("./Assets/PHP/configure.php"))
	{
	require_once("./Assets/PHP/configure.php");
	if(isset($MySql_szUser)){require_once("./Assets/PHP/class.inc/class.mysql.inc");}
	}


//***
//*** Load supporting classes
//***
require_once("./Assets/PHP/include.socket.defines.php"		);	//	Standard socket constants
require_once("./Assets/PHP/include.debug.php"			);	//	Handy debugging/tracing tools

require_once("./Assets/PHP/inc/AsterClick_signal.inc"		);	//	Signal Handling

require_once("./Assets/PHP/class.inc/class.shm.inc"		);	//	Shared Memory class for named shared memory objects
require_once("./Assets/PHP/class.inc/class.msg_queues.inc"	);	//	Class for working with System V message ques.

require_once("./Assets/PHP/class.inc/class.ini.inc"		);	//	reads ini file values.
require_once("./Assets/PHP/class.inc/class.pid.inc"		);	//	process ID file manager class
require_once("./Assets/PHP/class.inc/class.fork.inc"		);	//	Process forking class
require_once("./Assets/PHP/class.inc/class.wSockets.inc"	);	//	Socket communications for HTML5.
require_once("./Assets/PHP/class.inc/class.phone.inc"		);	//	Phone/Channel member abstraction class.

require_once("./Assets/PHP/class.inc/class.cliParser.inc"	);	//	Attempts to parse CLI command results
									//	into meaningfull Arrays.

require_once("./Assets/PHP/inc/AsterClick_argv.inc"		);	//	Handles situtations related
									//	Commandline startup and related arguments.

require_once("./Assets/PHP/inc/AsterClick_socket.inc"		);	//	Functions handling socket interactions
									//	with Asterisk AMI.
//require_once("AsterClick_signal.inc"				);	//	Functions handling signal handlers

require_once("./Assets/PHP/caseEvent.php"			);	//	Used to vector AMI events by name as well
									//	as some pseudo AsterClick events.

$oINIagents	=	new CLASSini(Array("FILE"=>"../agents.conf"));

$szSavecallsin	=	$oINIagents->getIniValue("agents","savecallsin");
//print "\nsavecallsin=".$szSavecallsin;



$oSHM		= new shm()		;	// Gets set to an instance of the shm shared memory class
$oMSGqueue	= new msgQueue()	;	// System V message queue
$iSequence	=1			;	// Command sequence number


/*	Function	:	array2command()
**	Parameters	:	Array() $aParams	- Name/Value array
**	Returns		:	Constructed packet to send to Asterisk
**	Description	:	This function builds a string with a ":" between
**				each name and value, and and a CR+LF between each name/value pair.
*/
function array2command($aParams=Array())
	{
	global $szCRLF				;
	$szPacket		=""		;
	foreach($aParams as $key=>$value)$szPacket.=$key.": ".$value.$szCRLF;
	return $szPacket.$szCRLF;
	}
/*	Function	:	AMImessageRemove()
**	Parameters	:	None
**	Returns		:	None
**	Description	:	Scans for shared memory event messages that have
**				already been read and removes them. This function 
**				is called only by the AMIsocket_loop() function.
*/
function AMImessageRemove()
	{
	$oSHM		= new shm()	;
	$oSHM->bInUpdate=TRUE		;
	$aMessages	=$oSHM->aMessages;
	if(!is_array($aMessages)	){$oSHM->aMessages=Array();return;}
	if(count($aMessages)>0		)
		if($aMessages[0]["eventSequence"]<=$oSHM->iEventSequenceRead)
			{
			dPrint("\nAMImessageRemove ES(".$aMessages[0]["eventSequence"].") R(".$oSHM->iEventSequenceRead.")");
			array_shift($aMessages);
			$oSHM->aMessages=$aMessages;
			}
	usleep(200);
	$oSHM->bInUpdate=FALSE;
	}

/*	Function	:	ACMIaction()
**	Parameters	:	(String		)	$szAction	-
**				(Array		)	$aParams	-	[Array()]
**				(Boolean	)	$bStack		-	[TRUE	]
**	Returns		:	Constructed packet to send to Asterisk
*/
function ACMIaction($szAction,$aParams=Array(),$bStack=TRUE)
	{

	dPrint("*** ACMIaction ".print_r($aParams,TRUE),iCare_dPrint_AMIaction);

	}
/*	Function	:	AMIaction()
**	Parameters	:	(String		)	$szAction	-
**				(Array		)	$aParams	-	[Array()]
**				(Boolean	)	$bStack		-	[TRUE	]
**	Returns		:	Constructed packet to send to Asterisk
**	Description	:	This function builds via a call to array2command() a string with a ":" between
**				each name and value, and and a CR+LF between each name/value pair
**				Inserted into the array is the ACTION set to $szAction and
**				a ActionID set to timestamp.
**				The result is inserted into the global $oSHM->aStackCommands array
**				which in a FIFO fashion is dispatched to the Asterisk AMI
**				
*/
function AMIaction($szAction,$aParams=Array(),$bStack=TRUE)
	{
	global	$lStartTime			,
		$iSequence			;
	$oSHM			= new shm()	;
	$szORIGIN		="SYSTEM_AMIaction";//FastAMI"	;
	dPrint("AMIaction origin",iCare_dPrint_AMIaction);
	if(isset(			$aParams["ORIGIN"]))
		{
		$szORIGIN	=	$aParams["ORIGIN"];
		unset(			$aParams["ORIGIN"]);
		}
	$szParams	=(EMPTY($aParams)?"None":print_r($aParams,TRUE));
	dPrint("AMIaction (origin=$szORIGIN)\n action=$szAction params=".$szParams,iCare_dPrint_AMIaction);
//	dPrint("AMIaction (origin=$szORIGIN)\n action=$szAction params=".print_r($aParams,TRUE),iCare_dPrint_AMIaction);

	/*	There are conditions where AsterClick can be receiving requests
	**	from web clients while not yet being logged into asterisk AMI.
	**	This section of code stacks those requests in a buffer.
	**	within shared memory.
	*/
	if($oSHM->bAuthenticated===FALSE)
		{
		dPrint("AMIaction bAuthenticated",iCare_dPrint_AMIaction);
		if($szAction!="Login")
			{
			$aPendingLogin	=$oSHM->aPendingLogin;

			$aPendingLogin[]=Array(	"szAction"	=>$szAction	,
						"aParams"	=>$aParams	,
						"bStack"	=>$bStack	);
			dPrint("COMMAND PRIOR TO AUTHENTICATION ".$szAction." (".count($aPendingLogin).")" ,3);
			$oSHM->aPendingLogin=$aPendingLogin;
			return;
			}	//	End	if	Not login action
		}		//	End	if	Not authenticated.	


	$aAction	=Array(	"ACTION"	=>$szAction,
				"ActionID"	=>mktime()."_".($iSequence++)."_".$szORIGIN);


// "!" recording monitor commands.
	if(strpos($szAction		,"acmi_")!==FALSE	)
		{
		return ACMIaction($szAction,$aParams=Array(),$bStack=TRUE);
		}


	$aAction	=array_merge($aAction,$aParams);
	dPrint("AMIcommand aAction",iCare_dPrint_AMIaction);

	//Here we want to trap the getconfig command so that 
	//we can send a preamble event to the requesting client
	//so that their browser event code can have notice
	//of what file is about to arrive.
	if(isset($aAction["ACTION"])	===TRUE		)
	if($aAction["ACTION"]		=="getconfig"	)
		{
//		print "\nAMIaction() (getconfig) aAction=". print_r($aAction,TRUE);
		$aStackBreadCrumbs=$oSHM->aStackBreadCrumbs;
		$aStackBreadCrumbs[$aAction["ActionID"]]	=$aAction;
		if(count($aStackBreadCrumbs)>30)array_shift($aStackBreadCrumbs);
		$oSHM->aStackBreadCrumbs=$aStackBreadCrumbs;
		}

	if($bStack==TRUE)
		{
		dPrint("AMIcommand bStack"		,iCare_dPrint_AMIaction);
		$aStackCommands				=$oSHM->aStackCommands	;
		$aStackCommands[$aAction["ActionID"]]	=array2command($aAction);
		$oSHM->aStackCommands			=$aStackCommands 	;
		}

	dPrint("/AMIcommand",iCare_dPrint_AMIaction);
	$lStartTime		=mktime();
	return $oSHM->aStackCommands[$aAction["ActionID"]];
	}

/*	Function	:	handleAMI_ListCommands()
**	Parameters	:	(Array)	$aResponse
**	Returns		:	None
**	Description	:	The AMI command "Listcommands", tends to produce some majorly 
**				bulky output, so this handler breaks up that output into event sized pieces.
*/
function handleAMI_ListCommands($aResponse)
	{
	global $lEndTime;
	require_once("caseEvent.php");	
	$aListCommandsStart=Array(	"Response"	=>"Success",
					"Event"		=>"_ListCommandsStart",
					"ActionID"	=>$aResponse["ActionID"]);
	vectorEvent($aListCommandsStart);
	foreach($aResponse as $key=>$value)
		{
		if($key=="Response")continue;
		if($key=="ActionID")continue;
		vectorEvent(Array(	"Event"		=>"_ListCommandEntry"	,
					"ActionID"	=>$aResponse["ActionID"],
					"Command"	=>$key			,
					"Desc"		=>$value		));
		}
	$aListCommandsEnd=Array(	"Response"	=>"Success"		,
					"Event"		=>"_ListCommandsComplete",
					"ActionID"	=>$aResponse["ActionID"]);
		vectorEvent($aListCommandsEnd);
	$lEndTime=mktime();
	return;
	}
/*	Function	:	handleAMI_Command()
**	Parameters	:	$aResponse
**	Returns		:	None
**	Description	:	The AMI command "command" invokes an Asterisk CLI command.
**			This function handles the parsing of the CLI response from a text blob
**			into structured data and then chunking the results into event sized pieces that
**			can be passed through the System V messaging system. 
**
**			The reason we do this is that some CLI responses can be very bulky, and System V messaging
**			only affords so much space for messages at a time.
*/
function handleAMI_CLIresponse($aResponse)
	{
	global $lEndTime;
	require_once("caseEvent.php");	// This is the event dispatch code.

//print "\nhandleAMI_CLIresponse()=".print_r($aResponse,TRUE);

	// The CLIparserClass is defined in class.cliParser.inc
	// Here we declare an instance of the CLIparser class and pass it
	// the text of the CLI command resonse.

	$oCLIparse= new CLIparserClass(Array("szCLIResponse"=>$aResponse["CLIResponse"]));
	// dPrint("\nHANDLE CLI RESPONSE rows=(".count($oCLIparse->aResults["rows"]).") footers=(".count($oCLIparse->aResults["footers"]).")\n",9);

	// If the CLI response id but a single row, just pass the response straight on through.
	if(count($oCLIparse->aResults["rows"])==0)
		{
//dPrint($aResponse["CLIResponse"],9);
		$aResponse["Event"]="_CLIresponse"		;
//		print_r($aResponse);
		vectorEvent($aResponse);
		}else{
		$aResponseChunk=Array("ActionID"=>$aResponse["ActionID"],"Event"=>"_CLIList");
		vectorEvent($aResponseChunk);
		$aTrans = Array("/"=>"_FS_");
		foreach($oCLIparse->aResults["rows"] as $key => $value)
			{
			$aResponseChunk=Array("ActionID"=>$aResponse["ActionID"],"Event"=>"_CLIRow");
			foreach ($value as $colName=>$colValue)
				{
				$szColName			=strtr($colName,$aTrans);
				$aResponseChunk[$szColName]	=$colValue;
				}
//			dPrint("_CLIRow=".print_r($aResponseChunk,TRUE));
			vectorEvent($aResponseChunk);
			}
		}// end else  
	foreach($oCLIparse->aResults["footers"] as $key => $value)
		{
		if(strlen(trim($value))<1)continue;
		$aResponseChunk=Array("ActionID"=>$aResponse["ActionID"],"Event"=>"_CLIFooter");
		$aResponseChunk["footer_".$key]=$value;
		vectorEvent($aResponseChunk);
		}
	$aResponseChunk=Array("ActionID"=>$aResponse["ActionID"],"Event"=>"_CLIComplete");
	vectorEvent($aResponseChunk);
	dPrint("\n HANDLE CLI RESPONSE CHUNKING \n");
	$lEndTime=mktime();
	return;
	}// end function
/*	Function	:	result2array()
**	Parameters	:	(String		)	$szResultIn	-
**				(Array		)	&$aResultOut	-	[Array()]
**				(Boolean	)	$bFlush		-	[TRUE]
**	Returns		:	None
**	Description	:	Once the AMIsocket_read() function (defined in AsterClick_socket.inc)
**			has received enough data from Asterisk to
**			form a whole response, this function parses
**			that material into a name/value Array()
**			which AMIsocket_read then passes on to processAMIresponse()
**			which is also currently defined in AsterClick.php
*/
function result2array($szResultIn,&$aResultOut=Array(),$bFlush=TRUE)
	{
	$__iDebugLevel			=10				;
//	dPrint("result2Array() szResultIn=\n $szResultIn",iCare_dPrint_parse)	;
	if($bFlush==TRUE)$aResultOut	=Array()			;
	$szResultIn			=trim($szResultIn,"\r\n")	;
	$aResultIn			=explode("\n",$szResultIn)	;
	$bIsCLICommand			=FALSE				;
	$szCLICommand			=""				;
	dPrint("result2array() szResultIn=\n".$szResultIn,iCare_dPrint_parse);

	if($aResultIn[count($aResultIn)-1]=="--END COMMAND--")
		{
		dPrint("result2array() --END COMMAND--",iCare_dPrint_parse);
		$bIsCLICommand=TRUE;
		}

	$bEndOfHeaders		=	FALSE	;
//	$bEventQueues		=	FALSE	;
//	$szQUEUESResponse	=	""	;

	if(strpos($aResultIn[0],"holdtime"	)!==FALSE)
	if(strpos($aResultIn[0],"talktime"	)!==FALSE)
	if(strpos($aResultIn[0],"strategy"	)!==FALSE)
	if(strpos($aResultIn[0],"calls"		)!==FALSE)
			{
			$aResultOut=Array(	"Event"		=>"_AMIQueues"	,
						"Response"	=>"_AMIQueues"	,
						"QUEUESResponse"=>implode("\n",$aResultIn)	);
			global $szBufferIn;
			$szBufferIn=ltrim($szBufferIn);
			return $aResultOut;
			}
	foreach($aResultIn as $key => $value)
		{
		$iColon=strpos($value,":");if($iColon===FALSE)$iColon=-1;
		$iSpace=strpos($value," ");if($iSpace===FALSE)$iSpace=-1;
		if($iColon==-1||($iColon>$iSpace&&$iSpace>-1)||$bEndOfHeaders===TRUE)
			{
			if($bIsCLICommand===TRUE)
				if($value!="--END COMMAND--")
					$szCLICommand.=$value."\n";
			$bEndOfHeaders=TRUE;
			continue;
			}
		$szName			=	substr($value, 0, strpos($value, ':'));
		$szValue		=trim(	substr($value, strpos($value, ':') + 1));
		if(empty($szName))continue;
		$aResultOut[$szName]	= $szValue;
		}



	if($bIsCLICommand===TRUE)
		{
		$aResultOut["CLIResponse"]=$szCLICommand;
		}

	if(count($aResultOut)==0)
		{
		$aResultOut=Array(	"Response"	=>"Authenticate"				,
					"Message"	=>$szResultIn	);
		}
	return $aResultOut;
	}
/*	Function	:	processAMIresponse()
**	Parameters	:	$aResponse
**	Returns		:	None
**	Description	:	Currently all calls to this function arrive from 
**			the AMIsocket_read() function contained in the AsterClick_socket.inc file.
**			prior to this function being called, the AMIsocket_read()
**			function has both made sure there is an entire response from
**			Asterisk and that the response has been converted into a
**			name/value array by calling result2Array() which is also currently
**			defined in AsterClick.php
**
**			Since Asterisk AMI conveys many things as events and 
**			other items as message , and still other items in other formats,
**			this function aims to sorta normalize all the communications
**			responses into Event messages. All the pseudo event names
**			start with "_AMI" so as to make it easier to tell the natural
**			events from the forged events.
*/
function processAMIresponse($aResponse)
	{
	global	$szAMIusername		,
		$szAMIsecret		,
		$lStartTime		,
		$lEndTime		,
		$szPacketTerminator	,
		$szCRLF			;

//print_r($aResponse);

//	if(	isset($aResponse["Event"]))
////	if(strtolower($aResponse["Event"])=="configentry")
	//	{//	print "\nDetected (configentry) ".print_r($aResponse,true);//	}


	if(	isset($aResponse["Response"	])	)
		{
		switch($aResponse["Response"	])
			{
		case	"_AMIQueues"	:
	//		if(isset($aResponse["QUEUESResponse"]))
	//			{
//dPrint("QUEUES RESPONSE HERE!!!!!!!!!!!!!!!!".print_r($aResponse,TRUE),iCare_dPrint_always);
	//			vectorEvent($aResponse);
	//			return;
	//			}
	//		return;
			break;
		case	"Authenticate"	:// Remap the authentication request to look like an event.


//dPrint("Authenticate REQUEST",iCare_dPrint_always);

			$aResponse["Event"]="_AMIauthenticate"			;break;


		case	"Success"	:// Remap success	packets to pseudo events.





			// Provide special handling for the huge list
			// returned by the "listcommands" command.
			if(isset($aResponse["WaitEvent"		]))
			if(isset($aResponse["MeetmeList"	]))
			if(isset($aResponse["DAHDIRestart"	]))
				{
				handleAMI_ListCommands($aResponse);
				return;
				}



			if(	isset($aResponse["Message"	])	)
				{
				switch($aResponse["Message"	]		)
					{
				case	"Authentication accepted"://Remap to look like event
					$oSHM = new shm();
					$oSHM->bAuthenticated	=TRUE	;	// Flag indicating if we have been
										// Authenticated by AMI.

				// Get a copy of the AsterClick.conf user logins and pass it back
				// to code in class.wSockets.inc to reflect the list into that
				// process's thread.
				AMIaction("getconfig",Array(	"ORIGIN"	=>"SYSTEM_ASTERCLICK"	,
								"filename"	=>"AsterClick.conf"	)		);


					$aPendingLogin=$oSHM->aPendingLogin;
					dPrint("AMI AUTHENTICATION ACCEPTED (".count($aPendingLogin).")",3);



					for(;count($aPendingLogin);)
						{
						$value=array_shift($aPendingLogin);
						AMIaction($value["szAction"],$value["aParams"],$value["bStack"]);
						}
					$oSHM->aPendingLogin=Array();
					$aResponse["Event"]="_AMIauthenticateAccepted"	;
					break;
				case	"Meetme user list will follow":
					$aResponse["Event"]="MeetmeListStarts"	;
					break;



				default :
					$aResponse["Event"]="_AMIsuccess"		;
					break;
					}// /switch(Message)
				}else{


					// Reporting of events setting ("On","Off") .
					if(isset($aResponse["Events"]			)){$aResponse["Event"]="Events";break;}



//			print "\nAMIresponse (success ) now test for config stuff ".print_r($aResponse,TRUE);

					
					if(isset($aResponse["Category-000000"]		))
						if(isset($aResponse["Line-000000-000000"]))
							{
							$aResponse["Event"]="_AMIConfigFile";break;
							}else{
							$aResponse["Event"]="_AMIConfigFileCategories";break;
							}





	$aResponse["Event"	]="_AMIpayload";
	dPrint("\nprocessAMIresponse(".count($aResponse).")[".$aResponse["Response"	]."|||".$aResponse["Event"	]."]");
				}
			break;
		case	"Failure"	:// Remap Failure	packets to pseudo Event
			$aResponse["Event"]="_AMIfailure"			;break;
		case	"Error"		:// Remap error		packets to pseudo Event
			$aResponse["Event"]="_AMIerror"				;break;
		case	"Follows"	:// Remap follows	packets to pseudo Event
			$aResponse["Event"]="_AMIfollows"			;
			if(isset($aResponse["CLIResponse"]))
				{
				handleAMI_CLIresponse($aResponse);	// parses CLI text blob responses into structured data,
									// divides them up into nice sized chunks
									// and sends them to vectorEvent().
				return;
				}
			break;
		default			:
			dPrint("switch(Response) ERROR , unhanded response ("	.
				$aResponse["Response"	]			.")");
			break;
			}// /switch(Response)
		}else{
//		if(!isset($aResponse["Event"]))
//		dPrint("\nDAH , No Response ".print_r($aResponse,TRUE));
		}// end big if


	if(	isset($aResponse["Event"	])	)
		{
//		require_once("caseEvent.php");	
		vectorEvent($aResponse);
		}
	$lEndTime=mktime();
	}


AsterClick_argv_main($szArgv);


//Declare and clear global memory.
$oSHM			= new shm();
$oSHM->kill();
// Forks off the process oriniginally started from the command line
// so that the user or shell processes can continue on their way.
// The child portion of the fork calls the startAMIsocket_loop() function defined
// in AsterClick_socket.inc.

$oForkCommandLine	= new fork(Array(	"szfChild"	=>"startAMIsocket_loop"));
dPrint("*** AsterCLick		END Run",iCare_dPrint_always);
print "\n\n";
/******
*******
******* MAIN LEAD IN FROM THE COMMAND LINE
*******
*******
*******
******/
if($argc<2)// If no arguments were provided , show help and exit.
	{
	AsterClick_argv_help();
	exit();
	}
?>
