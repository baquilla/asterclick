<?php
/*	File		:	AsterClick_signal.inc
**	Author		:	Dr. Clue
**	Description	:	Deals with PHP signal handlers.
*/


// tick use required as of PHP 4.3.0
declare(ticks = 1);
// signal handler function
/*	Function	:	AsterClick_signal_handler()
**	Parameters	:	$iSignalNum
**	Returns		:	None
**	Description	:	
*/
function AsterClick_signal_handler($iSignalNum)
	{
	switch ($iSignalNum)
		{
	case SIGTERM	:	dPrint("Caught SIGTERM..."	,iCare_dPrint_Signal);exit();	break;	// handle shutdown tasks
	case SIGHUP	:			// handle restart tasks
				dPrint("Caught SIGHUP..."	,iCare_dPrint_Signal);break;
	case SIGUSR1	:	dPrint("Caught SIGUSR1..."	,iCare_dPrint_Signal);break;
	case SIGPIPE	:	dPrint("Caught SIGPIPE..."	,iCare_dPrint_Signal);break;


	default		:
		dPrint("Caught SIGxxxx ($iSignalNum)..."	,iCare_dPrint_Signal);break;
		// handle all other signals
		}// End Switch
	}// End Function
/*	Function	:	AsterClick_signal_handler_setup()
**	Parameters	:	None
**	Returns		:	None
**	Description	:	Setup Signal handlers.
*/
function AsterClick_signal_handler_setup()
	{
//return;
	dPrint("AsterClick_signal_handler_setup()...",iCare_dPrint_Signal);
	pcntl_signal(SIGTERM	, "AsterClick_signal_handler");
	pcntl_signal(SIGHUP	, "AsterClick_signal_handler");
	pcntl_signal(SIGUSR1	, "AsterClick_signal_handler");
	pcntl_signal(SIGPIPE	, "AsterClick_signal_handler");
	dPrint("/AsterClick_signal_handler_setup()...",iCare_dPrint_Signal);
	}


?>
