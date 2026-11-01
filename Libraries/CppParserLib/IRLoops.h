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
#ifndef __I_R_LOOPS_H__
#define __I_R_LOOPS_H__
#include "IRDominators.h"
#include "IRDefinitions.h"


//A natural loop: its head and the blocks that reach a block jumping back to the head (a latch) without going
//through the head.  Back edges to one head make one loop.
struct SIRLoop
{
	CIRBlock*	pcHead;
	CIRBlock*	pcEntering;			//The one block outside that jumps to the head, NULL when there are several.
	CIRBlock*	pcPreheader;		//pcEntering when the head is its only successor, else NULL.
	int32		iParent;			//The loop it is directly in, -1 for none.
	int32		iDepth;				//1 for a loop in no other loop.
	int32		iFirstBlock;		//Its blocks (the head first) in CIRLoops::mapcBlocks.
	int32		iNumBlocks;
	int32		iFirstLatch;		//Its latches in CIRLoops::mapcLatches.
	int32		iNumLatches;
};


typedef CArrayTemplate<SIRLoop>	CArrayIRLoops;


//An edge out of a loop: from a block in it to a block that isn't.
struct SIRLoopExit
{
	CIRBlock*	pcFrom;
	CIRBlock*	pcTo;
};


typedef CArrayTemplate<SIRLoopExit>	CArrayIRLoopExits;


//The natural loops of a function's reachable blocks, numbered so a loop comes before the loops inside it.  The
//function must be numbered, have its predecessors and its dominators (CIRDominators::Compute).
class CIRLoops
{
protected:
	CIRFunction*			mpcFunction;
	CIRDominators*			mpcDominators;
	CArrayIRLoops			masLoops;
	CArrayIRBlocks			mapcBlocks;
	CArrayIRBlocks			mapcLatches;
	CArrayIRBlockNumbers	maiInnermost;		//Per block index: the innermost loop it is in, -1 for none.
	CArrayIRBlocks			mapcStack;

public:
	void		Init(void);
	void		Kill(void);

	void		Compute(CIRFunction* pcFunction, CIRDominators* pcDominators);

	size		NumLoops(void);
	SIRLoop*	GetLoop(size uiLoop);
	int32		Innermost(CIRBlock* pcBlock);
	int32		Depth(CIRBlock* pcBlock);
	bool		Contains(size uiLoop, CIRBlock* pcBlock);
	bool		IsHead(CIRBlock* pcBlock);

	size		NumBlocks(size uiLoop);
	CIRBlock*	GetBlock(size uiLoop, size uiIndex);
	size		NumLatches(size uiLoop);
	CIRBlock*	GetLatch(size uiLoop, size uiIndex);
	void		GetExits(size uiLoop, CArrayIRLoopExits* pasExits);

	bool		IsInvariant(size uiLoop, SIRReference sValue, CIRDefinitions* pcDefinitions);

	void		SetPreheader(size uiLoop, CIRBlock* pcPreheader);

protected:
	void		AddLoop(CIRBlock* pcHead);
};


#endif // __I_R_LOOPS_H__
