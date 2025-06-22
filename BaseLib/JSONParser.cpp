/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

This file is part of The Codaphela Project: Codaphela BaseLib

Codaphela BaseLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela BaseLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela BaseLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#include "Logger.h"
#include "JSONParser.h"
#include "JSONWhitespace.h"
#include "JSONString.h"
#include "JSONIdentifier.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJSONParser::Init(char* szText, char* szFileName, CMarkup* pcMarkup, CLogger* pcLogger)
{
	Init(szText, szFileName, pcMarkup->mpcDoc, pcLogger);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJSONParser::Init(char* szText, char* szFileName, CMarkupDoc* pcDoc, CLogger* pcLogger)
{
	CBaseParser::Init(szText, szFileName, pcLogger, SkipJSONWhitespace, ParseJSONString, ParseJSONExactIdentifier, ParseJSONIdentifier);

	mpcDoc = pcDoc;
	miDepth = 0;
	mpcCurrent = NULL;
	mpcLogger = pcLogger;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJSONParser::Init(char* szText, size iTextLen, char* szFileName, CMarkup* pcMarkup, CLogger* pcLogger)
{
	Init(szText, iTextLen, szFileName, pcMarkup->mpcDoc, pcLogger);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJSONParser::Init(char* szText, size iTextLen, char* szFileName, CMarkupDoc* pcDoc, CLogger* pcLogger)
{
	CBaseParser::Init(szText, iTextLen, szFileName, pcLogger, SkipJSONWhitespace, ParseJSONString, ParseJSONExactIdentifier, ParseJSONIdentifier);

	mpcDoc = pcDoc;
	miDepth = 0;
	mpcCurrent = NULL;
	mpcLogger = pcLogger;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJSONParser::Kill(void)
{
	CBaseParser::Kill();

	mpcCurrent = NULL;
	mpcDoc = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CJSONParser::Parse(void)
{
	TRISTATE	tResult;

	tResult = mcParser.SkipUTF8BOM();
	if (tResult == TRIERROR)
	{
		return TRIERROR;
	}

	tResult = ParseElement("");

	if (tResult != TRITRUE)
	{
		return TRIERROR;
	}
	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CJSONParser::ParseElement(char* szElementName)
{
	TRISTATE	tResult;

	mcParser.PushPosition();
	mcParser.SkipWhitespace();

	tResult = ParseValue(szElementName);

	mcParser.SkipWhitespace();

	if (tResult != TRITRUE)
	{
		mcParser.PopPosition();
		return tResult;
	}

	mcParser.PassPosition();
	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CJSONParser::ParseValue(char* szElementName)
{
	TRISTATE	tResult;

	tResult = ParseObject(szElementName);
	ReturnOnError(tResult);
	if (tResult == TRITRUE)
	{
		return TRITRUE;
	}

	tResult = ParseArray(szElementName);
	ReturnOnError(tResult);
	if (tResult == TRITRUE)
	{
		return TRITRUE;
	}

	tResult = ParseLiteral(szElementName);
	ReturnOnError(tResult);
	if (tResult == TRITRUE)
	{
		return TRITRUE;
	}

	tResult = ParseString(szElementName);
	ReturnOnError(tResult);
	if (tResult == TRITRUE)
	{
		return TRITRUE;
	}

	tResult = ParseNumber(szElementName);
	ReturnOnError(tResult);
	if (tResult == TRITRUE)
	{
		return TRITRUE;
	}

	return tResult;
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CJSONParser::ParseObject(char* szElementName)
{
	TRISTATE	tResult;
	bool		bFirst;
	bool		bLast;
	size		iLine;
	size		iColumn;

	iLine = mcParser.Line();
	iColumn = mcParser.Column();

	tResult = mcParser.GetExactCharacter('{');
	ReturnOnErrorAndFalse(tResult);

	mcParser.PushPosition();

	mpcCurrent = mpcDoc->AppendTag(mpcCurrent, szElementName, iLine, iColumn);

	bFirst = true;
	bLast = false;
	for (;;)
	{
		mcParser.SkipWhitespace();
		tResult = mcParser.GetExactCharacter('}');
		if (tResult == TRIERROR)
		{
			mcParser.PopPosition();
			return TRIERROR;
		}
		if ((tResult == TRIFALSE) && bLast)
		{
			Error("Expected '}'");
			mcParser.PopPosition();
			return TRIERROR;
		}
		if (tResult == TRITRUE)
		{
			mpcCurrent = mpcCurrent->mpcParent;
			mcParser.PassPosition();
			return TRITRUE;
		}

		if (!bFirst)
		{
			tResult = mcParser.GetExactCharacter(',');
			if (tResult == TRIERROR)
			{
				mcParser.PopPosition();
				return TRIERROR;
			}
			if (tResult == TRIFALSE)
			{
				Error("Expected ','");
				mcParser.PopPosition();
				return TRIERROR;
			}
		}


		bFirst = false;
		tResult = ParseMember();
		if (tResult == TRIERROR)
		{
			mcParser.PopPosition();
			return TRIERROR;
		}
		if (tResult == TRIFALSE)
		{
			bLast = true;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CJSONParser::ParseArray(char* szElementName)
{
	TRISTATE	tResult;
	bool		bFirst;
	bool		bLast;

	tResult = mcParser.GetExactCharacter('[');
	ReturnOnErrorAndFalse(tResult);

	mcParser.PushPosition();

	bFirst = true;
	bLast = false;
	for (;;)
	{
		mcParser.SkipWhitespace();
		tResult = mcParser.GetExactCharacter(']');
		if (tResult == TRIERROR)
		{
			mcParser.PopPosition();
			return TRIERROR;
		}
		if ((tResult == TRIFALSE) && bLast)
		{
			Error("Expected ']'");
			mcParser.PopPosition();
			return TRIERROR;
		}

		if (tResult == TRITRUE)
		{
			mcParser.PassPosition();
			return TRITRUE;
		}

		if (!bFirst)
		{
			tResult = mcParser.GetExactCharacter(',');
			if (tResult == TRIERROR)
			{
				mcParser.PopPosition();
				return TRIERROR;
			}
			if (tResult == TRIFALSE)
			{
				Error("Expected ','");
				mcParser.PopPosition();
				return TRIERROR;
			}
		}

		bFirst = false;
		tResult = ParseElement(szElementName);
		if (tResult == TRIERROR)
		{
			mcParser.PopPosition();
			return TRIERROR;
		}
		if (tResult == TRIFALSE)
		{
			bLast = true;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CJSONParser::ParseMember(void)
{
	TRISTATE	tResult;
	CChars		szString;

	tResult = GetString(&szString);
	ReturnOnErrorAndFalse(tResult);

	tResult = mcParser.GetExactCharacter(':');
	if (tResult == TRIERROR)
	{
		szString.Kill();
		return TRIERROR;
	}
	
	if (tResult == TRIFALSE)
	{
		szString.Kill();
		Error("Expected ':'");
		return TRIERROR;
	}

	tResult = ParseElement(szString.Text());
	if (tResult == TRIERROR)
	{
		szString.Kill();
		return TRIERROR;
	}

	if (tResult == TRIFALSE)
	{
		szString.Kill();
		Error("Expected Element");
		return TRIERROR;
	}

	szString.Kill();
	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CJSONParser::ParseLiteral(char* szElementName)
{
	TRISTATE	tResult;
	
	tResult = mcParser.GetExactCharacterSequence("\"true\"");
	ReturnOnError(tResult);
	if (tResult == TRITRUE)
	{
		mpcCurrent->AddBooleanAttribute(szElementName, true);
		return TRITRUE;
	}

	tResult = mcParser.GetExactCharacterSequence("\"false\"");
	ReturnOnError(tResult);
	if (tResult == TRITRUE)
	{
		mpcCurrent->AddBooleanAttribute(szElementName, false);
		return TRITRUE;
	}

	tResult = mcParser.GetExactCharacterSequence("\"null\"");
	ReturnOnError(tResult);
	if (tResult == TRITRUE)
	{
		mpcCurrent->AddNullAttribute(szElementName);
		return TRITRUE;
	}

	return TRIFALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CJSONParser::ParseString(char* szElementName)
{
	TRISTATE	tResult;
	CChars		szString;

	tResult = GetString(&szString);
	ReturnOnError(tResult);

	if (tResult == TRIFALSE)
	{
		return TRIFALSE;
	}

	mpcCurrent->AddStringAttribute(szElementName, szString.Text());

	szString.Kill();
	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CJSONParser::ParseNumber(char* szElementName)
{
	TRISTATE	tResult;
	CNumber		cNumber;

	tResult = mcParser.GetNumber(&cNumber);
	ReturnOnError(tResult);

	if (tResult == TRIFALSE)
	{
		return TRIFALSE;
	}

	mpcCurrent->AddNumberAttribute(szElementName, &cNumber);
	
	return TRITRUE;
}

