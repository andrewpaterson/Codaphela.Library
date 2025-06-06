#ifndef __JSON_STRING_H__
#define __JSON_STRING_H__
#include "PrimitiveTypes.h"
#include "TextParserConfig.h"


TRISTATE ParseJSONString(CTextParser* pcTextParser, char* szString, size* piLength);


#endif // __JSON_STRING_H__

