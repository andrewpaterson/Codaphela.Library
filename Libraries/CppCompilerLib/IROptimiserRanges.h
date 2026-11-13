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
#ifndef __I_R_OPTIMISER_RANGES_H__
#define __I_R_OPTIMISER_RANGES_H__
#include "IRPasses.h"


//What range a value is in and whether a pointer is null, for the other passes; and the passes that use it
//themselves: narrowed truncations, multiplies and comparisons, and null tests folded.
class CIRRangePasses : public CIRPasses
{
public:
	bool			NarrowTruncations(void);
	bool			NarrowMultiplies(void);
	bool			RangeAt(CIRBlock* pcBlock, SIRReference sValue, int64* plliMin, int64* plliMax, int iDepth = IR_SIGNED_RANGE_DEPTH, CIRBlock* pcTo = NULL);
	bool			NotNegative(CIRBlock* pcBlock, SIRReference sValue);
	bool			BranchNonNull(CIRBlock* pcFrom, CIRBlock* pcTo, SIRReference sValue);
	bool			NonNullAt(CIRBlock* pcBlock, SIRReference sValue, int iDepth);
	bool			FoldNullTests(void);
	bool			SignedRange(SIRReference sValue, int64* plliMin, int64* plliMax, int iDepth = IR_SIGNED_RANGE_DEPTH);
	bool			NarrowComparison(SIRInstruction* psInstruction);
	uint64			UpperBound(SIRReference sValue, int iDepth);

protected:
	void			DominatingRange(CIRBlock* pcBlock, SIRReference sValue, int64* plliMin, int64* plliMax);
	void			BranchRange(CIRBlock* pcFrom, CIRBlock* pcTo, SIRReference sValue, int64* plliMin, int64* plliMax);
	bool			DominatingNotLess(CIRBlock* pcBlock, SIRReference sLeft, SIRReference sRight);
	SIRInstruction*	ExtendedMultiple(SIRReference sValue, int64* plliMultiple, int iDepth);
	bool			BoundIndices(void);
	CIRBlock*		RangeStep(CIRBlock* pcTo, bool* pbBranch);
	bool			DivisionRange(SIRInstruction* psInstruction, int64* plliMin, int64* plliMax, int iDepth);
	bool			ArithmeticRange(SIRInstruction* psInstruction, int64* plliMin, int64* plliMax, int iDepth);
	SIRReference	Unextended(SIRReference sValue, uint16 uiOpcode, SIRType* psType);
	void			ComparisonRange(SIRInstruction* psInstruction);
	SIRReference	FreeNarrow(SIRReference sValue, SIRType sType);
	uint64			EdgeBound(CIRBlock* pcFrom, CIRBlock* pcTo, SIRReference sValue);
	SIRInstruction*	EdgeComparison(CIRBlock* pcFrom, CIRBlock* pcTo, SIRReference sValue, uint16* puiOpcode, SIRReference* psOther);
	SIRReference	Narrowed(SIRReference sValue, SIRType sType, CArrayIRInstructions* pasNew, int iDepth);
};


#endif // __I_R_OPTIMISER_RANGES_H__

