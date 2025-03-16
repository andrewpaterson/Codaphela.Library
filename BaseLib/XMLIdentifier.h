#ifndef __XML_IDENTIFIER_H__
#define __XML_IDENTIFIER_H__
#include "PrimitiveTypes.h"
#include "TextParserConfig.h"


TRISTATE ParseXMLExactIdentifier(CTextParser* pcTextParser, char* szIdentifier);
TRISTATE ParseXMLIdentifier(CTextParser* pcTextParser, char* szIdentifier, size* piLength);


#endif // __XML_IDENTIFIER_H__

