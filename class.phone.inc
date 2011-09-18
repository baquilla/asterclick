<?php
/*	File		:	class.phone.inc
**	Author		:	Dr. Clue
**	Description	:	This class is an abstraction class
**			for the various types of phone channels in 
**			Asterisk (SIP,IAX,DAHDI,ZAP) 
*/


class AsteriskPhone
	{
	public static $aMembers=Array();
/*	CONSTRUCTOR	:
**	Parameters	:
**	Returns		:
**	Description	:
*/
function	AsteriskPhone($aParams=Array())
	{
	foreach($aPArams as $key=>$value)
		$this->$key=$value;
	AsteriskPhone::$aMembers[]=$this;
	}
/*	CONSTRUCTOR	:
**	Parameters	:
**	Returns		:
**	Description	:
*/
function	__destruct()
	{
	$oSelf=array_search($this,AsteriskPhone::$aMembers);
	if($oSelf!==FALSE)unset(AsteriskPhone::$aMembers[$oSelf]);
	return;
	}
/*	Function	:
**	Parameters	:
**	Returns		:
**	Description	:
*/
function setDoNotDisturb($bOnOff=FALSE)
	{
	}
/*	Function	:
**	Parameters	:
**	Returns		:
**	Description	:
*/
function getDoNotDisturb($szExtension=NULL)
	{
	}
/*	Function	:
**	Parameters	:
**	Returns		:
**	Description	:
*/
function getOtherParty()
	{

	}
/*	Function	:
**	Parameters	:
**	Returns		:
**	Description	:
*/
function isRegistered($szExtension=NULL)
	{

	}
/*	Function	:
**	Parameters	:
**	Returns		:
**	Description	:
*/
function isRinging($szExtension=NULL)
	{

	}
/*	Function	:
**	Parameters	:
**	Returns		:
**	Description	:
*/
function setCallForwarding($szExtension=NULL)
	{

	}
/*	Function	:
**	Parameters	:
**	Returns		:
**	Description	:
*/
function transferCallTo($szExtension=NULL)
	{

	}
/*	Function	:
**	Parameters	:
**	Returns		:
**	Description	:
*/
function setEntensionVisible($szExtension=NULL)
	{

	}
/*	Function	:
**	Parameters	:
**	Returns		:
**	Description	:
*/
function setEntensionHidden($szExtension=NULL)
	{

	}
/*	Function	:
**	Parameters	:
**	Returns		:
**	Description	:
*/
function parkCall($szExtension=NULL)
	{


	}
/*	Function	:
**	Parameters	:
**	Returns		:
**	Description	:
*/
function unparkCall($szExtension=NULL)
	{

	}
/*	Function	:
**	Parameters	:
**	Returns		:
**	Description	:
*/
function hangup($szExtension=NULL)
	{


	}
/*	Function	:
**	Parameters	:
**	Returns		:
**	Description	:
*/
function record($szExtension=NULL,$bOnOff=FALSE)
	{


	}
/*	Function	:
**	Parameters	:
**	Returns		:
**	Description	:
*/
function getCallerId($szExtension=NULL)
	{


	}
/*	Function	:
**	Parameters	:
**	Returns		:
**	Description	:
*/
function getCallerInfo($szExtension=NULL)
	{


	}
/*	Function	:
**	Parameters	:
**	Returns		:
**	Description	:
*/
function setCallLog($szExtension=NULL,$bOnOff=TRUE)
	{

	}
/*	Function	:
**	Parameters	:
**	Returns		:
**	Description	:
*/
function dial($szNumber,$szExtension=NULL)
	{

	}
/*	Function	:
**	Parameters	:
**	Returns		:
**	Description	:
*/
function getVoiceMailStatus($szExtension=NULL)
	{

	}
/*	Function	:
**	Parameters	:
**	Returns		:
**	Description	:
*/
function getFollowme($szExtension=NULL)
	{

	}
/*	Function	:
**	Parameters	:
**	Returns		:
**	Description	:
*/
function setConference($aExtensions=Array())
	{

	}
/*	Function	:
**	Parameters	:
**	Returns		:
**	Description	:
*/
function delConference($aExtensions=Array())
	{

	}
/*	Function	:
**	Parameters	:
**	Returns		:
**	Description	:
*/
function getConference($szExtension=NULL)
	{

	}
/*	Function	:
**	Parameters	:
**	Returns		:
**	Description	:
*/
function dialConference($szExtension=NULL,$szNumber=NULL)
	{

	}
/*	Function	:
**	Parameters	:
**	Returns		:
**	Description	:
*/
function setMute($aExtensions=Array(),$bOnOff=FALSE)
	{

	}
/*	Function	:
**	Parameters	:
**	Returns		:
**	Description	:
*/
function getMute($aExtensions=Array())
	{

	}

} // end class AsteriskPhone

?>
