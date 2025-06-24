#include "MarkupTag.h"
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
	mpcCurrent = NULL;
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
	mpcCurrent = NULL;
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
	mpcCurrent = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CBaseParser::GetString(SBaseParserString* psString, bool bSkipWhitespace)
{
	TRISTATE	tResult;
	size		iLength;

	mcParser.PushPosition();
	if (bSkipWhitespace)
	{
		mcParser.SkipWhitespace();
	}

	tResult = mcParser.GetString(NULL, &iLength, false);
	if (tResult == TRIERROR)
	{
		return TRIERROR;
	}
	else if (tResult == TRIFALSE)
	{
		return TRIFALSE;
	}
	mcParser.PopPosition();

	psString->szString.InitLength(iLength);

	if (bSkipWhitespace)
	{
		mcParser.SkipWhitespace();
	}

	psString->uiLine = mcParser.Line();
	psString->uiColumn = mcParser.Column();
	mcParser.GetString(psString->szString.Text(), NULL, false);
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


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CBaseParser::AddStringAttribute(char* szElementName, char* szString)
{
	bool	bNoName;
	bool	bResult;

	bNoName = StrEmpty(szElementName);
	if (bNoName)
	{
		bResult = mpcCurrent->AddStringAttribute(szString);
	}
	else
	{
		bResult = mpcCurrent->AddStringAttribute(szElementName, szString);
	}

	ReturnErrorOnFalse(bResult);
	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CBaseParser::AddNumberAttribute(char* szElementName, CNumber* pcNumber)
{
	bool	bNoName;
	bool	bResult;

	bNoName = StrEmpty(szElementName);
	if (bNoName)
	{
		bResult = mpcCurrent->AddNumberAttribute(pcNumber);
	}
	else
	{
		bResult = mpcCurrent->AddNumberAttribute(szElementName, pcNumber);
	}

	ReturnErrorOnFalse(bResult);
	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CBaseParser::AddNullAttribute(char* szElementName)
{
	bool	bNoName;
	bool	bResult;

	bNoName = StrEmpty(szElementName);
	if (bNoName)
	{
		bResult = mpcCurrent->AddNullAttribute();
	}
	else
	{
		bResult = mpcCurrent->AddNullAttribute(szElementName);
	}

	ReturnErrorOnFalse(bResult);
	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CBaseParser::AddBooleanAttribute(char* szElementName, bool bValue)
{
	bool	bNoName;
	bool	bResult;

	bNoName = StrEmpty(szElementName);
	if (bNoName)
	{
		bResult = mpcCurrent->AddBooleanAttribute(bValue);
	}
	else
	{
		bResult = mpcCurrent->AddBooleanAttribute(szElementName, bValue);
	}

	ReturnErrorOnFalse(bResult);
	return TRITRUE;
}

