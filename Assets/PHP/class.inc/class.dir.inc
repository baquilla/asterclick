<?php
/*	File		:	class.dir.inc
**	Author		:	Dr. Clue
**	Description	:
*/


class CLASSdir
	{
	var $PATH="./*";

function	CLASSdir($oParams=Array("PATH"=>"./*"))
	{
	foreach($oParams as $key=>$value)$this->$key=$value;

	}

function	readdir($szPath=NULL)
	{
	if (is_dir($szPath))
		{
		if ($dh = opendir($szPath))
			{
			$szHREF		 =".."		;
			$szHREFtype	 ="back"	;
			while (($file = readdir($dh)) !== false)
				{
				$szHREF		=$file				;
				$szHREFpath	=$dir."/".$file			;
				$oHREFpath_info	=pathinfo($szHREFpath)		;
				$szHREFtype	=filetype($szHREFpath)		;
				$bSkip		=FALSE;
				$szHREFsize	="-"	;
				$szHREFtime	=""	;
			switch($szHREFtype)
				{
			case "dir"	:
				$szHREFtype	="folder";
				$szHREFtime	= date("d-M-Y H:i", filectime($szHREFpath));
				switch($file)
					{
				case ".."	:
				case "."	:
					$bSkip=TRUE;
	//					print "===[".$file."]===";
					break;
					}// end switch
				break;
			case "file"	:
				$szHREFtype	="unknown";
				$szHREFsize	=filesize($szHREFpath);
				$szHREFsize_say	=formatBytes($szHREFsize);
				$szHREFtime	= date("d-M-Y H:i", filectime($szHREFpath));
					switch($oHREFpath_info["extension"])
						{
						case "txt"	:$szHREFtype="text";break;
						}// end switch
					break;
				}// end switch
			if($bSkip===TRUE)continue;
	//		print "===[".$file."]===";
			}// end while
	        closedir($dh);
	}// End function
/*	Function	:	formatBytes()
** 	Parameters	:	(Int	)	$iBytes		-Number of butes.
**				(Int	)	$iPrecision	-precision (optional).
**				if no precision is set, function will determine clean
**				result automatically.
**	Returns		:	(String	)	formatted number of bytes. 
**	Description	:
*/
function formatBytes($iBytes,$iPrecision = null)
	{
	if($iBytes		==  0	)return "0";
	$units	= array("B","kB","MB","GB","TB","PB","EB","ZB","YB");
	$c	=0;
	if(	!$iPrecision 		&&
		 $iPrecision	!== 0	)
		{
		foreach($units as $k => $u)
			{
			if(($iBytes / pow(1024,$k)) >= 1)
				{
				$r["bytes"] = $iBytes / pow(1024,$k);
				$r["units"] = $u;
				$c++;
				}
			}
		return number_format($r["bytes"],2) . " " . $r["units"];
		} else {
		return number_format($iBytes / pow(1024,$iPrecision)) . " " . $units[$iPrecision];
		}// if/else
	}// end function





	} // end class CLASSdir

?>

