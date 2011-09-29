/*	File		:	AsterClick_wSockets.class.js
**	Author		:	Dr. Clue	( A.K.A. Ian A. Storms )
**	Description	:	This javascript file supports the 
**		implementation of socket based communications between 
**		HTML5 and the AsterClick backend.
**	NOTES		:
**	URLS		:
**
*/

/*	CONSTRUCTOR	:	wSocket()
**	Parameters	:	Object	oParam	
**						String	oParam.host 		= the host name to connect to.		(e.g. "localhost"	)
**						String	oParam.port 		= the port to use for the connection	(e.g. "18345"		)
**						String	oParam.path		= A pseudo path, that at one point will be used for security.
**						Bool	oParam.bNoConnect	= if true, the connection to the server is deferred untill
**										  either a call to wSocketsConnect() or untill one attempts
**										  to send data.
**	Returns		:	None
**	Description	:
*/
function wSocket(oParams)
	{
	if(typeof oParams=="undefined")oParams={}
	// Create unique ID based on time.
	this.szWSOCKETinstance			="WSOCKET_base"+(new Date()-0)	;
	wSocket.prototype.oWSOCKETinstances[	this.szWSOCKETinstance]	=this	;
	// Assign the length of the instance array to instance.
	this.iWSOCKETinstance						=wSocket.prototype.iWSOCKETinstance=wSocket.prototype.aWSOCKETinstance.length;
	wSocket.prototype.aWSOCKETinstance[	this.iWSOCKETinstance]	=this	;
	//Create a string for [ new Function(...,...,fnbody) declarations.
	this.szWSOCKETpath		="wSocket.prototype.oWSOCKETinstances."+this.szWSOCKETinstance
	// Used to creat an in memory stylesheet used for converting XML objects to strings.
	this.oXSL_XML2TEXT		=this.StringToXML(this.szXSL_XML2TEXT);
	for(i in oParams)this[i]	=oParams[i];				 // Assign the passed params 
	this.wSocketsInit();							 // Inititalize the socket connection process
	if(this.bNoConnect		==false		)this.setWinterval(true);// If auto connection has not been supressed , connect
	}



wSocket.prototype			= new Function				;
wSocket.prototype.constructor		=wSocket				;
wSocket.prototype.DOMParser		=new DOMParser()			;
// Place to store event listeners addEventListener / removeEventListener
wSocket.prototype.oEventListeners	={}					;
/**
*** These three items are for instance tracking so that event handlers and the 
*** like can have a way of finding the wSocket instance that spawned events.
**/
wSocket.prototype.iWSOCKETinstance	=-1					;// A ordinal value representing the position
										 // in the aWSOCKET array where an instance reference is recorded
										 // at the time a wSocket instance is declared. 
wSocket.prototype.aWSOCKETinstance	=[]					;// Array of wSocket declaration references that each wSocket
										 // reference registers with during the execution of it's constructor
wSocket.prototype.oWSOCKETinstances	={}					;// An object containing named instances of wSockets.
/**
*** Connection related variables relating to the WebSocket itself and the host and port 
*** the connection is made to. Below are assigned the default values for host and port but they 
*** can be altered by passingan object to the wSocket constructor with member variables of the same name.
**/
wSocket.prototype.socket		=null					;// WebSocket , over which all communications occur.
wSocket.prototype.host			="127.0.0.1"				;// the hostname to connect to.
wSocket.prototype.port			="150"					;// The port being connected to.
wSocket.prototype.path			="/wSocket/wSockets.php"		;// The path being connected to.
wSocket.prototype.bNoConnect		=true					;// Tells the library not to autoconnect at startup.
										 // but rather to wait for the first send request
										 // or a manual call to wSocketsConnet.
wSocket.prototype.bInConnect		=false					;// Indicates one is in the process of connecting.
wSocket.prototype.aOutputBuffer		=[]					;// Buffer checked by interval function whose 
										 // contents get checked and sent when a socket is available.
										 // This buffer works on a FIFO basis.
wSocket.prototype.aInputBuffer		=[]					;
wSocket.prototype.IDinterval		=null					;// Resource id for setInterval function
wSocket.prototype.aszReadyState		=[	"Connecting"			,// An Array of human readable connection states
						"Open"				,// equating to the WebSocket readyState values 0-3
						"Closed"			,
						"Closing"			];
wSocket.prototype.lHeartBeatLast	=0					;// Used to send heartbeat event for things
										 // like updating UI driven call timers.
wSocket.prototype.lHeartBeatInterval	=1000			;		 //10000
/*	Function	:	wSocketsIsConnected()
**	Parameters	:	None
**	Returns		:	(Boolean	)
**	Description	:
*/
wSocket.prototype.wSocketsIsConnected =function()
		{
		if(this.socket			==null	)return false;
		if(this.socket.readyState	!=1	)return false;
		return true
		}
/*	Function	:	wSocketsInit()
**	Parameters	:	None
**	Returns		:	None
**	Description	:	This is a place holder function that you override in your derrived class.
**				This function is called only once in the constructor and the call is made
**				before any socket connection is established (if any, depending on bNoConnect.).
**				
**				This allows you to do any pre-connection onetime setup type of stuff.
*/
wSocket.prototype.wSocketsInit		=function()	{	}
/*	Function	:	wSocketsNotify()
**	Parameters	:	String szState		- Contains one of the following values
**							disconnected
**							connecting
**							negotiating
**							connected
**							closed
**
**	Returns		:	None
**	Description	:	This function is a place-holder function that you override in your derrived class
**				and lets you know that a change in state has occured in the connection.
*/
wSocket.prototype.wSocketsNotify	=function(szState)	{
//	console.debug("wSocketNotify="+szState)
	}
/*	Function	:	wSocketsNotifyError()
**	Parameters	:	(String		)	szMessage	The contents of the message is typically a JSONized string
**	Returns		:	None
**	Description	:	This function is a place-holder function that you override in your derrived class
**				and lets you know that an error has occured. In the case of an error, the socket will be destroyed
**				and interval processing canceled.
*/
wSocket.prototype.wSocketsNotifyError	=function(szMessage)	{
//	console.debug("wSocketNotifyError="+szState)
	}
/*	Function	:	wSocketsReceiveString()
**	Parameters	:	(String		)	szXML
**	Returns		:	None
**	Description	:	This function is a place-holder function that you override in your derrived class
**				and lets you receive the server response as a (String)
*/
wSocket.prototype.wSocketsReceiveString=function(szXML)		{	}
/*	Function	:	wSocketsReceiveXML()
**	Parameters	:	(Document	)	oXML
**	Returns		:	None
**	Description	:	This function is a place-holder function that you override in your derrived class
**				and lets you receive the server response as a (XML Object)
*/
wSocket.prototype.wSocketsReceiveXML	=function(oXML)		{	}
/*	Function	:	wSocketsSentString()
**	Parameters	:	(Document	)	oXML
**	Returns		:	None
**	Description	:	This function is a place-holder function that you override in your derrived class
**				and lets you receive a copy of what the sent commands are (XML Object)
*/
wSocket.prototype.wSocketsSentString	=function(szXML)	{	}
/*	Function	:	wSocketsSentXML()
**	Parameters	:	(Document	)	oXML
**	Returns		:	None
**	Description	:	This function is a place-holder function that you override in your derrived class
**				and lets you receive a copy of what the sent commands are (XML Object)
*/
wSocket.prototype.wSocketsSentXML	=function(oXML)		{	}
/*	Function	:	wSocketsDisconnect()
**	Parameters	:	None
**	Returns		:	None
**	Description	:
*/
wSocket.prototype.wSocketsDisconnect	=function()
	{
	this.setWinterval(false);
	if(this.socket				!=null	)	//If we have a socket instance
		if(this.socket.readyState	!=2	)	//If it is not already closing
			this.socket.close()		;	//Close the socket
	this.socket	=null				;	//Reset the socket to null
//	console.debug("wSocketDisconnect")
	this.wSocketsNotify("disconnected")		;	//Let the world know.
	}
/*	Function	:	wSocketsError()
**	Parameters	:	szMessage	
**	Returns		:	None
**	Description	:	Some sort of socket wrror occurred
*/
wSocket.prototype.wSocketsError=function(szMessage)
	{
	this.socket		=null		;// Destroy the socket
	this.setWinterval(false)		;// Cancel interval processing
//	console.debug("wSocketError="+szMessage)
	this.wSocketsNotifyError(szMessage)	;// Notify of error
	return false;
	}
/*	Function	:	wSocketsEventConnect()
**	Parameters	:	None
**	Returns		:	None
**	Description	:	Sends a pseudo event every time a wSocket is established.
*/
wSocket.prototype.wSocketsEventConnect=function()
	{
var	oEventConnect		=this.ObjectToXML({starttime:{time:(new Date()-0)	}},"event")	;
	oEventConnect.firstChild.setAttribute("name","asterclick_connect")				;
	this.wSocketsReceive({data:this.XMLToString(oEventConnect).split(">").join(">\n")})	;
	}
/*	Function	:	wSocketsEventHeartbeat()
**	Parameters	:	None
**	Returns		:	None
**	Description	:	Sends a pseudo event every time a wSocket is established.
*/
wSocket.prototype.wSocketsEventHeartbeat=function()
	{
var	oEventHeartbeat		=this.ObjectToXML({starttime:{time:(new Date()-0)	}},"event")	;
	oEventHeartbeat.firstChild.setAttribute("name","asterclick_heartbeat")				;
	this.wSocketsReceive({data:this.XMLToString(oEventHeartbeat).split(">").join(">\n")})	;
	}
/*	Function	:	wSocketsConnect()
**	Parameters	:	None
**	Returns		:	None
**	Description	:	Connect to the server. Connects to the server , establishes event handlers for
**				the WebSocket
*/
wSocket.prototype.wSocketsConnect=function(oParams)
	{
	if(	this.bInConnect==true)return		;	// We are already in the process of connecting , so bail.
	this.bInConnect		=true			;	// Set a gate flag so that no other connect attempts overwrite this one.
	if(typeof oParams=="undefined")var oParams={}
	for(i in {host:"",port:"",path:""})if(typeof oParams[i]!="undefined")this[i]=oParams[i];
	if(this.path.indexOf("/")!=0)this.path="/"+this.path
//	window.onerror=new Function("evt","console.debug('Window.onerror');console.dir(evt);return true;")
//	document.onerror=new Function("evt","console.debug('Document.onerror');console.dir(evt);return true;")
	this.wSocketsNotify("disconnected")				;// Show connection state as disconnected.
	this.hostURL		= "ws://"	+	this.host
						+":"+	this.port
						+	this.path	;//"/websocket/server.php";
	try	{
		this.wSocketsNotify("createSocket")	;
//		console.debug("new WebSocket("+this.hostURL+")");//console.debug("wSocket(create)")
		this.socket		= new WebSocket(this.hostURL);
		this.socket.onerror	= new Function("msg",//	"console.debug('wSocket(onerror)'+msg);"		+
								this.szWSOCKETpath+".wSocketsError(msg);"	);
		this.socket.onopen	= new Function("msg",//	"console.debug('wSocket(onopen)');"		+
								this.szWSOCKETpath+".wSocketsEventConnect(		);"	+
								this.szWSOCKETpath+".wSocketsNotify('connected'	);");
		this.socket.onmessage	= new Function("msg",//	"console.debug('wSocket(onmessage)');console.dir(msg);"+
								this.szWSOCKETpath+".wSocketsReceive(msg);"	);
		this.socket.onclose	= new Function("msg",//	"console.debug('wSocket(onclose)');console.dir(msg);"+
								this.szWSOCKETpath+".wSocketsNotify('closed');"+
								"AsterClick_killEvent(msg);return false;"	);
//		console.dir(this.socket)
//		console.debug("wSocket.wSocketsNotify('Connecting')");
		this.wSocketsNotify("connecting")	;
		this.setWinterval(true)			;
		}catch(ex)
		{
		document.body.style.backgroundColor="#ffffcc";
		this.setWinterval(false);
		}
//		this.socket.onerror	= new Function("msg","console.debug('wSocket(error)'+msg);"		+	this.szWSOCKETpath+".wSocketsError(	msg		);"	);
	this.bInConnect		=false			;
	}
/*	Function	:	fetchXML()
**	Parameters	:	szURL
**	Returns		:	(Document	)	XML fetched or null
**	Description	:	Fetches an XML file.
*/
wSocket.prototype.fetchXML=function(szURL)
	{
var	req		= new XMLHttpRequest()		;
	req.open('GET', szURL, false	)		;
	req.send(null			)		;
	if(req.status	== 200		)return wSocket.prototype.StringToXML(req.responseText)
	return null					;
	}
/*	Function	:	StringToXML()
**	Parameters	:	String	szXML		- A string representation of some XML.
**	Returns		:	XMLObject
**	Description	:	Takes the (String) representation of XML and creates an actual XML object from it.
*/
wSocket.prototype.StringToXML=function(szXML)
	{
	return this.DOMParser.parseFromString(String(szXML), "text/xml"); 
	}
/*	Function	:	XSLtransform
**	Parameters	:	(Document	)	oXML
**				(Document	)	oXSL
**	Returns		:	(String		)	String of HTML code
**	Description	:
*/
wSocket.prototype.XSLtransform=function(oXML,oXSL)
	{
var	oXSLT		=new XSLTProcessor()			;
	oXSLT.importStylesheet(oXSL)				;
var	oResult		=oXSLT.transformToDocument(oXML)	;
	return oResult.getElementsByTagName("body")[0].innerHTML;
	}
/*	Function	:	XMLToString()
**	Parameters	:	XMLObject	oXML	-This is the XML object to be converted to a string.
**	Description	:	Since wSockets deals with string data and some developers may wish to send real
**				XML objects, this function translates XML objects into strings.
**
**	NOTE:	This function may be a little flakey and I intend to replace it with a transform.
*/
wSocket.prototype.XMLToString=function(oXML)
	{
var	szResult	="<?xml version=\"1.0\" ?>\n"+ (new XMLSerializer()).serializeToString(oXML);
	return szResult;
	}
/*	Function	:	ObjectToXML()
**	Parameters	:	(Object		)	oOBJECT -A JSON object.
**				(oNode		)	oNode	-The current Node (if any)	
**	Returns		:	(oNode		)
**	Description	:	This function converts a javascript object into an XML object.
**
**				for each member of the object.....
**			
**				If the member is an object , a new child node is created with the 
**				nodeName equal to the member name,  and recursion takes place.
**				
**				If the member name is "__nodeValue" , a textNode is appended to the 
**				current node with the value of the member.
**
**				If the member name is "__CDATA"	, a CDATASection is appended to the 
**				current node with the value of the member.
**
**				If the member name is "__Comment"	, a Comment is appended to the 
**				current node with the value of the member.
**
**				All other discrete variable types casue the creation of an attribute with the member name and member value.
*/
wSocket.prototype.ObjectToXML=function(oOBJECT,oNode)
	{
var	oReturn			=null		;
var	szNodeName		="root"		;

	if(	typeof oNode	=="string"	){szNodeName=oNode;oNode=null;}
	if(	typeof oNode	=="undefined"	||
		oNode		==null		)oNode=self.document.implementation.createDocument("",szNodeName,null);

	oReturn			=oNode		;

	if(oNode.nodeName=="#document")
		{
		oNode=oNode.firstChild;
		}

	for(i in oOBJECT)
		{
var		oVar=oOBJECT[i];
		switch(i)
			{
		case "__nodeValue":
			var oText = oNode.ownerDocument.createTextNode(oVar)
			oNode.appendChild(oText);//nodeValue="DoggyStyle";//oVar
			break;
		case "__CDATA":
			var oCDATA = oNode.ownerDocument.createCDATASection(oVar)
			oNode.appendChild(oCDATA);//nodeValue="DoggyStyle";//oVar
			break;
		case "__Comment":
			var oComment = oNode.ownerDocument.createComment(oVar)
			oNode.appendChild(oComment);//nodeValue="DoggyStyle";//oVar
			break;
		default	:

var		szI=i.split("/").join("_fs_").split("-").join("_dash_").split("<").join("_lt_").split(">").join("_gt_").split("&").join("_amp_").split(";").join("_sc_")
			switch(typeof oVar)
				{
			case	"boolean"	:oVar=(oVar==true)?"true":"false";
			case	"string"	:
			case	"number"	:oNode.setAttribute(szI,oVar);break;
			case	"object"	:this.ObjectToXML(oVar,oNode.appendChild(oNode.ownerDocument.createElement(szI)));break;
			case	"function"	:
			case	"undefined"	:break;
			default			:
			$("oDebug").innerHTML+="OBJECTToXML() UnKnownType ("+(typeof oVar)+")"
				}// end switch
			}// end switch
		}// for( i in oOBJECT
	return 	oReturn;
	}
/*	Function	:	wSocketsReceive()
**	Parameters	:	Object	oMessage
**	Returns		:	None
**	Description	:	When data is received from the socket, this function sends that
**				data as both a string and as XML to two virtual functions so 
**				that the user can decide which function to override and as a result
**				which form of the data to use.
*/
wSocket.prototype.wSocketsReceive=function(oMessage)
	{
var	szData							=oMessage.data+"\n"		;
var	oXML							=this.StringToXML(szData)	;
	if(oXML.getElementsByTagName("parsererror").length	>0	)return			;// If the XML failed to parse , ditch it.
	if(this.callEventListeners(oXML)			==false	)return			;
	this.wSocketsReceiveString(	oMessage.data	)					;
	this.wSocketsReceiveXML(	oXML		)					;//send XML data
	}
/*	Function	:	wSocketsInterval()
**	Parameters	:	None
**	Returns		:	None
**	Description	:	This function is called as a result of a setInterval function 
**				which eabled and disabled bvia the setWinterval method.
**				This function checks the connection state (connecting if needed)
**				and if connected (open) , it chcks o see if there is any pending
**				data to send , and if so sends it.
*/
wSocket.prototype.wSocketsInterval=function()
	{
var	szReadyState				=3				;	//Initialize our variable to the closed state.
	if(this.socket				!=null				)	//If our socket object is not null record the
		szReadyState			=this.socket.readyState		;	// actual socket state.
	switch(szReadyState)
		{
	case	0:break								;	//Connecting.
	case	1:									//Open.
		if(this.aOutputBuffer.length	>0)					// If there is data pending to be sent
		try	{								// then attempt to send it.
var			szOutputBuffer		=this.aOutputBuffer.splice(0,1)	;
			this.socket.send(szOutputBuffer)			;
			return
			} catch(ex){
			this.wSocketsNotifyError(JSON.stringify(ex))		;
			}
		break								;
	case	3:									//Closed.
	case	2:									//Closing.
		if(this.bInConnect		==true)break			;	//If we are already in the process of connecting, bail.
		szReadyState			=4				;
		this.wSocketsConnect()						;
		break;
	default:
		this.wSocketsNotifyError(	"\nwSocketsInterval() readyState = "	+
						szReadyState				+" "+
						aszReadyState[szReadyState]		);
		}// End Switch
		
	if((new Date()-0) - this.lHeartBeatLast >= this.lHeartBeatInterval)		// Send a simulated event whose name is "HeartBeat".
		{
		this.lHeartBeatLast		=(new Date()-0)		;
		this.wSocketsEventHeartbeat()				;
		}
	}
/*	Function	:	setWinterval()
**	Parameters	:	boolean	bOnOff	-The parameter is optional , but if bOnOff is provided , then 
**						it indicates....
**						true	= Start the setInterval timer.
**						false	= Stop the setInterval timer.
**	Returns		:	boolean		- Indicates the resulting or current state of the setInterval timer
**						true	= Running
**						false	= Stopped
**	Description	:	The wSocket class uses an interval timer to call the eSocketsInterval() function every
**				500 milliseconds. That function checks to see if there is any data to be sent, or if the
**				current connection is closed do to network failure or whatever reason and reconnects
**				if needed.
*/
wSocket.prototype.setWinterval=function(bOnOff)
	{
	for(;typeof bOnOff=="boolean";)	
		{
		if(	this.IDinterval		!=null	&&
			bOnOff			==false	)
			{clearInterval(this.IDinterval);this.IDinterval=null;break;}
		if(this.IDinterval		==null	&&
			bOnOff			==true	)
			{
			this.IDinterval=setInterval(this.szWSOCKETpath+".wSocketsInterval()",500);break;
			}
		break;
		}// end forever
	return (this.IDinterval!=null)
	}// End function
/*	Function	:	wSocketsSend()
**	Parameters	:	String szSend	- This is a string (in XML format) to send to the backend.
**	Returns		:	None
**	Description	:	
*/
wSocket.prototype.wSocketsSend=function(oSend)
	{
var	oXML		;
try	{
	switch(typeof oSend)
		{
	case "undefined"	:return;
	case "string"		:oXML				=this.StringToXML(oSend);break;
	case "object"		:if(	typeof oSend.nodeName	!="undefined"	&&
					oSend.nodeName		=="#document"	)
					oXML			=oSend		;
				 else	{
					var szCommand		=""		;
					var oCommand		={}		;
					for(i in oSend)
						{
						szCommand	=i		;
						oCommand	=oSend[i]	;
						break;
						}
					oXML			=this.ObjectToXML(oCommand,szCommand);
					}
				 break;
		}//end switch

	}catch(oE){
	log("\nSEND ERROR FOR TYPE ("+(typeof oSend)+")");
	log("\nDETAILS "+oSend);
	}

	if(typeof oXML.firstChild					=="undefined"	)return;
	if(oXML.getElementsByTagName("parsererror"	).length	>0		)return;

var	szXML=this.XMLToString(	oXML)
	this.wSocketsSentString(szXML)
	this.wSocketsSentXML(	oXML)
	this.aOutputBuffer.push(szXML);
	if(this.IDinterval						==null		)this.setWinterval(true);
	}
/*	Function	:	wSocketsQuit()
**	Parameters	:	None
**	Returns		:	None
**	Description	:
*/
wSocket.prototype.wSocketsQuit=function()
	{
	this.setWinterval(false)	;
	this.wSocketsDisconnect()	;
	this.wSocketsNotify("cancel")	;
	}
/*	Function	:	callEventListener()
**	Parameters	:	oXML
**	Returns		:	None
**	Description	:
*/
wSocket.prototype.callEventListeners=function(oXML)
	{
var	bResult					=true								;
var	szEvent					=oXML.firstChild.getAttributeNS(null,"name").toLowerCase()	;
	if(szEvent				==null		)return true					;
var	szEvent					=szEvent.toLowerCase()						;
	if(typeof this.oEventListeners[szEvent]	=="undefined"	)return true					;
	for(i in this.oEventListeners[szEvent])
		{
		if(i				=="copyFrom"	)continue					;
		var oFunc			 =this.oEventListeners[szEvent][i].fFunction			;
		var bFuncResult			 =this.oEventListeners[szEvent][i].fFunction(oXML)		;
		if(	typeof bFuncResult	!="undefined"	&&
			bFuncResult		==false		)return false					;
		}
	return true;
	}
/*	Function	:	removeEventListener()
**	Parameters	:	szEvent
**				fFunction
**				bCapture
**	Returns		:	None
**	Description	:
*/
wSocket.prototype.removeEventListener=function(szEvent,fFunction,bCapture)
	{
var	szEvent					=szEvent.toLowerCase()			;
	if(typeof bCapture			=="undefined"	)var bCapture=false	;
	if(typeof this.oEventListeners[szEvent]	=="undefined"	)return			;
var	x		=0	;
var	szI		="_0"	;
	for(i in this.oEventListeners[szEvent])
		{
		if(this.oEventListeners[szEvent][i].fFunction==fFunction)
		if(this.oEventListeners[szEvent][i].bCapture==bCapture)
		delete this.oEventListeners[szEvent][i];
		x++			;
		szI="_"+x		;
		}
	}
/*	Function	:	addEventListener()
**	Parameters	:	szEvent
**				fFunction
**				bCapture
**	Returns		:	None
**	Description	:
*/
wSocket.prototype.addEventListener=function(szEvent,fFunction,bCapture)
	{
var	szEvent					=szEvent.toLowerCase()			;
	if(typeof bCapture			=="undefined")var bCapture=false	;
	if(typeof this.oEventListeners[szEvent]	=="undefined")
		this.oEventListeners[szEvent]={};
	var x					=0					;
	var szI					="_0"					;
	for(i in this.oEventListeners[szEvent])
		{
		if(this.oEventListeners[szEvent][i].fFunction	==fFunction	)
		if(this.oEventListeners[szEvent][i].bCapture	==bCapture	)break	;
		x++									;
		szI				="_"+x					;
		}
	this.oEventListeners[szEvent][szI]={fFunction:fFunction,bCapture:bCapture};
	}

