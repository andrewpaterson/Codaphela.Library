#ifndef __TEXT_PARSER_CONFIG_H__
#define __TEXT_PARSER_CONFIG_H__
#include "ErrorTypes.h"
#include "PrimitiveTypes.h"


class CTextParser;


typedef void(*TextParserSkipWhiteSpace)(CTextParser*);
typedef TRISTATE(*TextParserParseString)(CTextParser*, char*, size*);

struct STextParserConfig
{
	TextParserSkipWhiteSpace	fSkipWhiteSpace;
	TextParserParseString		fParseString;

	void Init(TextParserSkipWhiteSpace pfSkipWhiteSpace, TextParserParseString pfParseString);
};


#endif // __TEXT_PARSER_CONFIG_H__

