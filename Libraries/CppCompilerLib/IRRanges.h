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
#ifndef __I_R_RANGES_H__
#define __I_R_RANGES_H__
#include "IRPassAnalysis.h"


//The signed range of an integer value: from lliMin to lliMax.
struct SIRValueRange
{
	int64	lliMin;
	int64	lliMax;
};


//What a value's range is narrowed by, beyond what its definition says (CIRRanges::RangeAt).
#define IR_RANGE_AS_DEFINED		0x00
#define IR_RANGE_BRANCHES		0x01	//The branches to the block it is asked in (the predecessors must be found).
#define IR_RANGE_DOMINATORS		0x02	//...and those above a block with several predecessors, from its dominators (worked out).


#define IR_SIGNED_RANGE_DEPTH	8	//How many definitions back SignedRange looks (through phis, adds and extensions); the other limits are in IROptimiserInternal.h.


//How big an integer value can be, and whether a pointer is null: what the passes ask.  The largest it can be as
//unsigned is UpperBound (the rules are CIRMostRules', which the 65C64's back end asks too); its range as signed
//is SignedRange, and RangeAt narrows that by the branches to a block.  The context's definitions must be found
//(and its dominators worked out, for IR_RANGE_DOMINATORS).
class CIRRanges : public CIRPassAnalysis
{
public:
	bool			RangeAt(CIRBlock* pcBlock, SIRReference sValue, SIRValueRange* psRange, uint8 uiNarrowBy, int iDepth = IR_SIGNED_RANGE_DEPTH, CIRBlock* pcTo = NULL);
	bool			SignedRange(SIRReference sValue, SIRValueRange* psRange, uint8 uiNarrowBy, int iDepth = IR_SIGNED_RANGE_DEPTH);
	uint64			UpperBound(SIRReference sValue, int iDepth);
	bool			NotNegative(CIRBlock* pcBlock, SIRReference sValue);
	bool			BranchNonNull(CIRBlock* pcFrom, CIRBlock* pcTo, SIRReference sValue);
	bool			NonNullAt(CIRBlock* pcBlock, SIRReference sValue, int iDepth);
	SIRInstruction*	EdgeComparison(CIRBlock* pcFrom, CIRBlock* pcTo, SIRReference sValue, uint16* puiOpcode, SIRReference* psOther);

protected:
	void			DominatingRange(CIRBlock* pcBlock, SIRReference sValue, SIRValueRange* psRange, uint8 uiNarrowBy);
	void			BranchRange(CIRBlock* pcFrom, CIRBlock* pcTo, SIRReference sValue, SIRValueRange* psRange);
	bool			DominatingNotLess(CIRBlock* pcBlock, SIRReference sLeft, SIRReference sRight, uint8 uiNarrowBy);
	CIRBlock*		RangeStep(CIRBlock* pcTo, uint8 uiNarrowBy, bool* pbBranch);
	bool			DivisionRange(SIRInstruction* psInstruction, SIRValueRange* psRange, uint8 uiNarrowBy, int iDepth);
	bool			ArithmeticRange(SIRInstruction* psInstruction, SIRValueRange* psRange, uint8 uiNarrowBy, int iDepth);
};


#endif // __I_R_RANGES_H__
