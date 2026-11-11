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
	bool			SplitDivisor(SIRInstruction* psInstruction, int64 lliMin, int64 lliMax);
	bool			FindReciprocal(int64 lliMin, int64 lliMax, int64 lliDivisor, int iBits, bool bSigned, uint64* pulliMultiplier, int* piShift, int* piBits, int* piPreShift);
	bool			LeavesRemainder(SIRInstruction* psRemainder);
	bool			HasDivision(CIRBlock* pcBlock, uint16 uiOpcode, SIRReference sValue, SIRReference sDivisor);
	bool			IsByteTableDivision(int64 lliMin, int64 lliMax, int64 lliDivisor);
	bool			DivisionReciprocal(int64 lliMin, int64 lliMax, int64 lliDivisor, int iBits, bool bSigned, uint64* pulliMultiplier, int* piShift);
};


#endif // __I_R_OPTIMISER_DIVISIONS_H__

