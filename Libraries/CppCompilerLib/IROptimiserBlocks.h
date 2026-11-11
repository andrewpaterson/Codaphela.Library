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
#ifndef __I_R_OPTIMISER_BLOCKS_H__
#define __I_R_OPTIMISER_BLOCKS_H__
#include "IRPasses.h"


//The passes over the blocks and their jumps (SimplifyCFG and its parts), trivial phis, dead code and the
//comparisons sunk to their branches; and the edits of jumps and phi arguments the loop passes use too.
class CIRBlockPasses : public CIRPasses
{
protected:
	CArrayTemplate<uint8>	mabReached;			//RemoveUnreachable: per block, whether the entry reaches it...
	CArrayTemplate<int32>	maiReachSearch;		//...and the blocks found and not yet searched from (each at most once).

public:
	void			Init(CIRPassContext* pcContext) override;
	void			Kill(void) override;

	bool			SimplifyCFG(void);
	bool			RemoveTrivialPhis(void);
	bool			RemoveDeadCode(void);
	bool			SinkComparisons(void);
	void			RemovePhiArguments(CIRBlock* pcBlock, CIRBlock* pcPredecessor);
	void			RenamePhiArguments(CIRBlock* pcBlock, CIRBlock* pcOld, CIRBlock* pcNew);
	void			RedirectJump(CIRBlock* pcBlock, CIRBlock* pcOld, CIRBlock* pcNew);
	bool			RemoveUnreachable(void);
	bool			OnlyUsedInBlock(CIRBlock* pcBlock, bool bSuccessorPhis = false);
	bool			OnlyUsedIn(CArrayIRBlocks* papcBlocks, bool bSuccessorPhis);

protected:
	void			Bypass(CIRBlock* pcPredecessor, CIRBlock* pcBlock, CIRBlock* pcTarget);
	bool			FoldBranches(void);
	bool			SkipEmptyBlocks(void);
	int				KnownCondition(CIRBlock* pcPredecessor, CIRBlock* pcBlock, SIRReference sCondition);
	bool			ThreadBranches(void);
	void			CountSuccessorPhiUses(CIRBlock* pcBlock, CIRBlock* pcSuccessor, CArrayTemplate<uint32>* pauiInside);
	CIRBlock*		KnownTarget(CIRBlock* pcPredecessor, CIRBlock* pcBlock);
	bool			ThreadPhis(void);
	bool			DuplicateReturns(void);
	bool			JoinBlocks(void);
	SIRReference	SamePhi(CIRBlock* pcBlock, size uiPhi);
};


#endif // __I_R_OPTIMISER_BLOCKS_H__

