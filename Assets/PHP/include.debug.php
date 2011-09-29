<?php
/*	File		:	include.debug.php
**	Author		:	Dr. Clue
**	Description	:	Misc functions for debugging
*/


	//	Displays the function names/file/line numbers all the functions in the current call chain
function	trace_call()
		{
		$aDebug_backtrace=debug_backtrace()		;
		array_walk	(	$aDebug_backtrace	,
				function($a,$b) use ($aDebug_backtrace)
					{
					printf( "\r\n%15.15s : %5.5d - %s "	,
						((isset($aDebug_backtrace[$b+1]))?
							$aDebug_backtrace[$b+1]['function'	]."()":"...")	,
						$a['line'	]						,
						( (isset($a['file']) )?basename($a['file']):'???')		);
						}
				);	//	End	array_walk()
		}			//	End	Function
	//	Indicates if the caller is running standalone (TRUE) or is included (FALSE)
function	is_standalone()
		{
		$aDebug_backtrace	=debug_backtrace	();
		$aIncluded		=get_included_files	();
		$iOffset		=(($aDebug_backtrace[0]["file"]=="php shell code")?1:0);
		$szTrace		=$aDebug_backtrace[$iOffset]["file"];// The calling party's filename
		$bResult		=($aIncluded[0]==$szTrace);
return 		$bResult;
		}	//	End	Function	is_standalone


?>
