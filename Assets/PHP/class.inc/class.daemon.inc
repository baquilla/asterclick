<?php
/*	File		:	class.daemon.inc
**	Author		:	Dr. Clue	(A.K.A. Ian A. Storms)
**	Description	:	This PHP file supports the implementation of
**				a base class for PHP daemons (Persistent Processes)
**	NOTES		:
**	URLS		:
*/

/*
**	Adjust runtime environment
*/
error_reporting(E_ALL)		;	// We are not affraid of erros , le us here about them
set_time_limit(0)		;	// We want to run forever , so (0) disables the time limit
ob_implicit_flush()		;	// We really don't want PHP doing any buffering
declare(ticks = 1)		;	//

/*
** Signal Handling
*/
pcntl_signal(SIGTERM	, 'sig_handler');
pcntl_signal(SIGINT	, 'sig_handler');
pcntl_signal(SIGCHLD	, 'sig_handler');
/*	Function	:	sig_handler()
**	Parameters	:	$sig
**	Returns		:	None
**	Description	:	Signal handler
*/
function sig_handler($sig)
	{
	switch($sig)
		{
	case SIGTERM	:
	case SIGINT	:	exit()				;break;
	case SIGCHLD	:	pcntl_waitpid(-1, $status)	;break;
		}//End Switch
	}
class CLASSdaemon
{
	var $iVerbosity			=0		;	// Verbosity level. The higher the level (0-9) the more messages.
	var $stdError			=NULL		;	// STDERR
	var $sock			=NULL		;	// Hosting socket resource handle.
	var $connection			=NULL		;	// Client Connect socket resource handle
	var $host			="127.0.0.1"	;	// Host to run on
	var $port			=4543		;	// port to run on
	var $bDaemon_running		=FALSE		;	// Listens for requests and forks on each connection
/*	CONSTRUCTOR	:	CLASSdaemon()
**	Parameters	:	$szCommand
**	Returns		:	String - The output of the executed command.
**	Description	:
*/
function CLASSdaemon($params=Array())
	{
	$this->szPIDpath	=getcwd();
	$this->szPIDfilepath	=$this->szPIDpath."/".$this->getPIDfilename();
	foreach($params as $key=>$value)$this->$key=$value;
	}

/*	Function	:	shell_send_command()
**	Parameters	:	$szCommand
**	Returns		:	String - The output of the executed command.
**	Description	:	Executes a shell command and returns the textual results output from the command.
*/
function shell_send_command($szCommand)
	{
	ob_start();system($szCommand);$system_result=ob_get_contents();ob_end_clean();
	return $system_result;
	}
/*	Function	:	change_identity()
**	Parameters	:	$uid, $gid 
**	Returns		:	None
**	Description	:	Change the identity to a non-priveledged user
**				nobody/nogroup, ie. change_identity(65534, 65534);
*/
function change_identity( $uid, $gid )
	{
	if( !posix_setgid( $gid )){	print "Unable to setgid to " . $gid . "!\n";	exit;}
	if( !posix_setuid( $uid )){	print "Unable to setuid to " . $uid . "!\n";	exit;}
	}
/*	Function	:	isrunning()
**	Parameters	:	None
**	Returns		:	On success returns PID, otherwise returns FALSE
**	Description	:
*/
function isrunning()
	{
	global $argv;
	$szPIDfile	=$this->getPIDfilename();
	$szPIDstart	="";
	if(file_exists($szPIDfile)===FALSE)//	NO $szPIDFile FILE
		{return FALSE;	}else{	$szPIDstart=file_get_contents($szPIDfile);}
	if(empty($szPIDstart))return FALSE				;// isrunning NO PID found 
	$system_result	=$this->shell_send_command("ps -A | grep $szPIDstart ")	;//	Checking process list for $szPIDstart)
	if(empty($system_result))return FALSE				;//	PID $szPIDstart not running
	$aSystem_result	=explode(" ",$system_result)			;$szPIDstart=$aSystem_result[1];
	return $szPIDstart						;//	PID found and running return same.
	}
/*	Function	:	getPIDfilename()
**	Parameters	:	None
**	Returns		:	String - The name of the PID file used to store the process ID.
**	Description	:	This is simply the name of the running PHP script with the ".php"
**				extension replaced with a ".pid" extension.
*/
function getPIDfilename()
	{
	global $argv;
	$this->szPIDfile	=implode(".pid",explode(".php",$argv[0]));
	return $this->szPIDfile;
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
		if(!is_resource($this->stdError))$this->stdError		=fopen("php://stderr","w");

	fwrite($this->stdError,$szMessage);

		}
	if($bQuit===TRUE)exit();
	}
/*	Function	:	server_quit()
**	Parameters	:	$szMEssage=""
**	Returns		:	None
**	Description	:	Quit the server and exit
*/
function server_quit($szMessage="")
	{
	global $argv;
	$this->console_message("::server_quit Stopped {$argv[0]} Children $szMessage \n");
	exit();
	}
/*	Function	:	server_loop()
**	Parameters	:	$address 	string -The address to listen on
**				$port		int	The port to listen on
**	Returns		:
**	Description	:	Creates a server socket and listens for incoming client connections
*/
function server_loop($address, $port)
	{
	global $argv;
//	$this->szPIDfilepath;

	if(($this->sock	= @socket_create(AF_INET, SOCK_STREAM, SOL_TCP			))	===FALSE)
		{
		$this->console_message("Call to socket_create failed to create socket: "		.socket_strerror($this->sock)."\n");
		$this->server_stop();exit();	}

	if(($ret	= @socket_bind(		$this->sock, $this->host, $this->port	))	===FALSE)
		{$this->console_message("Call to socket_bind failed to bind socket: "		.socket_strerror($ret)."\n");$this->server_stop();	exit();	}

	if(($ret	= @socket_listen(	$this->sock, 0				))	===FALSE )
		{$this->console_message("Call to socket listen failed to listen to socket: "	.socket_strerror($ret)."\n");$this->server_stop();	exit();	}

	socket_set_nonblock($this->sock);
	$this->console_message("::server_loop listening on port {$this->port}\n\n");
	$this->bDaemon_running=TRUE;
	while ($this->bDaemon_running)
		{
		if(!file_exists($this->szPIDfilepath))$this->server_quit(" server_loop (1) ");// The PID file has been removed so we quit

		$this->connection = @socket_accept($this->sock);
		if ($this->connection === false)	{	usleep(100);continue;	}// No connection available, sleep and loop

		if(!file_exists($this->szPIDfilepath))$this->server_quit(" server_loop (2) ");// The PID file has been removed so we quit

		if ($this->connection > 0)
				{	$this->handle_client($this->sock, $this->connection);		continue;		}

		$this->console_message("server_loop (error): ".socket_strerror($connection));die;
		}// end while
	}
function keeprunning()
	{
	return TRUE;
	}


/*	Function	:	socket_read_response()
**	Parameters	:	$sock
**	Returns		:	String - Response read from socket.
**	Description	:	
*/
function socket_read_response($sock,$iTries=3)
	{
	$szRead		=""	;
	$iWaitStart	=60	;
	$iWaitContinue	=1	;
	if(is_resource($sock)===FALSE)return $szRead;	
	socket_set_nonblock($sock);
	$this->console_message("\n::socket_read_response",2);
	$iTimeStart	= time();
	$iTimeLastRead	=0;
	$bFOREVER	=TRUE;
	for(;$bFOREVER&&$this->keeprunning();)
		{
		if(is_resource($sock)===FALSE)return $szRead;	
		$vSelect=socket_select($r = array($sock), $w =NULL,$e= NULL, 0,500);
		if($vSelect===FALSE)// Select Error
			{
			$this->console_message("\n::socket_read_response socket_select() failed, reason: " .  socket_strerror(socket_last_error()) . "\n");
			break;
			}
		$iTimeNow=time();
		if($vSelect==0)	{
				if(($iTimeNow-$iTimeStart	)>$iWaitStart&&$iTimeLastRead==0)
					{
					$this->console_message("\n::socket_read_response timed out waitng for data");break;
					}
				if(($iTimeNow-$iTimeLastRead	)>$iWaitContinue&&$iTimeLastRead!=0)
					{
					$this->console_message("\n::socket_read_response end read",2);
					break;
					}
				$this->console_message("\r::socket_read_response waiting...".($iTimeNow-(($iTimeLastRead==0)?$iTimeStart:$iTimeLastRead)),2);
		usleep(300);
				continue;
				}// Nothing Happened
		foreach($r as $rs)
			{
			if(is_resource($rs)===FALSE)return $szRead;	
			$szReadThis=socket_read($rs,2048,PHP_BINARY_READ);
			if($szReadThis===FALSE||strlen($szReadThis)==0){$bFOREVER=FALSE;break;}
			$this->console_message("\n::socket_read_response read(".strlen($szReadThis).") bytes",2);
			$iTimeLastRead=time();
			$szRead.=$szReadThis;
			break;
			}
		$this->console_message("\n",2);
		}//End Forever
	$this->console_message("\n::socket_read_response Ends\n",2);
	return $szRead;
	}
/*	Function	:	socket_send_command()
**	Parameters	:	$szCommand
**	Returns		:	
**	Description	:
*/
function socket_send_command($szCommand,$usingSock=NULL,$iVerbosity=2)
	{
	$szSocketRead	="";
	$thisSock	=$this->connection;
	if(is_resource($usingSock))$thisSock=$usingSock;
	$this->console_message("\n::socket_send_command Sending :".$szCommand,$iVerbosity);
	if(is_resource($thisSock))
	if(@socket_write($thisSock,$szCommand)!==FALSE)
		{
		$szSocketRead=$this->socket_read_response($thisSock);
		$this->console_message("\n::socket_send_command Received :".trim($szSocketRead),2);
		}else{
		$this->console_message("\n::socket_send_command Failed :".$szCommand ."[]". socket_strerror(socket_last_error()));
		}
	return $szSocketRead;
	}
/*	Function	:	handle_client()
**	Parameters	:	$ssock, $csock
**	Returns		:
**	Description	:	Handle a new client connection
*/
function handle_client($ssock, $csock)
	{
	$pid = pcntl_fork();
	if ($pid == -1)
		{	/* fork failed */
		$this->console_message("::handle_client fork failure!\n");
		die;
		}
	if ($pid == 0)
		{	/* child process */
		$this->bDaemon_running = false;
		if(is_resource($this->sock))
		socket_close($this->sock);
		$this->HIT($this->connection);
		socket_close($this->connection);
		}else	{
		socket_close($this->connection);
		}
	}
/*	Function	:	socket_close_graceful()
**	Parameters	:	$sock
**	Returns		:	None
**	Description	:	
*/
function socket_close_graceful($sock)
	{
	socket_shutdown($sock, 1);//remote host yet can read
	usleep(500);//wait remote host
	socket_shutdown($sock, 0);//close reading
	socket_close($sock);//finaly we can free resource
	$sock=NULL;
	}

/*	Function	:	server_help()
**	Parameters	:	None
**	Returns		:	None
**	Description	:	Prints command help
*/
function server_help()
	{
	$szHelp =<<<EOT

start		Start the server
stop		Stop the server
restart		Restart the server
-geninitd	create an init.d script
-h		help

EOT;
	$this->console_message($szHelp,-1);
	}
/*	Function	:	server_setup()
**	Parameters	:	None
**	Returns		:	None
**	Description	:	Called by server start just prior to any forking
**		to allow any global resources to be configured.
*/
function server_setup()
	{
	$this->console_message("::server_Setup ",9);
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

	$this->szPIDstart	=$this->isrunning();
	$this->server_setup();
	$pid = pcntl_fork();
	if ($pid == -1)
		{			
		$this->console_message("\n::start_server fork failure!\n");		exit();	//fork failure
		}elseif ($pid)	{		exit();	//close the parent
		}else		{			//child becomes our daemon
		$this->console_message("\n::server_start Starting...");
		posix_setsid();
		chdir('/');
		umask(0);
		$newpid=posix_getpid();
		$this->console_message("\n::server_start Running with pid $newpid \n");//in {$this->szPIDfilepath} \n ";
		file_put_contents($this->szPIDfilepath,$newpid, LOCK_EX);
		$this->server_loop($this->host, $this->port);
		return $newpid;
		}
	exit();
	}
/*	Function	:	server_stop()
**	Parameters	:	None
**	Returns		:
**	Description	:
*/
function server_stop()
	{
	$this->szPIDstart	=$this->isrunning();
	if(!empty($this->szPIDstart)&&$this->szPIDstart!==FALSE)
		{
		$this->console_message("\n::server_stop Stopping Run For $this->szPIDstart ...");
		if(is_resource($this->sock))
			{
			if($this->sock)socket_close($this->sock);
			print $this->shell_send_command("kill $this->szPIDstart");
			sleep(2);
			}
		}
	if(file_exists($this->szPIDfilepath))unlink(	$this->szPIDfilepath);
	$this->console_message("\n::server_stop Stopped\n");
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

${argv[0]} daemon program for asterisk 
====================================================
EOT;
	}
/*	Function	:	server_main()
**	Parameters	:	$argc,$argv
**	Returns		:	None
**	Description	:	Processes command-line or pseudo command line arguments
**			and starts,stops,restarts the daemon or provides help text.
*/
function server_main($argc,$argv)
	{
	$this->szPIDstart	=$this->isrunning();
	$bGiveHelp	=FALSE	;
	$bStart		=FALSE	;
	$bStop		=FALSE	;
	$szCommand	="help"	;//Default command
	if($argc<2)$argv[1]="---help";
	if(in_array($argv[1],Array("-help","-h","---help"))===TRUE)$argv[1]="---help";

	switch($argv[1])
		{
	case	"start"		:
	case	"stop"		:
	case	"restart"	:
			$szCommand=$argv[1];
			if($szCommand!="start"	)$bStop		=TRUE;
			if($szCommand!="stop"	)$bStart	=TRUE;
			if($this->szPIDstart!==FALSE&&($bStart===TRUE&&$bStop===FALSE))
				{
				print "\n${argv[0]}  already running $this->szPIDstart \n";
				$bGiveHelp	=TRUE;
				$bStart		=FALSE;
				}
			break;
	case	"status"	:exit(0);
	case	"-geninitd"	:$this->geninitd();exit(0);
	case	"-h"		:
	case	"-help"		:
	case	"---help"	:
	default			:$bGiveHelp=TRUE;
		}
	$this->console_message($this->server_banner());
	if($bGiveHelp	)$this->server_help();
	if($bStop	)$this->server_stop()	;
	if($bStart	)$this->server_start()	;
	}
/*	Function	:	geninitd()
**	Parameters	:	None
**	Returns		:	None
**	Description	:	Generates an init.d script.
*/
function geninitd()
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

# Source function library.
. /etc/rc.d/init.d/functions


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

file_put_contents($szSCRIPT,$szinitscript);

	}


}// end class CLASSdaemon
?>
