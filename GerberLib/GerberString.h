#ifndef __GERBER_STRING_H__
#define __GERBER_STRING_H__
#include "BaseLib/PrimitiveTypes.h"
#include "BaseLib/TextParserConfig.h"


TRISTATE ParseGerberEscapeCharacter(CTextParser* pcTextParser, char* szEscape);
TRISTATE ParseGerberString(CTextParser* pcTextParser, char* szString, size* piLength);


#endif // __GERBER_STRING_H__

