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
#include "BaseLib/Chars.h"
#include "MicroprocessorLib/65C64Meta.h"
#include "MicroprocessorLib/Fast65C64.h"
#include "Asm65Output.h"
#include "Asm65HostFiles.h"


#define ASM65_MEMORY_LOW_SIZE		0x200000
#define ASM65_MEMORY_HIGH_BASE		0xFFFFF000
#define ASM65_MEMORY_HIGH_SIZE		0x1000

//The devices (the drive, the console, the host files and the memory size register) are in the page 00FF:0000 to
//00FF:FFFF, which is never memory even when the memory is bigger.
#define ASM65_DEVICE_BASE			0x00FF0000
#define ASM65_DEVICE_END			0x01000000

//A read only register at 00FF:0300: the 32 bit size of the memory at 0000:0000 (Init's uiLowSize).  Libc65's
//malloc puts its heap at 0020:0000 when there is more than 2MB, and at 0100:0000 (past the devices' page) when there
//is more than 16MB.
#define ASM65_MEMORY_SIZE_REGISTER	0x00FF0300

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

//A console at 00FF:0100: each byte written to it is appended to the CChars given to SetConsole.
#define ASM65_CONSOLE				0x00FF0100

#define ASM65_DRIVE_READ			1			//Copies the sectors to memory.
#define ASM65_DRIVE_WRITE			2			//Copies memory to the sectors.


//Flat memory for a 65C64 running assembled code: 2MB (or Init's uiLowSize) at 0000:0000 and the 4KB vector page at
//FFFF:F000.  An access anywhere else is counted as bad and reads 0.  TickHigh and TickLow are the C65C64Meta
//callbacks, with the memory as their context.  InitFast
//connects a CFast65C64 instead: it reads and writes the low memory directly and everything else through ReadWord and
//WriteByte.  A CFileDrive (such as a CMemoryDrive) can be attached as a storage device (see ASM65_DRIVE_BASE), a
//CChars as the console (ASM65_CONSOLE) and a CAsm65HostFiles as the host file device (ASM65_HOST_BASE).
class CAsm65Memory
{
protected:
	uint8*		mpuiLow;
	uint32		muiLowSize;
	uint8		mauiHigh[ASM65_MEMORY_HIGH_SIZE];
	uint8		mauiMemorySize[4];
	size		muiBadAccesses;
	size		muiCycles;
	CFileDrive*	mpcDrive;
	uint8		mauiDrive[ASM65_DRIVE_SIZE];
	size		muiDriveReads;
	CChars*		mpszConsole;
	CAsm65HostFiles*	mpcHostFiles;
	uint8		mauiHost[ASM65_HOST_SIZE];

public:
	void			Init(uint32 uiLowSize = ASM65_MEMORY_LOW_SIZE);
	void			Kill(void);
	void			CopyFrom(CAsm65Memory* pcSource);
	uint32			GetLowSize(void);

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
	void			SetConsole(CChars* pszConsole);
	void			SetHostFiles(CAsm65HostFiles* pcHostFiles);
	size			NumDriveReads(void);

	static void		TickHigh(C65C64Meta* pcMPU, void* pvMemory);
	static void		TickLow(C65C64Meta* pcMPU, void* pvMemory);

	void			InitFast(CFast65C64* pcMPU);
	static uint16	ReadWord(void* pvMemory, uint32 uiAddress);
	static void		WriteByte(void* pvMemory, uint32 uiAddress, uint8 uiValue);

protected:
	uint32			GetDriveRegister(uint32 uiRegister);
	void			SetDriveRegister(uint32 uiRegister, uint32 uiValue);
	void			DriveCommand(uint8 uiCommand);
	void			HostCommand(uint8 uiCommand);
};


#endif // __ASM65_MEMORY_H__

