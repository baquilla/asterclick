<?php
/*	File		:	class.ini.ini
**	Author		:	Dr. Clue	(A.K.A. Ian A. Storms)
**	Description	:	This PHP file supports the implementation of
**	NOTES		:
**	URLS		:


/(?:\[?([^\]=]+).(.*))/
/(?:\[?([^\]=]+).(.*)?)/
Splits either "[somename]" or "somename=something"

/(?:\[?([^\]=]+)((?:\])|(?:=)))([^;]+)?(;)?(.+?)?/
/(?:\[?([^\]=]+)((?:\])|(?:=)))[\s]{0,}?(;|[^;]+)?(.+)?/
/(?:\[?([^\]=;]{0,})((?:\])|(?:=)|(?:;)))[\s]{0,}?(;|[^;]+)?(.+)?/
Splits either ("[somename]" | "somename=something" ) ([+ ;* comment])



String(" [test]" ).match(/[\s]{0,}((?:\[)?([^\]=]+)([=\]])(.{0,}))/i)
String(" test=7;").match(/[\s]{0,}((?:\[)?([^\]=;]+)([=\];])(.{0,}))/i)
*/


class CLASSini
	{
function CLASSini($aParams=Array("filename"=>""))
	{
	if(isset($aParams["FILE"]	)){$aParams["filename"]=$aParams["FILE"];unset($aParams["FILE"]);}
	$this->szINI		="";
	if(isset($aParams["filename"]	))$this->szINI=file_get_contents($aParams["filename"]);
	$this->aINI		=explode("\n",$this->szINI);
	$this->aaINI		=Array("filename"=>pathinfo($aParams["filename"]));
	$aaINIcomment		=Array("filename"=>pathinfo($aParams["filename"]));
	$szCurrent		=NULL		;
	$szCurrentComment	="_globals"	;
	$szVarName		=""		;
	foreach($this->aINI as $key =>$value)
		{
		$szRegExp	="/(?:\[?([^\]=;]{0,})((?:\])|(?:=)|(?:;)))[\s]{0,}?(;|[^;]+)?(.+)?/";
		$aPregMatchCNT	=preg_match($szRegExp,trim($value),$aPregMatch)	;
		$szComment	=""						;
		if($aPregMatchCNT==0)continue					;
		switch($aPregMatch[2])// Character that followed the variable/group label
			{
		case	";"://	print "\nNEW Comment (;${aPregMatch[3]})"				;
				if(isset($aPregMatch[3]))
				$szComment		=";".trim($aPregMatch[3],";");
				break;
		case	"=":	$szSetValue		=trim($aPregMatch[3])				;
				$szVarName		=trim($aPregMatch[1])				;
			//	print "\nNEW Variable (".$szVarName.")=$szSetValue "			;
				$this->setIniValue	($szCurrent,$szVarName,$szSetValue)		;
				$szCurrentComment	=$szVarName					;
				if(isset($aPregMatch[4]))$szComment		=";".trim($aPregMatch[4],";")			;
				break;
		case	"]":	$groupName		=$aPregMatch[1]					;
			//	print "\nNEW Group (".$groupName.")"					;
				$szCurrent		=$szCurrentComment	=$groupName		;
				if(isset($aPregMatch[4]))$szComment		=";".trim($aPregMatch[4],";")			;
				break;
//		default	:
//			print_r($aPregMatch);
			}// Emd Switch
		if(!isset(	$this->aaINIcomment[$szCurrent][$szCurrentComment]	))
				{
				$this->aaINIcomment[$szCurrent][$szCurrentComment]=Array();
				}

//		$this->aaINIcomment[$groupName][$szCurrentComment][]	=$szComment;

	}//End For Each
	foreach($this->aaINI as $key=>$value)
		{
//		print "\n[$key]";
		if(isset(	$this->aaINIcomment[$key][$key]))
		if(is_array(	$this->aaINIcomment[$key][$key]))
				{
			//	print "\t\t\t".implode("\n",$this->aaINIcomment[$key][$key]);
				}
		foreach($this->aaINI[$key] as $keyVar=>$valueVar)
			{
			//print "\n\t$keyVar\t\t=$valueVar";
			if(isset($this->aaINIcomment[$key][$keyVar]))
				{
			//	print "\t".implode("\n",$this->aaINIcomment[$key][$keyVar]);
				}
			}
		}
//print "\n=================";
//print_r($this->aaINIcomment);
//print_r($this->aaINI);
	}//End Function
function getArray()
	{
	return $this->aaINI;
	}
function getIniValue($szSection,$szName,$szDefault=NULL)
	{
	if(!isset($szDefault				))$szDefault=NULL	;
	if(!isset($this->aaINI[$szSection]		))return NULL		;
	if(!isset($this->aaINI[$szSection][$szName]	))return NULL		;
	return $this->aaINI[$szSection][$szName]				;
	}
function setIniValue($szSection,$szName,$szValue)
	{
	$this->aaINI[$szSection][$szName]=$szValue	;
//	if(!isset($szDefault				))$szDefault=NULL	;
//	if(!isset($this->aaINI[$szSection]		))return NULL		;
//	if(!isset($this->aaINI[$szSection][$szName]	))return NULL		;
//	return $this->aaINI[$szSection][$szName]				;
	}

};// End class CLASSini


?>
