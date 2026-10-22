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
#ifndef __65_INSTRUCTION_H__
#define __65_INSTRUCTION_H__
#include "BaseLib/PrimitiveTypes.h"
#include "BaseLib/Chars.h"
#include "StandardLib/Unknown.h"
#include "65InstructionCycles.h"


//An instruction is shared by every opcode with the same bus cycles.  The opcode's register
//fields are decoded by the processor when the opcode is read.
class C65Instruction : public CUnknown
{
CONSTRUCTABLE(C65Instruction);
protected:
	uint32					muiForm;
	CChars					mszName;
	CChars					mszDescription;
	C65InstructionCycles*	mpcCycles;

public:
	void					Init(uint32 uiForm, C65InstructionCycles* pcCycles, char* szName, char* szDescription);
	void					Kill(void) override;

	uint32					GetForm(void);
	char*					GetName(void);
	char*					GetDescription(void);
	C65InstructionCycles*	GetCycles(void);
	C65BusCycle*			GetBusCycle(int16 iCycle);
};


#endif // __65_INSTRUCTION_H__

