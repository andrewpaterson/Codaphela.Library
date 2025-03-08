#ifndef __TEXT_PARSER_CONFIG_H__
#define __TEXT_PARSER_CONFIG_H__


class CTextParser;


typedef void(*TextParserSkipWhiteSpace)(CTextParser*);

struct STextParserConfig
{
	TextParserSkipWhiteSpace	fSkipWhiteSpace;

	void Init(TextParserSkipWhiteSpace pfSkipWhiteSpace);
};


#endif // __TEXT_PARSER_CONFIG_H__

