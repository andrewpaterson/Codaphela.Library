#include "CPPString.h"
#include "JSONString.h"


//////////////////////////////////////////////////////////////////////////
//
// A JSON string is parsed as a C++ string is.
//
/////////////////////////////////////////////////////////////////////////
TRISTATE ParseJSONString(CTextParser* pcTextParser, char* szString, size* piLength)
{
	return ParseCPPString(pcTextParser, szString, piLength);
}

