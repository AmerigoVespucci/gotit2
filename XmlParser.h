#if !defined(XmlParser_H)
#define XmlParser_H


#include <algorithm>
#include <string>
#include <sstream>
#include <iostream>
#include <list>
using namespace std;


struct XmlNode;
typedef list<XmlNode *> CListXmlNode;


struct XmlNode
{
	string Name;
	string Value;
	bool bLeaf;
	CListXmlNode Vals;

	XmlNode() 
	{
		bLeaf = true;
	}
	~XmlNode() 
	{
		if (!bLeaf) {
			while (Vals.size() > 0) {
				XmlNode * pChild = Vals.back();
				Vals.pop_back();
				delete pChild;
			}
		}
	}
};



/////////////////////////////////////////////////////////////////////////////////
// XmlNotify
//
// Purpose:		abstract class used to notify about xml tags found. 

class XmlNotify
{
public:


	XmlNotify ()
	{}




	// notify methods
	virtual void foundNode		( string & name, string & attributes ) = 0;
	virtual void foundElement	( string & name, string & value, string & attributes ) = 0;

	virtual void startElement	( string & name, string & value, string & attributes ) = 0;
	virtual void endElement		( string & name, string & value, string & attributes ) = 0;

	
};








/////////////////////////////////////////////////////////////////////////
// define tag markers

#define idTagLeft	"<"
#define idTagRight	">"
#define idTagEnd	"</"
#define idTagNoData "/>"


#define idTagLeftLength		1
#define idTagRightLength	1
#define idTagEndLength		2


#include <string>
using namespace std;



//////////////////////////////////////////////////////////////////////////////////
// XmlUtil
//
// Purpose:		provides xml utility methods

class XmlUtil
{

	// tag helper methods
	static string getStartTag ( string & text )
	{
		string tag = idTagLeft;
		tag += text;
		tag += idTagRight;

		return string(tag);
	}

	// static helper methods
	static string getEndTag ( string & text )
	{
		string tag = idTagEnd;
		tag += text;
		tag += idTagRight;

		return string(tag);
	}

	static string getStartTag ( char * text )
	{
		string tag = idTagLeft;
		tag += text;
		tag += idTagRight;

		return string(tag);
	}

	static string getEndTag ( char * text )
	{
		string tag = idTagEnd;
		tag += text;
		tag += idTagRight;

		return string(tag);
	}
};


//////////////////////////////////////////////////////////////////////////////
// XmlParserr
//
// Purpose: used to parse a buffer that has xml formatted data

class XmlParser
{
public:

	static long _parseid;
	
	// id of parser (for debugging)
	long		_id;

	// provides string interface to buffer
	string		_strParse;

	// parse buffer and offsets of where we are parsing
	char *		_buffer;
	long		_parseLength;

	// current position in parse buffer
	long		_current;

	// tag, attributes and data position information
	long		_firstTagStart;
	long		_firstTagEnd;

	long		_lastTagStart;
	long		_lastTagEnd;

	long		_nameStart;
	long		_nameEnd;
	long		_attrStart;
	long		_attrEnd;
	long		_valueStart;
	long		_valueEnd;

	int			_bError;


	XmlParser () :
		_strParse()
	{ 
		_bError = 0;

		clear(); 

		// get parser id
		_id = _parseid;

		// next id
		_parseid++;
	}

	// create/release
	bool create ( const char * buffer, long parseStart, long parseEnd )
	{
		// if invalid stop
		if ( !buffer || parseStart < 0 || parseEnd < parseStart )
			return false;

		return create( buffer + parseStart, getLength(parseStart,parseEnd) );
	}	



	bool create ( const char * buffer, long length )
	{
		// if invalid stop
		if ( !buffer || length <= 0 )
			return false;

		// parse buffer and offsets of where we are parsing
		_buffer      = (char *) buffer;
		_parseLength = length;

		// current position in parse buffer
		_current    = 0;

		return true;
	}	

	void release ()
	{

	}

	// cur position methods
	long getCurrent ()
	{
		return _current;
	}

	long getParseLength ()
	{
		return _parseLength;
	}

	long getCurLength ()
	{
		return getOffsetLength(_current);
	}

	long getOffsetLength ( long offset )
	{
		return getLength(offset,_parseLength - 1);
	}


	char * getBufferPos ()
	{
		return _buffer;
	}

	char * getLastBufferPos ()
	{
		return _buffer + _parseLength;
	}

	char * getCurPos ()
	{
		if ( isValid() )
			return _buffer + _current;
		else
			return NULL;
	}

	char * getParseState ( long & parseLength )
	{
		// if not valid stop
		if ( !isValid() )
		{
			parseLength = 0;
			return getCurPos();
		}

		/*
		// if value has length
		if ( getValueLength() > 0 )
		{
			// if value has a tag it must be parsed
			if ( valueHasTag() )
			{
				// get parse length
				parseLength = getValueLength();

				// return ptr to start of value
				return _buffer + _valueStart;
			}
		}
		*/

		// get parse state
		parseLength = getCurLength();

		// get current buffer position
		char * buffer = getCurPos();

		// if last tag is valid
		/*
		if ( hasLastTag() )
		{
			// if current position is in last tag
			// pos then show stop
			if ( getCurPos() >= getLastTagPos() )
			{
				parseLength = 0;
				return getLastBufferPos();
			}
		}
		*/

		return getCurPos();

	}

	// get ref to parse buffer
	string & str ()
	{
		return _strParse;
	}

	// state methods
	void reset ()
	{
		resetTagPositions();
	}

	bool isValid ()
	{
		// if buffer state not valid
		if ( !_buffer || _parseLength <= 0 )
			return false;

		// if cur position not valid
		if ( _current < 0 || _current > _parseLength )
			return false;

		return true;
	}

	void resetTagPositions ( long start=-1 )
	{
		// set tag positions
		_firstTagStart  = start;
		_firstTagEnd	= start;

		_lastTagStart   = start;
		_lastTagEnd	    = start;

		_nameStart	    = start;
		_nameEnd		= start;

		_attrStart      = start;
		_attrEnd        = start;

		_valueStart     = start;
		_valueEnd       = start;
	}
	
	void clear ()
	{
		// parse buffer and offsets of where we are parsing
		_buffer		 = 0;
		_parseLength = 0;

		// current position in parse buffer
		_current	= 0;

		// reset tag positions
		reset();
	}

	// parse methods
	bool parse ( const char * buffer, long parseStart, long parseEnd )
	{
		// if create fails stop
		if ( !create(buffer,parseStart,parseEnd) )
			return false;

		return parse();
	}

	bool parse ( const char * buffer, long parseLength )
	{
		// if create fails stop
		if ( !create(buffer,parseLength) )
			return false;

		return parse();
	}

	bool parse () 
	{
		// init tag position
		_firstTagStart = _current;
		_firstTagEnd   = _current;

		_lastTagStart  = _current;
		_lastTagEnd    = _current;

				     
		// find first tag
		long first = find( idTagLeft, _current );
		if ( first == -1 ) {
			// not an error
			return false;
		}

		// if find right tag
		long last  = find( idTagRight, first );
		if ( last == -1  ) {
			_bError = 1;
			return false;
		}

		// set first tag start/end
		_firstTagStart = first;
		_firstTagEnd   = last;

		// now parse name
		if ( !parseName() ) {
			_bError = 1;
			return false;
		}

		// parse attributes
		parseAttributes();

		// if null tag no data or last tag
		if ( hasNullTag() )
		{
			// update cur position
			_current  = _firstTagEnd + idTagRightLength;

			// done so show success
			return true;
		}		

		// special case ?xml
		string TestName = getName();

		if (TestName == "?xml") {
			// update cur position
			_current = _firstTagEnd + idTagRightLength;

			// done so show success
			return true;
		}
		// form end tag
		string endTag;
		endTag  = idTagEnd;
		endTag += getName();
		endTag += idTagRight;

		// find last tag
		first = find( endTag, last );
		if ( first == -1 ) {
			_bError = 1;
			return false;
		}

		// set last tag start/end
		_lastTagStart = first;
		_lastTagEnd   = first + endTag.size();

		// parse the value if not a null tag
		if ( !hasNullTag() )
			parseValue();

		// update cur position
		// we have parsed a tag so look for the start
		// of a new tag, if found set current position
		// to it, else set to last tag
		long pos = find( idTagLeft, _lastTagEnd );
		if ( pos != -1 )
			_current = pos;
		else
			_current = _lastTagEnd;


		return true;
	}

	bool parse ( string & name,
				 string & value,
				 string & attributes,
				 long & current )
	{

		return true;
	}



	// tag search methods

	bool hasNullTag ()
	{
		// get end of first tag
		char * buffer = _buffer + _firstTagEnd - 1;

		// if null tag marker
		if ( *buffer == '/' && *(buffer+1) == '>' )
			return true;
		else
			return false;
	}

	/////////////////////////////////////////////////////////
	// these methods are protected because the state
	// of parsing might not be properly setup, and
	// if that were so then calling these methods
	// would cause errors

protected:

	// name search methods
	bool parseName ()
	{
		// if first tag search failed show failed
		if ( _firstTagStart < 0 ||  _firstTagEnd < 0 ||
			 _firstTagEnd <= _firstTagStart )
		{
			_nameStart = -1;
			_nameEnd   = -1;
			return false;
		}

		// init name start/end position
		_nameStart = _firstTagStart + idTagLeftLength;
		_nameEnd   = _firstTagEnd - 1;

		// if null tag then backup before
		// null tag marker
		if ( hasNullTag() )
			_nameEnd -= 1;

		// check for separator (i.e. there are attributes)
		long last = find(' ',_nameStart, getNameLength());
		if ( last != -1 )
		{
			// there are attributes so backup
			// before attributes
			_nameEnd = last - 1;
		}

		return true;
	}

	bool parseName ( string & name )
	{
		// set name state
		if ( !parseName() )
			return false;

		name = getName();

		return true;
	}

	// attribute search methods
	bool parseAttributes ()
	{
		// init name start/end position
		_attrStart = -1;
		_attrEnd   = -1;

		// if tag or name length invalid stop
		long tagLength  = getTagLength();
		long nameLength = getNameLength();
		if ( tagLength <= 0 || nameLength <= 0 )
			return 0;

		// if the difference in the lengths is
		// less than the length of the left/right marker
		// then no attributes
		long diff = getTagLength() - getNameLength();

		switch ( diff )
		{
			// no attribute case
			case 0:
			case 1:
			case 2:
				return false;

			// no attribute case but has null tag
			case 3:
				return false;
		}

		// init attributes start, move past space after name
		_attrStart = _nameEnd + 2;

		// init attribute end move before right tag marker
		// if null tag move before null tag marker
		_attrEnd = _firstTagEnd - 1;
		if ( hasNullTag() )
			_attrEnd -= -1;

		return true;
	}

	bool parseAttributes ( string & attributes )
	{
		// set name state
		if ( !parseAttributes() )
			return false;

		attributes = getAttributes();

		return true;
	}


	// data search methods
	bool parseValue ()
	{
		// if first tag search failed show failed
		if ( _firstTagStart < 0 ||  _lastTagEnd < 0 ||
			 _lastTagEnd <= _firstTagStart )
		{
			_valueStart = -1;
			_valueEnd   = -1;
			return false;
		}

		// init value start/end positions
		_valueStart = _firstTagEnd + 1;
		_valueEnd   = _lastTagStart - 1;

		return true;
	}


	bool parseValue ( string & value )
	{
		// set name state
		if ( !parseValue() )
			return false;

		value = getValue();

		return true;
	}

public:

	// name access methods
	char * getNamePos ()
	{
		if ( hasName() )
			return _buffer + _nameStart;
		else
			return NULL;
	}

	bool hasName ()
	{
		if ( getNameLength() > 0 )
			return true;
		else
			return false;
	}

	long getNameLength ()
	{
		long length = getLength(_nameStart,_nameEnd);
		return length;
	}


	string getName ()
	{
		// get name length
		long length = getNameLength();

		// if length invalid show null string
		// else get string
		if ( length <= 0 )
			return string("");
		else
			return substr(_nameStart,length);
	}


	// attribute access methods
	char * getAttributesPos ()
	{
		if ( hasAttributes() )
			return _buffer + _attrStart;
		else
			return NULL;
	}

	bool hasAttributes ()
	{
		if ( getValueLength() > 0 )
			return true;
		else
			return false;
	}

	long getAttributesLength ()
	{
		long length = getLength(_attrStart,_attrEnd);
		return length;
	}

	string getAttributes ()
	{
		// get attribute length
		long length = getAttributesLength();

		// if length invalid show null string
		// else get string
		if ( length <= 0 )
			return string("");
		else
			return substr(_attrStart,length);
	}

	// value access methods
	char * getValuePos ()
	{
		if ( hasValue() )
			return _buffer + _valueStart;
		else
			return NULL;
	}

	bool hasValue ()
	{
		if ( getValueLength() > 0 )
			return true;
		else
			return false;
	}

	long getValueLength ()
	{
		long length = getLength(_valueStart,_valueEnd);
		return length;
	}

	string getValue ()
	{
		// get tag data length
		long length = getValueLength();

		// if length invalid show null string
		// else get string
		if ( length <= 0 )
			return string("");
		else
			return substr(_valueStart,length);
	}

	char * getValueState ( long & valueLength )
	{
		// get value state
		valueLength = getValueLength();

		// return value buffer pos
		return _buffer + _valueStart;
	}

	bool valueHasTag ()
	{
		// if find end tag
		long pos = find( idTagLeft, _valueStart, getValueLength() );

		// if found tag
		if ( pos != -1 )
			return true;
		else
			return false;
	}

	// tag access methods
	long getTagLength ()
	{
		long length = getLength( _firstTagStart, _firstTagEnd );
		return length;
	}

	long getLastTagLength ()
	{
		long length = getLength( _lastTagStart, _lastTagEnd );
		return length;
	}

	bool hasTag ()
	{
		if ( getTagLength() > 0 )
			return true;
		else
			return false;
	}

	bool hasLastTag ()
	{
		if ( getLastTagLength() > 0 )
			return true;
		else
			return false;
	}

	char * getTagPos ()
	{
		if ( hasTag() )
			return _buffer + _firstTagStart;
		else
			return NULL;
	}

	char * getLastTagPos ()
	{
		if ( hasTag() )
			return _buffer + _lastTagStart;
		else
			return NULL;
	}

	string getTag ()
	{
		// get tag data length
		long length = getTagLength();
		return substr(_firstTagStart,length);
	}

	// string utility methods
	long getLength ( long startPos,
			         long endPos )
	{
		// if positions invalid show no length
		if ( startPos < 0 || endPos < 0 ||
			 endPos < startPos )
			return 0;

		// get length
		long length = endPos - startPos + 1;
		return length;
	}
/*
	string ::iterator begin ()
	{
		string s(_buffer);
		//string::iterator buf = _buffer;
		string::iterator buf = s.begin();
		return string::iterator(buf);
	}

	string ::iterator end ()
	{
		string::iterator buf = _buffer + _parseLength;
		return string::iterator(buf);
	}
*/
	long find ( char srchChar, long offset, long length = -1 )
	{
		// if no length set to length to
		// end of parse buffer
		if ( length == -1 )
			length = getOffsetLength(offset);

		// set start and end of search 
		string s(_buffer);
		string::iterator buf = s.begin();
		//string::iterator start = _buffer + offset;
		string::iterator start = buf + offset;
		//string::iterator end   = _buffer + (offset + length);
		string::iterator end   = buf + (offset + length);

		// search for it
		string::iterator found = std::find( start, end, srchChar );

		// if at end did not find it
		if ( found >= end )
		{
			return -1;
		}
		else
		{
			// as a last check make sure found is valid
			if ( found < start )
				return -1;

			// set position
			long pos = (found - start);
			
			pos += offset;

			return pos;
		}
	}

	long find ( char * srchStr, long offset, long length = -1 )
	{
		// if no length set to length to
		// end of parse buffer
		if ( length == -1 )
			length = getOffsetLength(offset);

		// set start and end of search 
		/*
		string::iterator start = _buffer + offset;
		string::iterator end   = _buffer + (offset + length);
		*/
		string s(_buffer);
		int l = s.length();
		int si = s.size();
		string::iterator buf = s.begin();
		string::iterator tend = s.end();
		string::iterator start = buf + offset;
		string::iterator end   = buf + (offset + length);


		string sSearch(srchStr);
		string::iterator itsrchStr = sSearch.begin();
		string::iterator srchStart = itsrchStr;
		string::iterator srchEnd   = itsrchStr + strlen(srchStr);

		// search for it
		string::iterator found = std::search( start, end, srchStart, srchEnd );

		// if at end did not find it
		if ( found >= end )
		{
			return -1;
		}
		else
		{
			// as a last check make sure found is valid
			if ( found < start )
				return -1;

			// set position
			long pos = (found - start);
			pos += offset;

			return pos;
		}
	}

	long find ( string & srchStr, long offset, long length = -1 )
	{
		cout << "do std find" << endl;
		// if no length set to length to
		// end of parse buffer
		if ( length == -1 )
			length = getOffsetLength(offset);

		// set start and end of search 
		/*
		string::iterator start = _buffer + offset;
		string::iterator end   = _buffer + (offset + length);
		*/
		string s(_buffer);
		string::iterator buf = s.begin();
		string::iterator start = buf + offset;
		string::iterator end   = buf + (offset + length);


		string::iterator srchStart = srchStr.begin();
		string::iterator srchEnd   = srchStr.end();

		// search for it
		string::iterator found = std::search( start, end, srchStart, srchEnd );

		// if at end did not find it
		if ( found >= end )
		{
			return -1;
		}
		else
		{
			// as a last check make sure found is valid
			if ( found < start )
				return -1;

			// set position
			long pos = (found - start);
			pos += offset;

			return pos;
		}
	}

	long rfind ( char srchChar, long offset, long length )
	{
		// setup srch string
		char srchStr[2];
		srchStr[0] = srchChar;
		srchStr[1] = '\0';

		return rfind(srchStr,offset,length);
	}

	long rfind ( char * srchStr, long offset, long length )
	{
		/*
		// set start and end of search 
		string::reverse_iterator revStart = _buffer + (offset + length)
		string::reverse_iterator revEnd   = _buffer + offset;

		// search for it
		string::reverse_iterator found = std::find( start, end, srchStr );

		// get position 
		long pos = found - revStart;
		*/
		long pos = 0;

		return pos;
	}


	string substr ( long offset, long length )
	{
		// get start of sub string
		char * ptr = _buffer + offset;

		// create string for it
		string str;
		str.assign( ptr, length );

		return string(str);
	}


};

//////////////////////////////////////////////////////////////////////////////
// XmlStream
//
// Purpose: stores a string that contain start,end tag delimited value


class XmlStream :
	public string
{
	XmlNotify *			_subscriber;	// notification subscriber
	XmlNode * _NodeHead;
public:

	XmlStream () :

		string (),
		_subscriber(NULL),
		_NodeHead(NULL)

	{}

	XmlStream (XmlNotify * Notifier) :

		string (),
		_subscriber(Notifier),
		_NodeHead(NULL)

	{}

	virtual ~XmlStream ()
	{ 
		if (_NodeHead) {
			delete _NodeHead;
			_NodeHead = NULL;
		}
		release(); 
	}

	// release resources
	bool create ()
	{
		return true;
	}

	bool create ( char * buffer, long len )
	{
		if ( buffer && len > 0 )
		{
			assign( buffer, len );
			return true;
		}
		else
			return false;
	}

	void release ()
	{
		erase( begin(), end() );
	}

	bool GetConfigValue(string& val, string& key)
	{
		if (_NodeHead == NULL) {
			return false;
		}

		int KeyLength = key.length();
		//int ColonPos;
		XmlNode * pNode = _NodeHead;
		string::iterator buf = key.begin();
		int offset = 0;
		int length = KeyLength;
#define KEY_SEP ":"
		string sSearch(KEY_SEP);
		string::iterator itsrchStr = sSearch.begin();
		string::iterator srchStart = itsrchStr;
		string::iterator srchEnd   = itsrchStr + strlen(KEY_SEP);

		while (1) {
			//found = std::search( start, end, srchStart, srchEnd );
			string::iterator start = buf + offset;
			string::iterator end   = buf + (offset + length);


			string::iterator found = 
						std::search( start, end, srchStart, srchEnd );
			//ColonPos = key.find(":", KeyLength);
			if (found >= end) {
				if (pNode->bLeaf) {
					return false;
				}
				string Tag = key.substr(offset, length);
				CListXmlNode::iterator liVals = pNode->Vals.begin();
				bool bFound = false;
				while (liVals != pNode->Vals.end()) {
					XmlNode * pNodeTest = *liVals;
					if (pNodeTest->Name == Tag) {
						bFound = true;
						if (pNodeTest->bLeaf) {
							val = pNodeTest->Value;
							return true;
						}
						else {
							return false;
						}
						pNode = pNodeTest;
						break;
					}
					liVals++;
				}
				if (!bFound) {
					return false;
				}
				break;
			}
			else {
				if (found < start) {
					break;
				}
				long pos = (found - start);
				//pos += offset;
				string Tag = key.substr(offset, pos);

				if (pNode->bLeaf) {
					return false;
				}
				CListXmlNode::iterator liVals = pNode->Vals.begin();
				bool bFound = false;
				while (liVals != pNode->Vals.end()) {
					XmlNode * pNodeTest = *liVals;
					if (pNodeTest->Name == Tag) {
						bFound = true;
						pNode = pNodeTest;
						break;
					}
					liVals++;
				}
				if (!bFound) {
					return false;
				}
				offset += pos+1;
				length -= pos+1;

			}
		}

		return true;
	}

	// notify methods
	void foundNode		( string & name, string & attributes );
	void foundElement	( string & name, string & value, string & attributes );

	void startElement	( string & name, string & value, string & attributes );
	void endElement		( string & name, string & value, string & attributes );

	// save/load stream
	bool save		( char * buffer );
	bool load		( char * buffer );

	// parse the current buffer
	bool parse			();
	bool parse			( char * buffer, long parseLength );
	bool parseNodes		(	XmlNode * pNode, XmlParser & parser, 
							char * buffer, long parseLength );

	// get/set subscriber
	bool hasSubscriber ()
	{
		if ( _subscriber )
			return true;
		else
			return false;
	}

	XmlNotify * getSubscriber ()
	{
		return _subscriber;
	}

	void setSubscriber ( XmlNotify & set )
	{
		_subscriber = &set;
	}


	// get ref to tag stream
	XmlStream & getTagStream ()
	{ return *this; }


	// get string ref
	string & str()
	{ return (string &) *this; }
};



#endif