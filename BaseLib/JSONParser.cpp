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
	mpcLogger = pcLogger;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJSONParser::Kill(void)
{
	CBaseParser::Kill();

	mpcDoc = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CJSONParser::Parse(void)
{
	TRISTATE			tResult;
	SBaseParserString	sString;

	tResult = mcParser.SkipUTF8BOM();
	if (tResult == TRIERROR)
	{
		return TRIERROR;
	}

	sString.Init(mcParser.Column(), mcParser.Line());
	tResult = ParseElement(&sString);
	sString.Kill();

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
TRISTATE CJSONParser::ParseElement(SBaseParserString* psElement)
{
	TRISTATE	tResult;

	mcParser.PushPosition();
	mcParser.SkipWhitespace();

	tResult = ParseValue(psElement);

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
TRISTATE CJSONParser::ParseValue(SBaseParserString* psElement)
{
	TRISTATE	tResult;

	tResult = ParseObject(psElement);
	ReturnOnError(tResult);
	if (tResult == TRITRUE)
	{
		return TRITRUE;
	}

	tResult = ParseArray(psElement);
	ReturnOnError(tResult);
	if (tResult == TRITRUE)
	{
		return TRITRUE;
	}

	tResult = ParseLiteral(psElement->GetString());
	ReturnOnError(tResult);
	if (tResult == TRITRUE)
	{
		return TRITRUE;
	}

	tResult = ParseString(psElement->GetString());
	ReturnOnError(tResult);
	if (tResult == TRITRUE)
	{
		return TRITRUE;
	}

	tResult = ParseNumber(psElement->GetString());
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
TRISTATE CJSONParser::ParseObject(SBaseParserString* psElement)
{
	TRISTATE	tResult;
	bool		bFirst;
	bool		bLast;

	tResult = mcParser.GetExactCharacter('{');
	ReturnOnErrorAndFalse(tResult);

	mcParser.PushPosition();

	mpcCurrent = mpcDoc->AppendTag(mpcCurrent, psElement->szString.Text(), psElement->uiLine, psElement->uiColumn);

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
TRISTATE CJSONParser::ParseArray(SBaseParserString* psElement)
{
	TRISTATE			tResult;
	bool				bFirst;
	bool				bLast;
	SBaseParserString	sString;

	tResult = mcParser.GetExactCharacter('[');
	ReturnOnErrorAndFalse(tResult);

	mcParser.PushPosition();

	mpcCurrent = mpcDoc->AppendTag(mpcCurrent, psElement->GetString(), psElement->uiLine, psElement->uiColumn);

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
		sString.Init(mcParser.Column(), mcParser.Line());
		tResult = ParseElement(&sString);
		sString.Kill();
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
	TRISTATE			tResult;
	SBaseParserString	sString;

	tResult = GetString(&sString, true);
	ReturnOnErrorAndFalse(tResult);

	tResult = mcParser.GetExactCharacter(':');
	if (tResult == TRIERROR)
	{
		sString.Kill();
		return TRIERROR;
	}
	
	if (tResult == TRIFALSE)
	{
		sString.Kill();
		Error("Expected ':'");
		return TRIERROR;
	}

	tResult = ParseElement(&sString);
	if (tResult == TRIERROR)
	{
		sString.Kill();
		return TRIERROR;
	}

	if (tResult == TRIFALSE)
	{
		sString.Kill();
		Error("Expected Element");
		return TRIERROR;
	}

	sString.Kill();
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
		return AddBooleanAttribute(szElementName, true);
	}

	tResult = mcParser.GetExactCharacterSequence("\"false\"");
	ReturnOnError(tResult);
	if (tResult == TRITRUE)
	{
		return AddBooleanAttribute(szElementName, true);
	}

	tResult = mcParser.GetExactCharacterSequence("\"null\"");
	ReturnOnError(tResult);
	if (tResult == TRITRUE)
	{
		return AddNullAttribute(szElementName);
	}

	return TRIFALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CJSONParser::ParseString(char* szElementName)
{
	TRISTATE			tResult;
	SBaseParserString	sString;

	tResult = GetString(&sString, true);
	ReturnOnError(tResult);

	if (tResult == TRIFALSE)
	{
		return TRIFALSE;
	}

	tResult = AddStringAttribute(szElementName, sString.GetString());
	sString.Kill();

	ReturnErrorOnFalse(tResult);
	return tResult;
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

	tResult = AddNumberAttribute(szElementName, &cNumber);

	ReturnErrorOnFalse(tResult);
	return tResult;
}

