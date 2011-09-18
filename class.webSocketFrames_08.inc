<?php
/*	File		:	webSocketFrames.inc
**	Author		:	Dr. Clue
**	Description	:	Well, the simplicity of earlier message wrappings has given way to
**			a structured frame protocol which we implement here. It is my hope to 
**			in separating the wrap/unwrap functionality to this separate framing
**			allow a greater degree of flexibility in supporting past , current and future 
**			gyrations of the WebSocket protocol as it evolves.
**	URLS		:	http://tools.ietf.org/html/draft-ietf-hybi-thewebsocketprotocol-08#section-4
				https://github.com/jam1401/PHP-Websockets-Server
				https://github.com/jam1401/PHP-Websockets-Server/blob/master/lib/protocolhybi.class.php

	 0		   1		   2		   3
	 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
	+-+-+-+-+-------+-+-------------+-------------------------------+
	|F|R|R|R| opcode|M| Payload len |    Extended payload length    |
	|I|S|S|S|  (4)  |A|     (7)     |	     (16/63)		|
	|N|V|V|V|       |S|		|   (if payload len==126/127)   |
	| |1|2|3|       |K|		|				|
	+-+-+-+-+-------+-+-------------+ - - - - - - - - - - - - - - - +
	|     Extended payload length continued, if payload len == 127  |
	+ - - - - - - - - - - - - - - - +-------------------------------+
	|				|Masking-key, if MASK set to 1	|
	+-------------------------------+-------------------------------+
	| Masking-key (continued)       |	  Payload Data		|
	+-------------------------------- - - - - - - - - - - - - - - - +
	:		     Payload Data continued ...			:
	+ - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
	|		     Payload Data continued ...			|
	+---------------------------------------------------------------+

**
**	$acf	=Array("FIN"=>1,"RSV1"=>0,"RSV2"=>0,"RSV3"=>0,"opcode"=>8,"MASK"=>0,"Payload_len"=>0,"Payload_len_extended"=>0,"Payload_Data"=>"");
*/


require_once("class.union.inc");		//	Used to label and manipulate bits in a value bi-directionally
class	WSF_class_union_byte0 extends union	{public	function	__construct(&$bindTo=NULL){parent::__construct(
	Array(		"_0x80_FIN"		=>NULL	,
			"_0x40_RSV1"		=>NULL	,
			"_0x20_RSV2"		=>NULL	,
			"_0x10_RSV3"		=>NULL	,
			"_0x0f_opcode"		=>NULL	),$bindTo);}			}	//	End	Class
class	WSF_class_union_byte1 extends union	{public	function	__construct(&$bindTo=NULL){parent::__construct(
	Array(		"_0x80_MASK"		=>NULL	,
			"_0x7f_Payload_len"	=>NULL	),$bindTo);}			}	//	End	Class




$WSF_types	=Array(	"continuation"		=>0x00,	//	Data	Frames
			"text"			=>0x01,"binary"		=>0x02,	"reserved3"=>0x03,"reserved4"=>0x04			,
			"reserved5"		=>0x05,"reserved6"	=>0x06,	"reserved7"=>0x07					,
			"close"			=>0x08,	//	Control	Frames
			"ping"			=>0x09,"pong"		=>0x0A,	"reservedB"=>0x0B,"reservedC"=>0x0B			,
			"reservedD"		=>0x0D,"reservedE"	=>0x0E,	"reservedF"=>0x0F					);
		//	Make a reverse lookup of the	$WSF_types array.
$WSF_typeNames	=array_flip(				$WSF_types								);
		//	Make defines for the 		$WSF_types array.
foreach(						$WSF_types as $k=>$v)
	if(!defined("WSF_".$k))
		define("WSF_".$k,$v	);


define('UNWRAP_PEER_CLOSE_FRAME'	,205);	//	Reset, send close frame and close socket
define('UNWRAP_NEED_MORE_BYTES'		,206);	//	Partial content received

class WS_UnwrapException		extends Exception
	{
	public	function	__construct($message,$code,$previous=NULL)
		{
		parent::__construct($message,$code,$previous);
		}
	}


	/*	A base class for all the different WebSocket protocol versions
	**	as well as perhaps some non-WebSocket protocols
	*/
class	WebSocket_codec		{
	//	Returns [width] digits in ($number) when using ($base) ||| [mask]	a AND value to constrain a value's range
function	byteWidthMask($number	,$base		=16	)
		{
		return Array(	"mask"	=>($Width=pow((	$And=ceil(log($number,$base))),$base>>1)-1),
				"width"	=>		$And);	}	
	//	reverse the order of bytes in a multi byte value
function	flipByteOrder($value	,$packFormat	="d*"	)
			{
			$a		= array_reverse(array_merge(unpack("C*",pack($packFormat,$value))));
			$ab		=""			;
			reset($a)	;while (list($key, $val) = each($a)){/*echo "$key => $val\n";*/$ab.=chr($val);}
			return 		array_pop(unpack($packFormat,$ab));
			}	//	End	Function
	}	//	End	Class	WebSocket_codec



	//	CLASS	We keep all the WebSocket message frame properties here.
class	WebSocket_codec_frame_data	implements	arrayaccess, Countable, Iterator
	{
//	public		$bytes		=Array(1,0)	;
	public		$ubytes		=Array()	;
	public		$container	=Array()	;
	public		$b0		=0xff		,
			$b1		=0xff		;
	public		$u0,$u1				;

	static		function	construct		($oParams=Array()	){return new WebSocket_codec_frame_data($oParams);		}
	public		function	__construct		($oParams=Array()	)
		{
		$this->u0	=	new	WSF_class_union_byte0;
		$this->u1	=	new	WSF_class_union_byte1;
		$this->ubytes	=Array(&$this->u0,&$this->u1);
		$this->container=Array(  0			=>&$this->u0		,"byte0"	=>&$this->b0	//	First Header Byte
					,1			=>&$this->u1		,"byte1"	=>&$this->b1	//	Second	Header Byte
					,"Payload_len_extended"	=>doubleVal(0)	//	[next,conditional	(Payload_len	==126|127	)]	// Bits 16/64	when Payload_len==126/127
					,"Masking_key"		=>""		//	[next conditional	(MASK		==TRUE		)]	// Bits 32	Four random chars
					,"Payload_Data"		=>""		//	[next]
					);//	End	Array	container

		$this->reset_frame();
		if(count($oParams)>0)		$this->params($oParams);			
		}	//	End	Function	__construct
	public		function	reset_frame()
			{
			$this->b0=0;$this->b1=0;
//			$this->container[0			]->attach($this->b0)	;
//			$this->container[1			]->attach($this->b1)	;
			$this->u0->value(0);
			$this->u1->value(0);
			$this->container["Payload_len_extended"	]=	doubleVal(0)	;
			$this->container["Masking_key"		]=	""		;
			$this->container["Payload_Data"		]=	""		;
			}
	//		Return all non numeric array keys
	public		function	keys		(		)
			{
			return array_merge(array_filter(array_keys($this->container	),function ($k)	{return gettype($k)=="string";}),$this->container[0]->keys(),$this->container[1]->keys());
			}
	//		Apply values from $oParams to related properties and/or list them
	public		function	params		(&$oParams=NULL,$bPrint=FALSE)
			{
			$bEmpty=($oParams==NULL||count($oParams)==0)	;$bPrint=($bPrint|$bEmpty);
			foreach($this->keys()	as $k=>$v)
				{
				if(($bSetting	=(!$bEmpty)&&isset($oParams[$v])))
					{
//print "\n".__FUNCTION__."() Setting '$v' to ($oParams[$v]) ";
					
					$this->__set($v,$oParams[$v]	)		;
					}

				if(!$bPrint	)continue;
					$pv		=(string)$this->__get($v)				;
					switch($v)	{
					case	"opcode"	:$pv="$pv ({$GLOBALS["WSF_typeNames"][$pv]})"	;break;
					case	"Payload_Data"	:$zz="";foreach(str_split($pv) as $kk=>$vv)$zz.=sprintf(" % 3d",ord($vv));$pv="$pv ($zz)";break;
					case	"Masking_key"	:$zz="";foreach(str_split($pv) as $kk=>$vv)$zz.=sprintf(" % 3d",ord($vv));$pv="$pv ($zz)";break;
							}
					printf("\r\n%02d ) %-20.20s=%s%-50.50s",$k,$v,($bSetting?" (Setting) ":" "),$pv);
				}	//	End	foreach()

//print "\n/".__FUNCTION__."() Getting 'MASK' to (".$this->__get("MASK").") ";

			}		//	End	Function	params


	//		Displays those parameters qualifying for active participation in wrapping/unwrapping WebSock frames.
	public		function	paramsActive	(		)
		{
		$THIS	=&$this			;$wf	=&$this->container	;
		reset($wf)			;$data	=$wf			;
		$data	= array_filter($wf,function	($item		) use (&$wf,&$data,&$THIS)
			{
			$k		=key	($wf)	;//$value		=current($wf)	;

			if(($bResult=(gettype($k)!="integer")))
			switch($k."@")
				{
			case	"b0@"			:$bResult=FALSE;break;
			case	"b1@"			:$bResult=FALSE;break;
			case	"Payload_len_extended@"	:$bResult= (	($THIS->container[1]->__get("Payload_len"	))	>125	)	;break; 
			case	"Masking_key@"		:$bResult= (	 $THIS->container[1]->__get("MASK"		)	==1	)	;break;
			case	"Payload_Data@"		:$bResult= (	($THIS->container[1]->__get("Payload_len")	)	>0	)	;break; 
				}//	End	Switch
			next($wf);
			return $bResult;
			}	);
		return $data;
		}
//*****
//*****	implement __get()/__set()/__isset() overloads
//*****
	public		function	__isset		(	$name	)
			{
			return (isset($this->container[		$name]	)||
				isset($this->container[0]->	$name	)||
				isset($this->container[1]->	$name	)||$name=="Payload_Data_Length");
			}
	public		function	__set		($name,$value	)
			{


			if($name==="Payload_Data_Length")
				{
//	print "\n__set() SETTING (Payload_Data_Length)";
				$len				= $value						;
				$Payload_len			= $this->container[1]->__get("Payload_len")		;
				$Payload_len_extended		=&$this->container["Payload_len_extended"	]	;

				if	($len>0xfffe	){	$Payload_len_extended	=doubleVal(	$len)		;$this->container[1]->__set("Payload_len",0x7f);	}
				elseif	($len>0x7d	){	$Payload_len_extended	=intVal(	$len)&0xFE	;$this->container[1]->__set("Payload_len",0x7e);	}
				else			 {	$Payload_len_extended	=NULL				;$this->container[1]->__set("Payload_len",$len);	}
				return;
				}

			if(	$name=="byte0"){$this->container[0]->value($value);return;}
			if(	$name=="byte1"){$this->container[1]->value($value);return;}

			if(	isset($this->container[		$name]	))
				{
				$this->container[	$name]	=$value;
				switch(		$name."@")
					{
				case	"Payload_Data@"	://			print "\nProcessing Payload __set";
					$len				=strlen($value)						;


					$Payload_len			=$this->container[1]->__get("Payload_len")		;
					$Payload_len_extended		=&$this->container["Payload_len_extended"	]	;

					if	($len>0xfffe	){	$Payload_len_extended	=doubleVal(	$len)		;$this->container[1]->__set("Payload_len",0x7f);	}
					elseif	($len>0x7d	){	$Payload_len_extended	=intVal(	$len)&0xFE	;$this->container[1]->__set("Payload_len",0x7e);	}
					else			 {	$Payload_len_extended	=NULL				;$this->container[1]->__set("Payload_len",$len);	}



					break;
					}	//	End	Switch
				return;
				};
			if(	isset($this->container[0]->$name	)){$this->container[0]->$name	=$value;return;};
			if(	isset($this->container[1]->$name	)){$this->container[1]->$name	=$value;return;};
			}
	public		function	__get		($name		)
			{
			if(	$name=="byte0")return $this->container[0]->value();
			if(	$name=="byte1")return $this->container[1]->value();
			if(	isset($this->container[0]->	$name	)){return $this->container[0]->	$name	;};
			if(	isset($this->container[1]->	$name	)){return $this->container[1]->	$name	;};
			if(	isset($this->container[		$name]	)){return $this->container[	$name]	;};
			return NULL;			
			}
//*****
//*****	implement arrayacccess
//*****
	public		function	offsetSet	($name, $value	){		$this->__set(	$name,$value);return;	}		//	End	Function	offset
	public		function	offsetExists	($name		){return	$this->__isset(	$name)	;}
	public		function	offsetUnset	($name		){}
	public		function	offsetGet	($name		){return	$this->__get(	$name)	;}
//*****
//*****	implement Countable
//*****
	public		function	count		(		){return	count($this->container);}
//*****
//*****	implement Iterator
//*****
	public		function	rewind		(		){		reset		($this->container);	}
	public		function	current		(		){return	current		($this->container);	}
	public		function	key		(		){return	key		($this->container);	}
	public		function	next		(		){return	next		($this->container);	}
	public		function	valid		(		){return	$this->current() !== false;		}    
	}	//	End	Class	WebSocket_codec_frame_data



	/*******************************************
	**	A class for coding and decoding  WebSocket frames for HyBi protocol version 7,8,9,10,11,12,13
	**	
	********************************************/
class	WebSocket_codec_frame	extends WebSocket_codec
	{
	public		$wframe			=NULL			;
	public		$WSocket_frame		=""			;
	public		$wp			=""			;
	//*******************
	//		Reset internal frame representation
	//*******************
//	public		function	resetFrame()		{	$this->wframe->reset_frame();		}



	//*******************
	//		When an instance is called as a function ,it simple lists the WebSocket frame properties
	//*******************
	public		function	__invoke	($oParams=Array()	,
							 $bPrint=FALSE		){$this->wframe->params($oParams,$bPrint);				}
	//*******************
	//		Construct frame with values matching those in the associative array.
	//*******************
	public		function	__construct	($oParams=Array()	){$this->wframe			=WebSocket_codec_frame_data::construct();	}
	//*******************
	//		Applies the (Masking_key) to the (Payload_Data). Calling this function repeatedly will toggle/flip the mask.
	//*******************
	public		function flipMask(&$buff,$Mask_key)
			{
			try	{		$aMask_key=str_split($Mask_key);
						for($i=0;$i<strlen($buff);$i++)	{$j		= $i % 4;$buff[$i]	= chr(ord($buff[$i]) ^ ord($aMask_key[$j]))	;}
				}catch(Exception	$e)	{	print "Exception ".print_r($e,TRUE);trace_call();	}
			return $buff;
			}	//	End	Function	flipMask
	//*******************
	//		Appends a regular string to a binary string.
	//*******************
	public		function binPrint(&$binStr,$szStr)
			{
			$aPackArguments		 =array_merge((array) "C*",array_map(function($c){return ord($c);},str_split($szStr)));
			$B			 =call_user_func_array("pack",$aPackArguments); 
			if($B)	{$binStr	.=$B;		}
			return $B;
			}	//	End	Function	binPrint()

	//*******************
	//		Wraps a a message in a WebSocket frame suitable for transmission to a browser or similar client.
	//*******************
	public		function	wrap		(	&$oParams	)//	"Payload_Data"
			{
			//	Zeros out the frame data to defaults	.
			$this->wframe->reset_frame();
			//	Build payload length values		.
if(!isset(		$oParams["Payload_len"	])		)	$oParams["Payload_len"	]=0	;
if(			$oParams["Payload_len"	]==0		)
if( isset(		$oParams["Payload_Data"	])		)
			$oParams["Payload_len"	]		=strlen($oParams["Payload_Data"	])	;
			$this->wframe->Payload_Data_Length	=	$oParams["Payload_len"	]	;
			$oParams["Payload_len"	]		=$this->wframe[1]->Payload_len		;
			$this->wframe->params($oParams,FALSE)						;
			$oParams["Payload_len"	]		=$this->wframe[1]->Payload_len		;

			$websocket_frame			=	&$oParams["websocket_frame"]	;
			$oParams["websocket_frame"]		=	""				;

//print "\nWRAP() paramsActive=".print_r($this->wframe->paramsActive(),TRUE);
			foreach($this->wframe->paramsActive() as $k=>$v)
				{
//				printf("\n".'wrapping item [%1$15.15s] with value %2$15.15s at (%3$d) ',$k,$this->wframe->$k,strlen($oParams["websocket_frame"]));//-strlen($frame));
				switch($k)
					{
				case	"byte0"			:$websocket_frame.=chr($this->wframe->byte0&0xff);break;
				case	"byte1"			:$websocket_frame.=chr($this->wframe->byte1&0xff);break;
				case	"Payload_len_extended"	:
					if(	$this->wframe->Payload_len==126){$websocket_frame.=pack("n"	,$v&0xffff);		}
					elseif(	$this->wframe->Payload_len==127){$websocket_frame.=pack("d"	,flipByteOrder($v));	}	;break;
				case	"Masking_key"		:
					if(!$this->wframe[1]->MASK)break;
//print "\nMASK=(".$this->wframe[1]->MASK.")Setting Random Masking_key. ";
					$oParams["Masking_key"]	= $this->wframe["Masking_key"]	=pack("N", rand(0, pow(255,4) - 1));
					$websocket_frame	.=$this->wframe["Masking_key"];						;break;

				case	"Payload_Data"		:
					$vOut	=$v;if($this->wframe["MASK"])$this->flipMask($vOut,$this->wframe["Masking_key"]);
				//	$websocket_frame	.=$vOut;
					$this->binPrint($websocket_frame,$vOut)								;
					break;
					}	//	End	Switch
				}		//	End	foreach

			return ($websocket_frame);
			$this->wframe->reset_frame();
			}			//	End	Function	wrap

	//*******************
	//		Unwraps a message from a WebSocket frame received from a browser or similar client
	//*******************
	public		function	unwrap		(	&$oParams	)//	WebSocket_frame
			{
			$this->wframe->reset_frame();		$this->wframe->params($oParams,FALSE);
			$frame			=	$oParams["websocket_frame"];
			$oParams["Payload_Data"]=""	;$Payload_Data=&$oParams["Payload_Data"];

			if(strlen($frame)<2)
				{
				print "\n unwrap()	UNWRAP_NEED_MORE_BYTES have(".strlen($frame).") want >2";
					$oParams["unwrap_exception"]=UNWRAP_NEED_MORE_BYTES;
//				throw new WS_UnwrapException("Not enough bytes to unwrap websocket_frame.",UNWRAP_NEED_MORE_BYTES);
				return FALSE;
				}
			$this->wframe["byte0"]	=ord(		$frame{0}	);
			$this->wframe["byte1"]	=ord(		$frame{1}	);

			$frame			=substr(	$frame,2	);
			foreach($this->wframe->paramsActive() as $k=>$v)
				{
//				printf("\nUnwrapping item [%15.15s] with value %15.15s  at (%d) ",$k,$v,strlen($oParams["websocket_frame"])-strlen($frame));
				switch($k)
					{
				case	"byte0"			:break;
				case	"byte1"			:break;
				case	"Payload_len_extended"	:
					switch($this->Payload_len)
						{
					case 126:	list(,	$this->wframe["Payload_len_extended"])=unpack("v"	,$frame);$frame=substr($frame,2);break;
					case 127:	list(,	$this->wframe["Payload_len_extended"])=unpack("d"	,strrev(substr($frame0,0,8)));$frame=substr($frame,8);	break;
					default		:	$this->wframe["Payload_len_extended"]=0;
						}
					break;
				case	"Masking_key"		:
					$this->wframe["Masking_key"	]	=substr($frame,0,4	);
					$frame					=substr($frame,4	);break;
				case	"Payload_Data"		:
					$this->wframe["Payload_Data"	]	=substr($frame,0,($this->wframe->Payload_len<126)?$payload_length=$this->wframe->Payload_len:$this->wframe->Payload_len_extended);
					if($this->wframe[1]->MASK)
						{
						$Payload_Data			=		$this->wframe["Payload_Data"];
						$this->flipMask(		 $Payload_Data,	$this->wframe["Masking_key"]);
						$this->wframe["Payload_Data"]	=$Payload_Data;
						}
					break;
					}	//	End	Switch
				}		//	End	foreach
			print "\nReceived Payload (".$this->wframe->Payload_Data.") opcode (".$this->wframe->opcode.")";
			if($this->wframe->opcode==WSF_close)
				{
				$oParams["unwrap_exception"]=UNWRAP_PEER_CLOSE_FRAME;
//				throw new WS_UnwrapException("unwrap() Received Client Close Frame.",UNWRAP_PEER_CLOSE_FRAME);
				}
//			$this->wframe->params();
			return $this->wframe->Payload_Data;
			}	//	End	Function	unwrap()
	}			//	End	class		WebSocket_codec_frame






function dumpFrame($websocket_frame,$szLabel="-unnamed-")
	{
	print "\n".__FUNCTION__."()\tlabel='$szLabel'\tframe length=(".strlen($websocket_frame).")";
	if($szLabel=="-unnamed-")trace_call();
	print "\nraw bytes=\t";
	foreach(str_split($websocket_frame) as $k=>$v)printf(' %03d',ord($v));
	print "\nraw chars=\t";
	foreach(str_split($websocket_frame) as $k=>$v)printf(' %- 3s',((ord($v)&127)==ord($v)&&ord($v)>31)?$v:"???");
	}




if(is_standalone()===TRUE)
	{
	if(TRUE	)
		{
	print "\n===[TESTING WebSocket_frame_text]==";
		$textFrame	=new WebSocket_frame_text(str_repeat("Hi Test !!!",50));
		$textFrame->params(NULL,TRUE);
		dumpFrame($textFrame(),"TESTING WebSocket_frame_text");
		}
	if(FALSE)
		{
		$closeFrame	=new WebSocket_frame_close("Closing Frame");
		dumpFrame($closeFrame());
		}
	if(FALSE)
		{
		$wsOut		=new WebSocket_codec_frame()			;
		$aTextFrame	=	Array(	"FIN"	=>1	,"RSV1"		=>0,"RSV2"			=>0,"RSV3"		=>0	,"opcode"	=>WSF_text	,
						"MASK"	=>1	,"Payload_len"	=>11,"Payload_len_extended"	=>0,"Masking_key"	=>"abcd","Payload_Data"	=>"Howdy Folks")	;
		$textFrame	=$wsOut->wrap($aTextFrame);
		dumpFrame($textFrame);
		}
	if(FALSE)
		{
		$wsOut		=new WebSocket_codec_frame()			;
		$aCloseFrame	=	Array(	"FIN"	=>1	,"RSV1"		=>0,"RSV2"			=>0,"RSV3"		=>0	,"opcode"	=>WSF_close	,
						"MASK"	=>0	,"Payload_len"	=>0,"Payload_len_extended"	=>0,"Masking_key"	=>NULL	,"Payload_Data"	=>NULL)		;
		$closeFrame	=$wsOut->wrap($aCloseFrame);
		dumpFrame($cf,"cf");
		}
	if(FALSE)
		{
$T=new WebSocket_codec_frame_data;
	print "\nFIN	=".$T->FIN	;
	print "\nopcode	=".$T->opcode	;
	print "\nbyte0	=".$T->byte0	;
	print "\nbyte1	=".$T->byte1	;

/*	$T=new WebSocket_frame_text("Howdy folks");
	print "\nFIN	=".$T->ws_codec->wframe->FIN	;
	print "\nopcode	=".$T->ws_codec->wframe->opcode	;
	print "\nbyte0	=".$T->ws_codec->wframe->byte0	;
	print "\nbyte1	=".$T->ws_codec->wframe->byte1	;

	$TT=&$T->ws_codec->wframe;
*/
		}



	};//	End	If	is_standalone()




function makeCodec()
	{
	return new WebSocket_codec_frame();
	}


class	WebSocket_frame
	{
	public	$ws_codec;
//	public	$oParams=Array("FIN"=>1,"RSV1"=>0,"RSV2"=>0,"RSV3"=>0,"opcode"=>0,"MASK"=>0,"Payload_len"=>0,"Payload_len_extended"=>0,"Payload_Data"=>"","websocket_frame"=>"");
	public	$oParams=Array("FIN"=>1,"RSV1"=>0,"RSV2"=>0,"RSV3"=>0,"opcode"=>0,"MASK"=>0,"Payload_len"=>0,"Payload_len_extended"=>0,"Payload_Data"=>"","websocket_frame"=>"");

	public	function	__toString()
		{
		return	$this->oParams["Payload_Data"];
		}		//	End	Function	__toString()
	//	A frame's implementation should return an array with a numeric [0] indexs for use with the list() construct, and optionally 
	//	one or more associative ["some name"] entries for structured data. The decoded data MUST NOT be stored 
	//	in the WebSocket_frame's (oParams) array property
	public	function	decode(){	}
	//	
	public	function	encode(){	}
	public	function	__invoke($io=NULL)
		{
		switch($type=gettype($io))
			{
		case	"string"	:break;
		case	"array"		:$this->params($io);break;
		case	"boolean"	:print get_called_class()." params ".print_r($this->oParams,TRUE);break;
		case	"NULL"		:return $this->ws_codec->wrap($this->oParams);
		default	:print "\nUnhandled Type (".$type.")";
			}	//	End	Switch
		}		//	End	Function	__invoke()
	public	function	params($oParams=NULL,$bPrint=FALSE)
		{
		if($oParams)foreach($this->oParams as $k=>$v)if(isset($oParams[$k]))$this->oParams[$k]=$oParams[$k];
		if($oParams==NULL||$bPrint)print "\nWebSocket_frame::params()=".print_r($this->oParams,TRUE);
		}		//	End	Function	params()
	public	function	__construct($oParams=NULL	)
		{
		$this->ws_codec=new WebSocket_codec_frame()	;
		$this->params($oParams)				;
		$this->oParams["opcode"]=$this->opcode		;
//	print "\nCONSTRUCT ".get_called_class()." opcode ".$this->opcode." params ".print_r($this->oParams,TRUE);
		}		//	End	Function	__construct()
	}			//	End	Class		WebSocket_frame
/*
**
**	DATA	FRAMES	(CONTINUATION,TEXT,BINARY)
**
*/

	//	CONTINUATION	frame
class	WebSocket_frame_continuation	extends WebSocket_frame
	{
	public	$opcode		=0x0				;
	public	function	__construct($oParams=NULL	)
		{
throw new Exception("continuation frames not yet implemented");
//		$this->Payload_Data	=sprintf("%",utf8_dencode(($message)?$message:""));
		parent::__construct($oParams);
		}
	}	//	End	Class	WebSocket_frame_continuation

	//	TEXT		frame
class	WebSocket_frame_text		extends WebSocket_frame
	{
	public	$opcode		=0x1	;
	public	$message	=""	;
	public	function	encode($oParams=NULL)
		{
		$message="--Error--";
		switch(gettype($oParams))
			{
		case	"array"	:if(isset($oParams["Payload_Data"]))$message=$oParams["Payload_Data"];break;
		case	"string":$message=$oParams;$oParams=Array();break;
			}
		$Payload_Data	=sprintf("%s",utf8_encode($this->message=$message));
		return array_merge(($oParams)?$oParams:Array(),Array("Payload_Data"=>$Payload_Data,"opcode"=>1,"Payload_len"=>strlen($Payload_Data),"MASK"=>0));
		}
	public	function	decode($oParams)
		{
		if(strlen($this->oParams["Payload_Data"])>1)list(,$this->message)=utf8_decode($this->oParams["Payload_Data"]);
		return Array(	"message"	=>$this->message,
				0		=>$this->message);
		}
	public	function	__construct($oParams=FALSE)
		{
		parent::__construct($this->encode($oParams)					);
		}


	}	//	End	Class	WebSocket_frame_text

	//	BINARY		frame
class	WebSocket_frame_binary		extends WebSocket_frame
	{
	public	$opcode		=0x2	;
	public	function	__construct($oParams=NULL	)
		{
		$this->Payload_Data	=$message;
		parent::__construct($oParams);
		}
	}	//	End	Class	WebSocket_frame_binary
/*
**
**	CONTROL	FRAMES	(CLOSE,PING,PONG)
**
*/
	//	CLOSE		frame
class	WebSocket_frame_close		extends WebSocket_frame
	{	//	https://developer.mozilla.org/en/WebSockets/WebSockets_reference/CloseEvent
	public	$opcode		=0x8	;
public	static	$aCloseCodes=Array(	1000	=>"1000 indicates a normal closure, meaning whatever purpose the connection was established for has been fulfilled."
				,	1001	=>"1001 indicates that an endpoint is (going away), such as a server going down, or a browser having navigated away from a page."
				,	1002	=>"1002 indicates that an endpoint is terminating the connection due to a protocol error."
	 			,	1003	=>"1003 indicates that an endpoint is terminating the connection because it has received a type of data it cannot accept (e.g. an endpoint that understands only text data MAY send this if it receives a binary message)."
				,	1004	=>"1004 indicates that an endpoint is terminating the connection because it has received a frame that is too large."
				,	1005	=>"1005 is a reserved value and MUST NOT be set as a status code in a Close control frame by an endpoint.  It is designated for use in applications expecting a status code to indicate that no status code was actually present."
				,	1006	=>"1006 is a reserved value and MUST NOT be set as a status code in a Close control frame by an endpoint.  It is designated for use in applications expecting a status code to indicate that the connection was closed abnormally, e.g. without sending or receiving a Close control frame."
				,	1007	=>"1007 indicates that an endpoint is terminating the connection because it has received data that was supposed to be UTF-8 (such as in a text frame) that was in fact not valid UTF-8 [RFC3629]." );
	public	function	encode($oParams=NULL)
		{
		$this->code	=(isset($oParams)&&$oParams["code"	])?$oParams["code"	]:1000							;
		$this->message	=(isset($oParams)&&$oParams["message"	])?$oParams["message"	]:self::$aCloseCodes[$this->code]	;
		$Payload_Data	=sprintf("%s%s",pack("n",$this->code),utf8_encode($this->message));
		return array_merge(($oParams)?$oParams:Array(),Array("Payload_Data"=>$Payload_Data));
		}
	public	function	decode($oParams)
		{
		if(strlen($this->oParams["Payload_Data"])>1)list(,$this->code)=unpack("v",$this->oParams["Payload_Data"]);
		$this->message=($this->oParams["Payload_Data"]>2)				?
				utf8_decode(substr($this->oParams["Payload_Data"],2))	:
				self::$aCloseCodes[$this->code]			;
		return Array(	"code"		=>$this->code	,
				"message"	=>$this->message,
				0		=>$this->code.":".$this->message);
		}
	public	$code		=1000						;
	public	$message	="";//self::$aCloseCodes[1000]			;
	public	function	__construct($oParams=FALSE)
		{
		parent::__construct($this->encode($oParams)					);
		}
	}	//	End	Class	WebSocket_frame_close

	//	PING		frame
class	WebSocket_frame_ping		extends WebSocket_frame
	{
	public	$opcode		=0x9	;
	public	function	__construct($oParams=NULL	)
		{
		$this->Payload_Data	=sprintf("%",utf8_decode(($message)?$message:""));
		parent::__construct($oParams);
		}
	}	//	End	Class	WebSocket_ping_frame

	//	PONG	frame
class	WebSocket_frame_pong		extends WebSocket_frame
	{
	public	$opcode		=0xa	;
	public	function	__construct($oParams=NULL	)
		{
		$this->Payload_Data	=sprintf("%",utf8_encode(($message)?$message:""));
		parent::__construct();
		}
	}	//	End	Class	WebSocket_frame_pong


class	WebSocket_endpoint
	{
public	$URL		=""	;
const	CLOSED		=3	;
const	CLOSING		=2	;
const	OPEN		=1	;
const	CONNECTING	=0	;
public	$bufferedAmount	=0	;
public	$readyState	=WebSocket_endpoint::CONNECTING;
public	$protocol	=""	;
public	$onclose	=Array();
public	$onerror	=Array();
public	$onmessage	=Array();
public	$onopen		=Array();


public	function	close()
		{

		}
public	function	send($data)
		{

		}

public	function	__construct()
		{

		}
	}	//	End	Class	WebSocket_endpoint














/****** CentOS
---Was--
kernel /vmlinuz-2.6.18-194.32.1.e15 ro root=/dev/VolGroup01/LogVa100
---Edit To--
kernel /vmlinuz-2.6.18-194.32.1.e15 ro root=/dev/VolGroup01/LogVa100=/single

***/



	//		Apply values from $oParams to related properties and/or list them
/*	public		function	params		(&$oParams=NULL,$bPrint=FALSE)
			{

$this->wframe->params($oParams,$bPrint);


			$bEmpty=($oParams==NULL||count($oParams)==0)	;$bPrint=($bPrint|$bEmpty);
			foreach($this->wframe->keys()	as $k=>$v)
				{
				if(($bSetting	=(!$bEmpty)&&isset($oParams[$v])))$this->wframe->__set($v,$oParams[$v]	)		;
				if(!$bPrint	)continue;
					$pv		=(string)$this->__get($v)				;
					switch($v)	{
					case	"opcode"	:$pv="$pv ({$GLOBALS["WSF_typeNames"][$pv]})"	;break;
					case	"Payload_Data"	:
						$zz="";
						foreach(str_split($pv) as $kk=>$vv)$zz.=sprintf(" % 3d",ord($vv));
						$pv="$pv ($zz)";
						break;
					case	"Masking_key"	:
						$zz="";
						foreach(str_split($pv) as $kk=>$vv)$zz.=sprintf(" % 3d",ord($vv));
						$pv="$pv ($zz)";
						break;
							}
					printf("\r\n%02d ) %-20.20s=%s%-50.50s",$k,$v,($bSetting?" (Setting) ":" "),$pv);
				}



//			return &
			}*/	//	End	Function	params
	//		Displays those parameters qualifying for active participation in wrapping/unwrapping WebSock frames.
/*	public		function	paramsActive	(			){return $this->wframe->paramsActive();				}*/

	//		Checks multiple property collections in wframe for the ($name) variable
//	public		function	getContainer	($name			){
//trace_call();

//return $this->wframe->getContainer();
//foreach(Array(&$this->wframe,&$this->wframe[0],&$this->wframe[1]) as $k=>$v)if(isset($v->$name))return $v;return NULL;
//}
	//		Set the value of a wframe property.
/*	public		function	__set		($name			,
							 $value			)
		{
//		if($o=&$this->getContainer($name))$o->$name=$value;
		$this->wframe[$name]=$value;
		}
	//		Does ($name) exist
	public		function	__isset		($name			){return in_array($name,$this->keys());				}
	//		Retrieve ($name)
	public		function	__get		($name			){
return $this->wframe->$name;
//return ($o=&$this->getContainer($name))?$o->$name:NULL;

	}*/

?>
