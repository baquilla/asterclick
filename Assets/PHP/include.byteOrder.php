<?php
/*	File		:	include.byteOrder.php
**	Author		:	Dr. Clue
**	Description	:
**	Urls		:	http://bytes.com/topic/php/answers/500026-binary-manipulation-floating-point-number
**				http://www.codeproject.com/KB/cpp/endianness.aspx
**	Notes		:
//		$a=unpack("N",pack("V",0xAABBCCDD));
//		print_r(unpack('Lhigh/Llow', pack('d',12.015)));
//		$x = unpack('L2long'	, pack('d',999999.999999));
//		$z =array_merge($x,$y);
//		print_r($z);
//		list($high,$low,$dval)=$z;
//		printf("\n %x %x %x \n",$high,$low,$dval);
*/

	
	function numberHexWidth		($num		){	return (int)	(log($num	,0xF+1		)+1);}//	Width of number in hex digits
	function numberHexWidth_AND	($numWidth	){	return		(pow(0xF+1	,$numWidth	)-1);}//	AND hex bounding value 
	//	Toggle the byte order.
	function flipByteOrder		($num		)
		{
		$numWidth	=	numberHexWidth		($num		)	;
		$numAND		=	numberHexWidth_AND	($numWidth	)	;
		$result		=0							;
		if(is_double($num))$numWidth=16;
		switch($numWidth)// width in hex digits
			{
		case	16	:	$x=array_reverse(array_merge(	unpack('C*'	, pack('d'	,	$num			)	)	));
					$y=array_merge(			unpack('d'	, pack('C*'	,	$x[0],$x[1],$x[2],$x[3]	,
														$x[4],$x[5],$x[6],$x[7]	)	)	);
					return $y[0];
		case	8	:	$result=	(	(($num&0x000000FF)<<24)	+(($num&0x0000FF00)<<8	)+
								(($num&0x00FF0000)>>8)	+(($num&0xFF000000)>>24))	&$numAND;break;	//	long	32 bits 4 bytes
		case	4	:	$result=	(	(($num>> 8))		| ($num << 8))			&$numAND;break;	//	int	16 bits 2 bytes
			}
		return $result	&$numAND;
		}




?>
