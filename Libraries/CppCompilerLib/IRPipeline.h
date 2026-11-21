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


//The optimiser's passes, a row each: its name (IRP_Name in the enum, and the function of that name in its group),
//the group that has it (CIROptimiser's member, and the group's class), and what it does.  Everything that lists
//the passes is made from these rows: the enum, the names, the optimiser's CIRPass for each.  The enum's order is the
//rows' (ReduceDivisions, the last added, is last); the order the passes run in is a pipeline's
//(CIRPipeline::SetStandard), which a target takes passes out of (CCPPTarget::BuildPipeline).
#define IR_PASSES(X) \
	X(SimplifyCFG, mcBlockPasses, CIRBlockPasses, \
		"Removes unreachable blocks, jumps through empty ones, joins a block to its only successor, folds a brif on a constant.") \
	X(UnifySlotTypes, mcMemoryPasses, CIRMemoryPasses, \
		"A slot's pointer also loaded or stored as an integer is only loaded and stored as a pointer, so the slot can be split.") \
	X(SplitSlots, mcMemoryPasses, CIRMemoryPasses, \
		"A slot only loaded and stored at constant offsets that don't overlap (an object's fields) becomes a slot for each offset.") \
	X(PromoteSlots, mcPromotionPasses, CIRPromotionPasses, \
		"Slots and argaddrs that are only loaded from and stored to become temporaries and phis.") \
	X(RemoveTrivialPhis, mcBlockPasses, CIRBlockPasses, \
		"A phi whose arguments are all one value is that value; one the same as an earlier phi of its block is that phi.") \
	X(FoldConstants, mcFoldingPasses, CIRFoldingPasses, \
		"Instructions of constants are their results; x * 2^n and unsigned x / 2^n are shifts; a comparison a branch decides is a constant.") \
	X(FoldNullTests, mcRangePasses, CIRRangePasses, \
		"p == 0 and p != 0 of a pointer that can't be null there are false and true.") \
	X(NarrowTruncations, mcRangePasses, CIRRangePasses, \
		"A trunc of an add, sub, mul, and, or, xor or shift left by a constant is the operation done narrow.") \
	X(BoundDivisions, mcDivisionPasses, CIRDivisionPasses, \
		"An x / c or x % c with x known to be below c is 0 or x, and below 2c a compare and a subtract.") \
	X(SignedPowerDivisions, mcDivisionPasses, CIRDivisionPasses, \
		"A signed x / 2^n or x % 2^n is shifts and adds.") \
	X(DivisibilityTests, mcDivisionPasses, CIRDivisionPasses, \
		"x % c == 0 and x % c != 0 without a divide: a multiply by c's inverse and a compare.") \
	X(ConstantDivisions, mcDivisionPasses, CIRDivisionPasses, \
		"x / c and x % c with x in a known range are a multiply by a reciprocal and a shift: for a target whose divides are calls.") \
	X(ShareDivisions, mcDivisionPasses, CIRDivisionPasses, \
		"x % d next to x / d is x - (x / d) * d, or is put where the divide leaves the remainder: one divide for two.") \
	X(RemoveCommonSubexpressions, mcFoldingPasses, CIRFoldingPasses, \
		"An instruction without effects that is the same as one that dominates it is that one's result.") \
	X(RemoveRedundantLoads, mcMemoryPasses, CIRMemoryPasses, \
		"A load of what is known to be in memory (loaded or stored before, nothing between that may change it) is that value.") \
	X(MergeAddressPhis, mcAddressPasses, CIRAddressPasses, \
		"A phi of pointers that are one base plus i16 indices is the base plus a phi of the indices.") \
	X(RotateLoops, mcLoopPasses, CIRLoopPasses, \
		"A loop whose head tests and whose latch jumps back to it tests at the bottom.") \
	X(LessLoopTests, mcLoopPasses, CIRLoopPasses, \
		"A loop that goes round while a <= b goes round while a < b + 1, where that can't wrap.") \
	X(ReplaceLoopCounters, mcCounterPasses, CIRCounterPasses, \
		"A loop with two counters that only tests one of them tests the other, and the first goes.") \
	X(NarrowCounters, mcCounterPasses, CIRCounterPasses, \
		"A 32 bit loop counter that stays in 16 bits counts in 16 bits.") \
	X(ReduceMultiplies, mcCounterPasses, CIRCounterPasses, \
		"A multiply of a loop's counter by a value the loop doesn't change is a value stepped beside the counter.") \
	X(ReplaceExitValues, mcLoopPasses, CIRLoopPasses, \
		"A value stepped each time round a loop that goes round a known number of times is worked out from that number where it leaves.") \
	X(DeleteDeadLoops, mcLoopPasses, CIRLoopPasses, \
		"A loop that does nothing and whose values nothing outside it reads goes.") \
	X(ReduceStrength, mcCounterPasses, CIRCounterPasses, \
		"An address worked out from a loop's counter each time round is a pointer of its own, stepped.") \
	X(SplitLoopAddresses, mcAddressPasses, CIRAddressPasses, \
		"In a loop, p + (a + b) with p and a unchanged is (p + a) + b, whose inner add is then moved out.") \
	X(HoistInvariants, mcLoopPasses, CIRLoopPasses, \
		"An instruction in a loop whose operands the loop doesn't change moves to the block that enters the loop.") \
	X(SinkLoads, mcLoopPasses, CIRLoopPasses, \
		"A load whose only use is in another block moves to just before that use.") \
	X(HoistCommon, mcLoopPasses, CIRLoopPasses, \
		"The same instruction in both blocks a brif goes to is worked out once, before the branch.") \
	X(UnswitchLoops, mcLoopPasses, CIRLoopPasses, \
		"A small loop that branches on a condition that is the same each time round is two loops, one for each way.") \
	X(ReplaceLoopIdioms, mcIdiomPasses, CIRIdiomPasses, \
		"A loop that fills or copies memory, measures a string or looks for a byte is a call of memset, memmove, strlen, memcmp, memchr or strchr.") \
	X(RemoveDeadCode, mcBlockPasses, CIRBlockPasses, \
		"Instructions without effects whose results nothing uses go.") \
	X(SinkComparisons, mcBlockPasses, CIRBlockPasses, \
		"An integer comparison only a brif uses moves to the end of the brif's block.") \
	X(NarrowMultiplies, mcRangePasses, CIRRangePasses, \
		"A multiple of an extended i8 or i16 value that fits 16 bits is a 16 bit multiply, extended.") \
	X(BiasSignedIndices, mcAddressPasses, CIRAddressPasses, \
		"p + sext(x), x an i16, is (p - 32768) + zext(x ^ 32768), which the target adds in a register as it loads.") \
	X(ReduceDivisions, mcCounterPasses, CIRCounterPasses, \
		"A loop's counter divided by a constant each time round has its quotient and its remainder stepped beside it.")


enum EIRPass
{
#define IR_PASS_ENUM(NAME, GROUP, GROUPCLASS, DOES)	IRP_##NAME,
	IR_PASSES(IR_PASS_ENUM)
#undef IR_PASS_ENUM
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
	const char*		GetPassName(void) const;
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
	int					NumSteps(void) const;
	SIRPipelineStep*	GetStep(int iStep);

	static const char*	GetPassName(EIRPass ePass);
	static const char*	GetPassDescription(EIRPass ePass);

protected:
	void				Add(EIRPass ePass, EIRPipelineStep eStep);
};


#endif // __I_R_PIPELINE_H__

