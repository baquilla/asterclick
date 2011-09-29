<?php
/*	File		:	include.socket.defines.php
**	Author		:	Dr. Clue
**	Description	:
*/
if(!defined('ENOTSOCK'))
	{
define('ENOTSOCK'	,88	);	//			Socket operation on non-socket			
define('EDESTADDRREQ'	,89	);	//			Destination address required			
define('EMSGSIZE'	,90	);	//			Message too long				
define('EPROTOTYPE'	,91	);	//			Protocol wrong type for socket			
define('ENOPROTOOPT'	,92	);	//			Protocol not available				
define('EPROTONOSUPPORT',93	);	//			Protocol not supported				
define('ESOCKTNOSUPPORT',94	);	//			Socket type not supported			
define('EOPNOTSUPP'	,95	);	//			Operation not supported on transport endpoint	
define('EPFNOSUPPORT'	,96	);	//			Protocol family not supported			
define('EAFNOSUPPORT'	,97	);	//			Address family not supported by protocol	
define('EADDRINUSE'	,98	);	//			Address already in use				
define('EADDRNOTAVAIL'	,99	);	//			Cannot assign requested address			
define('ENETDOWN'	,100	); 	//			Network is down					
define('ENETUNREACH'	,101	); 	//			Network is unreachable				
define('ENETRESET'	,102	); 	//			Network dropped connection because of reset	
define('ECONNABORTED'	,103	); 	//			Software caused connection abort		
define('ECONNRESET'	,104	); 	// SOCKET_ECONNRESET	Connection reset by peer			
define('ENOBUFS'	,105	); 	//			No buffer space available			
define('EISCONN'	,106	); 	//			Transport endpoint is already connected		
define('ENOTCONN'	,107	); 	//			Transport endpoint is not connected		
define('ESHUTDOWN'	,108	); 	//			Cannot send after transport endpoint shutdown	
define('ETOOMANYREFS'	,109	); 	//			Too many references: cannot splice		
define('ETIMEDOUT'	,110	); 	//			Connection timed out				
define('ECONNREFUSED'	,111	); 	//			Connection refused				
define('EHOSTDOWN'	,112	); 	//			Host is down					
define('EHOSTUNREACH'	,113	); 	//			No route to host				
define('EALREADY'	,114	); 	//			Operation already in progress			
define('EINPROGRESS'	,115	); 	//			Operation now in progress			
define('EREMOTEIO'	,121	); 	//			Remote I/O error				
define('ECANCELED'	,125	); 	//			Operation Canceled				
	}
?>
