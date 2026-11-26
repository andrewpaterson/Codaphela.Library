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
#ifndef __I_R_OPTIMISER_DIVISIONS_H__
#define __I_R_OPTIMISER_DIVISIONS_H__
#include "IRPasses.h"
#include "IRDivisionCosts.h"


//How ConstantDivisions does a divide or remainder by a constant.
enum EIRConstantDivision
{
	IRCD_Table,			//A hot one of a byte's range: made an i8 one for the target's table.
	IRCD_Reciprocal,	//A multiply and a shift (sReciprocal).
	IRCD_Split,			//A wide divisor 2^k * m: divided by 2^k, then by m.
	IRCD_None
};


//A divide or remainder by a constant ConstantDivisions found a cheaper way of.
struct SIRFoundDivision
{
	CIRBlock*		pcBlock;
	size			uiInstruction;
	uint8			uiKind;			//EIRConstantDivision.
	SIRValueRange	sRange;			//The dividend's.
	SIRReciprocal	sReciprocal;	//IRCD_Reciprocal.
};


//The passes over divisions: by a constant as a multiply, bounded, by a power of two, shared with their
//remainders, and tests of divisibility.
class CIRDivisionPasses : public CIRPasses
{
public:
	bool			BoundDivisions(void);
	bool			ConstantDivisions(void);
	bool			ShareDivisions(void);
	bool			DivisibilityTests(void);
	bool			SignedPowerDivisions(void);

protected:
	bool			SplitDivisor(SIRType sType, int64 lliDivisor, int64 lliMin, int64 lliMax);
	bool			FindReciprocal(int64 lliMin, int64 lliMax, int64 lliDivisor, int iBits, bool bSigned, SIRReciprocal* psReciprocal);
	bool			IsReciprocalCheaper(CIRBlock* pcBlock, SIRInstruction* psInstruction, SIRConstantDivision* psDivision, SIRValueRange* psRange, SIRReciprocal* psReciprocal);
	bool			LeavesRemainder(SIRInstruction* psRemainder);
	bool			HasDivision(CIRBlock* pcBlock, uint16 uiOpcode, SIRReference sValue, SIRReference sDivisor);
	bool			IsByteTableDivision(int64 lliMin, int64 lliMax, int64 lliDivisor);
	bool			DivisionReciprocal(int64 lliMin, int64 lliMax, int64 lliDivisor, int iBits, bool bSigned, uint64* pulliMultiplier, int* piShift);
};


#endif // __I_R_OPTIMISER_DIVISIONS_H__

