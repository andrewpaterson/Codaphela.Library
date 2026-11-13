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
#ifndef __I_R_OPTIMISER_COUNTERS_H__
#define __I_R_OPTIMISER_COUNTERS_H__
#include "IRPasses.h"


//The passes over loop counters: counters replaced and narrowed, multiplies and addresses strength reduced; and
//matching a counter and what guards its loop, for the loop passes.
class CIRCounterPasses : public CIRPasses
{
public:
	bool			ReplaceLoopCounters(void);
	bool			ReduceMultiplies(void);
	bool			ReduceDivisions(void);
	bool			MatchCounter(size uiLoop, SIRPhi* psPhi, uint8 uiFlags, SIRLoopCounter* psCounter);
	bool			LoopGuarded(size uiLoop, uint16 uiOpcode, SIRReference sInitial, SIRReference sBound);
	bool			ReduceStrength(void);
	bool			NarrowCounters(void);

protected:
	bool			StepsByOne(SIRReference sStepped, SIRReference sValue);
	SIRReference	ExtendedCounter(SIRReference sValue, SIRReference sCounter, bool bExtended, bool bZeroExtends, uint16* puiExtend);
	bool			CounterUnwrapped(size uiLoop, CIRBlock* pcLatch, SIRReference sCounter, SIRReference sInit, SIRReference sNext, int64 lliStep, uint64 ulliMask);
	bool			UnsignedMaximum(CIRBlock* pcBlock, CIRBlock* pcStay, SIRReference sValue, uint64 ulliMask, uint64* pulliMax, SIRReference* psBound, uint16* puiCompare);
	bool			NarrowCounter(size uiLoop, CIRBlock* pcBlock);
	bool			NarrowLoopGuarded(size uiLoop, uint16 uiOpcode, SIRReference sInitial, SIRReference sBound);
	bool			SteppedExitUses(size uiLoop, CIRBlock* pcBlock, SIRPhi* psCounter, SIRInstruction* psCompare, SIRReference sStepped, CArrayIRExitArguments* papsExits);
	bool			NarrowGuardedAs(size uiLoop, uint16 uiOpcode, SIRReference sA, SIRReference sB);
};


#endif // __I_R_OPTIMISER_COUNTERS_H__

