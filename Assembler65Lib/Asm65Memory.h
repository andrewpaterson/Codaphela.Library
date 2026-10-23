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
#ifndef __ASM65_MEMORY_H__
#define __ASM65_MEMORY_H__
#include "BaseLib/PrimitiveTypes.h"
#include "MicroprocessorLib/65C64Meta.h"
#include "Asm65Output.h"


#define ASM65_MEMORY_LOW_SIZE		0x200000
#define ASM65_MEMORY_HIGH_BASE		0xFFFFF000
#define ASM65_MEMORY_HIGH_SIZE		0x1000


//Flat memory for a 65C64 running assembled code: 2MB at 0000:0000 and the 4KB vector page at FFFF:F000.  An
//access anywhere else is counted as bad and reads 0.  It is about 2MB, so allocate it rather than putting it on
//the stack.  TickHigh and TickLow are the C65C64Meta callbacks, with the memory as their context.
class CAsm65Memory
{
protected:
	uint8	mauiLow[ASM65_MEMORY_LOW_SIZE];
	uint8	mauiHigh[ASM65_MEMORY_HIGH_SIZE];
	size	muiBadAccesses;
	size	muiCycles;

public:
	void			Init(void);

	uint8*			Byte(uint32 uiAddress);
	uint8			GetByte(uint32 uiAddress);
	void			SetByte(uint32 uiAddress, uint8 uiValue);
	uint16			GetWord(uint32 uiAddress);
	uint32			GetLong(uint32 uiAddress);
	uint64			GetQuad(uint32 uiAddress);
	void			SetLong(uint32 uiAddress, uint32 uiValue);
	void			Load(CAsm65Output* pcOutput);

	size			NumBadAccesses(void);
	size			NumCycles(void);

	static void		TickHigh(C65C64Meta* pcMPU, void* pvMemory);
	static void		TickLow(C65C64Meta* pcMPU, void* pvMemory);
};


#endif // __ASM65_MEMORY_H__

