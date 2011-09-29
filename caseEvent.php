<?php
/*	File		:	caseEvent.php
**	Author		:	Dr. Clue
**	Description	:	Development code for processing Asterisk AMI events.
**			AFAIK there are 62 Asterisk events and each and everyone of them
**			is vectored here in a giant switch statement, although 
**			the actuall processing will probably evolve a great deal
**
**			currently all calls to the vectorEvent() function arrive
**			from the processAMIresponse($aResponse) function
**			in the FastAMI.php file.
**	URLS		:
**	http://www.voip-info.org/wiki/view/asterisk+manager+events
*/




$aASTERCLICKusers	=Array();
$aASTERCLICKlogins	=Array();
/*	Function	:	processAsterClickResponse()
**	Parameters	:	(Array		)	$aResponse	-
**	Returns		:
**	Description	:
*/
function	processAsterClickResponse($aResponse)
	{
	if($aResponse["__userID"]=="AMIaction"	)return;
	if($aResponse["__userID"]!="ASTERCLICK"	)return;

	global $aASTERCLICKusers,$aASTERCLICKlogins;
	global $oMSGqueue;	// System V message queue class.

	switch($aResponse["Event"])
		{
		case "ConfigListStarts":
			if($aResponse["filename"]!="AsterClick.conf")
				{
				print "\n".__FILE__.":".__LINE__." Not our File";
				}
			$aASTERCLICKusers=Array()		;

			$aMessage= Array();
			$aMessage["Event"	]="LoginListStarts"	;
			$aMessage["__groupID"	]="SYSTEM"		;
			$aMessage["__userID"	]="ALL"			;
			$oMSGqueue->msg_send(Array("message"=>$aMessage));
			break;
		case "ConfigEntry":
			if(!isset($GLOBALS["aASTERCLICKusers"])){break;}

//			print "\nConfigListEntry";//.print_r($aResponse,TRUE);

			$szSectionName	=$aResponse["name"];
			$aTarget	=($aASTERCLICKusers[$szSectionName]=Array());

			foreach($aResponse as $key=>$value)
				{
				if(strpos(	$key	,"Line"	)!==	0)continue;
				$aNamVal=explode("=",$value,2);
				if(count(	$aNamVal	)<	2)continue;
				$aASTERCLICKusers[$szSectionName][$aNamVal[0]]=$aNamVal[1];
				}


			break;
		case "ConfigListComplete"			:
			if(!isset($GLOBALS["aASTERCLICKusers"])){break;}

			foreach($aASTERCLICKusers as $key=>$value)
				{
				$aASTERCLICKlogins[$key]	=$aASTERCLICKusers[$key];
				$aMessage			=Array();
				$aMessage["Event"	]	="LoginListEntry"		;
				$aMessage["__groupID"	]	="SYSTEM"		;
				$aMessage["__userID"	]	="ALL"			;
				$aMessage["__Extension"	]	=$key			;
			
				foreach($aASTERCLICKusers[$key] as $nam =>$val)
					$aMessage[$nam]=$val;	

				$oMSGqueue->msg_send(Array("message"=>$aMessage));
				}

			$aMessage= Array();
			$aMessage["Event"	]="LoginListComplete"	;
			$aMessage["__groupID"	]="SYSTEM"		;
			$aMessage["__userID"	]="ALL"			;
			$oMSGqueue->msg_send(Array("message"=>$aMessage));


//			print "\nConfigListComplete \n".print_r($aMessage,TRUE);
			break;

		default:
			print "\n SAW GROUP ASTERCLICK \n".print_r($aResponse,TRUE)."\n";
		}//end switch
	}
/*	Function	:	vectorEventPrint()
**	Parameters	:	(String)	$szMessage	The text message to print.
**				(Bool)		$bPrint		Indicates if the message should actually be printed.
**	Returns		:	None
**	Description	:	This is simply a diagnostic/debugging function used in the vectorEvent() funtion further
**				down below. It allows individual events to be flagged and a programmer to 
**				send diagnosic messages to the console.
*/
function vectorEventPrint($szMessage,$bPrint)
	{
	if($bPrint)dPrint($szMessage,iCare_dPrint_AMIevent);
	}
/*	Function	:	vectorEventSend()
**	Parameters	:	$aResponse
**	Returns		:	None
**	Description	:	This function prepares and then sends packets via System V messaging.
**				Those messages will be retrieved by the HTML5 wSockets processor in
**				class.wSockets.inc
**
**				This function makes sure there is an "ActionID" and if not 
**				fashions a broadcast "ActionID" so that the un targeted packet
**				will be sent to all browser connections once retrieved.
**
**				The ActionID is a "_" delimited string consisting of 
**				requestTime_sequence_groupID_userID
**
**				requestTime	= Either the time of the request or if absent the current time.
**				sequence	= An incremental value assigned to each request.
**				groupID		= A broadcast group to which individual clients may subscribe
**				userID		= Identifies an individual browser connection.
**
**				This function also includes some throttling code that imposes a usleep()
**				for a period that increases depending upon the number of outstanding messages
**				in the system V message queue.
*/
function vectorEventSend($aResponse)
	{
	global $oMSGqueue;	// System V message queue class.
	if(!isset(	$aResponse["ActionID"]))
			$aResponse["ActionID"]	=mktime()."_0_ALL_ALL";
	$aActionID		=explode("_",$aResponse["ActionID"]);
	for($x=0;$x<4;$x++)if(!isset($aActionID[$x]))$aActionID[$x]="";
		$aActionIDmap=Array(	"timestamp"	=>$aActionID[0]		,
					"sequence"	=>$aActionID[1]		,
					"groupID"	=>$aActionID[2]		,
					"userID"	=>$aActionID[3]		,
					"response"	=>$aResponse		);
	$aResponse["__userID"	]=$aActionID[3];
	$aResponse["__groupID"	]=$aActionID[2];

	foreach($aResponse as $key =>$value)
		{
		$aResponse[$key]=trim($value);
		}

	switch($aResponse["__groupID"	])
		{
	case 	"SYSTEM"	:
		processAsterClickResponse($aResponse);
		dPrint("SYSTEM EVENT:".print_r($aResponse,TRUE),iCare_dPrint_SYSTEMevent);
		break;
	case	"FastAMI"	:
	case	"ALL"		:
//	print "\n SAW GROUP ALL"	;
	case	"HTML5"		:	
//	print "\n SAW GROUP HTML5"	;
		$oMSGqueue->msg_send(Array("message"=>$aResponse));
		break;
	default			:
		print "\n SAW UNKNOWN GROUP ".$aResponse["__groupID"	]	;break;
		}// End Switch
	$aStatus	=$oMSGqueue->msg_stat_queue()	;
	$iQueued	=$aStatus["msg_qnum"]		;
	$iPauseUnit	=95				;
	$iPause		=$iQueued*$iPauseUnit		;
	dPrint("\n QUE LENGTH(".$iQueued.")\n".($iQueued*$iPauseUnit)."\n");
	usleep($iPause);
	}// END FUNCTION vectorEventSend($aResponse)
/*	Function	:	vectorEvent($aResponse)
**	Parameters	:	(Array) $aResponse
**	Returns		:	None
**	Description	:	Here we chack the type of event in the $aResponse parameter 
**				and verify with a switch statement that we indeed handle it.
**
**				The individual case: statements can also make last minute changes 
**				to the event, discard it , or take other actions.
**
**				once the event has been processed here , it in most cases will
**				be sent to the vectorEventSend() function above to be tossed
**				over the wall via System V messaging to the HTML5 WebSockets
**				code to be sent to the appropriate browser connection(s).
*/
function vectorEvent($aResponse)
	{
	global $oMSGqueue;
	$oSHM		= new shm();
	$bMustPrint	=FALSE;
	if(!isset($aResponse["Event"	])	)$aResponse["Event"	]="NonEvent";

	$szResponseText	=$aResponse["Event"	];
		switch($aResponse["Event"	])
			{
		// RTCP/RTP events have an extremely high frequency and so are suppressed here.
		case	"RTPReceiverStat"	:return;
		case	"RTCPReceived"		:return;
		case	"RTCPSent"		:return;
		case	"hold"			:break;
		case	"Shutdown"		:
//	$oSHM = new shm();
			$oSHM->bLoggedIn		=0			;
			print "Asterisk SHUTDOWN DETECTED";
			global	$szPacketTerminator	,
				$szCRLF			,
				$szBufferIn		,
				$szAMIusername		,// set in /etc/asterisk/AsterClick/AsterClickServer.conf
				$szAMIsecret		;// set in /etc/asterisk/AsterClick/AsterClickServer.conf
			$szBufferIn="";
			$szPacketTerminator=$szCRLF.$szCRLF;
			usleep(5000);
			AMIsocket_create();
			AMIaction("Login",Array(	"Username"	=>$szAMIusername,
							"Secret"	=>$szAMIsecret));
			break;
		case	"_AMIQueues"			://dPrint("VECTOREVENT _AMIQueues",iCare_dPrint_always);
			break;
		// AUTHENTICATE
		// There really is NO SUCH EVENT, but because I like transactions to travel
		// in an orderly fashion , the processAMIresponse() function in AsterClick.php
		// remaps the message to look like an event so that it will arrive here.
		case "_AMIauthenticate"		:
			global	$szPacketTerminator		,
				$szCRLF				,
				$szAMIusername			,// set /etc/asterisk/AsterClick/AsterClickServer.conf([ami]username)
				$szAMIsecret			;// set /etc/asterisk/AsterClick/AsterClickServer.conf([ami]secret)
			$szPacketTerminator=$szCRLF.$szCRLF	;
			AMIaction("Login",Array(	"Username"	=>$szAMIusername,
							"Secret"	=>$szAMIsecret));
			break;
		case "_AMIauthenticateAccepted"	:
			$oSHM->bLoggedIn=1;
//			AMIaction("EVENTS",Array("EVENTMASK"	=>"system,call,log,verbose,command,agent,user"	));
//			AMIaction("EVENTS",Array("EVENTMASK"	=>"all,system,call,log,verbose,command,agent,user,config,dtmf,reporting"));
			AMIaction("EVENTS",Array("EVENTMASK"	=>"all,system,call,log,verbose,command,agent,user,config,dtmf"));
			break;
		case "_AMIsuccess"			:break;
		case "_AMIConfigFile"			:
		case "_AMIConfigFileCategories"		:
			$szACTION_ID = $aResponse["ActionID"];unset($aResponse["ActionID"]);
			unset($aResponse["Response"	]);
			unset($aResponse["Event"	]);

			//	CHECK for breadcrumbs
			$aStackBreadCrumbs=$oSHM->aStackBreadCrumbs;
			if(isset($aStackBreadCrumbs[$szACTION_ID]))
					{
					$szCategory="_None_";
					if(isset($aStackBreadCrumbs[$szACTION_ID]["category"]	))
						$szCategory=$aStackBreadCrumbs[$szACTION_ID]["category"];

					$szFilename				="";
					if(isset($aStackBreadCrumbs[$szACTION_ID]		))
					if(isset($aStackBreadCrumbs[$szACTION_ID]["filename"]	))
						$szFilename=$aStackBreadCrumbs[$szACTION_ID]["filename"];

					$aStart=Array(		"Event"		=>"ConfigListStarts"	,
								"filename"	=>$szFilename		,
								"category"	=>$szCategory		,
								"ActionID"	=>$szACTION_ID		,
								"Count"		=>count($aResponse)	);
					vectorEventSend($aStart);
					unset($aStackBreadCrumbs[$szACTION_ID]);
					}
			$oSHM->aStackBreadCrumbs=$aStackBreadCrumbs;
			//	 /CHECK for breadcrumbs

			$aSend =Array();
			foreach($aResponse as $key=>$value)
				{
				if(strpos($key,"Category")!==FALSE)
					{
					if(count($aSend)>0)vectorEventSend($aSend);
					$aCatNo	=explode("-",$key);
					$aSend	=Array("Event"=>"ConfigEntry","id"=>$aCatNo[1],"name"=>$value,"ActionID"=>$szACTION_ID);
					}else{
					$aLine	=explode("-",$key);
					$aSend	=array_merge($aSend,Array("Line".$aLine[2]=>$value));
					}
				}
			if(count($aSend)>0)vectorEventSend($aSend);
			$aEnd=Array("Event"=>"ConfigListComplete","ActionID"=>$szACTION_ID);
			vectorEventSend($aEnd);
//			print "Break up config file here".print_r($aResponse,TRUE);
			return;
		case "_AMIfollows":break;
		case "_AMIfailure":break;
//		$aFailure=Array(0	=>"No Such extension or number"	,
//				1	=>"No Answer"			,
//				4	=>"Answered"			,
//				8	=>"Congested or not available"	);
//		print "Response (Failure)(".$aFailure[$aResponse["Reason"]].")".print_r($aResponse,TRUE)."\n";	break;
		case "_AMIerror"://print "_AMIerror\n";
//			print_r($aResponse);
		case "_AMIpayload":break;
			break;
//AGENTS
		case "Agentcallbacklogin"	:break;case "Agentcallbacklogoff"	:break;
		case "AgentCalled"		:break;case "AgentsComplete"		:break;
		case "AgentConnect"		:break;case "AgentDump"		:break;
		case "Agentlogin"		:break;case "Agentlogoff"		:break;
		case "Events"			:break;
		case "CoreShowChannel"		;break;
		case "CoreShowChannelsComplete"	:break;
		case "DAHDIShowChannelsComplete":break;
		case "ListDialplan"		:break;
//Parked Calls
		case "ParkedCallsComplete"	:case "ParkedCall"		:case "UnParkedCall"		:break;

		case "PeerEntry"		:break;	case "PeerlistComplete"		:break;
		case "RegistrationsComplete"	:break;
		case "Registry"			:break;
// QUEUES
		case "QueueStatusComplete"	:break;		case "QueueStatusEnd"		:break;
		case "QueueParams"		:break;		case "QueueMember"		:break;
		case "QueueMemberAdded"		:break;		case "QueueMemberPaused"	:break;
		case "QueueMemberRemoved"	:break;
		case "QueueMemberStatus"	:break;

		case "Newchannel"		:break;
		case "ChannelUpdate"		:break;
		case "Newstate"			:break;
		case "NewAccountCode"		:break;

		case "Dial"			:break;
		case "OriginateResponse"	:break;

		case "Newexten"			:break;
		case "VoicemailUserEntry"	:break;

		case "VoicemailUserEntryComplete":break;

		case "Hangup"			:break;
		case "NewCallerid"		:break;
		case "Bridge"			:break;
		case "Unlink"			:break;
// Monitor	/var/spool/asterisk/monitor/ 
		case "MonitorStart"		:break;
		case "MonitorStop"		:break;
// Status
		case "Status"			:break;
		case "PeerStatus"		:break;
		case "StatusComplete"		:break;
// Cdr
		case "Cdr"			:break;
		case "SetCDRUserField"		:break;

		case "ExtensionStatus"		:break;
		case "MusicOnHold"		:break;
		case "Join"			:break;
		case "Leave"			:break;
		case "Link"			:break;
		case "MeetmeListStarts"		:break;
		case "MeetmeList"		:break;
		case "MeetmeJoin"		:break;
		case "MeetmeLeave"		:break;
		case "MeetmeStopTalking"	:break;

		case "MessageWaiting"		:break;
		case "NewCallerid"		:break;
		case "NewExtension"		:break;
		case "Rename"			:break;
//System Status Events

		case "RTPSenderStat"		:return;

		case "VarSet"			:
			if($aResponse[		"Channel"	]!="none"	)return;
			if($aResponse[		"Variable"	]!="CONSOLE"	)return;

			$aResponse[		"Event"		]="DialplanReload"	;
			unset($aResponse[	"Channel"	]);
			unset($aResponse[	"Variable"	]);
			unset($aResponse[	"Value"		]);
			dynamicDialplanEntries();
			break;
		case "Alarm"			:break;	case "AlarmClear"		:break;
		case "DNDState"			:break;
		case "Reload"			:break;//print "\nSAW RELOAD";
		case "Shutdown"			:break;
		case "UserEvent"		:
//print "\nDecode USEREVENT ".print_r($aResponse,TRUE);
break;
		case "ZapShowChannels"		:break;	case "ZapShowChannelsComplete"	:break;

/*	The ListCommandXXXXXXX pseudo events are generated in
**	AsterClick.php, by function handleAMI_ListCommands()
**	The AMI command "Listcommands", tends to produce some majorly 
**	bulky output, so these events break up the output into
**	event sized pieces.						********/
		case "_ListCommandsStart"	:	// Beginning of command list
		case "_ListCommandEntry"	:	// Entry in the command list
		case "_ListCommandsComplete"	:break;	// No more commands to list.
/*	The _CLIXXXXXXXXX pseudo events are generated in 
**	AsterClick.php, by function handleAMI_Command()
**
**	These events are generated as AsterClick parses CLI response text blobs
**	returned from an AMI "command" action. The basic reason we do this is that
**	some CLI responses can be very bulky, so AsterClick chunks the result into
**	logical event sized pieces that easily pass through the System V messaging
**
**	AsterClick also attempts to parse these text blobs into structured
**	data in a generic way. See /etc/asterisk/AsterClick/class.cliParser.inc if
**	your really curious.
*/
		case "_CLIresponse"		:	// Single line CLI response
		case "_CLIList"			:	// Announce the beginning of a CLI response list.
		case "_CLIRow"			:	// Row of parsed CLI response data.
		case "_CLIFooter"		:	// Row of parsed CLI response data (footer of tabular response)
		case "_CLIComplete"		:break; // Announce the ending of a CLI response list.

		case "DBGetResponse"		:
//dPrint("SAW DBGetResponse ".print_r($aResponse,TRUE) ,iCare_dPrint_AMIevent);
break;


		// If we have arrived here , then we have neglected to define the 
		// event in one of the case statements , so we make an announcement about that and bail.
		default	:	$szResponseText	= "Unvectored event(".$aResponse["Event"	].")\n".print_r($aResponse,TRUE);
				$bMustPrint	=TRUE;
				vectorEventPrint($szResponseText,$bMustPrint);
				return;
			}// end switch
	vectorEventPrint(	$szResponseText,$bMustPrint	);	// $bMustPrint defaults to FALSE at the beginning 
									// of this function but any of the case: statements
									// can turn $bMustPrint TRUE and the response
									// text representing by defat the event name
									// will be printed
	vectorEventSend(	$aResponse			);	// send the event to the browser System V message queue.
	}// end function

/*	Function	:	
**	Parameters	:	
**	Returns		:
**	Description	:
*/
function dynamicDialplanEntries()
	{
//	AMIaction("Login",Array(	"Username"	=>$szAMIusername,
//					"Secret"	=>$szAMIsecret));
//AMIaction=command params=Array
//(
 //   [ORIGIN] => HTML5_4d40ca81b8c41
 //   [command] => dialplan add extension 3097,1(DOG),GoTo,default,AsterClickForward/SIP/3097,1 into AsterClick
//)
//$dline = Array()
/*
	exten	=> _AsterClickForward!,	1		,Answer();
	same	=>			n		,Verbose(4,dstcontext = ${CDR(dcontext)} / dst = ${CDR(dst)} )
	same	=>			n		,Set(CDR(__AsterClickForwardFrom)=${EXTEN:18})			;
	same	=>			n		,Set(CDR(__AsterClickForwardTo)=${DB(${EXTEN})})		;
	same	=>			n		,Verbose(4,"FORWARDING ${CDR(AsterClickForwardFrom)} TO ${CDR(AsterClickForwardTo)}");
	same	=>			n		,ExecIf($[DIALPLAN_EXISTS(AsterClickForward)]?GoSub(AsterClickForward,start,1(${CDR(AsterClickForwardFrom)},${CDR(AsterClickForwardTo)})):Goto(default));
	same	=>			n(dialresult)	,Verbose(4,"FORWARDING RESULT = ${DIALSTATUS}")
	same	=>			n		,ExecIf($[DIALPLAN_EXISTS(${CDR(dcontext)},${CDR(dcontext)},forward-${DIALSTATUS})]?Goto(${CDR(dcontext)},${CDR(dcontext)},forward-${DIALSTATUS}))
	same	=>			n		,ExecIf($[DIALPLAN_EXISTS(${CDR(dcontext)},${CDR(dcontext)},${DIALSTATUS})]?Goto(${CDR(dcontext)},${CDR(dcontext)},${DIALSTATUS}):Goto(${DIALSTATUS}));
	same	=>			n(NOANSWER)	,NoOp(NOANSWER		)
	same	=>			n(BUSY)		,NoOp(BUSY		)
	same	=>			n(CHANUNAVAIL)	,NoOp(CHANUNAVAIL	)
	same	=>			n(CONGESTION)	,NoOp(CONGESTION	)
	same	=>			n		,Playback(followme/sorry);
	same	=>			n		,Goto(default,s,1	)
	same	=>			n(ANSWER)	,NoOp(CONGESTION	)
	same	=>			n		,Hangup();
	same	=>			n(default)	,Playback(followme/pls-hold-while-try)	;
	same	=>			n		,Dial(${CDR(AsterClickForwardTo)},20,m)		;
	same	=>			n		,Goto(dialresult);
*/
	}
?>
