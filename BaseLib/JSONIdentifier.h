#ifndef __JSON_IDENTIFIER_H__
#define __JSON_IDENTIFIER_H__
#include "PrimitiveTypes.h"
#include "TextParserConfig.h"


TRISTATE ParseJSONExactIdentifier(CTextParser* pcTextParser, char* szIdentifier);
TRISTATE ParseJSONIdentifier(CTextParser* pcTextParser, char* szIdentifier, size* piLength);


#endif // __JSON_IDENTIFIER_H__

