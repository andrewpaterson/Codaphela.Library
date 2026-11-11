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
#ifndef __I_R_OPTIMISER_INTERNAL_H__
#define __I_R_OPTIMISER_INTERNAL_H__
#include "IROptimiser.h"


//What the optimiser's files (IROptimiser.cpp, IRPasses.cpp and the groups of passes' IROptimiser*.cpp) share.


//The optimiser's limits: how far its searches look, and how much it does at a time, so a large function stays
//quick to optimise.  (IR_SIGNED_RANGE_DEPTH, SignedRange's, is in IROptimiser.h for its default arguments.)
#define IR_OPTIMISE_ROUNDS			16		//The most times Optimise runs the passes on a function.
#define IR_ROTATE_LIMIT				8		//The most instructions a loop's head may have for RotateLoops to copy it.
#define IR_THREAD_LIMIT				8		//The most instructions a block may have for ThreadPhis to work out.
#define IR_KNOWN_DEPTH				8		//How many single predecessors back KnownCondition and ImpliedComparison look for a branch.
#define IR_EXTENDED_DEPTH			6		//How many definitions back Extended looks.
#define IR_CSE_SEARCH_LIMIT			4096	//The most earlier instructions RemoveCommonSubexpressions compares one with.
#define IR_AVAILABLE_LIMIT			64		//The most values RemoveRedundantLoads keeps known in memory (and stores HoistInvariants looks through).
#define IR_BASE_VALUES				16		//How many phis and their arguments PointerBase looks through.
#define IR_GUARD_DEPTH				16		//How many immediate dominators up LoopGuarded looks for the branch guarding a loop.
#define IR_REDUCE_VALUES			32		//The most values a counter's web may have (ReduceMultiplies).
#define IR_SINK_LIMIT				32		//The most loads SinkLoads moves in a function each time.
#define IR_BOUND_DEPTH				6		//How many definitions back UpperBound looks.
#define IR_RANGE_DEPTH				8		//How many single predecessors up DominatingRange looks.
#define IR_DIVISION_RANGE			0x20000	//The most values ConstantDivisions checks a reciprocal for.
#define IR_DIVISION_TABLE_SAVING	50		//The cycles a byte divide's table saves over its reciprocal (a hot one's, IRF_Hot).
#define IR_NON_NULL_DEPTH			6		//How many definitions back NonNullAt looks (through phis and pointer adds).
#define IR_UNSWITCH_INSTRUCTIONS	48		//The most instructions and phis a loop unswitched may have...
#define IR_UNSWITCH_LOOPS			4		//...and the most loops unswitched in a function each time it is optimised.
#define IR_NARROW_DEPTH				4		//How deep NarrowTruncations follows operations to narrow.
#define IR_IDIOM_MIN_BYTES			16		//A loop filling or copying a known number of bytes up to this many stays a loop.


//IROptimiserFolding.cpp: true if two instructions work out the same value (either way round for a commutative opcode).
bool	IROptimiserSameInstruction(CIRFunction* pcFunction, SIRInstruction* psEarlier, SIRInstruction* psInstruction);

//IROptimiserLoops.cpp: true if an instruction may be moved out of a loop when its operands are worked out outside it.
bool	IROptimiserHoistable(CIRFunction* pcFunction, SIRInstruction* psInstruction);


#endif // __I_R_OPTIMISER_INTERNAL_H__
