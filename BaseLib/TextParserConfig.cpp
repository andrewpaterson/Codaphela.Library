#include "TextParserConfig.h"

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void STextParserConfig::Init(TextParserSkipWhiteSpace pfSkipWhiteSpace, TextParserParseString pfParseString)
{
	fSkipWhiteSpace = pfSkipWhiteSpace;
	fParseString = pfParseString;
}

