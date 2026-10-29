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
#ifndef __I_R_LIVENESS_H__
#define __I_R_LIVENESS_H__
#include "IRFunction.h"
#include "IRLoops.h"


//Positions [iFrom, iTo) a temporary is live over.
struct SIRRange
{
	int32	iFrom;
	int32	iTo;
};


typedef CArrayTemplate<SIRRange>	CArrayIRRanges;


//Where each temporary is live, for register allocation (IR.txt Back ends).  The blocks are numbered in the
//function's order: a block starts at Start(block), its phis are defined there, its instructions are at Position
//(block, index) (they read their operands there and write their result one after), its jump is at Jump(block)
//and the next block starts two after that.  Parameters are defined at 0.  A temporary's ranges have holes where
//it isn't live (between a loop's blocks, say), so a phi can share a place with the values it takes.  The phi
//arguments a block passes are live to its end; a phi is live from its block's start.
class CIRLiveness
{
protected:
	CIRFunction*			mpcFunction;
	size					muiWords;			//Words in a block's live in (or out) set.
	CArrayTemplate<uint32>	mauiLiveIn;			//Per block index: muiWords words of bits, one per temporary.
	CArrayTemplate<uint32>	mauiLiveOut;
	CArrayIRBlockNumbers	maiStarts;			//Per block index.
	CIRLoops				mcLoops;
	CArrayIRRanges			masRanges;			//Every temporary's ranges, in order, from maiFirstRanges.
	CArrayIRBlockNumbers	maiFirstRanges;		//Per temporary + 1.
	CArrayTemplate<uint32>	mauiWeights;		//Per temporary: its definition and uses, 8 times more per loop.

public:
	void			Init(void);
	void			Kill(void);

	void			Compute(CIRFunction* pcFunction, CIRDominators* pcDominators);

	int32			Start(CIRBlock* pcBlock);
	int32			Position(CIRBlock* pcBlock, size uiInstruction);
	int32			Jump(CIRBlock* pcBlock);
	int32			LoopDepth(CIRBlock* pcBlock);

	size			NumRanges(SIRReference sTemporary);
	SIRRange*		GetRanges(SIRReference sTemporary);
	uint32			GetWeight(SIRReference sTemporary);
	static uint32	LoopWeight(int32 iDepth);

	static bool		Intersect(SIRRange* psRanges1, size uiNum1, SIRRange* psRanges2, size uiNum2);

protected:
	void			Number(void);
	uint32*			LiveIn(size uiBlock);
	uint32*			LiveOut(size uiBlock);
	void			ComputeSets(void);
	void			AddWeight(SIRReference sValue, int32 iDepth);
	void			BuildRanges(void);
};


#endif // __I_R_LIVENESS_H__
