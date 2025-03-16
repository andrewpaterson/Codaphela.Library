#ifndef __GERBER_STRING_H__
#define __GERBER_STRING_H__
#include "BaseLib/PrimitiveTypes.h"
#include "BaseLib/TextParserConfig.h"


TRISTATE ParseGerberEscapeCharacter(CTextParser* pcTextParser, char* szEscape);
TRISTATE ParseGerberCommentString(CTextParser* pcTextParser, char* szString, size* piLength);
TRISTATE ParseGerberFieldString(CTextParser* pcTextParser, char* szString, size* piLength);
TRISTATE ParseGerberStringUnset(CTextParser* pcTextParser, char* szString, size* piLength);


#endif // __GERBER_STRING_H__

