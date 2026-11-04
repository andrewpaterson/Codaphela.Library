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
#ifndef __C_P_P_65_UNIT_WORKSPACE_H__
#define __C_P_P_65_UNIT_WORKSPACE_H__
#include "BaseLib/ReuseAllocator.h"
#include "BaseLib/ScratchPadAllocator.h"
#include "PreprocessedTokens.h"
#include "CPPSyntaxTree.h"


//The most of its arena a workspace keeps from one unit to the next (a unit's syntax tree is mostly smaller).
#define CPP65_WORKSPACE_KEPT_ARENA	(16 MB)


//What compiling unit after unit (CCPP65Builder::CompileUnit) keeps from one unit to the next, so a unit's memory isn't
//allocated again for every unit: allocators that keep the memory of its biggest arrays (its preprocessed tokens, its
//C++ tokens and its syntax tree's), and an arena for its syntax tree's nodes, their own arrays, and its scopes and
//symbols, emptied for each unit (Begin).  Each thread compiling units has its own, so threads don't wait for each
//other's allocations.
//
//A unit's arrays are killed (their memory given back) before its workspace compiles another unit.
class CCPP65UnitWorkspace
{
protected:
	CReuseAllocator					mcTokens;
	CReuseAllocator					mcExpansions;
	CReuseAllocator					mcInactiveRegions;
	CReuseAllocator					mcDirectiveLines;
	CReuseAllocator					mcCPPTokens;
	CReuseAllocator					mcSyntaxes;
	CReuseAllocator					mcTokenSymbols;
	CScratchPadAllocator			mcNodeArrays;
	CReuseAllocator					macBlockReading[CPP_SYNTAX_BLOCK_READING_ARRAYS];	//A syntax block's reading's arrays (CCPPSyntaxBlockReader::Read).
	SPreprocessedTokensMallocators	msPreprocessedTokens;
	SCPPSyntaxTreeMallocators		msSyntaxTree;

public:
	void							Init(void);
	void							Kill(void);

	void							Begin(void);

	SPreprocessedTokensMallocators*	GetPreprocessedTokensMallocators(void);
	CMallocator*					GetCPPTokensMallocator(void);
	SCPPSyntaxTreeMallocators*		GetSyntaxTreeMallocators(void);

	lsize							GetKeptMemory(void);
};


#endif // __C_P_P_65_UNIT_WORKSPACE_H__
