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
#include <stdlib.h>
#include <string.h>
#include "Asm65Memory.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Memory::Init(void)
{
	memset(mauiLow, 0, sizeof(mauiLow));
	memset(mauiHigh, 0, sizeof(mauiHigh));
	muiBadAccesses = 0;
	muiCycles = 0;
	mpcDrive = NULL;
	memset(mauiDrive, 0, sizeof(mauiDrive));
	muiDriveReads = 0;
}


//////////////////////////////////////////////////////////////////////////
//
// The byte at uiAddress, or NULL if there is no memory there.
//
//////////////////////////////////////////////////////////////////////////
uint8* CAsm65Memory::Byte(uint32 uiAddress)
{
	if (uiAddress < ASM65_MEMORY_LOW_SIZE)
	{
		return &mauiLow[uiAddress];
	}
	if (uiAddress >= ASM65_MEMORY_HIGH_BASE)
	{
		return &mauiHigh[uiAddress - ASM65_MEMORY_HIGH_BASE];
	}
	if (mpcDrive && (uiAddress >= ASM65_DRIVE_BASE) && (uiAddress < ASM65_DRIVE_BASE + ASM65_DRIVE_SIZE))
	{
		return &mauiDrive[uiAddress - ASM65_DRIVE_BASE];
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint8 CAsm65Memory::GetByte(uint32 uiAddress)
{
	uint8*	puiByte;

	puiByte = Byte(uiAddress);
	if (puiByte)
	{
		return *puiByte;
	}
	muiBadAccesses++;
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Memory::SetByte(uint32 uiAddress, uint8 uiValue)
{
	uint8*	puiByte;

	puiByte = Byte(uiAddress);
	if (puiByte)
	{
		*puiByte = uiValue;
		if (puiByte == &mauiDrive[ASM65_DRIVE_COMMAND])
		{
			DriveCommand(uiValue);
		}
	}
	else
	{
		muiBadAccesses++;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 CAsm65Memory::GetWord(uint32 uiAddress)
{
	return (uint16)(GetByte(uiAddress) | (GetByte(uiAddress + 1) << 8));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CAsm65Memory::GetLong(uint32 uiAddress)
{
	return (uint32)GetWord(uiAddress) | ((uint32)GetWord(uiAddress + 2) << 16);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint64 CAsm65Memory::GetQuad(uint32 uiAddress)
{
	return (uint64)GetLong(uiAddress) | ((uint64)GetLong(uiAddress + 4) << 32);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Memory::SetLong(uint32 uiAddress, uint32 uiValue)
{
	int	i;

	for (i = 0; i < 4; i++)
	{
		SetByte(uiAddress + i, (uint8)(uiValue >> (8 * i)));
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Writes every segment of an assembled or linked output.
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Memory::Load(CAsm65Output* pcOutput)
{
	size			i;
	size			j;
	SAsm65Segment*	psSegment;

	for (i = 0; i < pcOutput->NumSegments(); i++)
	{
		psSegment = pcOutput->GetSegment(i);
		for (j = 0; j < psSegment->cBytes.NumElements(); j++)
		{
			SetByte(psSegment->uiAddress + (uint32)j, psSegment->cBytes.GetValue(j));
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Loads a program and points the reset vector at uiResetAddress.
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Memory::LoadProgram(CAsm65Output* pcOutput, uint32 uiResetAddress)
{
	SetLong(VECTOR_65C64_BASE + 4 * VECTOR_65C64_RESET, uiResetAddress);
	Load(pcOutput);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CAsm65Memory::NumBadAccesses(void)
{
	return muiBadAccesses;
}


//////////////////////////////////////////////////////////////////////////
//
// The bus cycles TickHigh has answered since Init.
//
//////////////////////////////////////////////////////////////////////////
size CAsm65Memory::NumCycles(void)
{
	return muiCycles;
}


//////////////////////////////////////////////////////////////////////////
//
// Memory responds while PHI2 is high: a read puts the word at the address on the data bus, a write stores the
// bytes the byte enables select.  Cycles with no valid address are ignored.
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Memory::TickHigh(C65C64Meta* pcMPU, void* pvMemory)
{
	CAsm65Memory*	pcMemory;
	uint32			uiAddress;
	uint16			uiData;

	pcMemory = (CAsm65Memory*)pvMemory;
	if (!pcMPU->GetRWB()->IsHigh() && !pcMPU->GetRWB()->IsLow())
	{
		return;
	}
	if (!pcMPU->GetVPA()->IsHigh() && !pcMPU->GetVDA()->IsHigh() && pcMPU->GetVPB()->IsHigh())
	{
		return;
	}

	uiAddress = pcMPU->GetAddress()->Get();
	if (pcMPU->GetRWB()->IsHigh())
	{
		pcMPU->GetData()->Set(pcMemory->GetWord(uiAddress));
	}
	else
	{
		uiData = pcMPU->GetData()->Get();
		if (pcMPU->GetLowerByte()->IsHigh())
		{
			pcMemory->SetByte(uiAddress, (uint8)uiData);
		}
		if (pcMPU->GetUpperByte()->IsHigh())
		{
			pcMemory->SetByte(uiAddress + 1, (uint8)(uiData >> 8));
		}
	}
	pcMemory->muiCycles++;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Memory::TickLow(C65C64Meta* pcMPU, void* pvMemory)
{
}


//////////////////////////////////////////////////////////////////////////
//
// Attaches a storage device at ASM65_DRIVE_BASE (NULL detaches it).  The drive
// belongs to the caller.
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Memory::SetDrive(CFileDrive* pcDrive)
{
	mpcDrive = pcDrive;
	memset(mauiDrive, 0, sizeof(mauiDrive));
	if (mpcDrive)
	{
		SetDriveRegister(ASM65_DRIVE_SECTOR_SIZE, mpcDrive->GetSectorSize());
	}
}


//////////////////////////////////////////////////////////////////////////
//
// How many read commands the program has run.
//
//////////////////////////////////////////////////////////////////////////
size CAsm65Memory::NumDriveReads(void)
{
	return muiDriveReads;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CAsm65Memory::GetDriveRegister(uint32 uiRegister)
{
	return (uint32)mauiDrive[uiRegister] | ((uint32)mauiDrive[uiRegister + 1] << 8) | ((uint32)mauiDrive[uiRegister + 2] << 16) | ((uint32)mauiDrive[uiRegister + 3] << 24);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Memory::SetDriveRegister(uint32 uiRegister, uint32 uiValue)
{
	int	i;

	for (i = 0; i < 4; i++)
	{
		mauiDrive[uiRegister + i] = (uint8)(uiValue >> (8 * i));
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Copies the sectors between the drive and memory.  The status is 1 for an unknown
// command or when the drive fails (sectors past its end).
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Memory::DriveCommand(uint8 uiCommand)
{
	uint32	uiSector;
	uint32	uiAddress;
	uint32	uiCount;
	uint32	uiBytes;
	uint32	i;
	uint8*	puiBuffer;
	bool	bResult;

	uiSector = GetDriveRegister(ASM65_DRIVE_SECTOR);
	uiAddress = GetDriveRegister(ASM65_DRIVE_ADDRESS);
	uiCount = GetDriveRegister(ASM65_DRIVE_COUNT);
	uiBytes = uiCount * mpcDrive->GetSectorSize();

	bResult = (uiCommand == ASM65_DRIVE_READ) || (uiCommand == ASM65_DRIVE_WRITE);
	if (bResult && (uiCount != 0))
	{
		puiBuffer = (uint8*)malloc(uiBytes);
		if (uiCommand == ASM65_DRIVE_READ)
		{
			muiDriveReads++;
			bResult = mpcDrive->Read(uiSector, uiCount, puiBuffer);
			for (i = 0; bResult && (i < uiBytes); i++)
			{
				SetByte(uiAddress + i, puiBuffer[i]);
			}
		}
		else
		{
			for (i = 0; i < uiBytes; i++)
			{
				puiBuffer[i] = GetByte(uiAddress + i);
			}
			bResult = mpcDrive->Write(uiSector, uiCount, puiBuffer);
		}
		free(puiBuffer);
	}
	SetDriveRegister(ASM65_DRIVE_STATUS, bResult ? 0 : 1);
}
