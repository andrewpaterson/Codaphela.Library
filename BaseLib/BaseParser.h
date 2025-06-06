#ifndef __BASE_PARSER_H__
#define __BASE_PARSER_H__
#include "TextParser.h"
#include "Logger.h"


class CBaseParser
{
protected:
	CTextParser		mcParser;
	CChars			mszFilename;
	CLogger*		mpcLogger;

public:
	void		Init(char* szText, size iTextLen, char* szFileName, CLogger* pcLogger, TextParserSkipWhitespace pfSkipWhitespace, TextParserParseString pfParseString, TextParserParseExactIdentifier pfParseExactIdentifier, TextParserParseIdentifier pfParseIdentifier);
	void		Init(char* szText, char* szFileName, CLogger* pcLogger, TextParserSkipWhitespace pfSkipWhitespace, TextParserParseString pfParseString, TextParserParseExactIdentifier pfParseExactIdentifier, TextParserParseIdentifier pfParseIdentifier);
	void		Kill(void);

protected:
	TRISTATE	Error(char* szError);
	TRISTATE	GetString(CChars* psz);
};


#endif // !__BASE_PARSER_H__

