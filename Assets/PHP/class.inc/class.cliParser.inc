<?php
/*	File		:	class.cliParser.inc
**	Author		:	Dr. Clue
**	Description	:	This file implements a class for parsing responses from
**			Asterisk CLI commands issued through the Asterisk AMI.
**			At present this class is aimed mostly at snagging the
**			contents of tabular output.
*/
class CLIparserClass
	{
	var $szCLIResponse="";
	var $aResults	=Array(	"rows"		=>Array()	,
				"footers"	=>Array()	);
					// Contains the results of the parse.
					// as an Array with two entries.
					//
					// 1.) an array of rows, each being a namevalue array of colume values.
					// 2.) an array of footer messages following the table.
/*	Function	:	CLIparserClass()
**	Parameters	:	$aParams
**	Returns		:	None
**	Description	:
*/
function CLIparserClass($aParams)
	{
	if(isset($aParams)===FALSE)$aParams=Array();
	foreach($aParams as $key=>$value)$this->$key=$value;
	if(isset($aParams["szCLIResponse"]))$this->parse($this->szCLIResponse);
	}
/*	Function	:	row2cols()
**	Parameters	:	$aTH
**				$szLine
**	Returns		:	(Array	)
**	Description	:
*/
function row2cols($aTH,$szLine)
	{
	$aRow=Array();
	foreach($aTH as $key=>$value)
		{
		$szName		=trim(	$value[0]);
		$iLen		=$value[2];//strlen($value[0]);
		$iOffset	=$value[1];
		$aRow[$szName]	=trim(substr($szLine,$iOffset,$iLen));
		}
	return $aRow;
	}// end function
/*	Function	: parse()
**	Parameters	: (String) $szCLIResponse
**	returns		: (Array)
**	Description	: Attempts to parse the freeform text of a CLI command response
**			into a meaningfull structure.
*/
function parse($szCLIResponse)
	{
	$aLines		=explode("\n",$szCLIResponse);
	$aMatch		=Array()		;
	$bFooter	=FALSE			;
	$aTH		=Array()		;
	$x		=1			;

	dPrint(<<<EOL
Parsing CLI response
================
$szCLIResponse
================
EOL
,iCare_dPrint_CLIparse);

	if(strpos($aLines[0],"   ")===FALSE)	// If we don't see a few spaces in a row,
		{				// This is probably not a tablular result
		dPrint( "No Table Found",iCare_dPrint_CLIparse);
		$bFooter	=TRUE	;
		$x		=0	;
		}

	dPrint(print_r($aLines,TRUE),iCare_dPrint_CLIparse);

	if($bFooter	===FALSE	)
	if(preg_match_all(	'/^[A-Za-z\_]+ {2,}\/[A-Za-z\_\/]+[\.]conf/'	,
				$aLines[0],$aOut)!==FALSE)
		{
		if(count($aOut[0])>0)
			{
		dPrint( "SAW config list" .print_r($aOut,TRUE),iCare_dPrint_CLIparse);
			$szMkHeader="Module".str_repeat(" ",strpos($aLines[0],"/")-strlen("Module"))."Configuration File                                   ";
		dPrint("New Header\n$szMkHeader",iCare_dPrint_CLIparse);
			array_unshift($aLines,$szMkHeader);
			}else{
		dPrint("aOut was empty",iCare_dPrint_CLIparse);
			}
		}
						// Attempt to devine a table header.
//	if(preg_match_all(	'/(Nat|Dyn|ACL|[A-Za-z\/]+ {1}[A-Za-z\/]+|[A-Za-z\/\(\)]+)[ ]+/'	,
	if(preg_match_all(	'/(Refresh|dnsmgr|Nat|Dyn|ACL|[A-Za-z\/]+ {1}[A-Za-z\/]+|[A-Za-z\/\(\)]+)[ ]+/'	,
				$aLines[0],$aMatch,PREG_OFFSET_CAPTURE)===FALSE)
				{

				dPrint("No table detect",iCare_dPrint_CLIparse);
				$bFooter=TRUE;
//				return Array("rows"=>Array(),"footers"=>Array());
				}else{
				dPrint("table detected",iCare_dPrint_CLIparse);
				}

	$aTH		=$aMatch[0]		;
foreach($aTH as $key => $value)
	{
	$aTH[$key][2]=strlen($aTH[$key][0]);
	$aTH[$key][0]=implode("_",explode(" ",trim($aTH[$key][0])));
	}

dPrint(print_r($aTH,TRUE),iCare_dPrint_CLIparse);

	$iHeadLen	=strlen($aLines[0])	;
	$aRows		=Array()		;
	$aFooters	=Array()		;
	for(;$x<count($aLines);$x++)
		{
		$value	=$aLines[$x];
		$iDashes=strpos($value,"---");
		if($iDashes===FALSE);$iDashes=-1;


		dPrint("iDashes= $iDashes \n",iCare_dPrint_CLIparse);

		if($iDashes==0)continue;
//		if(strpos($value,"---")==0)continue;
//		$iDiff	=(abs($iHeadLen-strlen($value)));
//		$iPDiff	=round(($iDiff/$iHeadLen)*100);
		if($bFooter					===TRUE	){		$aFooters[]=$value;continue;	}
		if(preg_match("/^[-]{2,}/"	,$value,$aaa)	>0	)continue;
		if(preg_match("/^[0-9]{1,3} /"	,$value,$aaa)	>0	)$bFooter=TRUE;
		if($bFooter					===TRUE	){		$aFooters[]=$value;continue;	}
		$aRows[]=$this->row2cols($aTH,$value);
		}// end for
	$this->aResults=Array("rows"=>$aRows,"footers"=>$aFooters);
	return $this->aResults;
	}// end function
} // end class CLIparseClass

?>
