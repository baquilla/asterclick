<?php
/*	File		:	class.msg_queues.inc
**	Author		:	Dr. Clue
**	Description	:
**	Notes		:	If you are getting this message on your *NIX box:

Warning: msg_get_queue() [function.msg-get-queue]: failed for key 0x12345678: No space left on device in /path/to/script.php on line 1

you may use the command "ipcrm" as root to clear the message queue. Use "man ipcrm" to get more info on it.
The default setting for maximum messages in the queue is stored in /proc/sys/fs/mqueue/msg_max. To increase it to a maximum of 100 messages, just run:
echo 100 > /proc/sys/fs/mqueue/msg_max

Please ensure to follow a good programming style and close/free all your message queues before your script exits to avoid those warning messages.
*/
error_reporting(E_ALL);
//print "Included class file";


class	msgQueue
	{
var	$aStatus		=Array()	;
var	$iMsgQueueId		=109943		;

// Sendoptions
var	$bSendSerialized	=TRUE		;	// Must the transfer data be serialized ?
							// If your data is of the type that does not need serialization
							// then turn this value to FALSE and possiblly save a little 
							// space in your message que
var	$bSendBlocking		=false		;	// Block if Message could not be send (Queue full...) (true/false)
var	$iSendMsgType		=1		;	// Any Integer above 0. It signeds every Message. So you could handle multible message
							// type in one Queue.
// Receiveoptions
var	$iRecvMsgType		=1		;	// Which type of Message we want to receive ? (Here, the type is the same as the type we send,
							// but if you set this to 0 you receive the next Message in the Queue with any type.
var	$iRecvMaxSize		=6000		;	// How long is the maximal data you like to receive.
var	$iRecvOptions		=MSG_IPC_NOWAIT	;	// If there are no messages of the wanted type in the Queue continue without waiting.
							// If is set to NULL wait for a Message.
function __construct($aParams=Array())
	{
	if(function_exists("msg_get_queue")===FALSE)
		{
		die("\nYour PHP configuration needs adjustment. See: http://us2.php.net/manual/en/sem.setup.php");
		}



	$aDef=Array(		"iMsgQueueId"		=>$this->iMsgQueueId	,
				"bSendSerialized"	=>$this->bSendSerialized,
				"bSendBlocking"		=>$this->bSendBlocking	,
				"iSendMsgType"		=>$this->iSendMsgType	,
				"iRecvMsgType"		=>$this->iRecvMsgType	,
				"iRecvMaxSize"		=>$this->iRecvMaxSize	,
				"iRecvOptions"		=>MSG_IPC_NOWAIT);
	foreach($aDef		as $key=>$value)if(!isset($aParams[$key]))$aParams[$key]=$value;
	foreach($aParams	as $key=>$value)$this->$key=$value;
// Create System V Message Queue. Integer value is the number of the Queue
	$this->hQueue			= msg_get_queue($this->iMsgQueueId,0777);
	}
/*	Function	:	msg_queue_exists()
**	Parameters	:	None
**	Returns		:	Bool	-	 TRUE if queue exists
**						FALSE if queue does not exist
**	Description	:	Checks whether the message queue key exists
**			as assigned to $this->iMsgQueueId.
*/
function msg_queue_exists()
	{
	return msg_queue_exists($this->iMsgQueueId);
	}
/*	Function	:	msg_remove_queue()
**	Parameters	:	None
**	Returns		:	Bool	FALSE	There was some sort of error
**					TRUE	Success.
**	Description	:	
*/
function msg_remove_queue()
	{
	if(!$this->msg_queue_exists())return FALSE;
	return msg_remove_queue($this->hQueue);
	}
/*	Function	:	msg_set_queue()
**	Parameters	:	Array
**	Returns		:	Bool
**	Description	:
*/
function msg_set_queue($aParam=Array())
	{
	if(!$this->msg_queue_exists()) return FALSE;
	return msg_set_queue($this->hQueue,$aParams);
	}
/*	Function	:	msg_error()
**	Parameters	:	Object	-
**	Returns		:	Bool	FALSE	Always returns FALSE
**	Description	:
*/
function msg_error(&$err)
	{
//print "\nMSG_IPC_NOWAIT=".MSG_IPC_NOWAIT;//1
//print "\nMSG_EAGAIN=".MSG_EAGAIN;//11
//print "\nMSG_ENOMSG=".MSG_ENOMSG;//42
//print "\nMSG_NOERROR=".MSG_NOERROR;//2
//print "\nMSG_EXCEPT=".MSG_EXCEPT;//4
	//print "msg_error($err)";
	return FALSE;
	}
/*	Function	:	msg_send()
**	Parameters	:	Array()	message	=> [mixed] The message object.
**					
**	Returns		:	Bool
**	Description	:
*/
function msg_send($aParams=Array("message"=>"El empty message my friend"))
	{
	$aDef=Array(	"message"		=>"El empty message my friend"	,
			"iSendMessageType"	=>$this->iSendMsgType		,
			"bSendSerialized"	=>$this->bSendSerialized	,
			"bSendBlocking"		=>$this->bSendBlocking		);
	foreach($aDef as $key=>$value)if(!isset($aParams[$key]))$aParams[$key]=$value;

	if(msg_send(	$this->hQueue			,
			$aParams["iSendMessageType"	],
			$aParams["message"		],
			$aParams["bSendSerialized"	],
			$aParams["bSendBlocking"	],
			$err				)===FALSE)
			return $this->msg_error($err);
	return TRUE;
	}// End function
/*	Function	:	msg_receive()
**	Parameters	:	$aParams	=Array()
**	Returns		:
**	Description	:	will receive the first message from the specified queue of the
**			type specified by desiredmsgtype.
*/
function msg_receive($aParams=Array())
	{
	$this->msg_stat_queue($queue_status);// Check for messages
	// No messages remain in queue, so bail
	if ($this->msg_count()<1)return NULL;

	$aDef=Array(	"iRecvMsgType"		=>$this->iRecvMsgType		,
			"iRecvMaxSize"		=>$this->iRecvMaxSize		,
			"bSendSerialized"	=>$this->bSendSerialized	,
			"bSendBlocking"		=>$this->bSendBlocking		,
			"iRecvOptions"		=>$this->iRecvOptions		);
	foreach($aDef as $key=>$value)if(!isset($aParams[$key]))$aParams[$key]=$value;

	if (msg_receive(	$this->hQueue			 ,
				$aParams["iRecvMsgType"		],// Desired message type (0=any)
				$iRecvMsgTyped			 ,// Actual  message type received.
				$aParams["iRecvMaxSize"		],// Maximum message size acceptable. FAILS or trunctates depending on FLAGS.
				$message_received		 ,// The received message
				$aParams["bSendSerialized"	],// Deserialize message
				$aParams["iRecvOptions"		],// Reception FLAGS (MSG_IPC_NOWAIT|MSG_EXCEPT|MSG_NOERROR) 
				$err				)===FALSE)	return $this->msg_error($err);
//	echo "Received data".print_r($message_received,TRUE)."\n";
	return $message_received;
	}// end function
/*	Function	:	msg_stat_queue()
**	Parameters	:	&$aStatus=Array()
**	Returns		:	Array The return value is an array whose keys and values
**				have the following meanings:
**
**	Description	:
**		
**		msg_perm.uid	The uid of the owner of the queue.
**		msg_perm.gid	The gid of the owner of the queue.
**		msg_perm.mode	The file access mode of the queue.
**		msg_stime	The time that the last message was sent to the queue.
**		msg_rtime	The time that the last message was received from the queue.
**		msg_ctime	The time that the queue was last changed.
**		msg_qnum	The number of messages waiting to be read from the queue.
**		msg_qbytes	The maximum number of bytes allowed in one message queue.
**				On Linux, this value may be read and modified via /proc/sys/kernel/msgmnb.
**		msg_lspid	The pid of the process that sent the last message to the queue.
**		msg_lrpid	The pid of the process that received the last message from the queue
*/
function msg_stat_queue(&$aStatus=Array())
	{
	$aStatus=msg_stat_queue($this->hQueue);
	return $aStatus;
	}
/*	Function	:	msg_count()
**	Parameters	:	None
**	Returns		:	Int	Number of messages available
**	Description	:
*/
function msg_count($bRefresh=TRUE)
	{
	if(empty($this->aStatus)	)$bRefresh=TRUE;
	if($bRefresh===TRUE		)$this->msg_stat_queue($this->aStatus);
	return $this->aStatus['msg_qnum'];
	}
}// end class msg_queue
?>

