#include "TextParserConfig.h"

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void STextParserConfig::Init(TextParserSkipWhitespace pfSkipWhitespace, TextParserParseString pfParseString)
{
	fSkipWhitespace = pfSkipWhitespace;
	fParseString = pfParseString;
}

