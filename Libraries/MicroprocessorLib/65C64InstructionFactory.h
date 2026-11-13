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
#ifndef __65C64_INSTRUCTION_FACTORY_H__
#define __65C64_INSTRUCTION_FACTORY_H__
#include "65C64Instruction.h"
#include "65C64InstructionCycleFactory.h"


#define NUM_65C64_OPCODES		0x10000
#define FORM_65C64_HASH_SIZE	4096


//Every 16 bit opcode maps to an instruction.  Opcodes whose bus cycles are the same share it.  Init and Kill are
//counted: the instructions are made by the first Init and freed by the last Kill, so each C65C64Chip can Init the
//factory for itself.
class C65C64InstructionFactory
{
protected:
	size				muiUsers;
	C65C64Instruction*		mapcInstructions[NUM_65C64_OPCODES];
	uint32				mauiFormKeys[FORM_65C64_HASH_SIZE];
	C65C64Instruction*		mapcForms[FORM_65C64_HASH_SIZE];
	size				muiNumForms;
	C65C64Instruction*		mpcReset;
	C65C64Instruction*		mpcFetchNext;
	C65C64Instruction*		mpcInterrupt;
	C65C64Instruction*		mpcWait;

public:
	static	C65C64InstructionFactory*	GetInstance(void);

			void				Init(void);
			void				Kill(void);

			C65C64Instruction*		GetInstruction(uint16 uiOpcode);
			C65C64Instruction*		GetReset(void);
			C65C64Instruction*		GetFetchNext(void);
			C65C64Instruction*		GetInterrupt(void);
			C65C64Instruction*		GetWait(void);
			size				NumForms(void);

protected:
			void				GetForm(S65C64Decoded* psDecoded, S65C64Form* psForm);
			uint32				GetFormKey(S65C64Form* psForm);
			C65C64Instruction*		GetOrCreateInstruction(S65C64Form* psForm);
			C65C64Instruction*		CreateInstruction(uint32 uiKey, C65C64InstructionCycles* pcCycles, E65Instruction eInstruction);
};


char*	Get65C64InstructionName(E65Instruction eInstruction);


#endif // __65C64_INSTRUCTION_FACTORY_H__

