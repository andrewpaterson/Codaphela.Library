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
#include "LinePreprocessor.h"
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
	CCBlockSet*				pcBlockSet;
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
			pcDirective = TokeniseDirective(&pacBlockSets->mpcFile->mcStack, eDirective, iBlock, iIndex);
			pcToken = pcDirective;
		}
		else
		{
			pcDirective = NULL;
			pcLine = Line(&pacBlockSets->mpcFile->mcStack);
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
			iBlock = pcBlockSet->miBlock;
		}

		if (pcToken)
		{
			pcBlockSet->mcRawTokens.Add(&pcToken);
			iIndex = pcBlockSet->mcRawTokens.mcArray.NumElements();
		}

		bLastDirective = bDirective;
	}

	//Debug shit.
	if (FALSE)
	{
		pcBlockSet->mpcFile->mszFullName.Dump();
		CChars::Dump("\n");
		for (int i = 0; i < pacBlockSets->NumElements(); i++)
		{
			pcBlockSet = pacBlockSets->Get(i);
			pcBlockSet->DumpRawTokens();
		}
		CChars::Dump("\n\n");
	}

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CPreprocessorTokeniser::Tokenise(CPPTokenHolder* pcTokens, CMemoryStackExtended* pcStack, char* szPos, int iLength, BOOL bAllowAnnotations, int iBlock, int iIndex)
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

			pcToken = TokeniseDirective(pcStack, eDirective, iBlock, iIndex+iLine);
		}
		else
		{
			pcToken = Line(pcStack);
		}

		if (pcToken)
		{
			pcTokens->Add(&pcToken);
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
void CPreprocessorTokeniser::TokeniseDefine(CPPTokenHolder* pcHolder, char* sz, CMemoryStackExtended* pcStack)
{
	mcParser.Init(sz, NULL, FALSE);
	CLinePreprocessor::Do(pcHolder, &mcParser, pcStack, TRUE);
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
CPPDirective* CPreprocessorTokeniser::TokeniseDirective(CMemoryStackExtended* pcStack, EPreprocessorDirective eDirective, int iBlock, int iIndex)
{
	switch (eDirective)
	{
	case PPD_include:
		return TokeniseHashInclude(pcStack);
	case PPD_define:
		return TokeniseHashDefine(pcStack);
	case PPD_ifdef:
		return TokeniseHashIfdef(pcStack, iBlock, iIndex);
	case PPD_ifndef:
		return TokeniseHashIfndef(pcStack, iBlock, iIndex);
	case PPD_endif:
		return TokeniseHashEndif(pcStack, iBlock, iIndex);
	case PPD_else:
		return TokeniseHashElse(pcStack, iBlock, iIndex);
	case PPD_if:
		return TokeniseHashIf(pcStack, iBlock, iIndex);
	case PPD_elif:
		return TokeniseHashElif(pcStack, iBlock, iIndex);
	case PPD_undef:
		return TokeniseHashUndef(pcStack);
	case PPD_error:
		return TokeniseHashError(pcStack);
	case PPD_pragma:
		return TokeniseHashPragma(pcStack);
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CPPDirective* CPreprocessorTokeniser::TokeniseHashDefine(CMemoryStackExtended* pcStack)
{
	CPPDirective*	pcDefine;

	mcParser.SkipWhiteSpace();
	
	pcDefine = CPPDirective::Construct(pcStack->Add(sizeof(CPPDirective)));
	pcDefine->Init(PPD_define, mcParser.miLine, mcParser.miColumn);
	CLinePreprocessor::Do(&pcDefine->mcTokens, &mcParser, pcStack, TRUE);
	return pcDefine; 
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CPPDirective* CPreprocessorTokeniser::TokeniseHashUndef(CMemoryStackExtended* pcStack)
{
	CPPDirective*			pcUndef;

	mcParser.SkipWhiteSpace();

	pcUndef = CPPDirective::Construct(pcStack->Add(sizeof(CPPDirective)));
	pcUndef->Init(PPD_undef, mcParser.miLine, mcParser.miColumn);
	CLinePreprocessor::Do(&pcUndef->mcTokens, &mcParser, pcStack, TRUE);
	return pcUndef;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CPPDirective* CPreprocessorTokeniser::TokeniseHashInclude(CMemoryStackExtended* pcStack)
{
	CPPInclude*			pcInclude;

	mcParser.SkipWhiteSpace();

	pcInclude = CPPInclude::Construct(pcStack->Add(sizeof(CPPInclude)));
	pcInclude->Init(mcParser.miLine, mcParser.miColumn);
	CLinePreprocessor::Do(&pcInclude->mcTokens, &mcParser, pcStack, FALSE);
	return pcInclude;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CPPDirective* CPreprocessorTokeniser::TokeniseHashIfndef(CMemoryStackExtended* pcStack, int iBlock, int iIndex)
{
	CPPConditional*		pcIfNDef;

	pcIfNDef = CPPConditional::Construct(pcStack->Add(sizeof(CPPConditional)));
	pcIfNDef->Init(PPD_ifndef, iBlock, iIndex, mcParser.miLine, mcParser.miColumn);
	CLinePreprocessor::Do(&pcIfNDef->mcTokens, &mcParser, pcStack, TRUE);
	NextConditional(pcIfNDef);
	return pcIfNDef;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CPPDirective* CPreprocessorTokeniser::TokeniseHashIfdef(CMemoryStackExtended* pcStack, int iBlock, int iIndex)
{
	CPPConditional*		pcIfDef;

	pcIfDef = CPPConditional::Construct(pcStack->Add(sizeof(CPPConditional)));
	pcIfDef->Init(PPD_ifdef, iBlock, iIndex, mcParser.miLine, mcParser.miColumn);
	CLinePreprocessor::Do(&pcIfDef->mcTokens, &mcParser, pcStack, TRUE);
	NextConditional(pcIfDef);
	return pcIfDef;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CPPDirective* CPreprocessorTokeniser::TokeniseHashElse(CMemoryStackExtended* pcStack, int iBlock, int iIndex)
{
	CPPConditional*		pcElse;

	pcElse = CPPConditional::Construct(pcStack->Add(sizeof(CPPConditional)));
	pcElse->Init(PPD_else, iBlock, iIndex, mcParser.miLine, mcParser.miColumn);
	CLinePreprocessor::Do(&pcElse->mcTokens, &mcParser, pcStack, TRUE);
	NextConditional(pcElse);
	return pcElse;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CPPDirective* CPreprocessorTokeniser::TokeniseHashIf(CMemoryStackExtended* pcStack, int iBlock, int iIndex)
{
	//IF and Elif are special cases because the calculator does it's own parsing.
	CPPConditional*		pcIf;

	pcIf = CPPConditional::Construct(pcStack->Add(sizeof(CPPConditional)));
	pcIf->Init(PPD_if, iBlock, iIndex, mcParser.miLine, mcParser.miColumn);
	CLinePreprocessor::Do(&pcIf->mcTokens, &mcParser, pcStack, TRUE);
	NextConditional(pcIf);
	return pcIf;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CPPDirective* CPreprocessorTokeniser::TokeniseHashElif(CMemoryStackExtended* pcStack, int iBlock, int iIndex)
{
	//IF and Elif are special cases because the calculator does it's own parsing.
	CPPConditional*		pcElif;

	pcElif = CPPConditional::Construct(pcStack->Add(sizeof(CPPConditional)));
	pcElif->Init(PPD_elif, iBlock, iIndex, mcParser.miLine, mcParser.miColumn);
	CLinePreprocessor::Do(&pcElif->mcTokens, &mcParser, pcStack, TRUE);
	NextConditional(pcElif);
	return pcElif;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CPPDirective* CPreprocessorTokeniser::TokeniseHashEndif(CMemoryStackExtended* pcStack, int iBlock, int iIndex)
{
	CPPConditional*		pcEndif;

	pcEndif = CPPConditional::Construct(pcStack->Add(sizeof(CPPConditional)));
	pcEndif->Init(PPD_endif, iBlock, iIndex, mcParser.miLine, mcParser.miColumn);
	CLinePreprocessor::Do(&pcEndif->mcTokens, &mcParser, pcStack, TRUE);
	NextConditional(pcEndif);
	return pcEndif;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CPPDirective* CPreprocessorTokeniser::TokeniseHashError(CMemoryStackExtended* pcStack)
{
	CPPDirective*		pcError;

	pcError = CPPDirective::Construct(pcStack->Add(sizeof(CPPDirective)));
	pcError->Init(PPD_error, mcParser.miLine, mcParser.miColumn);
	CLinePreprocessor::Do(&pcError->mcTokens, &mcParser, pcStack, FALSE);
	return pcError;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CPPDirective* CPreprocessorTokeniser::TokeniseHashPragma(CMemoryStackExtended* pcStack)
{
	CPPDirective*		pcPragma;

	pcPragma = CPPDirective::Construct(pcStack->Add(sizeof(CPPDirective)));
	pcPragma->Init(PPD_pragma, mcParser.miLine, mcParser.miColumn);
	CLinePreprocessor::Do(&pcPragma->mcTokens, &mcParser, pcStack, FALSE);
	return pcPragma;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CPPLine* CPreprocessorTokeniser::Line(CMemoryStackExtended* pcStack)
{
	CPPLine		cLine;
	CPPLine*	pcLine;

	cLine.Init(mcParser.miLine, mcParser.miColumn);
	CLinePreprocessor::Do(&cLine.mcTokens, &mcParser, pcStack, TRUE);
	if (cLine.IsEmpty())
	{
		cLine.Kill();  //Stuff is still allocated on pcStack, it'll be killed when the stack is.  It's too small to worry about.
		return NULL;
	}
	else
	{
		pcLine = (CPPLine*)pcStack->Add(sizeof(CPPLine));
		memcpy(pcLine, &cLine, sizeof(CPPLine));
		return pcLine;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CPPToken* CPreprocessorTokeniser::AddToken(CPPToken* pcToken, CArrayPPTokens* pcTokens)
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

