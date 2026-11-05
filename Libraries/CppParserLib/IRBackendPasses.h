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


//Rewrites of a function's IR that a back end may make before writing it, for any target: the blocks in the order they
//are written (OrderBlocks), loops' tests moved after their bodies (MoveLoopTests: block layout only, unlike
//CIROptimiser's RotateLoops, which copies a loop's head) and loads moved down to their only use (SinkLoads).  The
//65C64 back end (CIR65Backend) uses them.
class CIRBackendPasses
{
protected:
	CIRFunction*			mpcFunction;
	CIRDominators			mcDominators;
	CArrayTemplate<uint32>	mauiUses;			//Per temporary: how many times it is used.

public:
	void	Init(void);
	void	Kill(void);

	void	OrderBlocks(CIRFunction* pcFunction);
	void	MoveLoopTests(CIRFunction* pcFunction);
	void	SinkLoads(CIRFunction* pcFunction);
};


#endif // __I_R_BACKEND_PASSES_H__
