#ifndef __GERBER_IDENTIFIER_H__
#define __GERBER_IDENTIFIER_H__
#include "BaseLib/PrimitiveTypes.h"
#include "BaseLib/TextParserConfig.h"


TRISTATE ParseGerberExactIdentifier(CTextParser* pcTextParser, const char* szIdentifier);
TRISTATE ParseGerberExactIdentifierUnset(CTextParser* pcTextParser, const char* szIdentifier);

TRISTATE ParseGerberIdentifier(CTextParser* pcTextParser, char* szIdentifier, size* piLength);
TRISTATE ParseGerberdentifierUnset(CTextParser* pcTextParser, char* szIdentifier, size* piLength);


#endif // __GERBER_IDENTIFIER_H__

