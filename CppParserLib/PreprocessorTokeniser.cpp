/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2022 Andrew Paterson

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
void CPreprocessorTokeniser::Init(void)
{
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
BOOL CPreprocessorTokeniser::Tokenise(CArrayCBlockSet* pacBlockSets, char* szPos, int iLength, BOOL bAllowAnnotations)
{
	char*					szEnd;
	EPreprocessorDirective	eDirective;
	CPPBlockSet*			pcBlockSet;
	BOOL					bNewBlock;
	BOOL					bLastDirective;
	BOOL					bDirective;
	CPPLine*				pcLine;
	int						iBlock;
	CPPDirective*			pcDirective;
	CPPToken*				pcToken;
	int						iLine;
	int						iIndex;
	BOOL					bMustAdd;

	if (pacBlockSets->IsRawProcessed())
	{
		return TRUE;
	}

	pacBlockSets->RawProcessed();

	pcBlockSet = NULL;
	mpcPrev = NULL;
	szEnd = szPos + iLength-1;
	mcParser.Init(szPos, szEnd, bAllowAnnotations);

	bNewBlock = TRUE;
	bLastDirective = FALSE;
	iBlock = pacBlockSets->NumElements();
	iIndex = 0;

	while (!mcParser.mbEndOfFile)
	{
		iLine = mcParser.miLine;
		mcParser.SkipWhiteSpace();
		if ((*mcParser.mszPos) == '#')
		{
			bDirective = TRUE;
			mcParser.StepRight();
			eDirective = GetDirective();
			if (eDirective == PPD_unknown)
			{
				return FALSE;
			}
			pcLine = NULL;
			pcDirective = TokeniseDirective(pacBlockSets->mpcTokenMemory, eDirective, iBlock, iIndex);
			pcToken = pcDirective;
		}
		else
		{
			pcDirective = NULL;
			pcLine = Line(pacBlockSets->mpcTokenMemory);
			pcToken = pcLine;
			if (pcLine)
			{
				bDirective = FALSE;;
			}
			else
			{
				bDirective = bLastDirective;
			}
		}

		bMustAdd = FALSE;
		if (pcBlockSet)
		{
			if ((bDirective && !bLastDirective) || (pcLine && bLastDirective))
			{
				bMustAdd = TRUE;
			}
		}
		else if (pcToken)
		{
			bMustAdd = TRUE;
		}

		if (bMustAdd)
		{
			pcBlockSet = pacBlockSets->Add(iLine, !bDirective);
			iBlock = pcBlockSet->Block();
		}

		if (pcToken)
		{
			pcBlockSet->GetRawTokensHolder()->Add(pcToken);
			iIndex = pcBlockSet->GetRawTokensHolder()->NumTokens();
		}

		bLastDirective = bDirective;
	}

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CPreprocessorTokeniser::Tokenise(CPPTokenHolder* pcTokenHolder, CPPTokens* pcTokens, char* szPos, int iLength, BOOL bAllowAnnotations, int iBlock, int iIndex)
{
	char*					szEnd;
	int						iLine;
	EPreprocessorDirective	eDirective;
	CPPToken*				pcToken;

	mpcPrev = NULL;
	szEnd = szPos + iLength-1;
	mcParser.Init(szPos, szEnd, bAllowAnnotations);

	iLine = 0;
	while (!mcParser.mbEndOfFile)
	{
		mcParser.SkipWhiteSpace();
		if ((*mcParser.mszPos) == '#')
		{
			mcParser.StepRight();
			eDirective = GetDirective();
			if (eDirective == PPD_unknown)
			{
				return FALSE;
			}

			pcToken = TokeniseDirective(pcTokens, eDirective, iBlock, iIndex+iLine);
		}
		else
		{
			pcToken = Line(pcTokens);
		}

		if (pcToken)
		{
			pcTokenHolder->Add(pcToken);
		}

		//Add Token here.
		iLine++;
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CPreprocessorTokeniser::TokeniseDefine(CPPTokenHolder* pcHolder, char* sz, CPPTokens* pcTokens)
{
	mcParser.Init(sz, NULL, FALSE);
	CPreprocessorLineTokensier::Preprocess(pcHolder, &mcParser, pcTokens, TRUE);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
EPreprocessorDirective CPreprocessorTokeniser::GetDirective(void)
{
	CPreprocessorDirective* pcDirective;
	CExternalString			cName;

	mcParser.SkipWhiteSpace();
	if (mcParser.mbEndOfFile)
	{
		return PPD_nomore;
	}

	cName.Init(mcParser.mszPos, mcParser.mszEnd);
	pcDirective = mcDirectives.GetDirective(&cName, FALSE);
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
CPPDirective* CPreprocessorTokeniser::TokeniseDirective(CPPTokens* pcTokens, EPreprocessorDirective eDirective, int iBlock, int iIndex)
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
CPPDirective* CPreprocessorTokeniser::TokeniseHashDefine(CPPTokens* pcTokens)
{
	CPPDirective*	pcDefine;

	mcParser.SkipWhiteSpace();
	
	pcDefine = pcTokens->AddDirective();
	pcDefine->Init(PPD_define, mcParser.miLine, mcParser.miColumn);
	CPreprocessorLineTokensier::Preprocess(pcDefine->GetTokens(), &mcParser, pcTokens, TRUE);
	return pcDefine; 
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CPPDirective* CPreprocessorTokeniser::TokeniseHashUndef(CPPTokens* pcTokens)
{
	CPPDirective*			pcUndef;

	mcParser.SkipWhiteSpace();

	pcUndef = pcTokens->AddDirective();
	pcUndef->Init(PPD_undef, mcParser.miLine, mcParser.miColumn);
	CPreprocessorLineTokensier::Preprocess(pcUndef->GetTokens(), &mcParser, pcTokens, TRUE);
	return pcUndef;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CPPDirective* CPreprocessorTokeniser::TokeniseHashInclude(CPPTokens* pcTokens)
{
	CPPInclude*			pcInclude;

	mcParser.SkipWhiteSpace();

	pcInclude = pcTokens->AddInclude();
	pcInclude->Init(mcParser.miLine, mcParser.miColumn);
	CPreprocessorLineTokensier::Preprocess(pcInclude->GetTokens(), &mcParser, pcTokens, FALSE);
	return pcInclude;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CPPDirective* CPreprocessorTokeniser::TokeniseHashIfndef(CPPTokens* pcTokens, int iBlock, int iIndex)
{
	CPPConditional*		pcIfNDef;

	pcIfNDef = pcTokens->AddConditional();
	pcIfNDef->Init(PPD_ifndef, iBlock, iIndex, mcParser.miLine, mcParser.miColumn);
	CPreprocessorLineTokensier::Preprocess(pcIfNDef->GetTokens(), &mcParser, pcTokens, TRUE);
	NextConditional(pcIfNDef);
	return pcIfNDef;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CPPDirective* CPreprocessorTokeniser::TokeniseHashIfdef(CPPTokens* pcTokens, int iBlock, int iIndex)
{
	CPPConditional*		pcIfDef;

	pcIfDef = pcTokens->AddConditional();
	pcIfDef->Init(PPD_ifdef, iBlock, iIndex, mcParser.miLine, mcParser.miColumn);
	CPreprocessorLineTokensier::Preprocess(pcIfDef->GetTokens(), &mcParser, pcTokens, TRUE);
	NextConditional(pcIfDef);
	return pcIfDef;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CPPDirective* CPreprocessorTokeniser::TokeniseHashElse(CPPTokens* pcTokens, int iBlock, int iIndex)
{
	CPPConditional*		pcElse;

	pcElse = pcTokens->AddConditional();
	pcElse->Init(PPD_else, iBlock, iIndex, mcParser.miLine, mcParser.miColumn);
	CPreprocessorLineTokensier::Preprocess(pcElse->GetTokens(), &mcParser, pcTokens, TRUE);
	NextConditional(pcElse);
	return pcElse;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CPPDirective* CPreprocessorTokeniser::TokeniseHashIf(CPPTokens* pcTokens, int iBlock, int iIndex)
{
	//IF and Elif are special cases because the calculator does it's own parsing.
	CPPConditional*		pcIf;

	pcIf = pcTokens->AddConditional();
	pcIf->Init(PPD_if, iBlock, iIndex, mcParser.miLine, mcParser.miColumn);
	CPreprocessorLineTokensier::Preprocess(pcIf->GetTokens(), &mcParser, pcTokens, TRUE);
	NextConditional(pcIf);
	return pcIf;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CPPDirective* CPreprocessorTokeniser::TokeniseHashElif(CPPTokens* pcTokens, int iBlock, int iIndex)
{
	//IF and Elif are special cases because the calculator does it's own parsing.
	CPPConditional*		pcElif;

	pcElif = pcTokens->AddConditional();
	pcElif->Init(PPD_elif, iBlock, iIndex, mcParser.miLine, mcParser.miColumn);
	CPreprocessorLineTokensier::Preprocess(pcElif->GetTokens(), &mcParser, pcTokens, TRUE);
	NextConditional(pcElif);
	return pcElif;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CPPDirective* CPreprocessorTokeniser::TokeniseHashEndif(CPPTokens* pcTokens, int iBlock, int iIndex)
{
	CPPConditional*		pcEndif;

	pcEndif = pcTokens->AddConditional();
	pcEndif->Init(PPD_endif, iBlock, iIndex, mcParser.miLine, mcParser.miColumn);
	CPreprocessorLineTokensier::Preprocess(pcEndif->GetTokens(), &mcParser, pcTokens, TRUE);
	NextConditional(pcEndif);
	return pcEndif;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CPPDirective* CPreprocessorTokeniser::TokeniseHashError(CPPTokens* pcTokens)
{
	CPPDirective*		pcError;

	pcError = pcTokens->AddDirective();
	pcError->Init(PPD_error, mcParser.miLine, mcParser.miColumn);
	CPreprocessorLineTokensier::Preprocess(pcError->GetTokens(), &mcParser, pcTokens, FALSE);
	return pcError;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CPPDirective* CPreprocessorTokeniser::TokeniseHashPragma(CPPTokens* pcTokens)
{
	CPPDirective*		pcPragma;

	pcPragma = pcTokens->AddDirective();
	pcPragma->Init(PPD_pragma, mcParser.miLine, mcParser.miColumn);
	CPreprocessorLineTokensier::Preprocess(pcPragma->GetTokens(), &mcParser, pcTokens, FALSE);
	return pcPragma;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CPPLine* CPreprocessorTokeniser::Line(CPPTokens* pcTokens)
{
	CPPLine		cLine;
	CPPLine*	pcLine;

	cLine.Init(mcParser.miLine, mcParser.miColumn);
	CPreprocessorLineTokensier::Preprocess(cLine.GetTokens(), &mcParser, pcTokens, TRUE);
	if (cLine.IsEmpty())
	{
		cLine.Kill();  //Stuff is still allocated on pcStack, it'll be killed when the stack is.  It's too small to worry about.
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

