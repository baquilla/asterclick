<?php
/*	File		:	class.fork.inc
**	Author		:	Dr. Clue
**	Description	:	Provides a utility class for forking 
**			the current process. One can either pass an array
**			containing the names of functions to call, or
**			extend the class and override member functions.
**			(parent(),child() and failed() )
*/



class fork
	{
	var $szfParent	=NULL;
	var $szfChild	=NULL;
	var $szfFailed	=NULL;

/*	CONSTRUCTOR	:	fork()
**	Parameters	:	Array()	$aParams	- Optional array of parameters.
**							"szfParent" name of function to call as parent after fork.
**							"szfChild"  name of function to call as child after fork.
**							"szfFailed" name of function to call incase for fails.
**	Returns		:	None
**	Description	:
*/
function fork($aParams=Array())
	{
	global $$this->szfParent;
	foreach($aParams as $key=>$value)$this->$key=$value;
	$pid = pcntl_fork();
	if ($pid == -1)
		{
		if(is_callable($this->szfFailed	, false, $pfFailed	))call_user_func($pfFailed);
		else $this->failed();

		} else if ($pid)	{	// we are the parent

		if(is_callable($this->szfParent	, false, $pfParent	))call_user_func($pfParent);
		else $this->parent();
		pcntl_wait($status,WNOHANG);	//Protect against Zombie children

		} else			{	// we are the child

		if(is_callable($this->szfChild	, false, $pfChild	))call_user_func($pfChild);
		else $this->child();

		}
	return;
	}
/*	Function	:	parent()
**	Parameters	:	None
**	Returns		:	None
**	Description	:	Place holder function meant to be overridden by a derrived class
**				or via the constructor arguments that specify target functions 
*/
function parent()
	{
//	echo "We the parent\n";
	exit();
	}
/*	Function	:	child()
**	Parameters	:	None
**	Returns		:	None
**	Description	:	Place holder function meant to be overridden by a derrived class
**				or via the constructor arguments that specify target functions 
*/
function child()
	{
//	echo "We the child\n";
	exit();
	}
/*	Function	:	failed()
**	Parameters	:	None
**	Returns		:	None
**	Description	:	Place holder function meant to be overridden by a derrived class
**				or via the constructor arguments that specify target functions 
*/
function failed()
	{
	die('We failed to fork\n');
	exit();
	}

	}// end class fork


?>
