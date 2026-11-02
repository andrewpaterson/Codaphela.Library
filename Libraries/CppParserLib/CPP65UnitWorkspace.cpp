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
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPP65UnitWorkspace::Kill(void)
{
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
// Another unit is to be compiled: the last one's syntax tree's node
// arrays are let go all at once.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65UnitWorkspace::Begin(void)
{
	mcNodeArrays.GetScratchPad()->Reset();
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
