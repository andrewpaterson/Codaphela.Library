#ifndef __TEXT_PARSER_CONFIG_H__
#define __TEXT_PARSER_CONFIG_H__
#include "ErrorTypes.h"
#include "PrimitiveTypes.h"


class CTextParser;


typedef void(*TextParserSkipWhitespace)(CTextParser*);
typedef TRISTATE(*TextParserParseString)(CTextParser*, char*, size*);

struct STextParserConfig
{
	TextParserSkipWhitespace	fSkipWhitespace;
	TextParserParseString		fParseString;

	void Init(TextParserSkipWhitespace pfSkipWhitespace, TextParserParseString pfParseString);
};


#endif // __TEXT_PARSER_CONFIG_H__

