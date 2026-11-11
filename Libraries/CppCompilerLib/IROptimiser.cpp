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
#include "IRInliner.h"
#include "IROptimiserInternal.h"


//////////////////////////////////////////////////////////////////////////
//
// The context, and the groups of passes, which find each other in it.
//
//////////////////////////////////////////////////////////////////////////
void CIROptimiser::Init(CIRModule* pcModule)
{
	mcContext.Init(pcModule);
	mcAddressPasses.Init(&mcContext);
	mcContext.mpcAddressPasses = &mcAddressPasses;
	mcBlockPasses.Init(&mcContext);
	mcContext.mpcBlockPasses = &mcBlockPasses;
	mcCounterPasses.Init(&mcContext);
	mcContext.mpcCounterPasses = &mcCounterPasses;
	mcDivisionPasses.Init(&mcContext);
	mcContext.mpcDivisionPasses = &mcDivisionPasses;
	mcFoldingPasses.Init(&mcContext);
	mcContext.mpcFoldingPasses = &mcFoldingPasses;
	mcIdiomPasses.Init(&mcContext);
	mcContext.mpcIdiomPasses = &mcIdiomPasses;
	mcLoopPasses.Init(&mcContext);
	mcContext.mpcLoopPasses = &mcLoopPasses;
	mcMemoryPasses.Init(&mcContext);
	mcContext.mpcMemoryPasses = &mcMemoryPasses;
	mcPromotionPasses.Init(&mcContext);
	mcContext.mpcPromotionPasses = &mcPromotionPasses;
	mcRangePasses.Init(&mcContext);
	mcContext.mpcRangePasses = &mcRangePasses;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIROptimiser::Kill(void)
{
	mcRangePasses.Kill();
	mcPromotionPasses.Kill();
	mcMemoryPasses.Kill();
	mcLoopPasses.Kill();
	mcIdiomPasses.Kill();
	mcFoldingPasses.Kill();
	mcDivisionPasses.Kill();
	mcCounterPasses.Kill();
	mcBlockPasses.Kill();
	mcAddressPasses.Kill();
	mcContext.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The function the passes are run on from now (NULL for none): the
// context's and each group's.
//
//////////////////////////////////////////////////////////////////////////
void CIROptimiser::Begin(CIRFunction* pcFunction)
{
	mcContext.mpcFunction = pcFunction;
	mcAddressPasses.Begin(pcFunction);
	mcBlockPasses.Begin(pcFunction);
	mcCounterPasses.Begin(pcFunction);
	mcDivisionPasses.Begin(pcFunction);
	mcFoldingPasses.Begin(pcFunction);
	mcIdiomPasses.Begin(pcFunction);
	mcLoopPasses.Begin(pcFunction);
	mcMemoryPasses.Begin(pcFunction);
	mcPromotionPasses.Begin(pcFunction);
	mcRangePasses.Begin(pcFunction);
}


//////////////////////////////////////////////////////////////////////////
//
// The type memset's int is (16 or 32 bits): loops that fill or copy memory
// become calls to the library's memset and memmove (ReplaceLoopIdioms).
// IRTK_None (the default) leaves them loops.
//
//////////////////////////////////////////////////////////////////////////
void CIROptimiser::SetLibraryInt(SIRType sInt)
{
	mcContext.msLibraryInt = sInt;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the target's loads and stores add a pointer and an index for
// nothing (the 65C64's [R1 + R2]; the W65C816's don't): if not, p + i in a
// loop is strength reduced too.
//
//////////////////////////////////////////////////////////////////////////
void CIROptimiser::SetIndexedAddresses(bool bIndexedAddresses)
{
	mcContext.mbIndexedAddresses = bIndexedAddresses;
}


//////////////////////////////////////////////////////////////////////////
//
// Where the comparisons found always true or always false because one side
// is zero extended from a type that can't hold the constant on the other are
// described (ComparisonRange), a line each.  The front end can only see the
// types: x != 0xFFFFFFFF with a 32 bit x assigned a 16 bit value is seen
// here, once the values are followed.
//
//////////////////////////////////////////////////////////////////////////
void CIROptimiser::SetWarnings(CChars* pszWarnings)
{
	mcContext.mpszWarnings = pszWarnings;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether NarrowMultiplies runs (for a target whose multiplies cost more
// the wider they are: the W65C816's are shifts and adds or calls).
//
//////////////////////////////////////////////////////////////////////////
void CIROptimiser::SetNarrowMultiplies(bool bNarrowMultiplies)
{
	mcContext.mbNarrowMultiplies = bNarrowMultiplies;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether ConstantDivisions runs (for a target whose divides are calls:
// the W65C816's).
//
//////////////////////////////////////////////////////////////////////////
void CIROptimiser::SetConstantDivisions(bool bConstantDivisions)
{
	mcContext.mbConstantDivisions = bConstantDivisions;
}


//////////////////////////////////////////////////////////////////////////
//
// The target's divides (IR_DIVIDE_REMAINDERS_ widths, and signed ones)
// that leave the remainder where a urem or srem straight after them with
// the same operands can take it (the W65C816's R816_UDiv16, R816_SDiv32
// ...: D + 5; the 65C64's div: B).  ShareDivisions puts such a urem right
// after its udiv.
//
//////////////////////////////////////////////////////////////////////////
void CIROptimiser::SetDivideRemainders(uint32 uiDivideRemainders)
{
	mcContext.muiDivideRemainders = uiDivideRemainders;
}


//////////////////////////////////////////////////////////////////////////
//
// How often the profiled run ran each instruction's line, for the passes
// that spend memory on hot code (ConstantDivisions' byte tables).  NULL for
// no profile.
//
//////////////////////////////////////////////////////////////////////////
void CIROptimiser::SetProfile(CIRInlineProfile* pcProfile)
{
	mcContext.mpcProfile = pcProfile;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether NarrowCounters runs (for a target whose 32 bit arithmetic takes
// more instructions than 16 bit: the W65C816's).
//
//////////////////////////////////////////////////////////////////////////
void CIROptimiser::SetNarrowCounters(bool bNarrowCounters)
{
	mcContext.mbNarrowCounters = bNarrowCounters;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether BiasSignedIndices runs (for a target that adds an unsigned 16
// bit index to a pointer for nothing: the W65C816's [p],Y).
//
//////////////////////////////////////////////////////////////////////////
void CIROptimiser::SetBiasedIndices(bool bBiasedIndices)
{
	mcContext.mbBiasedIndices = bBiasedIndices;
}


//////////////////////////////////////////////////////////////////////////
//
// Runs the passes until none changes the function.  Returns true if any did.
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::Optimise(CIRFunction* pcFunction)
{
	bool	bChanged;
	bool	bAny;
	int		iRound;

	Begin(pcFunction);
	mcContext.mbRestrict = mcContext.mpcModule->HasRestrict() || (pcFunction->muiRestrictParameters != 0);
	mcContext.miUnswitched = 0;
	bAny = false;
	mcContext.mpcModule->Count(CPPCC_OptimisedFunctions);
	for (iRound = 0; iRound < IR_OPTIMISE_ROUNDS; iRound++)
	{
		mcContext.mpcModule->Count(CPPCC_OptimiserRounds);
		bChanged = mcBlockPasses.SimplifyCFG();
		bChanged |= mcMemoryPasses.UnifySlotTypes();
		bChanged |= mcMemoryPasses.SplitSlots();
		bChanged |= mcPromotionPasses.PromoteSlots();
		bChanged |= mcBlockPasses.RemoveTrivialPhis();
		bChanged |= mcFoldingPasses.FoldConstants();
		bChanged |= mcRangePasses.FoldNullTests();
		bChanged |= mcRangePasses.NarrowTruncations();
		bChanged |= mcDivisionPasses.BoundDivisions();
		bChanged |= mcDivisionPasses.SignedPowerDivisions();
		bChanged |= mcContext.mbConstantDivisions && mcDivisionPasses.DivisibilityTests();
		bChanged |= mcContext.mbConstantDivisions && mcDivisionPasses.ConstantDivisions();
		bChanged |= (mcContext.mbConstantDivisions || (mcContext.muiDivideRemainders != 0)) && mcDivisionPasses.ShareDivisions();
		bChanged |= mcFoldingPasses.RemoveCommonSubexpressions();
		bChanged |= mcMemoryPasses.RemoveRedundantLoads();
		bChanged |= mcContext.mbBiasedIndices && mcAddressPasses.MergeAddressPhis();
		bChanged |= mcLoopPasses.RotateLoops();
		bChanged |= mcLoopPasses.LessLoopTests();
		bChanged |= mcCounterPasses.ReplaceLoopCounters();
		bChanged |= mcContext.mbNarrowCounters && mcCounterPasses.NarrowCounters();
		bChanged |= mcCounterPasses.ReduceMultiplies();
		bChanged |= mcLoopPasses.ReplaceExitValues();
		bChanged |= mcLoopPasses.DeleteDeadLoops();
		bChanged |= mcCounterPasses.ReduceStrength();
		bChanged |= mcAddressPasses.SplitLoopAddresses();
		bChanged |= mcLoopPasses.HoistInvariants();
		bChanged |= mcLoopPasses.SinkLoads();
		bChanged |= mcLoopPasses.HoistCommon();
		bChanged |= mcLoopPasses.UnswitchLoops();
		bChanged |= mcIdiomPasses.ReplaceLoopIdioms();
		bChanged |= mcBlockPasses.RemoveDeadCode();
		bChanged |= mcBlockPasses.SinkComparisons();
		if (!bChanged)
		{
			break;
		}
		bAny = true;
	}
	if (mcContext.mbNarrowMultiplies && mcRangePasses.NarrowMultiplies())
	{
		mcFoldingPasses.FoldConstants();
		mcBlockPasses.RemoveDeadCode();
		bAny = true;
	}
	if (mcContext.mbBiasedIndices && mcAddressPasses.BiasSignedIndices())
	{
		mcFoldingPasses.RemoveCommonSubexpressions();
		mcLoopPasses.HoistInvariants();
		mcBlockPasses.RemoveDeadCode();
		bAny = true;
	}
	pcFunction->Renumber();
	pcFunction->ComputePredecessors();
	return bAny;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether pcFunction may write memory its callers can see (CIRPasses::
// WritesMemory).  CIRInterprocedural::MarkNoWrites works the unit's functions
// out with it, together.
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::WritesMemory(CIRFunction* pcFunction)
{
	bool	bWrites;

	Begin(pcFunction);
	bWrites = mcMemoryPasses.WritesMemory();
	Begin(NULL);
	return bWrites;
}

