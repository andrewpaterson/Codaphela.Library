/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CppParserLib

Codaphela CppParserLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CppParserLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CppParserLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __I_R_BACKEND_PASSES_H__
#define __I_R_BACKEND_PASSES_H__
#include "BaseLib/ArrayTemplate.h"
#include "IRFunction.h"
#include "IRDominators.h"
#include "IRLoops.h"


//Rewrites of a function's IR that a back end may make before writing it, for any target: the blocks in the order they
//are written (OrderBlocks), loops' tests moved after their bodies (MoveLoopTests: block layout only, unlike
//CIROptimiser's RotateLoops, which copies a loop's head), loads moved down to their only use (SinkLoads) and loop
//steps moved past their phi's last reads (SinkSteps).  The 65C64 back end (CIR65Backend) uses them all, the W65C816's
//(CIR816Backend) OrderBlocks (with each loop's blocks together) and SinkSteps.
class CIRBackendPasses
{
protected:
	CIRFunction*			mpcFunction;
	CIRDominators			mcDominators;
	CIRLoops				mcLoops;
	CArrayIRSearches		masStack;
	CArrayIRBlockNumbers	maiVisited;			//Per block index: 1 once the search reached it.
	CArrayTemplate<uint32>	mauiUses;			//Per temporary: how many times it is used.

public:
	void	Init(void);
	void	Kill(void);

	void	OrderBlocks(CIRFunction* pcFunction, bool bLoopsTogether);
	void	MoveLoopTests(CIRFunction* pcFunction);
	void	SinkLoads(CIRFunction* pcFunction);
	void	SinkSteps(CIRFunction* pcFunction);

protected:
	int32	SharedDepth(CIRBlock* pcBlock, CIRBlock* pcSuccessor);
	void	SinkStep(CIRBlock* pcBlock, SIRReference sStep, SIRReference sPhi);
};


#endif // __I_R_BACKEND_PASSES_H__
