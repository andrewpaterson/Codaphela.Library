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
#ifndef __I_R_W65_C816_FUSION_H__
#define __I_R_W65_C816_FUSION_H__
#include "IRBackend.h"


class CIRW65C816Backend;


//Which instructions the W65C816 back end doesn't write where they are: one whose result has a single use, by
//the next instruction written, is fused into it, so its words go through A to where they are wanted and never
//to a place of their own (CanFuse, CanFuseReturn).  Fused says whether an instruction was.
class CIRW65C816Fusion
{
protected:
	CIRW65C816Backend*	mpcBackend;

public:
	void	Init(CIRW65C816Backend* pcBackend);
	void	Kill(void);

	bool			CanFuse(CIRBlock* pcBlock, size uiIndex);
	bool			CanFuseReturn(CIRBlock* pcBlock, SIRInstruction* psInstruction);
	bool			FusedOverlaps(SIRReference sResult, SIRReference sInput, bool bAnyOverlap);
	bool			FusedInputsKept(SIRInstruction* psInstruction, SIRReference sResult);
	SIRInstruction*	Fused(SIRReference sValue);
};


#endif // __I_R_W65_C816_FUSION_H__
