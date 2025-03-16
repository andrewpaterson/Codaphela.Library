#ifndef __CPP_IDENTIFIER_H__
#define __CPP_IDENTIFIER_H__
#include "PrimitiveTypes.h"
#include "TextParserConfig.h"


TRISTATE ParseCPPExactIdentifier(CTextParser* pcTextParser, char* szIdentifier);
TRISTATE ParseCPPIdentifier(CTextParser* pcTextParser, char* szIdentifier, size* piLength);


#endif // __CPP_STRING_H__

