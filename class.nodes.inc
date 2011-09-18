<?php
/*	File		:	class.nodes.inc
**	Author		:	Dr. Clue ( A.K.A Ian A. Storms )
**	CLASS		:	node
**	Description	:	Provides a generic DOM like node tree
**			for producing a result set that can be rendered
**			as both XML and JSONized XML, with room to 
**			easily add other output formats,
**	Requires	:	None.
**	NOTES		:	This class has only partial documentation
**			at present , but like most of the code will
**			get further documentation as time goes on.
*/
class node	{
	var	$nodeName		="node"	;	// The name value of the node.
	var	$nodeType		="0"	;	// The type of the node
	var	$nodeValue		=""	;	// The value of the node.
	var	$CDATA			=""	;	// Used for encapsulating
	var	$comment		=""	;	// Used for comments
							// data in the <![CDATA[ thecontent ]]>
	var	$parentNode		=null	;	// The node whose $childNodes array has 
							// a reference to this node.
	var	$attributes		=Array();	// the name="foo" array for the node
	var	$childNodes		=Array();	// a list of nodes representing
							// a collection of sub nodes.

	var	$bLockLowerCase		=TRUE	;	// Converts all node names to lowercase on output.

	var	$xmlParser		=""	;	// 
	var	$nodeNow		=null	;	// In a XML tree traversal this
							// is the current node in that process
	var	$iLevel			=0	;	// Used to track node nesting level
	var	$szXSL			=""	;	// Used to indicate an XSL stylesheet

//	Possible note: If you want to sure that PHP treats an UTF-8 encoded file correctly, make sure that it begins with the corresponding 3 byte BOM header (0xEF 0xBB 0xBF)
	var	$szOutput_encoding	="ISO-8859-1";

	//Used to indicate node names that should never be collapsed, because some browsers
	//flake out if certain elements don't include a closing tag.
	static	$aNeverCollapse	=Array(	"textarea"	=>"textarea"	,
					"script"	=>"script"	);

/*	CONSTRUCTOR	:	node()
**	Parameters	:	$params = Array()
**	Returns		:	(node	)
**	Description	:
*/
	function node($params = Array())
		{
//		$this->szOutput_encoding=iconv_get_encoding("output_encoding");

		foreach($params as $key => $value)
			{
			$this->$key = $value;
			}
		$this->nodeNow=$this;
		}
/*	Method		:	__toString()
**	Parameters	:	None
**	returns		:	String
**	Description	:	Renders node tree as HTML
**
*/
	function __toString()
		{
		return "<pre>".htmlspecialchars($this->renderXML())."</pre>";
		}
/*	Method		:	printXMLoutput()
**	Parameters	:	$aArguments =Array()
**	Returns		:	$szOut	- Node tree rendred as XML syntax
**	Description	:
*/
	function printXMLoutput($aArguments =Array())
		{
		$this->aArguments=$aArguments;
		$this->szXSL	="";
		if(!empty($aArguments['F1_XSL']))$this->szXSL=$aArguments['F1_XSL'];

/*		$szOut	='<?xml version="1.0" encoding="'.$this->szOutput_encoding.'"?>";
		if(strlen($szXSL)>0)$szOut.= "\n<?xml-stylesheet type=\"text/xsl\" href=\"$szXSL\"?>";
//		$szOut	.="\n".$this->renderXML()."\n";*/
		$szOut	=$this->renderXML()."\n";
		return $szOut;
		}
/*	Method		:	printJSONoutput()
**	Parameters	:	$aArguments =Array()
**	Returns		:	$szOut	- The entire node tree
**				rendered in the form of javascript syntax 
**	Description	:	
*/
	function printJSONoutput($aArguments =Array())
		{
		$szCallback	 ="catchJSON";
		if(isset($aArguments['F1_JSONcallback']))
			$szCallback=$aArguments['F1_JSONcallback'];
		$szOut		 = "var ".$szCallback."Result=".$this->renderJSON()."\n";
		$szOut		.= "if(typeof ".$szCallback."!='undefined')".$szCallback."(".$szCallback."Result)\n";
		return $szOut;
		}
/*	Method		:	insertBefore()
**	Parameters	:	$newNode	- The node to be inserted.
**				$siblingNode	- [NULL] the sibling to insert the child before
**	Returns		:	None.
**	Description	:	Inserts the $newNode object to the 
**			childNodes collection associated with the current node.
*/
	function insertBefore($newNode,$siblingNode=NULL)
		{
		$x = count($this->childNodes);
		if($siblingNode==NULL)$x=0;
		for(;$x>0;$x--)
			{
			if($this->childNodes[$x]==$siblingNode)		break;
			}
		$newNode->parentNode=$this;
		array_splice($this->childNodes,$x,0,Array(&$newNode));
//		$this->childNodes[$x]=&$newNode;
//print "X=".$x ;
//print_r($this->childNodes);


		return $newNode;
		}
/*	Method		:	insertAfter()
**	Parameters	:	$newNode	- The node to be inserted.
**				$siblingNode	- [NULL] the sibling to insert the child before
**	Returns		:	None.
**	Description	:	Inserts the $newNode object to the 
**			childNodes collection associated with the current node.
*/
	function insertAfter($newNode,$siblingNode=NULL)
		{
		$x = 0;
		if($siblingNode==NULL)$x=0;
		for(;$x<count($this->childNodes);$x++)
			{
			if($this->childNodes[$x]==$siblingNode){$x++;		break;}
			}
		$newNode->parentNode=$this;
		array_splice($this->childNodes,$x,0,Array(&$newNode));
//		$this->childNodes[$x]=&$newNode;
//print "X=".$x ;
//print_r($this->childNodes);


		return $newNode;
		}
		
	function adoptChildren($parent)
		{
		foreach($parent->childNodes as $key=>$value)
			{
			$this->appendChild($value);
			}
		$parent->childNodes=Array();
		}		
		
		
/*	Method		:	appendChild()
**	Parameters	:	$childNode	- The node to be appended.
**	Returns		:	he child node.
**	Description	:	Appends the $childNode object to the 
**			childNodes collection associated with the current node.
*/
	function appendChild($childNode)
		{
		$childNode->parentNode=$this;
		$this->childNodes[]=&$childNode;
//		$this->childNodes =array_merge($this->childNodes,array($childNode->childNode=> &$childNode));
		return $childNode;
		}
/*	Method		:	getAttribute()
**	Parameters	:	$szName
**	Returns		:	The named attribute's value if any.
**	Description	:
*/
	function getAttribute($szName)
		{
		return $this->attributes[$szName];
		}
/*	Method		:	setAttribute()
**	Parameters	:	$szName		- Name of attribute
**				$szValue	- Value of attribute
**	Returns		:	None
**	Description	:	Sets or updates anode attribute value value.
*/
	function setAttribute($szName,$szValue)
		{
		$this->attributes[$szName]=$szValue;
		}
/*	Method		:	createNode()
**	Parameters	:	$params = array()
**	Returns		:	$obj	- the new node.
**	Description	:
*/
	function &createNode($params = array())
		{
		$bNoClass=TRUE;
		if(!empty($this))$bNoClass=(gettype($this)=='NULL');
		if($bNoClass===TRUE)// create new object
			{
			$oNewNode = new node($params);
			return $oNewNode;
			}else{
			foreach($params as $key => $value)
				{
				$this->$key = $value;
				}
			$oNewNode = $this;
			}
		return $oNewNode;
		}

/*	Method		:	removeNode()
**	Parameters	:	$params = array()
**	Returns		:	None
**	Description	:
*/
	function &removeNode($params = array())
		{
//		$bNoClass=TRUE;
		if(!empty($this))$bNoClass=(gettype($this)=='NULL');
		$oNode=null;
		if(!empty($params['node']))$oNode=$params['node'];
		if($oNode==null)return;
/*		if($bNoClass===TRUE)// create new object
			{
			$oNewNode = new node($params);
			return $oNewNode;
			}else{
			foreach($params as $key => $value)
				{
				$this->$key = $value;
				}
			$oNewNode = $this;
			}
		return $oNewNode;*/

		}
/*	Method		:	renderXML()
**	Parameters	:	None
**	Returns		:	$szResult	- The XML rendered node
**	Description	:	Outputs the node in the form
**				of XML syntax.
*/
	function renderXML()
		{
		$szNL		="\n"	;
		$szNL		=""	;
		$szResult	=""	;
		$bIsRoot=empty($this->parentNode);
		if($bIsRoot)
			{
			if(!empty($this->aArguments['F1_XSL']))$this->szXSL=$this->aArguments['F1_XSL'];
			$szResult.='<?xml version="1.0" encoding="'.$this->szOutput_encoding.'"?>'.$szNL;
			if(strlen($this->szXSL)>0)
				{
				$szResult.= "<?xml-stylesheet type=\"text/xsl\" href=\"".$this->szXSL."\"?>".$szNL;
				}
			if(isset($this->szDOCTYPE))
			if(strlen($this->szDOCTYPE)>0)
				{
				$szResult.= $this->szDOCTYPE.$szNL;
				}
			}

		$szNodeName	=implode("_",explode(" ",$this->nodeName));//Replace spaces with _ in nodeName
		if($this->bLockLowerCase===TRUE)$szNodeName=strtolower($szNodeName);

		$szResult	.="<".$szNodeName	;
		$szBody		=""			;
		if(count($this->attributes)>0)
			{
			foreach($this->attributes as $key => $value)
				{
				$szKey=$key;
				if($this->bLockLowerCase===TRUE)$szKey=strtolower($szKey);


				$szResult.=" ".$szKey."=\"".$value."\"";
				}
			}
		if(strlen(	$this->nodeValue	)>0)
			{
			$szSafe=str_replace("&","&amp;",$this->nodeValue);
			$szSafe=str_replace("<","&lt;",$szSafe		);
			$szSafe=str_replace(">","&gt;",$szSafe		);
			$szBody.=$szSafe;
			}
		if(strlen(	$this->CDATA		)>0)
			{
			$szBody.="<![CDATA[ ".$this->CDATA."]]>";
			}
		if(strlen(	$this->comment		)>0)
			{
			$szBody.="<!-- ".$this->comment." -->\n";
			}

			if(!empty($this->includes))
				{
				$szBody.='<includes src="'.$this->includes.'">'.$szNL;
				$szBody.=file_get_contents($this->includes);
				$szBody.="</includes>";
				}

		if(count(	$this->childNodes	)>0)
			{
			$szBody.="\n";
			foreach($this->childNodes as $key => $value)
				{
				$szBody.="".$value->renderXML();
				}
			}
		if(!isset(node::$aNeverCollapse[$this->nodeName])&&strlen($szBody)==0)return $szResult." />".$szNL;
		return $szResult.">".$szBody."</".$szNodeName.">".$szNL;
		}
/*	Method		:	renderJSON()
**	Parameters	:	None
**	Returns		:	$szResult	- The JSONized node
**	Description	:	renders the current node as a javascript object.
**			which as the following members
**			
**			n	: The nodeName
**			a	: The node attributes collection
**			v	: The contents of the node/element.
**			c	: The child node collection
**			d	: The CDATA section
*/
	function renderJSON()
		{
		$szResult	="{n:\"".$this->nodeName."\""	;
		$szBody		=""			;
		if(count($this->attributes)>0)
			{
			$szResult	.=",a:{";
			$szAttributes	="";
			foreach($this->attributes as $key => $value)
				{
				if(strlen($szAttributes)>0)$szAttributes.=",";
				$szAttributes.=$key.":\"".$value."\"";
				}
			$szResult.=$szAttributes."}";
			}
		if(strlen(	$this->nodeValue	)>0)
			{
			$this->nodeValue= rtrim($this->nodeValue,"\\");
			$szResult.=",v:\"".str_replace('"','\"',str_replace("\r","",str_replace("\n","\\n",$this->nodeValue)))."\"";
			}
		if(strlen(	$this->CDATA		)>0)$szResult.",d:"."\"".$this->CDATA."\"";
		if(count(	$this->childNodes	)>0)
			{
			$szResult.=",c:[";	$szChild="";
			foreach($this->childNodes as $key => $value)
				{
				if(strlen($szChild)>0)$szChild.=",";
				$szChild.=$value->renderJSON();
				}
			$szResult.=$szChild."]";
			}
		return $szResult."}";
		}

/*	Method		:	getElementValueByTagName()
**	Parameters	:	$szTagName
**	Returns		:	node
**	Description	:	searches child nodes and descendents 
**			for a node having the specified name
*/
	function getElementValueByTagName($szTagName)
		{
		$oResult=$this->getElementByTagName($szTagName);
		return ($oResult==NULL)?NULL:$oResult->nodeValue;
		}
/*	Method		:	getElementByTagName()
**	Parameters	:	$szTagName
**	Returns		:	node
**	Description	:	searches child nodes and descendents 
**			for a node having the specified name
*/
	function getElementByTagName($szTagName)
		{
		$nodeResult=NULL;
		foreach($this->childNodes as $key => $value)
			{
			if($value->nodeName==$szTagName)$nodeResult=&$value;
			else $nodeResult=$value->getElementByTagName($szTagName);
			if($nodeResult!=NULL)return $nodeResult;
			}
		return $nodeResult;
		}
/*	Method		:	tagOpen()
**	Parameters	:	$parser
**				$name
**				$attrs
**	Returns		:	None
**	Description	:
*/
	function tagOpen($parser, $name, $attrs)
		{
//print "\nHere open ".$name;
		$szName=$name;
		if($this->bLockLowerCase===TRUE)$szName=strtolower($szName);

		if($this->nodeNow==null)
			{
			$this->nodeNow		=$this;
			$nodeNew		=$this;
			$nodeNew->nodeName	=$szName;
			$nodeNew->attributes	=$attrs;
			}else{
			$nodeNew=new node(Array("nodeName"=>$szName,"attributes"=>$attrs));
			$this->nodeNow->appendChild($nodeNew);
			}
		$this->nodeNow=$nodeNew;
		}
/*	Method		:	tagClose()
**	Parameters	:	$parser
**				$tagData
**	Returns		:	None
**	Description	:
*/
	function tagClosed($parser, $name)
		{
//print "Here close $name\n";
		if($this->nodeNow->parentNode)$this->nodeNow=$this->nodeNow->parentNode;
		}
/*	Method		:	tagData()
**	Parameters	:	$parser		- handle to XML parser
**				$tagData	- the content that would
**						appear within the nodes.
**	Returns		:	None
**	Description	:	Callback function for the XML parser
**			which handles the *content* that would appear 
**			between the opening and closing tags <foo>...</foo> 
*/
	function tagData($parser, $tagData)
		{
		$trimmed= trim($tagData,"\r\n ");
		$this->nodeNow->nodeValue=htmlspecialchars($trimmed);



//print "\ntagData=($tagData)";


		}
/*	Method		:	parseXML()
**	Parameters	:	$szString
**	Returns		:	none
**	Description	:	parses the XML string and produces
**			a node tree
*/
	function parseXML($szString)
			{
			$this->xmlParser = xml_parser_create($this->$szOutput_encoding);
			xml_set_object(			$this->xmlParser,$this);
			xml_parser_set_option (		$this->xmlParser, XML_OPTION_CASE_FOLDING	, 0 );
			xml_parser_set_option (		$this->xmlParser, XML_OPTION_TARGET_ENCODING	, $this->$szOutput_encoding );
			xml_parser_set_option (		$this->xmlParser, XML_OPTION_SKIP_TAGSTART	, 0 );
			xml_parser_set_option (		$this->xmlParser, XML_OPTION_SKIP_WHITE	, 0 );
			xml_set_element_handler(	$this->xmlParser,"tagOpen","tagClosed");
			xml_set_character_data_handler(	$this->xmlParser,"tagData");
//			xml_set_character_data_handler($xml_parser, "cdata");

			$this->nodeNow	=null	;
			$iXMLsuccess	=	xml_parse(		$this->xmlParser, $szString,TRUE);
			if($iXMLsuccess	==0)
				{
				$iXMLerrorCode	=xml_get_error_code ($this->xmlParser);
				$szXMLerror	=xml_error_string ( $iXMLerrorCode );
				print "XMLparse error ".$iXMLerrorCode."[]".$szXMLerror;
				}
			xml_parser_free(	$this->xmlParser		);
//			print "XML Parsed ".count($this->childNodes)." children with PHP ".phpversion()."<hr />";
			}
/*	Method		:	traverseNodes()
**	Parameters	:	(Array	)	$params	[data		]
**							[callback	]
**	Returns		:
**	Description	:
**
*/
	function traverseNodes($params=Array(	"data"		=>"*"	,
						"callback"	=>""	))
		{
		$params['data'].=$this->nodeName." ";
		if(!empty($params['callback']))
			{
			$this->$params['callback']($params,$this);
			}
		if(count(	$this->childNodes	)>0)
			{
			foreach($this->childNodes as $key => $value)
				{
				$value->traverseNodes($params);
				}
			$szResult.=$szChild."]";
			}
		return $params;
		}
/*	Method		:	callbackGetElementsByTagName()
**	Parameters	:	&$params
**				$oNode
**	Returns		:
**	Description	:
**
*/
	function callbackGetElementsByTagName(&$params,$oNode)
		{
		if(	$params['nodeName']!=$oNode->nodeName	&&
			$params['nodeName']!="*"		)return;
		$params['nodeList'][]=$oNode;

		}
/*	Method		:	getElementsByTagName()
**	Parameters	:	$szTagName
**	Returns		:
**	Description	:
**
*/
	function getElementsByTagName($szTagName)
		{
		$params=Array("nodeName"=>$szTagName,"callback"=>"callbackGetElementsByTagName","nodeList"=>Array());
		$oResult = $this->traverseNodes($params);
		return $params['nodeList'];
		}

/*	Method		:	loadFromArray()
**	Parameters	:	Array aNamVal
**	returns		:	None
**	Description	:	Loads a name/value array as child nodes.
**
*/
	function loadFromArray($aNamVal=Array())
		{
		foreach($aNamVal as $key =>$value)
			{
			$this->appendChild(new node(Array("nodeName"=>$key,"nodeValue"=>$value)));
			}
		}
/*	Method		:	loadToArray()
**	Parameters	:	$szPrefix
**	returns		:	None
**	Description	:	Loads child nodes into a name/value array.
**
*/
	function loadToArray($szPrefix="",$bDeep=FALSE)
		{
		if(count($this->childNodes)==0)return Array($szPrefix.$this->nodeName=>$this->nodeValue);
		$aResult=null;
		foreach($this->childNodes as $value)
			{
			$aNEW=Array($szPrefix.$value->nodeName=>$value->nodeValue);

			if(is_array($aResult))		$aResult=array_merge($aResult,$aNEW);
			else $aResult=$aNEW;
			}

		return $aResult;
		}
	} // End of class
?>
