<?php
/*	File		:	class.pid.inc
**	Author		:	Dr. Clue
**	Description	:	This class implements a pid file to keep
**			track of the process id for the running program.
**			While any base filename can be used , the default 
**			behavior is to use the top level main PHP filename
**			simply replacing the .php extension with the .pid extension.
*/

class	pid
	{
	var $szPIDfilename	=""	;// The constructed name of the pid file
	var $szARGfilename	=""	;// The source file path to base the pid filename on
	var $iVerbosity		=10	;// The chattiness of the class
	var $stdError		=NULL	;// A reference to the stderr to write messages.
/*	CONSTRUCTOR	:	pid()
**	Parameters	:	None
**	Returns		:	None
**	Description	:	
*/
function	pid($aParams=Array())
	{
	$aIncludes		=get_included_files();
	$this->szARGfilename	=$aIncludes[0];
	foreach($aParams as $key=>$value)$this->$key=$value;
	$this->getPIDfilename()	;// constructs the PID filename and stores it.
	$this->stdError;
	}
/*	DESTRUCTOR	:	__destructor()
**	Parameters	:	None
**	Returns		:	None
**	Description	:	
*/
function	__destructor()
	{
	delPIDfilename();
	}
/*	Function	:	getPIDfilename()
**	Parameters	:	None
**	Returns		:	String - The name of the PID file used to store the process ID.
**	Description	:	This is simply the name of the running PHP script with the ".php"
**				extension replaced with a ".pid" extension.
*/
function getPIDfilename()
	{
	$this->szPIDfilename	=implode(".pid",explode(".php",$this->szARGfilename));
	$this->console_message("PID filename=".$this->szPIDfilename."\n",11);
	return $this->szPIDfilename;
	}
/*	Function	:	setPIDfilename()
**	Parameters	:	None
**	Returns		:	None
1**	Description	:	
*/
function setPIDfilename()
	{
	$newpid=posix_getpid();
	if(empty($this->szPIDfilename))$this->getPIDfilename();
	$this->console_message("\n::setPIDfilename(".$this->szPIDfilename.") with pid $newpid \n",11);//in {$this->szPIDfilepath} \n ";
	file_put_contents($this->szPIDfilename,$newpid, LOCK_EX);
	}
/*	Function	:	delPIDfilename()
**	Parameters	:	None
**	Returns		:	None
**	Description	:	
*/
function delPIDfilename()
	{
	if(file_exists($this->szPIDfilename))unlink(	$this->szPIDfilename);
	}
/*	Function	:	console_message()
**	Parameters	:	$szMessage
**				$iVerbosity	=[1] (0-9)
**				$bQuit		=[FALSE]
**	Returns		:	None
**	Description	:	Quit the server and exit
*/
function console_message($szMessage,$iVerbosity=1,$bQuit=FALSE)
	{
//	file_put_contents("./FastAGI.NARK",$szMessage, LOCK_EX|FILE_APPEND);
	if($this->iVerbosity>=$iVerbosity)
		{
		if(!is_resource($this->stdError))
			$this->stdError		=fopen("php://stderr","w");
		fwrite($this->stdError,$szMessage);
		}
	if($bQuit===TRUE)exit();
	}
/*	Function	:	isrunning()
**	Parameters	:	$bVerify
**	Returns		:	On success returns PID, otherwise returns FALSE
**	Description	:
*/
function isrunning($bVerify=FALSE)
	{
	$szPIDstart	=""				;//	To hold contents of PID file.
	if(file_exists($this->szPIDfilename)===FALSE)
		{
print "\n*** class.pid.inc *** isrunning return FALSE";

		return FALSE	;//	NO $szPIDFile FILE
		}
	if($bVerify===FALSE)return TRUE;		 //	In not using verification
							 //	we are simply checking that
							 //	the pid file exists.
	// If $bVerify is TRUE, we actually check to see if the process is trully running.
	$szPIDstart=file_get_contents($this->szPIDfilename)	;
	if(!empty($szPIDstart	))			 //	file exists and contains something
		{					 //	
		$system_result	=$this->shell_send_command("ps -A | grep $szPIDstart ")	;
		if(!empty($system_result	))	 //	Checking process list for $szPIDstart)
			{
			$aSystem_result	=explode(" ",$system_result)	;$szPIDstart=$aSystem_result[1];
			return $szPIDstart		;//	PID found and running return same.
			}
		}							 
	$this->delPIDfilename()				;//	so delete the file.
	return FALSE					;//	Bail.
	}// /function isrunning()
}// /class pid


?>
