#ifndef __XML_STRING_H__
#define __XML_STRING_H__
#include "PrimitiveTypes.h"
#include "TextParserConfig.h"


TRISTATE ParseXMLTextString(CTextParser* pcTextParser, char* szString, size* piLength);
TRISTATE ParseXMLValueString(CTextParser* pcTextParser, char* szString, size* piLength);
TRISTATE ParseXMLStringUnset(CTextParser* pcTextParser, char* szString, size* piLength);


#endif // __XML_STRING_H__

