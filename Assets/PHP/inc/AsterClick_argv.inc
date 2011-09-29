<?php
/*	File		:	AsterClick_argv()
**	Author		:	Dr. Clue
**	Description	:	Handles situtations related to the 
**			commandline arguments.
*/




/*	Function	:	AsterClick_argv_main()
**	Parameters	:	$szArg
**	Returns		:	None
**	Description	:	Called from the global scope in AsterClick.php
*/
function AsterClick_argv_main($szArgv)
	{
	switch($szArgv)
		{
	case "start"	:AsterClick_argv_start()	;// Performs some startup checks
							 // for PHP compatibility.
									 break;
	case "stop"	:AsterClick_argv_stop()		;	exit()	;break;
	case "initd"	:AsterClick_argv_geninitd()	;	exit()	;break;
	default		:AsterClick_argv_help()		;	exit()	;break;
		}// /switch
	}
/*	Function	:	AsterClick_argv_stop()
**	Parameters	:	None
**	Returns		:	None
**	Description	:	This is meant to signal all components of AsterClick to stop.
*/
function AsterClick_argv_stop()
	{
	dPrint("*** ArsterClick	END	SERVER",iCare_dPrint_always);
	$oPid= new pid();
	$oPid->delPIDfilename();
	$oSHM= new shm();
	$oSHM->bWebSocketLoop=0;
	exit();
	}


/*	Function	:	AsterClick_argv_start()
**	Parameters	:	None
**	Returns		:	
**	Description	:
*/
function AsterClick_argv_start()
	{
	global $szAsterClickVersion;
 print <<<EOL
*****************************************
*****************************************
***
*** AsterClick ($szAsterClickVersion) server startup
***
*****************************************
*****************************************
EOL;

	$szMissingModules="";
	if(function_exists("shm_attach")===FALSE)
		{
		$szMissingModules.=<<<EOL
+------------------------------------------------------
| AsterClick requires the PHP module that
| supports shared memory.
| See: http://us2.php.net/manual/en/shmop.setup.php

EOL;
		}
	if(function_exists("msg_get_queue")===FALSE)
		{
		$szMissingModules.=<<<EOL
+------------------------------------------------------
| AsterClick requires the PHP module that
| supports system V messaging.
| See: http://us2.php.net/manual/en/sem.setup.php

EOL;
		}

	if(strlen($szMissingModules)>0)
		{
print <<<EOL


*******************************************************
** Your PHP configuration needs adjustment.        ****
*******************************************************
** In order to function properly , your PHP must   ****
** be configured with the appropriate modules.     ****
** Generally missing modules can be corrected by   ****
** by a simple rebuild with the appropriate flags. ****
*******************************************************
$szMissingModules+------------------------------------------------------
*******************************************************
** Please make the required adjustments to your    ****
** PHP installation , and try again.               ****
*******************************************************
** /Your PHP configuration needs adjustment.       ****
*******************************************************

EOL;
exit();
		}// endif
	}
/*	Function	:	AsterClick_argv_help()
**	Parameters	:	None
**	Returns		:	Displays the command line parameters for
**			starting , stopping and otherwise interacting with
**			the AsterClick.php server.
**	Description	:
*/
function AsterClick_argv_help()
	{
	echo <<<EOL
start	- starts daemon process.
stop	- ends damon process.
initd	- generates an init.d script for booting AsterClick at startup.

EOL;
exit();
	}



/*	Function	:	AsterClick_argv_geninitd()
**	Parameters	:	None
**	Returns		:	None
**	Description	:	Generates an init.d script.
*/
function AsterClick_argv_geninitd()
	{
	global $argv;

$szCWD		=getcwd();
$szEXE		=implode("",explode("./",$argv[0]));
$szSTEM		=implode("",explode("./",implode("",explode(".php",$szEXE))));
$szSCRIPT	=$szSTEM."d";
$szinitscript=<<< EOT
#!/bin/sh
#
# ${szSCRIPT}	:	Startup script for the $szSTEM Server
# Description	:	$szSTEM server for Asterisk PBX 1.6

if [ -f /etc/rc.d/init.d/functions ] ; then
. /etc/rc.d/init.d/functions
fi

if [ -f /etc/apparmor/functions ] ;then 
. /etc/apparmor/functions
fi


# Adjust these environmental variables as desired
PATH=PATH=/usr/kerberos/sbin:/usr/kerberos/bin:/usr/local/sbin:/usr/local/bin:/sbin:/bin:/usr/sbin:/usr/bin:/root/bin
USER=root
_=/etc/init.d

export PATH
export USER
export _

# Path to the $szSTEM directory
${szSTEM}dir=$szCWD/


# start the $szSTEM.php server
start() {
	echo -n \$"Starting \$prog: "
	sleep 5
	cd \$${szSTEM}dir
	nohup ./$szEXE start > /dev/null &
	return 0
}
# stop the $szSTEM.php server
stop() {
	echo -n $"Stopping \$prog: "
	cd \$${szSTEM}dir
	./$szEXE stop
	return 0
}
# reload the $szSTEM.php server
reload() {
	echo -n $"Reloading \$prog: "
	cd \$${szSTEM}dir
	./$szEXE restart

}
# See how we were called.
case "$1" in
  start)
	start
	;;
  stop)
	stop
	;;
  restart|reload)
	stop
	start
	;;
  *)
	echo $"Usage: \$prog {start|stop|restart|status}"
	exit 1
esac

exit 0

EOT;

if(file_put_contents($szSCRIPT,$szinitscript)==FALSE)	{
	dPrint("Error creating ($szSTEM) init.d boot script file $szSCRIPT\n",
	iCare_dPrint_always);
	}else{
	dPrint("The $szSTEM init.d boot script has been created in file $szSCRIPT\n",
	iCare_dPrint_always);
	}// endif 

}// End Function


?>




