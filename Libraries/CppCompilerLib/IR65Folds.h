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
#ifndef __I_R65_FOLDS_H__
#define __I_R65_FOLDS_H__
#include "IRBackend.h"


class CIR65Backend;


//The pointers a 65C64 function never needs as values: a pointer plus a constant that is only loaded and stored
//through is folded into the instructions that use it (MarkFolded); FoldedBase is what one is folded to.
class CIR65Folds
{
protected:
	CIR65Backend*	mpcBackend;

public:
	void	Init(CIR65Backend* pcBackend);
	void	Kill(void);

	void				MarkFolded(void);
	bool				FoldedBase(SIRInstruction* psNext, SIRReference sValue);
};


#endif // __I_R65_FOLDS_H__
