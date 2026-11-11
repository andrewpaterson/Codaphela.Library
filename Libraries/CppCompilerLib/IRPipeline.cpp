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
#include "IRPasses.h"
#include "IRPipeline.h"


static const char*	gaszIRPasses[IRP_NumPasses] =
{
	"SimplifyCFG",
	"UnifySlotTypes",
	"SplitSlots",
	"PromoteSlots",
	"RemoveTrivialPhis",
	"FoldConstants",
	"FoldNullTests",
	"NarrowTruncations",
	"BoundDivisions",
	"SignedPowerDivisions",
	"DivisibilityTests",
	"ConstantDivisions",
	"ShareDivisions",
	"RemoveCommonSubexpressions",
	"RemoveRedundantLoads",
	"MergeAddressPhis",
	"RotateLoops",
	"LessLoopTests",
	"ReplaceLoopCounters",
	"NarrowCounters",
	"ReduceMultiplies",
	"ReplaceExitValues",
	"DeleteDeadLoops",
	"ReduceStrength",
	"SplitLoopAddresses",
	"HoistInvariants",
	"SinkLoads",
	"HoistCommon",
	"UnswitchLoops",
	"ReplaceLoopIdioms",
	"RemoveDeadCode",
	"SinkComparisons",
	"NarrowMultiplies",
	"BiasSignedIndices",
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRPass::Init(const char* szName, CIRPasses* pcGroup, FIRPass fRun)
{
	mszName = szName;
	mpcGroup = pcGroup;
	mfRun = fRun;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CIRPass::GetPassName(void)
{
	return mszName;
}


//////////////////////////////////////////////////////////////////////////
//
// Runs the pass on its group's function.  True if it changed it.
//
//////////////////////////////////////////////////////////////////////////
bool CIRPass::Run(void)
{
	return (mpcGroup->*mfRun)();
}


//////////////////////////////////////////////////////////////////////////
//
// No steps.
//
//////////////////////////////////////////////////////////////////////////
void CIRPipeline::Init(void)
{
	miSteps = 0;
}


//////////////////////////////////////////////////////////////////////////
//
// Every pass, in the order they have always run.  Each round simplifies
// the blocks, promotes slots, folds, takes out what is worked out twice,
// then works on the loops, and ends by removing what is dead.  After the
// rounds multiplies are narrowed and signed indices biased, each followed
// by the passes that tidy up after it.
//
//////////////////////////////////////////////////////////////////////////
void CIRPipeline::SetStandard(void)
{
	miSteps = 0;
	AddRound(IRP_SimplifyCFG);
	AddRound(IRP_UnifySlotTypes);
	AddRound(IRP_SplitSlots);
	AddRound(IRP_PromoteSlots);
	AddRound(IRP_RemoveTrivialPhis);
	AddRound(IRP_FoldConstants);
	AddRound(IRP_FoldNullTests);
	AddRound(IRP_NarrowTruncations);
	AddRound(IRP_BoundDivisions);
	AddRound(IRP_SignedPowerDivisions);
	AddRound(IRP_DivisibilityTests);
	AddRound(IRP_ConstantDivisions);
	AddRound(IRP_ShareDivisions);
	AddRound(IRP_RemoveCommonSubexpressions);
	AddRound(IRP_RemoveRedundantLoads);
	AddRound(IRP_MergeAddressPhis);
	AddRound(IRP_RotateLoops);
	AddRound(IRP_LessLoopTests);
	AddRound(IRP_ReplaceLoopCounters);
	AddRound(IRP_NarrowCounters);
	AddRound(IRP_ReduceMultiplies);
	AddRound(IRP_ReplaceExitValues);
	AddRound(IRP_DeleteDeadLoops);
	AddRound(IRP_ReduceStrength);
	AddRound(IRP_SplitLoopAddresses);
	AddRound(IRP_HoistInvariants);
	AddRound(IRP_SinkLoads);
	AddRound(IRP_HoistCommon);
	AddRound(IRP_UnswitchLoops);
	AddRound(IRP_ReplaceLoopIdioms);
	AddRound(IRP_RemoveDeadCode);
	AddRound(IRP_SinkComparisons);

	AddAfter(IRP_NarrowMultiplies);
	AddFollowing(IRP_FoldConstants);
	AddFollowing(IRP_RemoveDeadCode);

	AddAfter(IRP_BiasSignedIndices);
	AddFollowing(IRP_RemoveCommonSubexpressions);
	AddFollowing(IRP_HoistInvariants);
	AddFollowing(IRP_RemoveDeadCode);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRPipeline::Add(EIRPass ePass, EIRPipelineStep eStep)
{
	if (miSteps < IR_PIPELINE_MAX_STEPS)
	{
		masSteps[miSteps].ePass = ePass;
		masSteps[miSteps].eStep = eStep;
		miSteps++;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A step of each round, after those added before it.
//
//////////////////////////////////////////////////////////////////////////
void CIRPipeline::AddRound(EIRPass ePass)
{
	Add(ePass, IRPS_Round);
}


//////////////////////////////////////////////////////////////////////////
//
// A step run once after the rounds.
//
//////////////////////////////////////////////////////////////////////////
void CIRPipeline::AddAfter(EIRPass ePass)
{
	Add(ePass, IRPS_After);
}


//////////////////////////////////////////////////////////////////////////
//
// A step run after the last AddAfter's, if that changed the function.
//
//////////////////////////////////////////////////////////////////////////
void CIRPipeline::AddFollowing(EIRPass ePass)
{
	Add(ePass, IRPS_Following);
}


//////////////////////////////////////////////////////////////////////////
//
// Takes the pass out of the rounds, and out of what is run after them, with
// the steps that follow it there.  (Where it only follows another step it
// stays: that step still needs tidying up after.)
//
//////////////////////////////////////////////////////////////////////////
void CIRPipeline::Remove(EIRPass ePass)
{
	int		iStep;
	int		iKept;
	bool	bFollowing;

	iKept = 0;
	bFollowing = false;
	for (iStep = 0; iStep < miSteps; iStep++)
	{
		if (masSteps[iStep].eStep == IRPS_Following)
		{
			if (bFollowing)
			{
				continue;
			}
		}
		else
		{
			bFollowing = masSteps[iStep].ePass == ePass;
			if (bFollowing)
			{
				continue;
			}
		}
		masSteps[iKept] = masSteps[iStep];
		iKept++;
	}
	miSteps = iKept;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the pass is a step of the rounds or one run after them.
//
//////////////////////////////////////////////////////////////////////////
bool CIRPipeline::Has(EIRPass ePass)
{
	int		iStep;

	for (iStep = 0; iStep < miSteps; iStep++)
	{
		if ((masSteps[iStep].ePass == ePass) && (masSteps[iStep].eStep != IRPS_Following))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIRPipeline::NumSteps(void)
{
	return miSteps;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SIRPipelineStep* CIRPipeline::GetStep(int iStep)
{
	return &masSteps[iStep];
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CIRPipeline::GetPassName(EIRPass ePass)
{
	return gaszIRPasses[ePass];
}

