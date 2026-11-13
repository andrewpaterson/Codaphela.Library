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
#ifndef __I_R_OPTIMISER_FOLDING_H__
#define __I_R_OPTIMISER_FOLDING_H__
#include "IRPasses.h"


//Constant folding, identities and common subexpressions; and folding one instruction, for the block passes.
class CIRFoldingPasses : public CIRPasses
{
public:
	bool			FoldConstants(void);
	bool			RemoveCommonSubexpressions(void);
	bool			Fold(SIRInstruction* psInstruction, uint64* pulliResult);
	bool			Extended(SIRReference sValue, int iBits, bool bSigned, int iDepth);

protected:
	bool			UndoesExtension(SIRInstruction* psInstruction, SIRReference* psValue);
	bool			ConvertPhi(SIRInstruction* psInstruction, SIRReference* psValue);
	bool			IsIdentity(SIRInstruction* psInstruction, SIRReference* psValue);
	bool			Cancels(SIRInstruction* psInstruction, SIRReference* psValue);
	bool			PointerOffset(SIRInstruction* psInstruction);
	bool			PointerOffsets(SIRInstruction* psInstruction);
	bool			IntegerOffsets(SIRInstruction* psInstruction);
	int				EntryRelations(CIRBlock* pcPredecessor, CIRBlock* pcBlock, SIRInstruction* psInstruction);
	bool			ImpliedComparison(CIRBlock* pcBlock, SIRInstruction* psInstruction, uint64* pulliResult, bool* pbChanged);
};


#endif // __I_R_OPTIMISER_FOLDING_H__

