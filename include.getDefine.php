<?php
/*	File		:	include.getDefine.php
**	Author		:	Dr. Clue
**	Description	:
*/

function getDefineName(		$iValue)
	{
	$aConstantsPHP	=			get_defined_constants(	true);
	$aConstantsUSER	=			$aConstantsPHP[		'user']			;
	if(is_string(		$iValue))return	$aConstantsPHP[		'user'][$iValue]	;
	return array_search(	$iValue,	$aConstantsUSER					);
	}
?>
