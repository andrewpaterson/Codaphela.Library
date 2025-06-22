#include "BaseParser.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseParser::Init(char* szText, size iTextLen, char* szFileName, CLogger* pcLogger, TextParserSkipWhitespace pfSkipWhitespace, TextParserParseString pfParseString, TextParserParseExactIdentifier pfParseExactIdentifier, TextParserParseIdentifier pfParseIdentifier)
{
	STextParserConfig	sConfig;

	sConfig.Init(pfSkipWhitespace, pfParseString, pfParseExactIdentifier, pfParseIdentifier);

	mcParser.Init(szText, &sConfig);
	mszFilename.Init(szFileName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseParser::Init(char* szText, char* szFileName, CLogger* pcLogger, TextParserSkipWhitespace pfSkipWhitespace, TextParserParseString pfParseString, TextParserParseExactIdentifier pfParseExactIdentifier, TextParserParseIdentifier pfParseIdentifier)
{
	size	uiLength;

	uiLength = strlen(szText);

	Init(szText, uiLength, szFileName, pcLogger, pfSkipWhitespace, pfParseString, pfParseExactIdentifier, pfParseIdentifier);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseParser::Kill(void)
{
	mszFilename.Kill();
	mcParser.Kill();

	mpcLogger = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CBaseParser::GetString(CChars* psz)
{
	TRISTATE	tResult;
	size		iLength;

	mcParser.PushPosition();
	tResult = mcParser.GetString(NULL, &iLength, true);
	if (tResult == TRIERROR)
	{
		return TRIERROR;
	}
	else if (tResult == TRIFALSE)
	{
		return TRIFALSE;
	}
	mcParser.PopPosition();

	psz->InitLength(iLength);
	mcParser.GetString(psz->Text(), NULL, true);
	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CBaseParser::Error(char* szError)
{
	CChars			sz;
	SLogConfig		sLogConfig;

	mcParser.SetErrorSyntaxError();
	sz.Init(mszFilename);
	sz.Append('(');
	sz.Append(mcParser.Line());
	sz.Append(',');
	sz.Append(mcParser.Column());
	sz.Append("): ");
	sz.Append(szError);
	sz.AppendNewLine();
	mcParser.GetLineLocationMarkerString(&sz);
	mpcLogger->GetConfig(&sLogConfig);
	mpcLogger->SetBreakOnError(false);
	mpcLogger->Error2(__METHOD__, " ", sz.Text(), NULL);
	mpcLogger->SetConfig(&sLogConfig);
	sz.Kill();
	return TRIERROR;
}

