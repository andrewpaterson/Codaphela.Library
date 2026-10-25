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
#ifndef __I_R_DOMINATORS_H__
#define __I_R_DOMINATORS_H__
#include "IRFunction.h"


typedef CArrayTemplate<int32>	CArrayIRBlockNumbers;


//A block on the depth first search stack and the next of its successors (or dominator tree children) to visit.
struct SIRSearch
{
	int32	iBlock;
	int32	iNext;
};


typedef CArrayTemplate<SIRSearch>	CArrayIRSearches;


//The dominator tree of a function's reachable blocks (Cooper, Harvey and Kennedy, "A Simple, Fast Dominance
//Algorithm").  The function's blocks must be numbered (CIRFunction::Renumber) and have their predecessors.  The
//tree is numbered in pre and post order so Dominates is constant time.
class CIRDominators
{
protected:
	CIRFunction*			mpcFunction;
	bool					mbLastFirst;		//The search visits a block's last successor (a brif's else) first.
	CArrayIRBlockNumbers	maiImmediate;		//Per block index: its immediate dominator's index, -1 if unreachable.
	CArrayIRBlockNumbers	maiPostOrder;		//Per block index: its position in the CFG's post order, -1 if unreachable.
	CArrayIRBlocks			mapcReversePostOrder;
	CArrayIRBlockNumbers	maiTreePre;			//Per block index: the dominator tree's pre and post order numbers.
	CArrayIRBlockNumbers	maiTreePost;
	CArrayIRBlockNumbers	maiChildStart;		//Per block index + 1: where its children start in maiChildren.
	CArrayIRBlockNumbers	maiChildren;
	CArrayIRSearches		masStack;

public:
	void		Init(void);
	void		Kill(void);

	void		Compute(CIRFunction* pcFunction, bool bLastFirst = false);
	bool		IsReachable(CIRBlock* pcBlock);
	CIRBlock*	GetImmediate(CIRBlock* pcBlock);
	bool		Dominates(CIRBlock* pcDominator, CIRBlock* pcBlock);
	bool		IsBackEdge(CIRBlock* pcFrom, CIRBlock* pcHead);
	size		NumReachable(void);
	CIRBlock*	GetReversePostOrder(size uiIndex);

protected:
	void		Reset(CArrayIRBlockNumbers* paiNumbers, size uiNumBlocks);
	void		ComputePostOrder(void);
	void		ComputeImmediate(void);
	void		NumberTree(void);
	int32		Intersect(int32 iBlock1, int32 iBlock2);
};


#endif // __I_R_DOMINATORS_H__
