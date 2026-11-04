/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

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
#include "CPP65UnitWorkspace.h"


//////////////////////////////////////////////////////////////////////////
//
// The arrays' blocks are kept whatever their size: the next unit's are
// about as big.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65UnitWorkspace::Init(void)
{
	size	i;

	mcTokens.Init(0);
	mcExpansions.Init(0);
	mcInactiveRegions.Init(0);
	mcDirectiveLines.Init(0);
	mcCPPTokens.Init(0);
	mcSyntaxes.Init(0);
	mcTokenSymbols.Init(0);
	mcNodeArrays.Init();

	msPreprocessedTokens.pcTokens = &mcTokens;
	msPreprocessedTokens.pcExpansions = &mcExpansions;
	msPreprocessedTokens.pcInactiveRegions = &mcInactiveRegions;
	msPreprocessedTokens.pcDirectiveLines = &mcDirectiveLines;

	msSyntaxTree.pcSyntaxes = &mcSyntaxes;
	msSyntaxTree.pcTokenSymbols = &mcTokenSymbols;
	msSyntaxTree.pcNodeArrays = &mcNodeArrays;
	msSyntaxTree.pcArena = &mcNodeArrays;
	for (i = 0; i < CPP_SYNTAX_BLOCK_READING_ARRAYS; i++)
	{
		macBlockReading[i].Init(0);
		msSyntaxTree.apcBlockReading[i] = &macBlockReading[i];
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPP65UnitWorkspace::Kill(void)
{
	size	i;

	for (i = 0; i < CPP_SYNTAX_BLOCK_READING_ARRAYS; i++)
	{
		macBlockReading[i].Kill();
	}
	mcNodeArrays.Kill();
	mcTokenSymbols.Kill();
	mcSyntaxes.Kill();
	mcCPPTokens.Kill();
	mcDirectiveLines.Kill();
	mcInactiveRegions.Kill();
	mcExpansions.Kill();
	mcTokens.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Another unit is to be compiled: the last one's syntax tree's nodes,
// their arrays, scopes and symbols are let go all at once.  The arena's
// chunks (up to CPP65_WORKSPACE_KEPT_ARENA bytes) are kept for the unit.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65UnitWorkspace::Begin(void)
{
	mcNodeArrays.GetScratchPad()->Recycle(CPP65_WORKSPACE_KEPT_ARENA);
}


//////////////////////////////////////////////////////////////////////////
//
// For the unit's preprocessed tokens (CPreprocessedTokens::SetMallocators).
//
//////////////////////////////////////////////////////////////////////////
SPreprocessedTokensMallocators* CCPP65UnitWorkspace::GetPreprocessedTokensMallocators(void)
{
	return &msPreprocessedTokens;
}


//////////////////////////////////////////////////////////////////////////
//
// For the unit's C++ tokens (CCPPTokeniser::Init).
//
//////////////////////////////////////////////////////////////////////////
CMallocator* CCPP65UnitWorkspace::GetCPPTokensMallocator(void)
{
	return &mcCPPTokens;
}


//////////////////////////////////////////////////////////////////////////
//
// For the unit's syntax tree (CCPPSyntaxTree::Init).
//
//////////////////////////////////////////////////////////////////////////
SCPPSyntaxTreeMallocators* CCPP65UnitWorkspace::GetSyntaxTreeMallocators(void)
{
	return &msSyntaxTree;
}


//////////////////////////////////////////////////////////////////////////
//
// The bytes kept for the next unit: the arrays' blocks and the arena.
//
//////////////////////////////////////////////////////////////////////////
lsize CCPP65UnitWorkspace::GetKeptMemory(void)
{
	return mcTokens.BlockSize() + mcExpansions.BlockSize() + mcInactiveRegions.BlockSize() + mcDirectiveLines.BlockSize() +
		mcCPPTokens.BlockSize() + mcSyntaxes.BlockSize() + mcTokenSymbols.BlockSize() + mcNodeArrays.GetScratchPad()->GetMemorySize();
}
