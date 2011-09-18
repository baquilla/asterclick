<?php
/*	File		:	class.shm.inc
**	Author		:	Dr. Clue (drclue AT drclue DOT net)
**	Description	:	class for working with shared memory.
**			This class is fairly handy in that once declared
**			one can express $instance->anythingYouLike = whatever
**			and it will map names over values.

	When declaring an instance of this class , one can pass a name/value array
	which will be used to populate the initial settings, thus overriding the defaults.

	$shm_permissions	=0666		;	// Permissions on memory.
	$shm_size		=2048		;	// Initial size of shared memory
	$shm_key		=4096		;	// Unique key to shared memory resource
	$shm_handle		=NULL		;	// Handle to shared memory resource.
	$shm_bSticky		=FALSE		;	// If TRUE the destructor does not detach.

	Some of the key functions are described here.

	function shm_get_var($name,$bCache=FALSE)
		Get the value of the named variable, and if $bCache is set
		read the cached value instead of the shared memory value.

	function shm_put_var($name,$value=NULL)
		Set the variable named to the [mixed] value , or NULL if the value is not provided.

	function shm_remove_var($name)
		Remove the named value.

	function shm_has_var($name)
		Checks if the named variable exists.
**
*/

//***
//*** Configure environment
//***
error_reporting(	E_ALL	);	// We like error messages , bring em on.




if(function_exists("shm_has_var")===FALSE)
	{
	$aMaps	= Array();
function shm_has_var($iSeg,$iVar)
		{
		global	$aMaps			;
		$szSeg		="shm_".$iSeg;
		$bIsSet		=FALSE			;
		$aMaps[$szSeg]	=@shm_get_var($iSeg,0)	;
		if(!is_array($aMaps[$szSeg]))
				{
				$aMaps[$szSeg]=Array()	;
				shm_put_var($iSeg,0,$aMaps[$szSeg]);
				}
		$bIsSet=in_array($iVar,$aMaps[$szSeg])	;
		return $bIsSet;
		}
	}// end if 




class shm
	{
	var $shm_permissions	=0666		;	// Permissions on memory.
	var $shm_size		=1000000	;	// Initial size of shared memory
	var $shm_key		=4096		;	// Unique key to shared memory resource
	var $shm_handle		=NULL		;	// Handle to shared memory resource.
	var $aData		=Array()	;	// Local copy of data.
	var $aMap		=NULL		;	// Map of data names.
	var $shm_bSticky	=FALSE		;	// If TRUE the destructor does not detach.
/***
****	MAGIC METHODS STARTS
***/
/*	Function	:	CONSTRUCTOR
**	Parameters	:	Array()		- An array of name / value pairs to 
**					override the defaults.
**	Returns		:	None.
**	Description	:
*/
function shm($aParams=Array())
	{
	if(function_exists("shm_attach")===FALSE)
		{
		die("\nYour PHP configuration needs adjustment. See: http://us2.php.net/manual/en/shmop.setup.php");
		}
	if(function_exists("shm_has_var")===FALSE)
		{
		die("\nYour PHP version does not support the needed function shm_has_var");
		}


	$aDef=Array("shm_key"=>4096);
	foreach($aDef	 as $key=>$value)
			if(!isset($aParams[$key]))
					$aParams[$key]=$aDef[$key];
	foreach($aParams as $key=>$value)$this->$key=$value;
	$this->shm_attach($this->shm_key,$this->shm_size);
	}
/*	Function	:	__destruct()
**	Parameters	:	None
**	Returns		:	None
**	Description	:	Detaches , unless the $shm_bSticky member variable is set.
*/
function __destruct()
	{
	if($this->shm_bSticky===TRUE)return;
	$this->shm_detach();
	}
/*	Function	:	__set()
**	Parameters	:	$name	- [string] Desired variable name
**				$value	- [mixed] Desired value 
**	Returns		:	None
**	Description	:	Sets/creates the named variable in shared memory
**				using $instance->variablename = value syntax.
*/
public function __set($name, $value)
	{
        $this->shm_put_var($name,$value);
	}
/*	Function	:	__unset()
**	Parameters	:	$name [string] - name of variable 
**	Returns		:	None
**	Description	:	Deletes the named variable from memory.
*/
public function __unset($name)
	{
        $this->shm_remove_var($name);
	}
/*	Function	:	__get()
**	Parameters	:	$name [STRING] - name of variable
**	Returns		:	[mixed] value the stored variable or NULL.
**	Description	:	Returns variables reference as $instance->varname
*/
public function __get($name)
	{
	return  $this->shm_get_var($name);
	}
/***
****	/MAGIC METHODS ENDS
***/


/*	Function	:	shm_get_key()
**	Parameters	:	(String		)	$name		-Name of variable to get int key for.
**				(Boolean	)	$bCreate[TRUE]	-Create variable if not present
**	Returns		:	(Int		)	- shared memory key for variable
**	Description	:
*/
function shm_get_key($name,$bCreate=TRUE)
	{
	if($this->shm_handle==NULL)$this->shm_attach()	;
	if($this->shm_handle==NULL)return -1		;
	// If we already have the key , return it.
	if(isset($this->aMap[$name]))return $this->aMap[$name];
	// If there is an existing map , get it/ else create it.
	if(  shm_has_var($this->shm_handle,0		))
		{
		$this->aMap=shm_get_var($this->shm_handle,0);
		}
	else 
		{
		shm_put_var($this->shm_handle,0,($this->aMap=Array("aMap"=>0)));
		}
	// If we already have the key , return it.
	if(isset($this->aMap[$name]))return intval($this->aMap[$name]);
	// If we don't want to auto create bail -1
	if($bCreate==FALSE)return -1;
	//add new variable name/key to map
	$this->aMap[$name]=count($this->aMap);
	//place map in shared memory
	shm_put_var($this->shm_handle,0			,$this->aMap	);
	//place new variable name in shared memory with initial NULL value.
	shm_put_var($this->shm_handle,$this->aMap[$name],NULL		);
	// Return the int value key for the variable name.
	return (intval($this->aMap[$name]));
	}
/*	Function	:	shm_attach()
**	Parameters	:	None
**	Returns		:	shared memory handle
**	Description	:
*/
function shm_attach()
	{
	if($this->shm_handle!=NULL)$this->shm_detach();
	$this->shm_handle=shm_attach(intval($this->shm_key),$this->shm_size,0666);
	if(shm_has_var($this->shm_handle,0))
		{
		$this->aMap=shm_get_var($this->shm_handle,0);
		}
	return ($this->shm_handle);
	}
/*	Function	:	shm_detach()
**	Parameters	:	None
**	Returns		:	BOOL
**	Description	:
*/
function shm_detach()
	{
	if($this->shm_handle==NULL)return NULL			;
	$result			=shm_detach($this->shm_handle)	;
	$this->shm_handle	=NULL				;
	return $result;
	}
/*	Function	:	shm_get_var()
**	Parameters	:	$name	- Name of variable to fetch
**				$bCache	- Read from cache if available.
**	Returns		:	Variable value or NULL.
**	Description	:
*/
function shm_get_var($name=NULL,$bCache=FALSE)
	{
	$iKey=-1;
	if($name==NULL				)return NULL;
	if(($iKey=$this->shm_get_key($name))==-1)return NULL;
	if(!isset($aData[$name])		)$bCache=FALSE;
	if($bCache===FALSE			)
		{
		if(!shm_has_var($this->shm_handle,$iKey))
			{
			print "\nSHM $name not found as $iKey in get_var()";
			$aData[$name]=NULL;return $aData[$name];
			}

		try	{
			$aData[$name]=shm_get_var($this->shm_handle,$iKey);
			}catch(Exception $E)

			{
			print "\nshm_get_var($name | $iKey ) failed ".print_r($E,TRUE);
			$aData[$name]=NULL;
			}

		}
	return $aData[$name];
	}
/*	Function	:	shm_has_var()
**	Parameters	:	$name	- Name of variable to check for.
**	Returns		:	Bool
**	Description	:
*/
function shm_has_var($name)
	{
	if(($iKey=$this->shm_get_key($name,FALSE))==-1)return FALSE;
	return TRUE;
	}
/*	Function	:	shm_put_var()
**	Parameters	:	$name
**				$value
**	Returns		:	Bool
**	Description	:
*/
function shm_put_var($name,$value=NULL)
	{
	if(($iKey=$this->shm_get_key($name,TRUE ))==-1)return FALSE;
	return shm_put_var($this->shm_handle,$iKey,$value);
	}
/*	Function	:	shm_remove_var()
**	Parameters	:	$name	- Name of var.
**	Returns		:	Bool
**	Description	:
*/
function shm_remove_var($name)
	{
	if(($iKey=$this->shm_get_key($name,FALSE))==-1)return TRUE;
	unset($this->aMap[$name]);
	return shm_remove_var($this->shm_handle,$iKey);
	}
/*	Function	:	shm_list()
**	Parameters	:	None
**	Returns		:	Array() - Name/Value Array
**	Description	:
*/
function shm_list($bCache=FALSE)
	{
	$aList=Array();
	if($bCache===TRUE)return $this->aData;
	if($this->aMap!=NULL)
	foreach($this->aMap as $key=>$value)
		{
		if($key=="aMap")continue;
		$aList[$key]=$this->shm_get_var($key);
		}
	return $aList;
	}
/*	Function	:	shm_remove()
**	Parameters	:	None
**	Returns		:	Bool
**	Description	:	Destroys shared memory and clears both mappings and cached data.
*/
function shm_remove()
	{
	if($this->shm_handle==NULL)$this->shm_attach()		;
	if($this->shm_handle==NULL)return NULL			;
	$result			=shm_remove($this->shm_handle)	;
	$this->shm_handle	=NULL				;
	$this->aData		=NULL				;
	$this->aMap		=NULL				;
	return $result;
	}
/*	Function	:	kill()
**	Parameters	:	None
**	Returns		:	Bool
**	Description	:	Destroy the shared memory
**			Unlike the shm_remove, this one leaves internal mappings intact.
*/
function kill()
	{
	@shm_remove($this->shm_handle);
	}

	}// /class shm

?>
