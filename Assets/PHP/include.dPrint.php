<?php
/*	File		:	include.dPrint.php
**	Author		:	Dr. Clue
**	Description	:	Used for conditionally printing debugging messages
**			throughout the PHP applicationl.
*/
require_once("include.getDefine.php");
//	Defines for gating diagnostic messages	(NOTE: the TRUE just means case insensitive)
define ("iCare_dPrint_always"			,1000,TRUE)	;	// Always print the message
define ("iCare_dPrint_sockets"			,4000,TRUE)	;	// Socket related message
define ("iCare_dPrint_socketsAMI"		,4005,TRUE)	;	// Socket related message
define ("iCare_dPrint_webSockets"		,4050,TRUE)	;	// webSocket related message
define ("iCare_dPrint_webSockets_handshake"	,4055,TRUE)	;	// webSocket_handshake related message
define ("iCare_dPrint_webSockets_frame"		,4060,TRUE)	;	// webSocket_handshake related message
define ("iCare_dPrint_parse"			,4100,TRUE)	;	// parse  related message
define ("iCare_dPrint_AMIaction"		,4200,TRUE)	;	// AMIcommand related message
define ("iCare_dPrint_AMIevent"			,4250,TRUE)	;	// AMIevent related message
define ("iCare_dPrint_SYSTEMevent"		,4255,TRUE)	;	// pseudo events sent by AsterClick
define ("iCare_dPrint_Signal"			,4260,TRUE)	;	// Signal Handler
define ("iCare_dPrint_CLIparse" 		,4300,TRUE)	;	// CLI parsing related message





/*	Function	:	dPrint()
**	Parameters	:	(String|mixed	)	$oMessage		Message to display.
**				(Number		)	$iPriority	[1]	Message pririty , or Message Id.
**	Returns		:	TRUE
**	Description	:	A central location to print debugging messages so that the output of same
**			can be easily vectored or suppressed.
*/
$iCare_dPrint	=10		;	//	Sets a minimum priority for messages not otherwise configured.



function	dPrint($oMessage,$iPriority=1)
	{
	global $iCare_dPrint,$bIsTTYconnection,$iCare_dPrint_last;
	if(!$bIsTTYconnection)return;

	
	switch($iPriority)
		{
	case	iCare_dPrint_always			:break	;
	case	iCare_dPrint_CLIparse			:return	;
	case	iCare_dPrint_sockets			:break	;
	case	iCare_dPrint_socketsAMI			:return	;
	case	iCare_dPrint_parse			:return	;//break;
	case	iCare_dPrint_AMIaction			:return	;//return;
	case	iCare_dPrint_AMIevent			:return	;//return;
	case	iCare_dPrint_SYSTEMevent		:return	;//return;
	case	iCare_dPrint_webSockets			:break	;//return;
	case	iCare_dPrint_webSockets_handshake	:return	;
	case	iCare_dPrint_webSockets_frame		:break	;
	case	iCare_dPrint_Signal			:return		;
	default	:
		if($iPriority<$iCare_dPrint)		return	;
		}//	End Switch($iPriority)
	$oMessage	=	trim(	$oMessage,"\n\r\t "	);
	if(empty(			$oMessage)		)return TRUE;


$bIsTTYconnection	=posix_isatty(	STDOUT)		;

if($iCare_dPrint_last!=$iCare_dPrint)	printf("\n%s",($iPriority!=iCare_dPrint_always)?("[".getDefineName($iPriority)."]\n"):"");

printf("\nAsterClick>");
$iCare_dPrint_last=$iCare_dPrint;
	print $oMessage;
	return TRUE;

	}

/**
***	This function should not really be in this file , but I needed a place to put it for the moment.
**/

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



?>
