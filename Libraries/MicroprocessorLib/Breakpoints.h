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
#ifndef __BREAKPOINTS_H__
#define __BREAKPOINTS_H__
#include "BaseLib/Define.h"
#include "BaseLib/PrimitiveTypes.h"


#define BREAKPOINT_BANKS		0x10000			//The 32 bit address space's 64KB banks.
#define BREAKPOINT_BANK_BYTES	(0x10000 / 8)	//One bit for each address in a bank.


//A bank's breakpoints.  Allocated when the bank gets its first breakpoint and kept until Clear.
struct SBreakpointBank
{
	uint8	auiEnabled[BREAKPOINT_BANK_BYTES];
	uint8	auiPresent[BREAKPOINT_BANK_BYTES];	//Enabled or disabled.
};


//Execution breakpoints at 32 bit addresses (a W65C816 uses the low 24).  A breakpoint can be disabled and
//kept, to be enabled again later.  A processor only checks addresses while AnyEnabled, so disabled
//breakpoints cost nothing.  The bank table is allocated with the first breakpoint.
class CBreakpoints
{
protected:
	SBreakpointBank**	mapsBanks;			//BREAKPOINT_BANKS, NULL for a bank that has never had a breakpoint.
	size				muiNumBreakpoints;
	size				muiNumEnabled;

public:
	void		Init(void);
	void		Kill(void);

	bool		Add(uint32 uiAddress, bool bEnabled);
	bool		Remove(uint32 uiAddress);
	bool		Enable(uint32 uiAddress, bool bEnabled);
	void		EnableAll(bool bEnabled);
	void		Clear(void);

	bool		Contains(uint32 uiAddress);
	bool		IsEnabled(uint32 uiAddress);
	size		NumBreakpoints(void) const;
	size		NumEnabled(void) const;

	FORCE_INLINE bool	AnyEnabled(void);
	FORCE_INLINE bool	IsEnabledNext(uint32 uiAddress);
	bool				AnyEnabledInBank(uint32 uiBank);

protected:
	SBreakpointBank*	GetBank(uint32 uiAddress);
};


//////////////////////////////////////////////////////////////////////////
//
// For a processor, before each instruction: whether to check IsEnabledNext.
//
//////////////////////////////////////////////////////////////////////////
FORCE_INLINE bool CBreakpoints::AnyEnabled(void)
{
	return muiNumEnabled != 0;
}


//////////////////////////////////////////////////////////////////////////
//
// For a processor, only while AnyEnabled (so mapsBanks is there): an
// enabled breakpoint at uiAddress.
//
//////////////////////////////////////////////////////////////////////////
FORCE_INLINE bool CBreakpoints::IsEnabledNext(uint32 uiAddress)
{
	SBreakpointBank*	psBank;
	uint32				uiOffset;

	psBank = mapsBanks[uiAddress >> 16];
	uiOffset = uiAddress & 0xFFFF;
	return (psBank != NULL) && ((psBank->auiEnabled[uiOffset >> 3] & (1 << (uiOffset & 7))) != 0);
}


//The first watched access of an instruction (CDataWatches::Access): kept by the processor until its next Step or Run.
struct SDataWatchHit
{
	bool	bHit;
	bool	bWrite;
	uint32	uiAddress;

	void	Clear(void)		{ bHit = false; bWrite = false; uiAddress = 0; }
};


//The addresses a processor stops after reading or writing (a debugger's data breakpoints).  A processor only checks
//accesses that leave its fast paths: while a set has any enabled it sends that kind of access to them
//(WatchesChanged), so watches cost nothing until one is enabled.  Instruction fetches are never watched.
class CDataWatches
{
protected:
	CBreakpoints	mcReads;
	CBreakpoints	mcWrites;

public:
	void				Init(void);
	void				Kill(void);
	void				Clear(void);

	CBreakpoints*		GetReads(void);
	CBreakpoints*		GetWrites(void);
	FORCE_INLINE bool	AnyReads(void);
	FORCE_INLINE bool	AnyWrites(void);
	bool				Any(void);

	FORCE_INLINE void	Access(uint32 uiAddress, size uiBytes, bool bWrite, SDataWatchHit* psHit);
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
FORCE_INLINE bool CDataWatches::AnyReads(void)
{
	return mcReads.AnyEnabled();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
FORCE_INLINE bool CDataWatches::AnyWrites(void)
{
	return mcWrites.AnyEnabled();
}


//////////////////////////////////////////////////////////////////////////
//
// A processor read or wrote uiBytes at uiAddress: the first watched one
// is the instruction's hit (later accesses don't replace it).
//
//////////////////////////////////////////////////////////////////////////
FORCE_INLINE void CDataWatches::Access(uint32 uiAddress, size uiBytes, bool bWrite, SDataWatchHit* psHit)
{
	CBreakpoints*	pcWatches;
	size			i;

	pcWatches = bWrite ? &mcWrites : &mcReads;
	if (psHit->bHit || !pcWatches->AnyEnabled())
	{
		return;
	}
	for (i = 0; i < uiBytes; i++)
	{
		if (pcWatches->IsEnabledNext(uiAddress + (uint32)i))
		{
			psHit->bHit = true;
			psHit->bWrite = bWrite;
			psHit->uiAddress = uiAddress + (uint32)i;
			return;
		}
	}
}


#endif // __BREAKPOINTS_H__
