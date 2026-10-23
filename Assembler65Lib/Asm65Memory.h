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
#include "BaseLib/FileDrive.h"
#include "MicroprocessorLib/65C64Meta.h"
#include "Asm65Output.h"


#define ASM65_MEMORY_LOW_SIZE		0x200000
#define ASM65_MEMORY_HIGH_BASE		0xFFFFF000
#define ASM65_MEMORY_HIGH_SIZE		0x1000

//A simulated storage device's registers (SetDrive), at 00FF:0000.  Write the first sector, the address and
//the number of sectors, then a command: the sectors are copied at once.  Each register is 32 bits.
#define ASM65_DRIVE_BASE			0x00FF0000
#define ASM65_DRIVE_SECTOR			0x00		//The first sector.
#define ASM65_DRIVE_ADDRESS			0x04		//Where in memory.
#define ASM65_DRIVE_COUNT			0x08		//How many sectors.
#define ASM65_DRIVE_COMMAND			0x0C		//Writing its low byte runs a command.
#define ASM65_DRIVE_STATUS			0x10		//0 when the last command worked, 1 when it failed.
#define ASM65_DRIVE_SECTOR_SIZE		0x14		//Bytes in a sector (read only).
#define ASM65_DRIVE_SIZE			0x18

#define ASM65_DRIVE_READ			1			//Copies the sectors to memory.
#define ASM65_DRIVE_WRITE			2			//Copies memory to the sectors.


//Flat memory for a 65C64 running assembled code: 2MB at 0000:0000 and the 4KB vector page at FFFF:F000.  An
//access anywhere else is counted as bad and reads 0.  It is about 2MB, so allocate it rather than putting it on
//the stack.  TickHigh and TickLow are the C65C64Meta callbacks, with the memory as their context.  A
//CFileDrive (such as a CMemoryDrive) can be attached as a storage device (see ASM65_DRIVE_BASE).
class CAsm65Memory
{
protected:
	uint8		mauiLow[ASM65_MEMORY_LOW_SIZE];
	uint8		mauiHigh[ASM65_MEMORY_HIGH_SIZE];
	size		muiBadAccesses;
	size		muiCycles;
	CFileDrive*	mpcDrive;
	uint8		mauiDrive[ASM65_DRIVE_SIZE];
	size		muiDriveReads;

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
	void			LoadProgram(CAsm65Output* pcOutput, uint32 uiResetAddress);

	size			NumBadAccesses(void);
	size			NumCycles(void);

	void			SetDrive(CFileDrive* pcDrive);
	size			NumDriveReads(void);

	static void		TickHigh(C65C64Meta* pcMPU, void* pvMemory);
	static void		TickLow(C65C64Meta* pcMPU, void* pvMemory);

protected:
	uint32			GetDriveRegister(uint32 uiRegister);
	void			SetDriveRegister(uint32 uiRegister, uint32 uiValue);
	void			DriveCommand(uint8 uiCommand);
};


#endif // __ASM65_MEMORY_H__

