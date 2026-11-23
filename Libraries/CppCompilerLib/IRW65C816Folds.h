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
#ifndef __I_R_W65_C816_FOLDS_H__
#define __I_R_W65_C816_FOLDS_H__
#include "IRBackend.h"


class CIRW65C816Backend;


//The pointers a W65C816 function never needs as values: a pointer that is another plus a constant, or plus an
//i16, and is only loaded and stored through, is folded into the instructions that use it, which address with
//the constant or with the index in Y (FoldPointers, FoldIndices).  The back end's record of each temporary says
//what it is folded to; FoldedBase, FoldedIndex and IsFolded read it, and LoadY loads Y for an access.  A kept slot (a
//variable a debugger reads) only loaded and stored through, at constant offsets, is addressed in place ([D + n]) and
//never worked out either (FindDirectSlots, DirectSlot).
class CIRW65C816Folds
{
protected:
	CIRW65C816Backend*	mpcBackend;

public:
	void	Init(CIRW65C816Backend* pcBackend);
	void	Kill(void);

	void			FoldPointers(void);
	SIRReference	FoldedBase(SIRReference sValue);
	SIRReference	FoldedIndex(SIRReference sValue);
	bool			IsFolded(SIRInstruction* psInstruction);
	void			FoldIndices(CArrayTemplate<int32>* paiUses);
	bool			LoadY(SIRReference sPointer, int32 iOffset);
	void			FindDirectSlots(void);
	bool			IsDirectSlot(SIRReference sValue);
	bool			DirectSlot(SIRReference sPointer, int32* piOffset);
	SIRReference	Materialised(SIRReference sValue);
};


#endif // __I_R_W65_C816_FOLDS_H__
