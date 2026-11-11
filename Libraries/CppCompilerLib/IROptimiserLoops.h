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
#ifndef __I_R_OPTIMISER_LOOPS_H__
#define __I_R_OPTIMISER_LOOPS_H__
#include "IRPasses.h"


//The loop passes: rotation, invariants hoisted, loads sunk, exit values, dead loops, unswitching and fewer
//tests; and a loop's preheader, for the counter and idiom passes.
class CIRLoopPasses : public CIRPasses
{
public:
	bool			LessLoopTests(void);
	bool			ReplaceExitValues(void);
	bool			DeleteDeadLoops(void);
	bool			SinkLoads(void);
	bool			HoistCommon(void);
	bool			HoistInvariants(void);
	bool			RotateLoops(void);
	bool			UnswitchLoops(void);
	CIRBlock*		AddPreheader(size uiLoop);

protected:
	bool			SingleExit(size uiLoop, CIRBlock** ppcFrom, CIRBlock** ppcTo);
	bool			UsedOutside(size uiLoop, SIRReference sValue);
	void			ReplaceOutside(size uiLoop, SIRReference sValue, SIRReference sNew);
	bool			SinkLoad(CIRBlock* pcFrom, size uiLoad, CIRBlock* pcTo, size uiUse);
	bool			WritesBetween(CIRBlock* pcBlock, size uiStart, size uiEnd, SIRReference sRoot, int64 lliOffset, int iBytes);
	void			RewriteOutsideUses(SIRReference sValue, CArrayIRBlocks* papcBlocks, CArrayIRBlocks* papcCopies, CIRSSAUpdater* pcUpdater);
	bool			CanStep(SIRReference sValue, int iStep);
	CIRBlock*		AddMergedPreheader(size uiLoop);
	bool			CanRotate(size uiLoop, CIRBlock** ppcBody, CIRBlock** ppcExit);
	void			CopyHead(CIRBlock* pcHead, CIRBlock* pcBlock, CArrayIRReferences* pasMap);
	void			SplitPhiArguments(CIRBlock* pcBlock, CIRBlock* pcHead, CIRBlock* pcPreheader, CArrayIRReferences* pasPreheader, CIRBlock* pcLatch, CArrayIRReferences* pasLatch);
	void			RewriteUses(SIRReference sValue, CIRSSAUpdater* pcUpdater);
};


#endif // __I_R_OPTIMISER_LOOPS_H__

