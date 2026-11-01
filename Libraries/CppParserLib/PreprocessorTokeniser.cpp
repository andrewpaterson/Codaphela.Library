/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CppParserLib

Codaphela CppParserLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CppParserLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CppParserLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "PreprocessorTokeniser.h"
#include "PPLine.h"
#include "PPConditional.h"
#include "PPInclude.h"
#include "PreprocessorLineTokeniser.h"
#include "PPDirective.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CPreprocessorTokeniser::Init(char* szFileName)
{
	mszFileName = szFileName;
	mpacComments = NULL;

	mcDirectives.Init();
	mcDirectives.AddDirective("ifdef", PPD_ifdef);
	mcDirectives.AddDirective("ifndef", PPD_ifndef);
	mcDirectives.AddDirective("endif", PPD_endif);
	mcDirectives.AddDirective("else", PPD_else);
	mcDirectives.AddDirective("if", PPD_if);
	mcDirectives.AddDirective("elif", PPD_elif);
	mcDirectives.AddDirective("define", PPD_define);
	mcDirectives.AddDirective("include", PPD_include);
	mcDirectives.AddDirective("undef", PPD_undef);
	mcDirectives.AddDirective("error", PPD_error);
	mcDirectives.AddDirective("pragma", PPD_pragma);
	mcDirectives.AddDirective("warning", PPD_warning);
	mcDirectives.AddDirective("include_next", PPD_include_next);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CPreprocessorTokeniser::SetComments(CArrayPPComments* pacComments)
{
	//Comments found while tokenising are added to pacComments.
	mpacComments = pacComments;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CPreprocessorTokeniser::Kill(void)
{
	mcDirectives.Kill();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CPreprocessorTokeniser::TokeniseIntoBlockSets(CPPBlockSetArray* pacDestBlockSets, CPPTokenMemory* pcTokenMemory, char* szPos, int iLength)
{
	char*					szEnd;
	EPreprocessorDirective	eDirective;
	CPPBlockSet*			pcBlockSet;
	bool					bLastDirective;
	bool					bDirective;
	CPPLine*				pcLine;
	int						iBlock;
	CPPDirective*			pcDirective;
	CPPToken*				pcToken;
	int						iLine;
	int						iIndex;
	bool					bMustAdd;

	if (pacDestBlockSets->IsRawProcessed())
	{
		return true;
	}

	pacDestBlockSets->RawProcessed();

	pcBlockSet = NULL;
	mpcPrev = NULL;
	szEnd = szPos + iLength-1;
	mcParser.Init(szPos, szEnd, mszFileName);
	mcParser.SetComments(mpacComments);

	bLastDirective = false;
	iBlock = (int)pacDestBlockSets->NumElements();
	iIndex = 0;

	while (!mcParser.mbEndOfFile)
	{
		iLine = mcParser.miLine;
		mcParser.SkipWhitespace();
		if ((*mcParser.GetPos()) == '#')
		{
			bDirective = true;
			mcParser.StepRight();
			eDirective = GetDirective();
			pcLine = NULL;
			pcDirective = TokeniseDirective(pcTokenMemory, eDirective, iBlock, iIndex);
			pcToken = pcDirective;
		}
		else
		{
			pcDirective = NULL;
			pcLine = Line(pcTokenMemory);
			pcToken = pcLine;
			if (pcLine)
			{
				bDirective = false;;
			}
			else
			{
				bDirective = bLastDirective;
			}
		}

		bMustAdd = false;
		if (pcBlockSet)
		{
			if ((bDirective && !bLastDirective) || (pcLine && bLastDirective))
			{
				bMustAdd = true;
			}
		}
		else if (pcToken)
		{
			bMustAdd = true;
		}

		if (bMustAdd)
		{
			pcBlockSet = pacDestBlockSets->Add(iLine, !bDirective);
			iBlock = pcBlockSet->Block();
		}

		if (pcToken)
		{
			pcBlockSet->GetUnprocssedTokens()->Add(pcToken);
			iIndex = (int)pcBlockSet->GetUnprocssedTokens()->NumTokens();
		}

		bLastDirective = bDirective;
	}

	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CPreprocessorTokeniser::TokeniseDefine(CPPTokenList* pcHolder, char* sz, CPPTokenMemory* pcTokens)
{
	mcParser.Init(sz, NULL, mszFileName);
	CPreprocessorLineTokeniser::Preprocess(pcHolder, &mcParser, pcTokens);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
EPreprocessorDirective CPreprocessorTokeniser::GetDirective(void)
{
	CPreprocessorDirective* pcDirective;
	CExternalString			cName;
	char*					szName;
	char*					szEnd;

	mcParser.SkipWhitespace();
	if (mcParser.mbEndOfFile)
	{
		return PPD_nomore;
	}

	//The directive name is the whole identifier; '#include_next' is not '#include'.
	szName = mcParser.GetPos();
	szEnd = szName;
	while ((szEnd <= mcParser.GetEnd()) && (((*szEnd >= 'a') && (*szEnd <= 'z')) || ((*szEnd >= 'A') && (*szEnd <= 'Z')) || ((*szEnd >= '0') && (*szEnd <= '9')) || (*szEnd == '_')))
	{
		szEnd++;
	}

	if (szEnd == szName)
	{
		if (mcParser.IsNewLine())
		{
			return PPD_null;
		}
		return PPD_unknown;
	}

	cName.Init(szName, (int)(szEnd - szName));
	pcDirective = mcDirectives.Get(&cName, true);
	if (pcDirective == NULL)
	{
		//The name is left in the directive's tokens so it can be reported.
		return PPD_unknown;
	}
	mcParser.StepRight(pcDirective->GetNameLength());
	return pcDirective->GetType();
}


//////////////////////////////////////////////////////////////////////////
//
// Tokenises the rest of a directive line whose name has been read.
// #define and #undef skip the whitespace after the name before their
// position is taken; the others take it straight after the name.
//
//////////////////////////////////////////////////////////////////////////
CPPDirective* CPreprocessorTokeniser::TokeniseDirective(CPPTokenMemory* pcTokens, EPreprocessorDirective eDirective, int iBlock, int iIndex)
{
	CPPDirective*	pcDirective;

	switch (eDirective)
	{
	case PPD_include:
		return TokeniseHashInclude(pcTokens);
	case PPD_include_next:
		pcDirective = TokeniseHashInclude(pcTokens);
		pcDirective->Set(PPD_include_next);
		return pcDirective;
	case PPD_define:
	case PPD_undef:
		mcParser.SkipWhitespace();
		return TokeniseHashPlain(pcTokens, eDirective);
	case PPD_ifdef:
	case PPD_ifndef:
	case PPD_endif:
	case PPD_else:
	case PPD_if:
	case PPD_elif:
		return TokeniseHashConditional(pcTokens, eDirective, iBlock, iIndex);
	case PPD_error:
	case PPD_pragma:
	case PPD_warning:
	case PPD_null:
	case PPD_unknown:
		return TokeniseHashPlain(pcTokens, eDirective);
	default:
		break;
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Tokenises an #include (or #include_next) line.
//
//////////////////////////////////////////////////////////////////////////
CPPDirective* CPreprocessorTokeniser::TokeniseHashInclude(CPPTokenMemory* pcTokens)
{
	CPPInclude*			pcInclude;

	mcParser.SkipWhitespace();

	pcInclude = pcTokens->AddToken<CPPInclude>();
	pcInclude->Init(mcParser.miLine, mcParser.miColumn, mcParser.mszFileName);

	CPreprocessorLineTokeniser::Preprocess(pcInclude->GetTokenList(), &mcParser, pcTokens);
	return pcInclude;
}


//////////////////////////////////////////////////////////////////////////
//
// Tokenises an #if, #ifdef, #ifndef, #elif, #else or #endif line and
// links it to the previous conditional.
//
//////////////////////////////////////////////////////////////////////////
CPPDirective* CPreprocessorTokeniser::TokeniseHashConditional(CPPTokenMemory* pcTokens, EPreprocessorDirective eDirective, int iBlock, int iIndex)
{
	CPPConditional*		pcConditional;

	pcConditional = pcTokens->AddToken<CPPConditional>();
	pcConditional->Init(eDirective, iBlock, iIndex, mcParser.miLine, mcParser.miColumn, mcParser.mszFileName);

	CPreprocessorLineTokeniser::Preprocess(pcConditional->GetTokenList(), &mcParser, pcTokens);
	NextConditional(pcConditional);
	return pcConditional;
}


//////////////////////////////////////////////////////////////////////////
//
// Tokenises any other directive line (#define, #undef, #error,
// #pragma, #warning, a lone # or an unknown directive) into a plain
// directive holding the line's tokens.
//
//////////////////////////////////////////////////////////////////////////
CPPDirective* CPreprocessorTokeniser::TokeniseHashPlain(CPPTokenMemory* pcTokens, EPreprocessorDirective eDirective)
{
	CPPDirective*		pcDirective;

	pcDirective = pcTokens->AddToken<CPPDirective>();
	pcDirective->Init(eDirective, mcParser.miLine, mcParser.miColumn, mcParser.mszFileName);

	CPreprocessorLineTokeniser::Preprocess(pcDirective->GetTokenList(), &mcParser, pcTokens);
	return pcDirective;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CPPLine* CPreprocessorTokeniser::Line(CPPTokenMemory* pcTokens)
{
	CPPLine		cLine;
	CPPLine*	pcLine;

	cLine.Init(mcParser.miLine, mcParser.miColumn, mcParser.mszFileName);
	CPreprocessorLineTokeniser::Preprocess(cLine.GetTokenList(), &mcParser, pcTokens);
	if (cLine.IsEmpty())
	{
		cLine.Kill();
		return NULL;
	}
	else
	{
		pcLine = pcTokens->AddToken<CPPLine>();
		memcpy(pcLine, &cLine, sizeof(CPPLine));
		return pcLine;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CPreprocessorTokeniser::NextConditional(CPPConditional* pcCurrent)
{
	if (mpcPrev)
	{
		mpcPrev->Set(pcCurrent);
	}
	mpcPrev = pcCurrent;
}

