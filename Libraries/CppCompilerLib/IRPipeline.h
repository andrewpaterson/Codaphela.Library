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
#ifndef __I_R_PIPELINE_H__
#define __I_R_PIPELINE_H__
#include "BaseLib/PrimitiveTypes.h"


//The optimiser's passes (IROptimiser.h says what each does), in the order the standard pipeline runs them.
enum EIRPass
{
	IRP_SimplifyCFG,
	IRP_UnifySlotTypes,
	IRP_SplitSlots,
	IRP_PromoteSlots,
	IRP_RemoveTrivialPhis,
	IRP_FoldConstants,
	IRP_FoldNullTests,
	IRP_NarrowTruncations,
	IRP_BoundDivisions,
	IRP_SignedPowerDivisions,
	IRP_DivisibilityTests,
	IRP_ConstantDivisions,
	IRP_ShareDivisions,
	IRP_RemoveCommonSubexpressions,
	IRP_RemoveRedundantLoads,
	IRP_MergeAddressPhis,
	IRP_RotateLoops,
	IRP_LessLoopTests,
	IRP_ReplaceLoopCounters,
	IRP_NarrowCounters,
	IRP_ReduceMultiplies,
	IRP_ReplaceExitValues,
	IRP_DeleteDeadLoops,
	IRP_ReduceStrength,
	IRP_SplitLoopAddresses,
	IRP_HoistInvariants,
	IRP_SinkLoads,
	IRP_HoistCommon,
	IRP_UnswitchLoops,
	IRP_ReplaceLoopIdioms,
	IRP_RemoveDeadCode,
	IRP_SinkComparisons,
	IRP_NarrowMultiplies,
	IRP_BiasSignedIndices,
	IRP_ReduceDivisions,
	IRP_NumPasses
};


class CIRPasses;


typedef bool (CIRPasses::*FIRPass)(void);


//A pass: a name, and the function of a group of passes that runs it and says whether it changed the function.
class CIRPass
{
protected:
	const char*		mszName;
	CIRPasses*		mpcGroup;
	FIRPass			mfRun;

public:
	void			Init(const char* szName, CIRPasses* pcGroup, FIRPass fRun);
	const char*		GetPassName(void);
	bool			Run(void);
};


//When a step of a pipeline runs its pass.
enum EIRPipelineStep
{
	IRPS_Round,			//With the other round steps, in order, over and over until none of them changes the function.
	IRPS_After,			//Once, after the rounds...
	IRPS_Following,		//...and these after it, if it changed the function.
};


struct SIRPipelineStep
{
	EIRPass				ePass;
	EIRPipelineStep		eStep;
};


#define IR_PIPELINE_MAX_STEPS	64


//The passes the optimiser runs on a function and their order: the steps of each round, then the steps run once
//after the rounds, each with the steps that follow it if it changed anything.  A target builds its own
//(CCPPTarget::BuildPipeline): the standard one is every pass, and a target takes out those that don't pay on it.
//It is only steps: an optimiser has the passes themselves.
class CIRPipeline
{
protected:
	SIRPipelineStep		masSteps[IR_PIPELINE_MAX_STEPS];
	int					miSteps;

public:
	void				Init(void);
	void				SetStandard(void);
	void				AddRound(EIRPass ePass);
	void				AddAfter(EIRPass ePass);
	void				AddFollowing(EIRPass ePass);
	void				Remove(EIRPass ePass);
	bool				Has(EIRPass ePass);
	int					NumSteps(void);
	SIRPipelineStep*	GetStep(int iStep);

	static const char*	GetPassName(EIRPass ePass);

protected:
	void				Add(EIRPass ePass, EIRPipelineStep eStep);
};


#endif // __I_R_PIPELINE_H__

