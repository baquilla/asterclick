<?php
/*	File		:	class.union.inc
**	Author		:	Dr. Clue
**	Description	:

//	$bytes	=Array(129,133);
//	$ubytes	=Array(	$u1 = new union(Array("_0x01_FIN"	=>0,"_0x02_RSV1"	=>0,"_0x04_RSV2"=>0,"_0x08_RSV3"=>0,"_0xF0_opcode"=>0	),$bytes[0]),
//			$u2 = new union(Array("_0x01_MASK"	=>0,"_0xFE_Payload_len"	=>0							),$bytes[1]));


*/


/*
print ($zz=call_user_method("__get",$ub[1]["object"],"0"));
print $zz->opcode;
*/

class	ownedObject
	{
	public	$ownerObject=NULL;
	
	public	function __construct()
		{
		$bt=debug_backtrace(TRUE);
		while(	isset(	$bt[0]				)	&&
			isset(	$bt[0]["object"]		)	&&
			is_a(	$bt[0]["object"],"ownedObject")	)array_shift($bt)	;
		if(isset($bt[0]))if(isset($bt[0]["object"]))$this->ownerObject=&$bt[0]["object"];
	$GLOBALS["ubowner"]=&$this->ownerObject;//debug_backtrace(TRUE);
		}
	}



class	union	//extends ownedObject
	{
public	$__data		=0xff	;
public	$defs		=Array();
public	$__mask		=0xFF	;
public	$__width	=2	;
	static	function	maskWidth($number,$base=16)	// returns array	[width	]	number of digits in number in the given base
		{						//			[and	]	value to AND to constrain a number's size
		return Array( "mask"=>($Width=pow(($And=ceil(log($number,$base))),$base>>1)-1),"width"=>$And);
		}	
	public	function	getMaskShift($maskByte)
		{
		for(	$shiftTest	=	0				,	$shift		=	0				;
			$shiftTest			<	$this->__mask	&&	($maskByte	&$shiftTest)	==	0		;
			$shiftTest	=($shiftTest<<=1)?$shiftTest:1,$shift++									);
		return --$shift;
		}
	//	return an array of bitnames
	public	function	keys()	{return array_keys($this->defs);}
	//	returns bit field structure after optionally setting that bit field structure 
	public	function	getDef	($name,$value=NULL)
		{
		$setName=$name;
		if(preg_match('/_(0x[0-9a-f]{1,})_(.*)/i',$name,$M	))
				{
				list($def,$maskStringHex,$setName)	=$M							;
				$maskByte			=(hexdec($maskStringHex)		&$this->__mask)	;//	Try and restrict scaler.
				$shift				=$this->getMaskShift($maskByte);
				$this->defs[$setName]	=Array(	"mask"		=>$maskByte			,
								"shift"		=>$shift			,
								"default"	=>$value			,
								"name"		=>$setName			);
//				return $this->defs[$setName];
				}
		if(isset($this->defs[$setName])	)return $this->defs[$setName];
		return NULL;
		}
	public	function	value($iValue=NULL)
		{
		if($iValue!==NULL)$this->__data=$iValue;
		return $this->__data&$this->__mask;
		}
	public	function	values()
		{
		foreach($this->keys()	as	$k=>$v)printf("\n".'%15.15s =%d',$v,$this->__get($v));
		}
	public	function	attach(&$target=NULL)
		{
		if($target===NULL)return;
		$this->__data			=$target	;
		$this->{"__data"}		=&$target	;
		$target				=&$this->__data	;
		}
	public	function	__set			($name,$value		)
		{
		if(($d=$this->getDef($name))==NULL)throw new InvalidArgumentException("class.union.inc __set('$name',$value) '$name' is not defined in this instance");
		$this->setValue($name,$value);
		}
	public	function	__get			($name			)
		{
		if(($d=$this->getDef(			$name))===NULL	)throw new InvalidArgumentException("class.union.inc __get('$name') '$name' is not defined in this instance");
		return $this->getValue(			$name);
		}
	public	function	setValue		($name,$value		)
		{
		if(($d=$this->getDef($name))==NULL)throw new InvalidArgumentException("class.union.inc __set('$name',$value) '$name' is not defined in this instance");
		$set=$value<<$d['shift'];	$this->__data	=($this->__mask^	$d['mask'])&	$this->__data|$set;
		}
	public	function	getValue		($name			)
		{
		if(($d=$this->getDef($name))===NULL	)throw new InvalidArgumentException("class.union.inc __get('$name') is not defined in this instance");
		return	((			$this->__data&		$d['mask'])>>($d['shift']));
		}
	public	function	__isset		($name			){return (($d=$this->getDef($name))==NULL)?FALSE:TRUE;		}
	public	function	__construct	($structure	=Array(),&$target=NULL,$maxVal=0xff)
		{
//		parent::__construct();
		$T=&$target;	$this->attach($T);
//		$this->attach($target);
		foreach(union::maskWidth(($maxVal)?$maxVal:$this->__mask) as $k=>$v)
			{
			$_k="__".$k;$this->$_k=$v;
			}
		foreach($structure	as	$key=>$value)
			{
			$def=$this->getDef($key,$value);
//print "\ndef=".print_r($def,TRUE)." name=(".$def['name'].") key=($key) value=($value)";
			$this->setValue(	$def['name'],(($value===NULL)?$this->getValue($def['name']):$value))	;
//			$this->setValue(	$key	,(($value===NULL)?$this->getValue($key):$value))	;
			}
		}	//	End	Function	__construct()
	static	function	construct	($structure	=Array(),&$target=NULL,$maxVal=0xff)	{return new self($structure,$target);}
	public	function	__invoke	($value		=NULL	)
		{
		if($value!==NULL)$this->__data=$value	;
		if($value===NULL)$this->values		;
		return $this->__data&$this->__mask;
		}
	public	function	__toString	(			)
		{
		return (string)$this->__data;
		}

	}	//	End	class	union

class	WebSocket_Payload_len extends ownedObject
	{
	public	function	__construct	()
		{
		parent::__construct();
		}	
	}



if (			is_standalone()===TRUE)	//	Test Harness code
	{ 
	print	"\n".basename(__FILE__)." Loaded standalone\n";

class	WSF_class_union_byte0 extends union	{public	function	__construct(&$bindTo=NULL){parent::__construct(
	Array(		"_0x80_FIN"		=>NULL	,
			"_0x40_RSV1"		=>NULL	,
			"_0x20_RSV2"		=>NULL	,
			"_0x10_RSV3"		=>NULL	,
			"_0x0f_opcode"		=>NULL	),$bindTo);}			}	//	End	Class
class	WSF_class_union_byte1 extends union	{public	function	__construct(&$bindTo=NULL){parent::__construct(
	Array(		"_0x80_MASK"		=>NULL	,
			"_0x7f_Payload_len"	=>NULL	),$bindTo);}			}	//	End	Class



	$byte0	=129;
	$ubyte0	=new WSF_class_union_byte0();
/*	$ubyte0	=new union(Array(	"_0x80_FIN"	=>NULL	,
					"_0x40_RSV1"	=>NULL	,
					"_0x20_RSV2"	=>NULL	,
					"_0x10_RSV3"	=>NULL	,
					"_0x0f_opcode"	=>NULL	));
*/
	$ubyte0->attach($byte0);
	$byte1	=2;
/*	$ubyte1 =new union(Array("_0x80_MASK"		=>NULL
				,"_0x7f_Payload_len"	=>NULL	),$byte1)*/;
	$ubyte1=new WSF_class_union_byte1($byte1);
	$ubyte0->values();
	$ubyte1->values();	


	$u=Array(&$ubyte0,&$ubyte1);

	}




?>
