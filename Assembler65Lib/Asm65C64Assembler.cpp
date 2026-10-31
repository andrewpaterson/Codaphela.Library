/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela Assembler65Lib

Codaphela Assembler65Lib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela Assembler65Lib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela Assembler65Lib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "Asm65C64Assembler.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAsm65C64Assembler::Init(void)
{
	CAsm65BaseAssembler::Init();
	mbMemoryWidth = false;
}


//////////////////////////////////////////////////////////////////////////
//
// 65C64 instructions are whole words.
//
//////////////////////////////////////////////////////////////////////////
uint32 CAsm65C64Assembler::InstructionAlignment(void)
{
	return 2;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAsm65C64Assembler::Emit(SAsm65C64Instruction* psInstruction)
{
	uint8							i;
	SAsm65C64InstructionRelocation*	psRelocation;

	RecordLength(psInstruction->uiWords);

	for (i = 0; i < psInstruction->uiRelocations; i++)
	{
		psRelocation = &psInstruction->asRelocations[i];
		RecordRelocation(&psRelocation->sValue, muiAddress + 2 * psRelocation->uiWord, psRelocation->uiBytes, psRelocation->bSigned, psRelocation->bRelative, 2 * (psInstruction->uiWords - psRelocation->uiWord), psRelocation->psToken);
	}

	for (i = 0; i < psInstruction->uiWords; i++)
	{
		WriteByte((uint8)psInstruction->auiWords[i]);
		WriteByte((uint8)(psInstruction->auiWords[i] >> 8));
	}
}

