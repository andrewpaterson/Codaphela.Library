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
	mcBlockPasses.Init(&mcContext);
	mcContext.mpcBlockPasses = &mcBlockPasses;
	mcCounterPasses.Init(&mcContext);
	mcContext.mpcCounterPasses = &mcCounterPasses;
	mcDivisionPasses.Init(&mcContext);
	mcFoldingPasses.Init(&mcContext);
	mcContext.mpcFoldingPasses = &mcFoldingPasses;
	mcIdiomPasses.Init(&mcContext);
	mcLoopPasses.Init(&mcContext);
	mcMemoryPasses.Init(&mcContext);
	mcContext.mpcMemoryPasses = &mcMemoryPasses;
	mcPromotionPasses.Init(&mcContext);
	mcRangePasses.Init(&mcContext);
	mcContext.mpcRangePasses = &mcRangePasses;
	InitPasses();
	mcOwnPipeline.SetStandard();
	mpcPipeline = &mcOwnPipeline;
	mcContext.mpcPipeline = mpcPipeline;
}


//////////////////////////////////////////////////////////////////////////
//
// Each pass: its name and the function of its group that runs it, from
// its row of IR_PASSES.
//
//////////////////////////////////////////////////////////////////////////
void CIROptimiser::InitPasses(void)
{
#define IR_PASS_INIT(NAME, GROUP, GROUPCLASS, DOES)	macPasses[IRP_##NAME].Init(CIRPipeline::GetPassName(IRP_##NAME), &GROUP, (FIRPass)&GROUPCLASS::NAME);
	IR_PASSES(IR_PASS_INIT)
#undef IR_PASS_INIT
}


//////////////////////////////////////////////////////////////////////////
//
// The passes run and their order (the target's: CCPPTarget::BuildPipeline),
// in place of the optimiser's own, which is the standard one.  pcPipeline
// must outlive the optimiser.
//
//////////////////////////////////////////////////////////////////////////
void CIROptimiser::SetPipeline(CIRPipeline* pcPipeline)
{
	mpcPipeline = pcPipeline ? pcPipeline : &mcOwnPipeline;
	mcContext.mpcPipeline = mpcPipeline;
}


//////////////////////////////////////////////////////////////////////////
//
// The pipeline the optimiser runs: its own unless one was set.
//
//////////////////////////////////////////////////////////////////////////
CIRPipeline* CIROptimiser::GetPipeline(void) const
{
	return mpcPipeline;
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
	mcContext.Changed();
	mcContext.mcRanges.Begin(pcFunction);
	mcContext.mcAddresses.Begin(pcFunction);
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
// The type the library's size_t is (32 bits, the default, or 16): with 16
// a count that may not fit is passed to the 32 suffixed function (memset32
// and the others, the W65C816's Libc65).
//
//////////////////////////////////////////////////////////////////////////
void CIROptimiser::SetLibrarySize(SIRType sSize)
{
	mcContext.msLibrarySize = sSize;
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
// Runs the pipeline's passes on the function: the steps of a round, in order,
// over and over until a round changes nothing (IR_OPTIMISE_ROUNDS at most),
// then each step that runs after the rounds, with the steps that follow it if
// it changed the function.  Returns true if any pass changed it.
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::Optimise(CIRFunction* pcFunction)
{
	SIRPipelineStep*	psStep;
	int					iSteps;
	int					iStep;
	int					iRound;
	bool				bChanged;
	bool				bFollow;
	bool				bAny;

	Begin(pcFunction);
	mcContext.mbRestrict = mcContext.mpcModule->HasRestrict() || (pcFunction->muiRestrictParameters != 0);
	mcContext.miUnswitched = 0;
	bAny = false;
	iSteps = mpcPipeline->NumSteps();
	mcContext.mpcModule->Count(CPPCC_OptimisedFunctions);
	for (iRound = 0; iRound < IR_OPTIMISE_ROUNDS; iRound++)
	{
		mcContext.mpcModule->Count(CPPCC_OptimiserRounds);
		bChanged = false;
		for (iStep = 0; iStep < iSteps; iStep++)
		{
			psStep = mpcPipeline->GetStep(iStep);
			if (psStep->eStep == IRPS_Round)
			{
				bChanged |= RunPass(psStep->ePass, iRound);
			}
		}
		if (!bChanged)
		{
			break;
		}
		bAny = true;
	}

	bFollow = false;
	for (iStep = 0; iStep < iSteps; iStep++)
	{
		psStep = mpcPipeline->GetStep(iStep);
		if (psStep->eStep == IRPS_After)
		{
			bFollow = RunPass(psStep->ePass, CPP_NO_ROUND);
			bAny |= bFollow;
		}
		else if ((psStep->eStep == IRPS_Following) && bFollow)
		{
			RunPass(psStep->ePass, CPP_NO_ROUND);
		}
	}
	pcFunction->Renumber();
	pcFunction->ComputePredecessors();
	return bAny;
}


//////////////////////////////////////////////////////////////////////////
//
// Runs a pass, and counts it (and times it, when times are kept) in the
// unit's statistics.  True if it changed the function: the context is told,
// as its analyses then no longer describe it.
//
//////////////////////////////////////////////////////////////////////////
bool CIROptimiser::RunPass(EIRPass ePass, int iRound)
{
	CCPPCompileStatistics*	pcStatistics;
	int64					lliStart;
	bool					bChanged;

	pcStatistics = mcContext.mpcModule->mpcStatistics;
	if (!pcStatistics)
	{
		bChanged = macPasses[ePass].Run();
		if (bChanged)
		{
			mcContext.Changed();
		}
		return bChanged;
	}
	lliStart = pcStatistics->Now();
	bChanged = macPasses[ePass].Run();
	pcStatistics->CountPass(ePass, macPasses[ePass].GetPassName(), bChanged, iRound, pcStatistics->Now() - lliStart);
	if (bChanged)
	{
		mcContext.Changed();
	}
	return bChanged;
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

