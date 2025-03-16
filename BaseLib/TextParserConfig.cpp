#include "TextParserConfig.h"

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void STextParserConfig::Init(TextParserSkipWhitespace pfSkipWhitespace, TextParserParseString pfParseString, TextParserParseExactIdentifier pfParseExactIdentifier, TextParserParseIdentifier pfParseIdentifier)
{
	fSkipWhitespace = pfSkipWhitespace;
	fParseString = pfParseString;
	fParseExactIdentifier = pfParseExactIdentifier;
	fParseIdentifier = pfParseIdentifier;
}

