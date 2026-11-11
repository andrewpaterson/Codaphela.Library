/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CppCompilerLib

Codaphela CppCompilerLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CppCompilerLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CppCompilerLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __C_P_P_COMPILE_STATISTICS_H__
#define __C_P_P_COMPILE_STATISTICS_H__
#include "BaseLib/PrimitiveTypes.h"
#include "BaseLib/Chars.h"


//What a unit's compiling time is spent on.  A phase begun inside another takes the time from it until it ends, so
//each phase's time is its own (the types worked out while a body is written aren't the front end's).
enum ECPPCompilePhase
{
	CPPCP_Preprocess,
	CPPCP_Tokenise,
	CPPCP_Parse,
	CPPCP_Declarations,		//The generator outside function bodies: declarations, classes, templates, data.
	CPPCP_Types,			//Expressions' types (CCPP65TypeChecker).
	CPPCP_FrontEnd,			//Function bodies written as IR.
	CPPCP_Inline,			//Inlining and the passes over the whole unit.
	CPPCP_Optimise,			//CIROptimiser.
	CPPCP_BackEnd,			//IR to the target's code.
	CPPCP_CodePasses,		//The passes over the code a back end wrote.
	CPPCP_Assemble,
	CPPCP_Link,
	CPPCP_NumPhases
};


//What is counted while a unit is compiled.
enum ECPPCompileCount
{
	CPPCC_Functions,				//Function bodies written as IR.
	CPPCC_TypeOf,					//CCPP65Generator::TypeOf calls...
	CPPCC_TypeOfWorkedOut,			//...whose answer wasn't remembered.
	CPPCC_QuietTrials,				//CCPP65Generator::BeginQuiet calls that weren't inside another...
	CPPCC_QuietTrialsFailed,		//...and how many of them ended with an error noted.
	CPPCC_FindFunction,				//The ways in to choosing a function, each counted as it is called.
	CPPCC_FindNamedFunction,
	CPPCC_FindAddressedFunction,
	CPPCC_FindAssociatedFunction,
	CPPCC_FindOperator,
	CPPCC_FindFreeOperator,
	CPPCC_FindStaticFunction,
	CPPCC_FindConversion,
	CPPCC_FindMemberTemplate,
	CPPCC_ChooseMemberTemplate,
	CPPCC_ChooseTemplate,
	CPPCC_GetCallTemplate,
	CPPCC_PackExpansions,			//Calls whose pack arguments were expanded (ExpandPackArguments)...
	CPPCC_PackExpansionNodes,		//...and the syntax nodes made for them.
	CPPCC_UnsupportedExits,			//CCPP65IRFrontEnd::Unsupported calls.
	CPPCC_Code65Loaded,				//A 65C64 function's code decoded for a pass over it...
	CPPCC_Code65Refused,			//...and those it couldn't be (the pass leaves the code as it is).
	CPPCC_Dominators,				//CIRDominators::Compute calls by the optimiser, the inliner and the back ends.
	CPPCC_OptimisedFunctions,		//CIROptimiser::Optimise calls...
	CPPCC_OptimiserRounds,			//...and the rounds of passes they ran.
	CPPCC_NumCounts
};


//The times and counts of compiling a unit, or of a build (Add).  One is used by one thread at a time.  The counts are
//always kept; the times only when SetTimes(true), as reading the clock costs more than counting.
class CCPPCompileStatistics
{
protected:
	int64				malliMicroseconds[CPPCP_NumPhases];
	uint64				mauiCounts[CPPCC_NumCounts];
	ECPPCompilePhase	maePhases[16];		//The phases begun and not ended, the last begun last.
	int					miDepth;
	int64				mlliSince;			//When the last of them began, or the one begun in it ended.
	bool				mbTimes;

public:
	void		Init(void);
	void		Kill(void);

	void		SetTimes(bool bTimes);
	bool		IsTimed(void);
	void		Begin(ECPPCompilePhase ePhase);
	void		End(ECPPCompilePhase ePhase);
	void		Count(ECPPCompileCount eCount, uint64 uiCount = 1);

	void		Add(CCPPCompileStatistics* pcOther);
	void		Clear(void);
	int64		GetMicroseconds(ECPPCompilePhase ePhase);
	uint64		GetCount(ECPPCompileCount eCount);
	void		Print(CChars* pszDest);

	static const char*	GetPhaseName(ECPPCompilePhase ePhase);
	static const char*	GetCountName(ECPPCompileCount eCount);
};


#endif // __C_P_P_COMPILE_STATISTICS_H__

