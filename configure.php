<?PHP
/*	File		:	configure.php
**	Author		:	Dr. Clue	( A.K.A. Ian A Storms )
**	Description	:	This PHP file supports the configuration
**			options of php classes used in this project.
**			Scripts should require_once() this file and keep
**			master settings like database logins here , instead of
**			repeating them in a bunch of different files, where it's
**			a pain to find / edit them if they should change.
**
**			If your application does not use a particular
**			group of settings , simply ignore them, as over
**			time your likely to add aditional components that do use them.
**
**			NOTE: ASTERISK AMI/AGI users , your settings are at the bottom.
*/
/*************************************************
**	General PHP configuration options	**
*************************************************/
error_reporting(E_ERROR ); // E_ERROR | E_WARNING | E_PARSE);
ini_set('display_errors'		, true);
ini_set('html_errors'			, true);
ini_set('display_startup_errors'	, true);
error_reporting(E_ALL | E_ERROR | E_PARSE);
/*************************************************
**	check defines				**
*************************************************/
if(!defined('CLIENT_LONG_PASSWORD'	))define('CLIENT_LONG_PASSWORD'		,'1'		);// new more secure passwords 
if(!defined('CLIENT_FOUND_ROWS'		))define('CLIENT_FOUND_ROWS'		,'2'		);// Found instead of affected rows 
if(!defined('CLIENT_LONG_FLAG'		))define('CLIENT_LONG_FLAG'		,'4'		);// Get all column flags 
if(!defined('CLIENT_CONNECT_WITH_DB'	))define('CLIENT_CONNECT_WITH_DB'	,'8'		);// One can specify db on connect 
if(!defined('CLIENT_NO_SCHEMA'		))define('CLIENT_NO_SCHEMA'		,'16'		);// Don't allow database.table.column 
if(!defined('CLIENT_COMPRESS'		))define('CLIENT_COMPRESS'		,'32'		);// Can use compression protocol 
if(!defined('CLIENT_ODBC'		))define('CLIENT_ODBC'			,'64'		);// Odbc client 
if(!defined('CLIENT_LOCAL_FILES'	))define('CLIENT_LOCAL_FILES'		,'128'		);// Can use LOAD DATA LOCAL 
if(!defined('CLIENT_IGNORE_SPACE'	))define('CLIENT_IGNORE_SPACE'		,'256'		);// Ignore spaces before '(' 
if(!defined('CLIENT_PROTOCOL_41'	))define('CLIENT_PROTOCOL_41'		,'512'		);// New 4.1 protocol 
if(!defined('CLIENT_INTERACTIVE'	))define('CLIENT_INTERACTIVE'		,'1024'		);// This is an interactive client 
if(!defined('CLIENT_SSL'		))define('CLIENT_SSL'			,'2048'		);// Switch to SSL after handshake 
if(!defined('CLIENT_IGNORE_SIGPIPE'	))define('CLIENT_IGNORE_SIGPIPE'	,'4096'		);// IGNORE sigpipes 
if(!defined('CLIENT_TRANSACTIONS'	))define('CLIENT_TRANSACTIONS'		,'8192'		);// Client knows about transactions 
if(!defined('CLIENT_RESERVED'		))define('CLIENT_RESERVED'		,'16384'	);// Old flag for 4.1 protocol 
if(!defined('CLIENT_SECURE_CONNECTION'	))define('CLIENT_SECURE_CONNECTION'	,'32768'	);// New 4.1 authentication 
if(!defined('CLIENT_MULTI_STATEMENTS'	))define('CLIENT_MULTI_STATEMENTS'	,'65536'	);// Enable/disable multi-stmt support 
if(!defined('CLIENT_MULTI_RESULTS'	))define('CLIENT_MULTI_RESULTS'		,'131072'	);// Enable/disable multi-results 
//if(!defined(CLIENT_REMEMBER_OPTIONS	))define('CLIENT_REMEMBER_OPTIONS', (((ulong) 1) << 31));

/*************************************************
**	Document level defines			**
*************************************************/
//define ( DOC_TYPE,'<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">');

/*************************************************
**	CLASS: class.mysql.php							**
*************************************************/
// Avoid hanging your machine up with long result sets
// which in the web are probably errors in some request anyway.
// While I've set this to 350 , you can set it to whatever
// higher number you think the largest single displayed result
// set will be for your task task. See also 'F1_MAXROWS' in class.mysql.inc
// for programming limits in a query
$MySql_iMaxRows		=350			;
// Default XML root and row names typically used in the construction
// of MySql query result sets for XML and JSONized XML formats.
$Nodes_szNameRoot	="root"			;
$Nodes_szNameRow	="row"			;
// Check if this is the development machine's document root
// and use the first $MySql_xxxxx assigments below.
// If not, assume a production machine and use the second set
// of $MySql_xxxxx assigments below.
$bLocal			=FALSE;
$bUseMySql		=FALSE;

if(isset($_SERVER["SERVER_ADDR"])		)
if($_SERVER["SERVER_ADDR"]== "127.0.0.1" 	)$bLocal=FALSE;
if($bUseMySql	)
if($bLocal	)
	{	// Development server settings
	$MySql_szServer		="localhost"		;
	$MySql_szDatabase	="yourDatabaseName"	;
	$MySql_szUser		="yourSQLusername"	;
	$MySql_szPassword	="yourSQLpassword"	;
	}else{	// Production server settings
	$MySql_szServer		="localhost"		;
	$MySql_szDatabase	="yourDatabaseName"	;
	$MySql_szUser		="yourSQLusername"	;
	$MySql_szPassword	="yourSQLpassword"	;
	}
/*************************************************
**	This is for the programmers wBase.php 	**
**	interface to mysql.			**
*************************************************/
$szAdminLogin	="yourwBaseUsername";
$szAdminPassword="yourwBasePassword";
/*************************************************
**	This is for the asterisk PBX	 	**
**	interface as relates to the 		**
**	FastAGI.php				**
*************************************************/
$iAGIverbosity	=10				;// How chatty the diagnostic messages are.
$iAGIport	=4544				;// port to run the AGI client server on

/*************************************************
**	This is for the asterisk PBX	 	**
**	interface as relates to the 		**
**	FastAGI.php	and FastAMI.php		**
**************************************************
For AMI configuration , your etc/asterisk/manager.conf will look something like this.

;
; Asterisk Call Management support
;

; By default asterisk will listen on localhost only. 
[general]
enabled =yes
port =5038
bindaddr =127.0.0.1

; No access is allowed by default.
; To set a password, create a file in /etc/asterisk/manager.d
; use creative permission games to allow other serivces to create their own
; files
#include "manager.d/*.conf" 

[yourAMIusername]
secret=yourAMIsecret
read = all,system,call,log,verbose,command,agent,user,config
write = all,system,call,log,verbose,command,agent,user,config,originate

*************************************************/
$szAMIhost	="127.0.0.1"			;// Host running the asterisk server.
$szAMIusername	="AsterClick"			;// Found in asterisk etc/asterisk/manager.conf [username].
$szAMIsecret	="AsterClickSecret"		;// Found in asterisk etc/asterisk/manager.conf secret.
$iAMIport	=5038				;// port AsterClick connects to for Asterisk AMI
$szWebSocketHost="0.0.0.0"			;// Host to connect to. 
$iWebSocketPort	=150				;// Port your web browser will connect to.
$szPHPtimeZone	='America/Los_Angeles'		;// Local timezone to use.

include("class.ini.inc");
$oAsterClickConfig= new CLASSini(Array("FILE"=>"AsterClickServer.conf"));

//$szTest=$oAsterClickConfig->getIniValue("ami"		,"username"	,$szAMIusername	);
//print "AsterClick manager user (".$szTest.")";

$szAMIhost	=$oAsterClickConfig->getIniValue("ami"		,"host"		,$szAMIhost		);
$szAMIusername	=$oAsterClickConfig->getIniValue("ami"		,"username"	,$szAMIusername		);
$szAMIsecret	=$oAsterClickConfig->getIniValue("ami"		,"secret"	,$szAMIsecret		);
$szAMIport	=$oAsterClickConfig->getIniValue("ami"		,"port"		,$iAMIport		);
//$iAMIport	=5038				;// port AsterClick connects to for Asterisk AMI
$szWebSocketHost=$oAsterClickConfig->getIniValue("websockets"	,"host"		,$szWebSocketHost	);
$iWebSocketPort	=$oAsterClickConfig->getIniValue("websockets"	,"port"		,$iWebSocketPort	);

$szPHPtimeZone	=$oAsterClickConfig->getIniValue("php"		,"timezone"	,$szPHPtimeZone		);

date_default_timezone_set($szPHPtimeZone);

/*************************************************
**	SMTP parameters	"SMTP"|"MAIL"|"SENDMAIL"**
**	See class.smtp.inc for details.		**
*************************************************/
define("SMTP_SERVER"			,"mail.yourdomain.com"	,	TRUE);
define('SMTP_METHOD'			,"SMTP"			,	TRUE);// SMTP | MAIL
define("SMTP_AUTH"			,"login"		,	TRUE);
define("SMTP_USER"			,"you@your.com"		,	TRUE);
define("SMTP_FROM"			,"reply@your.com"	,	TRUE);
define("SMTP_PASS"			,"mailPassword"		,	TRUE);

define("SMTP_EHELO"			,"mail.yourdomain.com"	,	TRUE);








?>
