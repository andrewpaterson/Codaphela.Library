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
#include "PPUnknown.h"
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
	bool					bNewBlock;
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
	mcParser.Init(szPos, szEnd, mcParser.mszFileName);

	bNewBlock = true;
	bLastDirective = false;
	iBlock = pacDestBlockSets->NumElements();
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
			if (eDirective == PPD_unknown)
			{
				return false;
			}
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
			iIndex = pcBlockSet->GetUnprocssedTokens()->NumTokens();
		}

		bLastDirective = bDirective;
	}

	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CPreprocessorTokeniser::TokeniseIntoList(CPPTokenList* pcDestTokenHolder, CPPTokenMemory* pcTokens, char* szPos, int iLength, int iBlock, int iIndex)
{
	char*					szEnd;
	int						iLine;
	EPreprocessorDirective	eDirective;
	CPPToken*				pcToken;

	mpcPrev = NULL;
	szEnd = szPos + iLength-1;
	mcParser.Init(szPos, szEnd, mcParser.mszFileName);

	iLine = 0;
	while (!mcParser.mbEndOfFile)
	{
		mcParser.SkipWhitespace();
		if ((*mcParser.GetPos()) == '#')
		{
			mcParser.StepRight();
			eDirective = GetDirective();
			if (eDirective == PPD_unknown)
			{
				return false;
			}

			pcToken = TokeniseDirective(pcTokens, eDirective, iBlock, iIndex+iLine);
		}
		else
		{
			pcToken = Line(pcTokens);
		}

		if (pcToken)
		{
			pcDestTokenHolder->Add(pcToken);
		}

		//Add Token here.
		iLine++;
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
	CPreprocessorLineTokensier::Preprocess(pcHolder, &mcParser, pcTokens, true);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
EPreprocessorDirective CPreprocessorTokeniser::GetDirective(void)
{
	CPreprocessorDirective* pcDirective;
	CExternalString			cName;

	mcParser.SkipWhitespace();
	if (mcParser.mbEndOfFile)
	{
		return PPD_nomore;
	}

	cName.Init(mcParser.GetPos(), mcParser.GetEnd());
	pcDirective = mcDirectives.GetDirective(&cName, false);
	if (pcDirective == NULL)
	{
		return PPD_unknown;
	}
	mcParser.StepRight(pcDirective->GetNameLength());
	return pcDirective->GetType();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CPPDirective* CPreprocessorTokeniser::TokeniseDirective(CPPTokenMemory* pcTokens, EPreprocessorDirective eDirective, int iBlock, int iIndex)
{
	switch (eDirective)
	{
	case PPD_include:
		return TokeniseHashInclude(pcTokens);
	case PPD_define:
		return TokeniseHashDefine(pcTokens);
	case PPD_ifdef:
		return TokeniseHashIfdef(pcTokens, iBlock, iIndex);
	case PPD_ifndef:
		return TokeniseHashIfndef(pcTokens, iBlock, iIndex);
	case PPD_endif:
		return TokeniseHashEndif(pcTokens, iBlock, iIndex);
	case PPD_else:
		return TokeniseHashElse(pcTokens, iBlock, iIndex);
	case PPD_if:
		return TokeniseHashIf(pcTokens, iBlock, iIndex);
	case PPD_elif:
		return TokeniseHashElif(pcTokens, iBlock, iIndex);
	case PPD_undef:
		return TokeniseHashUndef(pcTokens);
	case PPD_error:
		return TokeniseHashError(pcTokens);
	case PPD_pragma:
		return TokeniseHashPragma(pcTokens);
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CPPDirective* CPreprocessorTokeniser::TokeniseHashDefine(CPPTokenMemory* pcTokens)
{
	CPPDirective*	pcDefine;

	mcParser.SkipWhitespace();
	
	pcDefine = pcTokens->AddDirective();
	pcDefine->Init(PPD_define, mcParser.miLine, mcParser.miColumn, mcParser.mszFileName);

	CPreprocessorLineTokensier::Preprocess(pcDefine->GetTokenList(), &mcParser, pcTokens, true);
	return pcDefine; 
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CPPDirective* CPreprocessorTokeniser::TokeniseHashUndef(CPPTokenMemory* pcTokens)
{
	CPPDirective*			pcUndef;

	mcParser.SkipWhitespace();

	pcUndef = pcTokens->AddDirective();
	pcUndef->Init(PPD_undef, mcParser.miLine, mcParser.miColumn, mcParser.mszFileName);

	CPreprocessorLineTokensier::Preprocess(pcUndef->GetTokenList(), &mcParser, pcTokens, true);
	return pcUndef;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CPPDirective* CPreprocessorTokeniser::TokeniseHashInclude(CPPTokenMemory* pcTokens)
{
	CPPInclude*			pcInclude;

	mcParser.SkipWhitespace();

	pcInclude = pcTokens->AddInclude();
	pcInclude->Init(mcParser.miLine, mcParser.miColumn, mcParser.mszFileName);

	CPreprocessorLineTokensier::Preprocess(pcInclude->GetTokenList(), &mcParser, pcTokens, false);
	return pcInclude;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CPPDirective* CPreprocessorTokeniser::TokeniseHashIfndef(CPPTokenMemory* pcTokens, int iBlock, int iIndex)
{
	CPPConditional*		pcIfNDef;

	pcIfNDef = pcTokens->AddConditional();
	pcIfNDef->Init(PPD_ifndef, iBlock, iIndex, mcParser.miLine, mcParser.miColumn, mcParser.mszFileName);

	CPreprocessorLineTokensier::Preprocess(pcIfNDef->GetTokenList(), &mcParser, pcTokens, true);
	NextConditional(pcIfNDef);
	return pcIfNDef;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CPPDirective* CPreprocessorTokeniser::TokeniseHashIfdef(CPPTokenMemory* pcTokens, int iBlock, int iIndex)
{
	CPPConditional*		pcIfDef;

	pcIfDef = pcTokens->AddConditional();
	pcIfDef->Init(PPD_ifdef, iBlock, iIndex, mcParser.miLine, mcParser.miColumn, mcParser.mszFileName);

	CPreprocessorLineTokensier::Preprocess(pcIfDef->GetTokenList(), &mcParser, pcTokens, true);
	NextConditional(pcIfDef);
	return pcIfDef;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CPPDirective* CPreprocessorTokeniser::TokeniseHashElse(CPPTokenMemory* pcTokens, int iBlock, int iIndex)
{
	CPPConditional*		pcElse;

	pcElse = pcTokens->AddConditional();
	pcElse->Init(PPD_else, iBlock, iIndex, mcParser.miLine, mcParser.miColumn, mcParser.mszFileName);

	CPreprocessorLineTokensier::Preprocess(pcElse->GetTokenList(), &mcParser, pcTokens, true);
	NextConditional(pcElse);
	return pcElse;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CPPDirective* CPreprocessorTokeniser::TokeniseHashIf(CPPTokenMemory* pcTokens, int iBlock, int iIndex)
{
	//IF and Elif are special cases because the calculator does it's own parsing.
	CPPConditional*		pcIf;

	pcIf = pcTokens->AddConditional();
	pcIf->Init(PPD_if, iBlock, iIndex, mcParser.miLine, mcParser.miColumn, mcParser.mszFileName);

	CPreprocessorLineTokensier::Preprocess(pcIf->GetTokenList(), &mcParser, pcTokens, true);
	NextConditional(pcIf);
	return pcIf;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CPPDirective* CPreprocessorTokeniser::TokeniseHashElif(CPPTokenMemory* pcTokens, int iBlock, int iIndex)
{
	//IF and Elif are special cases because the calculator does it's own parsing.
	CPPConditional*		pcElif;

	pcElif = pcTokens->AddConditional();
	pcElif->Init(PPD_elif, iBlock, iIndex, mcParser.miLine, mcParser.miColumn, mcParser.mszFileName);

	CPreprocessorLineTokensier::Preprocess(pcElif->GetTokenList(), &mcParser, pcTokens, true);
	NextConditional(pcElif);
	return pcElif;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CPPDirective* CPreprocessorTokeniser::TokeniseHashEndif(CPPTokenMemory* pcTokens, int iBlock, int iIndex)
{
	CPPConditional*		pcEndif;

	pcEndif = pcTokens->AddConditional();
	pcEndif->Init(PPD_endif, iBlock, iIndex, mcParser.miLine, mcParser.miColumn, mcParser.mszFileName);

	CPreprocessorLineTokensier::Preprocess(pcEndif->GetTokenList(), &mcParser, pcTokens, true);
	NextConditional(pcEndif);
	return pcEndif;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CPPDirective* CPreprocessorTokeniser::TokeniseHashError(CPPTokenMemory* pcTokens)
{
	CPPDirective*		pcError;

	pcError = pcTokens->AddDirective();
	pcError->Init(PPD_error, mcParser.miLine, mcParser.miColumn, mcParser.mszFileName);

	CPreprocessorLineTokensier::Preprocess(pcError->GetTokenList(), &mcParser, pcTokens, false);
	return pcError;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CPPDirective* CPreprocessorTokeniser::TokeniseHashPragma(CPPTokenMemory* pcTokens)
{
	CPPDirective*		pcPragma;

	pcPragma = pcTokens->AddDirective();
	pcPragma->Init(PPD_pragma, mcParser.miLine, mcParser.miColumn, mcParser.mszFileName);

	CPreprocessorLineTokensier::Preprocess(pcPragma->GetTokenList(), &mcParser, pcTokens, false);
	return pcPragma;
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
	CPreprocessorLineTokensier::Preprocess(cLine.GetTokenList(), &mcParser, pcTokens, true);
	if (cLine.IsEmpty())
	{
		cLine.Kill();
		return NULL;
	}
	else
	{
		pcLine = pcTokens->AddLine();
		memcpy(pcLine, &cLine, sizeof(CPPLine));
		return pcLine;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CPPToken* CPreprocessorTokeniser::AddToken(CPPToken* pcToken, CArrayPPTokenPtrs* pcTokens)
{
	CPPToken**	ppcToken;

	ppcToken = pcTokens->Add();
	*ppcToken = pcToken;
	return pcToken;
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

