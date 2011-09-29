<?php
/*	File		:	class.circularBuffer.inc
**	Author		:	Dr. Clue
**	Description	:
**	URLS		:	http://zerozaku.com/viewtopic.php?f=57&t=6717
*/
//require_once("include.debug.php");


class	circularBuffer
	{
var	$buffer		=Array()		;//new Array(nodeLength)	,
var	$head		=0			;
var	$tail		=0			;
var	$nodeLength	=0			;

function	circularBuffer	($nodeLength	)	///	_CONSTRUCT
	{
	$this->	bufferConfig($nodeLength);
	}

function	bufferConfig	($nodeLength	)
	{			
	$this->buffer	= Array()		;			//	Initialize private data fields
	$this->head	= 0			;
        $this->tail	= 0			;
	$this->nodeLength	=$nodeLength	;
				
	for($j = 0; $j < $this->nodeLength; $j++)			//	Set default data so that undefined is not returned
		{
		$this->buffer[$j] = null;
		}
	}	//	End	Function	bufferConfig()

function	put		($node		)	///	@param node the node inserted into the queue
	{
	$this->buffer[$this->tail++] = $node;
	$this->tail %= $this->nodeLength;				//	Set tail pointer to 0 when at the end of the array
	}	
function	get		(		)	///	@return node the current node where head is set to
	{
	$node	= null;
	if (!$this->is_empty())						//	Check if empty before returning nodes
		{
		$node = $this->buffer[	$this->head];
		$this->buffer[		$this->head++] = null;		//	Destroy the processed element
		}
	$this->head %= $this->nodeLength;				//	Set head pointer to 0 when at the end of the array
	return $node;
	}
function	peek		(		)	/// @return node the current node where head points to
	{
	return $this->buffer[$this->head];
	}
function	is_empty	(		)	/// @return boolean check if the queue is empty
	{
	return $this->head === $this->tail;
	}
}		//	End	Class	circularBuffer




if (			is_standalone()===TRUE)	//	Test Harness code
	{
	$cb=new circularBuffer(5);
	}







?>
