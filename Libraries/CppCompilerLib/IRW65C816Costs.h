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
#ifndef __I_R_W_65_C_816_COSTS_H__
#define __I_R_W_65_C_816_COSTS_H__
#include "IRDivisionCosts.h"


//Roughly the cycles of the W65C816's code where there is a choice of code: a multiply by a constant as its runtime
//call or as shifts and adds (the back end's, CIRBackend::ExpandMultiplies), and a divide by a constant as its runtime
//call or as a multiply and a shift (the optimiser's, CIRDivisionPasses::ConstantDivisions).  A value is iWords words
//of 16 bits.
int		IR816MultiplyCallCycles(int iWords);
int		IR816ShiftAddCycles(int iWords, int* aiPlaces, int* aiSigns, int iDigits);
int		IR816MultiplyCycles(int iWords, uint64 ulliMultiplier);
int		IR816RightShiftCycles(int iWords, int iShift);


//What the W65C816's divide routines cost against a reciprocal's multiply and shift (CCPPTargetW65C816::
//ConfigureOptimiser gives the optimiser one).  It keeps nothing, so one serves every unit and thread.
class CIRW65C816DivisionCosts : public CIRDivisionCosts
{
public:
	int		DivideCost(SIRType sType, SIRConstantDivision* psDivision, int64 lliMin, int64 lliMax) override;
	int		ReciprocalCost(SIRType sType, SIRConstantDivision* psDivision, int64 lliMin, int64 lliMax, SIRReciprocal* psReciprocal) override;

protected:
	static int	ValueWords(int64 lliMin, int64 lliMax, uint64 ulliScale, int iMostWords);
};


#endif // __I_R_W_65_C_816_COSTS_H__
