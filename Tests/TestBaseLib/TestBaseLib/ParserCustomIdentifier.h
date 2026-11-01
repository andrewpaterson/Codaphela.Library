#ifndef __PARSER_CUSTOM_IDENTIFIER_H__
#define __PARSER_CUSTOM_IDENTIFIER_H__
#include "BaseLib/PrimitiveTypes.h"
#include "BaseLib/TextParserConfig.h"


TRISTATE ParseCustomExactIdentifier(CTextParser* pcTextParser, const char* szIdentifier);
TRISTATE ParseCustomIdentifier(CTextParser* pcTextParser, char* szIdentifier, size* piLength);


#endif // __PARSER_CUSTOM_IDENTIFIER_H__

