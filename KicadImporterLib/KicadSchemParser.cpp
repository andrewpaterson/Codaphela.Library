/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela KicadImporterLib

Codaphela KicadImporterLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela KicadImporterLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela KicadImporterLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "BaseLib/Logger.h"
#include "BaseLib/CPPString.h"
#include "BaseLib/CPPIdentifier.h"
#include "KicadSchemParser.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CKicadSchemParser::Init(char* szText, lsize iTextLen, char* szFileName, CKicadSchemExpressions* pcExpressions)
{
	STextParserConfig	sConfig;
	bool				bResult;

	//Only the whitespace is KiCad's.  Lists, strings and symbols are read a character at a time below.
	sConfig.Init(SkipKicadSchemWhitespace, ParseCPPString, ParseCPPExactIdentifier, ParseCPPIdentifier);

	//The text must not contain carriage returns (see PassifyNewlines).
	bResult = mcParser.Init(szText, iTextLen, &sConfig);
	mszFileName.Init(szFileName);
	mpcExpressions = pcExpressions;

	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CKicadSchemParser::Kill(void)
{
	mpcExpressions = NULL;
	mszFileName.Kill();
	mcParser.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CKicadSchemParser::Parse(void)
{
	TRISTATE				tResult;
	CKicadSchemExpression*	pcRoot;

	mcParser.SkipUTF8BOM();

	tResult = ParseList(&pcRoot);
	if (tResult == TRIFALSE)
	{
		return Error("Expected '(' to start the file.");
	}
	else if (tResult == TRIERROR)
	{
		return TRIERROR;
	}

	mcParser.SkipWhitespace();
	if (!mcParser.IsOutside())
	{
		return Error("Expected the end of the file after the last ')'.");
	}

	mpcExpressions->SetRoot(pcRoot);
	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CKicadSchemParser::ParseList(CKicadSchemExpression** ppcList)
{
	TRISTATE				tResult;
	CKicadSchemExpression*	pcList;
	CKicadSchemExpression*	pcChild;
	const char*				szStart;
	size					uiLine;
	char					c;
	CChars					szError;

	mcParser.SkipWhitespace();
	if (mcParser.IsOutside() || (mcParser.Current() != '('))
	{
		return TRIFALSE;
	}

	uiLine = mcParser.Line() + 1;
	mcParser.StepRight();
	mcParser.SkipWhitespace();

	szStart = mcParser.Position();
	while (!mcParser.IsOutside() && !IsKicadSchemDelimiter(mcParser.Current()))
	{
		mcParser.StepRight();
	}
	if (mcParser.Position() == szStart)
	{
		return Error("Expected a name after '('.");
	}

	pcList = mpcExpressions->Add(KSET_List, szStart, mcParser.Position() - szStart, uiLine);
	for (;;)
	{
		mcParser.SkipWhitespace();
		if (mcParser.IsOutside())
		{
			szError.Init("Expected ')' to close (");
			szError.Append(pcList->GetText());
			szError.Append(" from line ");
			szError.Append(uiLine);
			szError.Append('.');
			tResult = Error(szError.Text());
			szError.Kill();
			return tResult;
		}

		c = mcParser.Current();
		if (c == ')')
		{
			mcParser.StepRight();
			*ppcList = pcList;
			return TRITRUE;
		}
		else if (c == '(')
		{
			tResult = ParseList(&pcChild);
		}
		else if (c == '"')
		{
			tResult = ParseString(&pcChild);
		}
		else
		{
			tResult = ParseSymbol(&pcChild);
		}

		if (tResult != TRITRUE)
		{
			return TRIERROR;
		}
		pcList->AddChild(pcChild);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CKicadSchemParser::ParseString(CKicadSchemExpression** ppcString)
{
	CKicadSchemExpression*	pcString;
	char					c;

	//KiCad escapes quotes and backslashes in strings and writes newlines in text as \n.
	pcString = mpcExpressions->Add(KSET_String, NULL, 0, mcParser.Line() + 1);
	mcParser.StepRight();
	for (;;)
	{
		if (mcParser.IsOutside())
		{
			return Error("Expected '\"' to close the string.");
		}

		c = mcParser.Current();
		if (c == '"')
		{
			mcParser.StepRight();
			*ppcString = pcString;
			return TRITRUE;
		}
		else if (c == '\\')
		{
			mcParser.StepRight();
			if (mcParser.IsOutside())
			{
				return Error("Expected '\"' to close the string.");
			}

			c = mcParser.Current();
			if (c == 'n')
			{
				c = '\n';
			}
			else if (c == 't')
			{
				c = '\t';
			}
			else if (c == 'r')
			{
				c = '\r';
			}
		}

		pcString->AppendText(c);
		mcParser.StepRight();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CKicadSchemParser::ParseSymbol(CKicadSchemExpression** ppcSymbol)
{
	const char*	szStart;
	size		uiLine;

	uiLine = mcParser.Line() + 1;
	szStart = mcParser.Position();
	while (!mcParser.IsOutside() && !IsKicadSchemDelimiter(mcParser.Current()))
	{
		mcParser.StepRight();
	}

	*ppcSymbol = mpcExpressions->Add(KSET_Symbol, szStart, mcParser.Position() - szStart, uiLine);
	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CKicadSchemParser::Error(char* szError)
{
	CChars			sz;
	SLogConfig		sLogConfig;

	mcParser.SetErrorSyntaxError();
	sz.Init(mszFileName);
	sz.Append('(');
	sz.Append(mcParser.Line() + 1);
	sz.Append(',');
	sz.Append(mcParser.Column() + 1);
	sz.Append("): ");
	sz.Append(szError);
	if (!mcParser.IsOutside())
	{
		sz.AppendNewLine();
		mcParser.GetLineLocationMarkerString(&sz);
	}
	gcLogger.GetConfig(&sLogConfig);
	gcLogger.SetBreakOnError(false);
	gcLogger.Error2(__METHOD__, " ", sz.Text(), NULL);
	gcLogger.SetConfig(&sLogConfig);
	sz.Kill();
	return TRIERROR;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SkipKicadSchemWhitespace(CTextParser* pcTextParser)
{
	while (!pcTextParser->IsOutside() && IsKicadSchemWhitespace(pcTextParser->Current()))
	{
		pcTextParser->StepRight();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool IsKicadSchemWhitespace(char c)
{
	return ((c == ' ') ||
		(c == '\n') ||
		(c == '\t') ||
		(c == '\f') ||
		(c == '\v'));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool IsKicadSchemDelimiter(char c)
{
	return IsKicadSchemWhitespace(c) || (c == '(') || (c == ')') || (c == '"');
}

