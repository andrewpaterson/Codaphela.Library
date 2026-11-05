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
	size		NumBreakpoints(void);
	size		NumEnabled(void);

	FORCE_INLINE bool	AnyEnabled(void);
	FORCE_INLINE bool	IsEnabledNext(uint32 uiAddress);

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


#endif // __BREAKPOINTS_H__
