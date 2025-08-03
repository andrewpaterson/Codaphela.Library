#ifndef __BASE_PARSER_H__
#define __BASE_PARSER_H__
#include "TextParser.h"
#include "Logger.h"
#include "BaseParserString.h"


class CMarkupTag;
class CBaseParser
{
protected:
	CTextParser		mcParser;
	CChars			mszFilename;
	CLogger*		mpcLogger;
	CMarkupTag*		mpcCurrent;

public:
	void		Init(char* szText, size iTextLen, char* szFileName, CLogger* pcLogger, TextParserSkipWhitespace pfSkipWhitespace, TextParserParseString pfParseString, TextParserParseExactIdentifier pfParseExactIdentifier, TextParserParseIdentifier pfParseIdentifier);
	void		Init(char* szText, char* szFileName, CLogger* pcLogger, TextParserSkipWhitespace pfSkipWhitespace, TextParserParseString pfParseString, TextParserParseExactIdentifier pfParseExactIdentifier, TextParserParseIdentifier pfParseIdentifier);
	void		Kill(void);

protected:
	TRISTATE	Error(char* szError);
	TRISTATE	GetString(SBaseParserString* psString, bool bSkipWhitespace);

	TRISTATE	AddStringAttribute(char* szElementName, char* szString, bool bAllowDuplicates);
	TRISTATE	AddNumberAttribute(char* szElementName, CNumber* pcNumber, bool bAllowDuplicates);
	TRISTATE	AddBooleanAttribute(char* szElementName, bool bValue, bool bAllowDuplicates);
	TRISTATE	AddNullAttribute(char* szElementName, bool bAllowDuplicates);
};


#endif // __BASE_PARSER_H__

