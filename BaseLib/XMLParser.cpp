/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2022 Andrew Paterson

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
#include "XMLParser.h"
#include "XMLWhitespace.h"
#include "XMLString.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CXMLParser::Init(CMarkup* pcMarkup, CLogger* pcLogger)
{
	Init(pcMarkup->mpcDoc, pcLogger);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CXMLParser::Init(CMarkupDoc* pcDoc, CLogger* pcLogger)
{
	mpcDoc = pcDoc;
	miDepth = 0;
	mpcCurrent = NULL;
	mpcLogger = pcLogger;
	mcProlog.Init();
	macEntities.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CXMLParser::Kill(void)
{
	size				i;
	CXMLEntity*		pcEntity;

	mpcCurrent = NULL;

	for (i = 0; i < macEntities.NumElements(); i++)
	{
		pcEntity = macEntities.Get(i);
		pcEntity->Kill();
	}
	macEntities.Kill();
	mcProlog.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CXMLParser::IsAllowedIdentifierChar(bool bFirstChar, uint8 cChar)
{
	if (((cChar >= 'a') && (cChar <= 'z')) || ((cChar >= 'A') && (cChar <= 'Z')))
	{
		return true;
	}

	if ((cChar == ':') || (cChar == '_') || (cChar == 0xf8))
	{
		return true;
	}

	if (((cChar >= 0xc0) && (cChar <= 0xd6)) || ((cChar >= 0xd8) && (cChar <= 0xf6)))
	{
		return true;
	}

	if (!bFirstChar)
	{
		if ((cChar >= '0') && (cChar <= '9'))
		{
			return true;
		}

		if ((cChar == '-') || (cChar == '.') || (cChar == 0xb7))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CXMLParser::IsAllowedTextChar(uint8 cChar)
{
	if ((cChar == '&') || (cChar == '>') || (cChar == '<'))
	{
		return false;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CXMLParser::Parse(char* szText, char* szSourceContext)
{
	TRISTATE			tResult;
	STextParserConfig	sConfig;

	mszSourceContext = szSourceContext;

	sConfig.Init(SkipXMLWhitespace, ParseXMLString);
	mcParser.Init(szText, &sConfig);
	tResult = mcParser.SkipUTF8BOM();
	if (tResult == TRIERROR)
	{
		return TRIERROR;
	}

	tResult = ParseProlog();
	if (tResult == TRIERROR)
	{
		return TRIERROR;
	}
	
	tResult = ParseComments();
	if (tResult == TRIERROR)
	{
		return TRIERROR;
	}

	tResult = ParseDocType();
	if (tResult == TRIERROR)
	{
		return TRIERROR;
	}

	tResult = ParseComments();
	if (tResult == TRIERROR)
	{
		return TRIERROR;
	}

	tResult = ParseElement();

	mcParser.Kill();
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
TRISTATE CXMLParser::FailOnErrorReturnOnFalse(TRISTATE tResult, char* szExpected)
{
	CChars	szError;

	if (tResult == TRIERROR)
	{
		szError.Init("Expected [");
		szError.Append(szExpected);
		szError.Append("].  Got ");
		mcParser.AppendError(&szError);
		szError.Append('.');
		szError.AppendNewLine();
		mcParser.PrintPosition(&szError);
		mpcLogger->Warning(szError.Text());
		szError.Kill();
	}
	return tResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CXMLParser::FailAlways(TRISTATE tResult, char* szExpected)
{
	CChars	szError;

	szError.Init("Expected [");
	szError.Append(szExpected);
	szError.Append("].");
	if (tResult == TRIERROR)
	{
		szError.Append("  Got ");
		mcParser.AppendError(&szError);
		szError.Append('.');
	}
	szError.AppendNewLine();
	mcParser.PrintPosition(&szError);
	mpcLogger->Warning(szError.Text());
	szError.Kill();

	return TRIERROR;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CXMLParser::ParseVersion(void)
{
	TRISTATE	tResult;
	double		pfVersion;
	char		acQuote[2];

	tResult = mcParser.GetExactIdentifier("version");
	if (NotTrue(tResult))
	{
		return FailAlways(tResult, "version");
	}

	tResult = mcParser.GetExactCharacter('=', false);
	if (NotTrue(tResult))
	{
		return FailAlways(tResult, "=");
	}
	
	acQuote[0] = '\0';
	acQuote[1] = '\0';
	tResult = mcParser.GetHFOpeningQuote(acQuote);
	if (NotTrue(tResult))
	{
		return FailAlways(tResult, "'] or [\"");
	}

	tResult = mcParser.GetFloat(&pfVersion);
	if (NotTrue(tResult))
	{
		return FailAlways(tResult, "${float}");
	}

	tResult = mcParser.GetHFClosingQuote(acQuote[0]);
	if (NotTrue(tResult))
	{
		return FailAlways(tResult, acQuote);
	}

	mcProlog.SetVersion((float)pfVersion);

	if (pfVersion != 1.0f)
	{
		mpcLogger->Warning("Expected XML version equal to 1.0");
		return TRIERROR;
	}
	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CXMLParser::ParseEncoding(void)
{
	TRISTATE		tResult;
	CChars			szEncoding;
	ECharEncoding	eEncoding;

	tResult = mcParser.GetExactIdentifier("encoding");
	if (tResult != TRITRUE)
	{
		return TRIERROR;
	}

	szEncoding.Init();
	tResult = ParseValue(&szEncoding);
	if (tResult != TRITRUE)
	{
		szEncoding.Kill();
		return TRIERROR;
	}

	szEncoding.StripWhitespace();

	eEncoding = CCharEncoding::ParseEncoding(szEncoding.Text());
	szEncoding.Kill();

	mcProlog.SetEncoding(eEncoding);
	if (eEncoding == CE_NoSet || eEncoding == CE_Unkown)
	{
		return TRIERROR;
	}

	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CXMLParser::ParseStandalone(void)
{
	TRISTATE	tResult;
	CChars		szStandalone;

	tResult = mcParser.GetExactIdentifier("standalone");
	if (tResult == TRIFALSE)
	{
		mcProlog.SetStandalone(true);
		return TRITRUE;
	}
	ReturnOnError(tResult);

	szStandalone.Init();
	tResult = ParseValue(&szStandalone);
	if (tResult != TRITRUE)
	{
		szStandalone.Kill();
		return TRIERROR;
	}

	szStandalone.StripWhitespace();
	if (szStandalone.EqualsIgnoreCase("yes"))
	{
		mcProlog.SetStandalone(true);
		szStandalone.Kill();
		return TRITRUE;
	}
	else if (szStandalone.EqualsIgnoreCase("no"))
	{
		mcProlog.SetStandalone(false);
		szStandalone.Kill();
		return TRITRUE;
	}
	else
	{
		return TRIERROR;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CXMLParser::ParseProlog(void)
{
	TRISTATE	tResult;

	mcParser.PushPosition();
	tResult = mcParser.GetExactCharacterSequence("<?xml");
	if (tResult != TRITRUE)
	{
		mcParser.PopPosition();
		return tResult;
	}

	tResult = ParseVersion();
	if (tResult != TRITRUE)
	{
		mcParser.PopPosition();
		return TRIERROR;
	}

	tResult = ParseEncoding();
	if (tResult == TRIERROR)
	{
		mcParser.PopPosition();
		return TRIERROR;
	}

	tResult = ParseStandalone();
	if (tResult == TRIERROR)
	{
		mcParser.PopPosition();
		return TRIERROR;
	}

	tResult = mcParser.GetExactCharacterSequence("?>");
	if (tResult != TRITRUE)
	{
		mcParser.PopPosition();
		return TRIERROR;
	}
	mcParser.PassPosition();
	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CXMLParser::ParseIdentifier(CChars* pszIdentifier)
{
	char		c;
	TRISTATE	tResult;
	size		i;

	mcParser.SkipWhitespace();
	for (i = 0;; i++)
	{
		tResult = mcParser.GetCharacter(&c);
		if (tResult != TRITRUE)
		{
			if (i == 0)
			{
				return TRIFALSE;
			}
			return TRITRUE;
		}

		if (IsAllowedIdentifierChar(i == 0, c))
		{
			pszIdentifier->Append(c);
		}
		else
		{
			mcParser.StepLeft();
			if (i == 0)
			{
				return TRIFALSE;
			}
			return TRITRUE;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CXMLParser::ParseValue(CChars* pszValue)
{
	TRISTATE	tResult;
	char*		szStart;

	tResult = mcParser.GetExactCharacter('=', false);
	if (tResult != TRITRUE)
	{
		return FailAlways(tResult, "=");
	}

	szStart = mcParser.mszParserPos;
	tResult = mcParser.GetString(NULL, NULL, true);
	ReturnOnError(tResult);
	if (tResult == TRIFALSE)
	{
		return FailAlways(tResult, "'] or [\"");
	}

	pszValue->AppendSubString(szStart + 1, mcParser.mszParserPos - 1);
	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CXMLParser::ParseStartTagOrEmptyTag(bool* pbStart, CChars* pszIdentifier)
{
	TRISTATE	tResult;
	TRISTATE	tEmpty;
	TRISTATE	tClosing;
	size		iLine;
	size		iColumn;

	mcParser.PushPosition();

	tResult =  mcParser.GetExactCharacter('<');
	if (tResult != TRITRUE)
	{
		mcParser.PopPosition();
		return TRIFALSE;
	}
	iLine = mcParser.Line();
	iColumn = mcParser.Column();

	tResult = ParseIdentifier(pszIdentifier);
	if (tResult != TRITRUE)
	{
		mcParser.PopPosition();
		return TRIFALSE;
	}
	
	if (!mpcCurrent)
	{
		//This should be a function on Doc.
		mpcCurrent = mpcDoc->SetRootTag(pszIdentifier->Text());
	}
	else
	{
		mpcCurrent = mpcCurrent->AppendTag(pszIdentifier->Text());
	}
	mpcCurrent->SetLineAndColumn(iLine, iColumn);

	for (;;)
	{
		tResult = ParseAttribute();
		if (tResult == TRIERROR)
		{
			mcParser.PopPosition();
			return TRIERROR;
		}
		if (tResult == TRIFALSE)
		{
			break;
		}
	}

	tEmpty =  mcParser.GetExactCharacter('/', false);
	if (tEmpty == TRIERROR)
	{
		mcParser.PopPosition();
		return TRIERROR;
	}
	else if (tEmpty == TRITRUE)
	{
		*pbStart = false;
	}
	else if (tEmpty == TRIFALSE)
	{
		*pbStart = true;
	}

	tClosing =  mcParser.GetExactCharacter('>', false);
	if (tClosing != TRITRUE)
	{
		mcParser.PopPosition();
		return TRIERROR;
	}

	if (!*pbStart)
	{
		mpcCurrent = mpcCurrent->mpcParent;
	}

	mcParser.PassPosition();
	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CXMLParser::ParseEndTag(char* szIdentifier)
{
	TRISTATE	tResult;
	CChars		szEndName;

	mcParser.PushPosition();

	tResult = mcParser.GetExactCharacterSequence("</");
	if (tResult != TRITRUE)
	{
		mcParser.PopPosition();
		return tResult;
	}

	szEndName.Init();
	tResult = ParseIdentifier(&szEndName);

	if (tResult != TRITRUE)
	{
		mcParser.PopPosition();
		szEndName.Kill();
		return TRIERROR;
	}

	if (!szEndName.Equals(szIdentifier))
	{
		mcParser.PopPosition();
		szEndName.Kill();
		return TRIERROR;
	}
	szEndName.Kill();

	tResult = mcParser.GetExactCharacter('>');
	if (tResult != TRITRUE)
	{
		mcParser.PopPosition();
		return TRIERROR;
	}

	mpcCurrent = mpcCurrent->mpcParent;
	mcParser.PassPosition();
	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CXMLParser::ParseContent(void)
{
	TRISTATE	tResult;

	for (;;)
	{
		tResult = ParseComment();
		if (tResult == TRITRUE)
		{
			continue;
		}
		else if (tResult == TRIERROR)
		{
			return TRIERROR;
		}

		tResult = ParseElement();
		if (tResult == TRITRUE)
		{
			continue;
		}
		else if (tResult == TRIERROR)
		{
			return TRIERROR;
		}

		tResult = ParseText();
		if (tResult == TRITRUE)
		{
			continue;
		}
		else if (tResult == TRIERROR)
		{
			return TRIERROR;
		}

		tResult = ParseNamedReference();
		if (tResult == TRITRUE)
		{
			continue;
		}
		else if (tResult == TRIERROR)
		{
			return TRIERROR;
		}
		break;
	}
 	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CXMLParser::ParseNamedReference(void)
{
	TRISTATE			tResult;
	size				iLine;
	size				iColumn;
	CChars				szIdentifier;
	CMarkupNamedRef*	pcNamedRef;

	mcParser.SkipWhitespace();

	iLine = mcParser.Line();
	iColumn = mcParser.Column();

	tResult = mcParser.GetExactCharacter('&', false);
	if (tResult == TRITRUE)
	{
		szIdentifier.Init();
		ParseIdentifier(&szIdentifier);
		if (!szIdentifier.Empty())
		{
			pcNamedRef = mpcCurrent->AppendNamedReference(szIdentifier.Text());
			pcNamedRef->SetLineAndColumn(mcParser.miLine, mcParser.miColumn);
			szIdentifier.Kill();
			tResult = mcParser.GetExactCharacter(';', false);
			if (tResult == TRIERROR)
			{
				return TRIERROR;
			}
			return TRITRUE;
		}
		else
		{
			szIdentifier.Kill();
			return TRIERROR;
		}
	}
	return tResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CXMLParser::ParseText(void)
{
	TRISTATE		tResult;
	CChars			szText;
	CMarkupText*	pcMarkupText;
	size			iLine;
	size			iColumn;

	iLine = mcParser.Line();
	iColumn = mcParser.Column();

	szText.Init();
	tResult = ParseText(&szText);
	if (tResult == TRITRUE)
	{
		if (!szText.IsWhitespace())
		{
			pcMarkupText = mpcCurrent->AppendText(szText.Text());
			pcMarkupText->SetLineAndColumn(mcParser.miLine, mcParser.miColumn);
		}
		szText.Kill();
		return TRITRUE;
	}
	szText.Kill();
	return tResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CXMLParser::ParseComment(void)
{
	TRISTATE	tResult;

	mcParser.PushPosition();
	tResult = mcParser.GetExactCharacterSequence("<!--");
	if (tResult != TRITRUE)
	{
		mcParser.PopPosition();
		return TRIFALSE;
	}

	//Nested comments aren't allowed but we should still fail gracefully on them.
	tResult = mcParser.FindExactCharacterSequence("-->");
	if (tResult != TRITRUE)
	{
		mcParser.PopPosition();
		return TRIERROR;
	}

	mcParser.GetExactCharacterSequence("-->");
	mcParser.PassPosition();
	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CXMLParser::ParseComments(void)
{
	TRISTATE tResult;

	for (;;)
	{
		tResult = ParseComment();
		if (tResult == TRIERROR)
		{
			return TRIERROR;
		}
		else if (tResult == TRIFALSE)
		{
			return TRITRUE;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CXMLParser::ParseElement(void)
{
	bool		bStart;
	TRISTATE	tResult;
	CChars		szIdentifier;

	mcParser.PushPosition();

	szIdentifier.Init();
	tResult = ParseStartTagOrEmptyTag(&bStart, &szIdentifier);
	if (tResult != TRITRUE)
	{
		szIdentifier.Kill();
		mcParser.PopPosition();
		if (tResult == TRIERROR)
		{
			return TRIERROR;
		}
		else
		{
			return TRIFALSE;
		}
	}

	if (bStart)
	{
		miDepth++;
		tResult = ParseContent();
		if (tResult == TRIERROR)
		{
			szIdentifier.Kill();
			mcParser.PopPosition();
			return TRIERROR;
		}

		tResult = ParseEndTag(szIdentifier.Text());
		if (tResult != TRITRUE)
		{
			szIdentifier.Kill();
			mcParser.PopPosition();
			return TRIERROR;
		}
		miDepth--;
	}

	szIdentifier.Kill();
	mcParser.PassPosition();
	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CXMLParser::ParseAttribute(void)
{
	CChars		szIdentifier;
	CChars		szValue;
	TRISTATE	tResult;
	bool		bResult;

	szIdentifier.Init();
	tResult = ParseIdentifier(&szIdentifier);
	if (tResult != TRITRUE)
	{
		szIdentifier.Kill();
		if (tResult == TRIERROR)
		{
			return TRIERROR;
		}
		else
		{
			return TRIFALSE;
		}
	}

	szValue.Init();
	tResult = ParseValue(&szValue);
	if (tResult != TRITRUE)
	{
		szValue.Kill();
		szIdentifier.Kill();
		return TRIERROR;
	}
	bResult = mpcCurrent->AddAttribute(szIdentifier.Text(), szValue.Text());

	szValue.Kill();
	szIdentifier.Kill();

	if (!bResult)
	{
		return TRIERROR;
	}
	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CXMLParser::ParseText(CChars* pszText)
{
	char		c;
	TRISTATE	tResult;
	size		i;

	for (i = 0;; i++)
	{
		tResult = mcParser.GetCharacter(&c);
		if (tResult != TRITRUE)
		{
			if (i == 0)
			{
				return TRIFALSE;
			}
			return TRITRUE;
		}

		if (IsAllowedTextChar(c))
		{
			pszText->Append(c);
		}
		else
		{
			if (c == '&')
			{
				mcParser.PushPosition();
				tResult = ParseTextReplacement(pszText);
				if (tResult == TRIERROR)
				{
					mcParser.PopPosition();
					return TRIERROR;
				}
				else if (tResult == TRIFALSE)
				{
					mcParser.StepLeft();
					if (i == 0)
					{
						return TRIFALSE;
					}
					return TRITRUE;
				}
			}
			else
			{
				mcParser.StepLeft();
				if (i == 0)
				{
					return TRIFALSE;
				}
				return TRITRUE;
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CXMLParser::ParseTextReplacement(CChars* pszText)
{
	TRISTATE	tResult;

	tResult = mcParser.GetExactCaseInsensitiveCharacterSequence("amp");
	if (tResult == TRITRUE)
	{
		pszText->Append('&');
	}
	else if (tResult == TRIERROR)
	{
		return TRIERROR;
	}
	else
	{
		tResult = mcParser.GetExactCaseInsensitiveCharacterSequence("gt");
		if (tResult == TRITRUE)
		{
			pszText->Append('>');
		}
		else if (tResult == TRIERROR)
		{
			return TRIERROR;
		}
		else
		{
			tResult = mcParser.GetExactCaseInsensitiveCharacterSequence("lt");
			if (tResult == TRITRUE)
			{
				pszText->Append('<');
			}
			else if (tResult == TRIERROR)
			{
				return TRIERROR;
			}
			else
			{
				tResult = mcParser.GetExactCaseInsensitiveCharacterSequence("apos");
				if (tResult == TRITRUE)
				{
					pszText->Append('\'');
				}
				else if (tResult == TRIERROR)
				{
					return TRIERROR;
				}
				else
				{
					tResult = mcParser.GetExactCaseInsensitiveCharacterSequence("quot");
					if (tResult == TRITRUE)
					{
						pszText->Append('"');
					}
					else
					{
						return TRIFALSE;
					}
				}
			}
		}
	}
	tResult = mcParser.GetExactCharacter(';', false);
	if (tResult == TRIERROR)
	{
		return TRIERROR;
	}
	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CXMLParser::ParseDocType(void)
{
	TRISTATE	tResult;
	CChars		szRootName;

	tResult = mcParser.GetExactCaseInsensitiveCharacterSequence("<!DOCTYPE");
	ReturnOnErrorAndFalse(tResult);

	szRootName.Init();
	tResult = ParseIdentifier(&szRootName);
	if (tResult == TRIERROR)
	{
		szRootName.Kill();
		return TRIERROR;
	}
	else if (tResult == TRIFALSE)
	{
		szRootName.Kill();
		return TRIERROR;
	}

	tResult = mcParser.GetExactCaseInsensitiveCharacterSequence("SYSTEM");
	if (tResult == TRIERROR)
	{
		szRootName.Kill();
		return TRIERROR;
	}
	else if (tResult == TRITRUE)
	{
		tResult = mcParser.GetString(NULL);
		if (tResult == TRIERROR)
		{
			szRootName.Kill();
			return TRIERROR;
		}
		else if (tResult == TRIFALSE)
		{
			szRootName.Kill();
			return TRIERROR;
		}
	}
	
	tResult = mcParser.GetExactCharacter('[');
	if (tResult == TRIERROR) 
	{
		szRootName.Kill();
		return TRIERROR;
	}
	else if (tResult == TRITRUE)
	{
		tResult = ParseEntities();
		if (tResult == TRIERROR)
		{
			szRootName.Kill();
			return TRIERROR;
		}

		tResult = mcParser.GetExactCharacter(']');
		if ((tResult == TRIERROR) || (tResult == TRIFALSE))
		{
			szRootName.Kill();
			return TRIERROR;
		}
	}

	tResult = mcParser.GetExactCharacter('>');
	if ((tResult == TRIERROR) || (tResult == TRIFALSE))
	{
		szRootName.Kill();
		return TRIERROR;
	}

	szRootName.Kill();
	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CXMLParser::ParseEntities(void)
{
	TRISTATE	tResult;

	for (;;)
	{
		tResult = ParseEntity();
		if (tResult == TRIERROR)
		{
			return TRIERROR;
		}
		else if (tResult == TRIFALSE)
		{
			break;
		}
	}
	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CXMLParser::ParseEntity(void)
{
	TRISTATE	tResult;
	CChars		szEntityName;
	bool		bSystem;
	size		iLength;
	CChars		szContent;

	tResult = mcParser.GetExactCaseInsensitiveCharacterSequence("<!ENTITY");
	if (tResult == TRIERROR)
	{
		return TRIERROR;
	}
	else if (tResult == TRIFALSE)
	{
		return TRIFALSE;
	}

	szEntityName.Init();
	tResult = ParseIdentifier(&szEntityName);
	if (tResult == TRIERROR)
	{
		szEntityName.Kill();
		return TRIERROR;
	}
	else if (tResult == TRIFALSE)
	{
		szEntityName.Kill();
		return TRIERROR;
	}

	bSystem = false;
	tResult = mcParser.GetExactCaseInsensitiveCharacterSequence("SYSTEM");
	if (tResult == TRIERROR)
	{
		szEntityName.Kill();
		return TRIERROR;
	}
	else if (tResult == TRITRUE)
	{
		bSystem = true;
	}

	mcParser.PushPosition();
	tResult = mcParser.GetString(NULL, &iLength);
	if (tResult == TRIERROR)
	{
		szEntityName.Kill();
		return TRIERROR;
	}
	else if (tResult == TRIFALSE)
	{
		szEntityName.Kill();
		return TRIERROR;
	}
	mcParser.PopPosition();

	szContent.InitLength(iLength);
	mcParser.GetString(szContent.Text());

	if (bSystem)
	{
		ParseExternalEntity(szEntityName.Text(), szContent.Text());
	}
	else
	{
		ParseInternalEntity(szEntityName.Text(),szContent.Text());
	}

	tResult = mcParser.GetExactCharacter('>');
	if ((tResult == TRIERROR) || (tResult == TRIFALSE))
	{
		szEntityName.Kill();
		return TRIERROR;
	}

	szContent.Kill();
	szEntityName.Kill();
	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CXMLParser::ParseExternalEntity(char* szName, char* szContent)
{
	CTextParser		cParser;
	TRISTATE		tResult;
	EXMLEntityType	eType;
	CXMLEntity*		pcEntity;

	cParser.Init(szContent);
	tResult = cParser.GetExactCaseInsensitiveCharacterSequence("file:");
	if (tResult == TRIERROR)
	{
		return TRIERROR;
	}
	if (tResult == TRIFALSE)
	{
		eType = XET_External;
	}
	else
	{
		eType = XET_ExternalFile;
	}

	pcEntity = macEntities.Add();
	pcEntity->Init(szName, cParser.mszParserPos, eType);

	cParser.Kill();
	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CXMLParser::ParseInternalEntity(char* szName, char* szContent)
{
	CXMLEntity*		pcEntity;

	pcEntity = macEntities.Add();
	pcEntity->Init(szName, szContent, XET_Internal);
	return TRITRUE;
}

