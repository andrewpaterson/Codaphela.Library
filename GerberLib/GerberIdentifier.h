#ifndef __GERBER_IDENTIFIER_H__
#define __GERBER_IDENTIFIER_H__
#include "BaseLib/PrimitiveTypes.h"
#include "BaseLib/TextParserConfig.h"


TRISTATE ParseGerberExactIdentifier(CTextParser* pcTextParser, char* szIdentifier);
TRISTATE ParseGerberIdentifier(CTextParser* pcTextParser, char* szIdentifier, size* piLength);


#endif // __GERBER_IDENTIFIER_H__

