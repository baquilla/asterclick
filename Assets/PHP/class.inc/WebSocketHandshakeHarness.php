<?php
/*	File		:
**	Author		:	Dr. Clue
**	Description	:
*/
header("content-type:text/plain");

require_once("class.webSocketHandshake_08.inc");


$test=<<<EOL
GET /AsterClick HTTP/1.1
Upgrade: websocket
Connection: Upgrade
Host: 192.168.1.170:150
Sec-WebSocket-Origin: http://127.0.0.1
Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==
Sec-WebSocket-Version: 8

EOL;



$oUpgrade		=new WebSocketHandshake_08()	;
$upgrade		=$oUpgrade->doHandShake($test)	;



?>
