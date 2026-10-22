/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela MicroprocessorLib

Codaphela StandardLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela StandardLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela StandardLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __65_READ_OPCODE_H__
#define __65_READ_OPCODE_H__
#include "65DataOperation.h"


//Reads the opcode and starts the instruction.  The instruction's first cycle is this cycle.
class C65ReadOpcode : public C65DataOperation
{
CONSTRUCTABLE(C65ReadOpcode);
public:
	void	Init(void);

	void	Execute(C65C64* pcCPU) override;
	bool	IsInitialSide(void) override;
	void	Print(CChars* psz) override;
};


#endif // __65_READ_OPCODE_H__

