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
#ifndef __I_R_DIVISION_COSTS_H__
#define __I_R_DIVISION_COSTS_H__
#include "IRFunction.h"


//x / c as (x * ulliMultiplier) >> iShift, for every x of a range (CIRDivisionPasses::FindReciprocal).
struct SIRReciprocal
{
	uint64	ulliMultiplier;
	int		iShift;
	int		iBits;			//What the product is worked out in: the division's type's bits, or 32.
	int		iPreShift;		//For c = 2^k * m: x is shifted right k first and the reciprocal is m's; 0 for none.
};


//What a divide or remainder by a constant costs a target two ways: as its divide, and as the multiply and shift that
//give the same (a reciprocal, with the multiply back and the subtract for a remainder).  The measure is the target's
//own (the W65C816's is cycles); x, the dividend, is of sType and runs from lliMin to lliMax.  A target whose divides
//are calls gives the optimiser one (CIROptimiser::SetDivisionCosts), and ConstantDivisions then writes a reciprocal
//only where it costs less than the divide.
class CIRDivisionCosts
{
public:
	virtual int	DivideCost(SIRType sType, SIRConstantDivision* psDivision, int64 lliMin, int64 lliMax) = 0;
	virtual int	ReciprocalCost(SIRType sType, SIRConstantDivision* psDivision, int64 lliMin, int64 lliMax, SIRReciprocal* psReciprocal) = 0;
};


#endif // __I_R_DIVISION_COSTS_H__
