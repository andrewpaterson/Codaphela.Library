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
#include <stdlib.h>
#include <string.h>
#include "Breakpoints.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBreakpoints::Init(void)
{
	mapsBanks = NULL;
	muiNumBreakpoints = 0;
	muiNumEnabled = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBreakpoints::Kill(void)
{
	Clear();
}


//////////////////////////////////////////////////////////////////////////
//
// The bank uiAddress is in, NULL if it has never had a breakpoint.
//
//////////////////////////////////////////////////////////////////////////
SBreakpointBank* CBreakpoints::GetBank(uint32 uiAddress)
{
	if (mapsBanks == NULL)
	{
		return NULL;
	}
	return mapsBanks[uiAddress >> 16];
}


//////////////////////////////////////////////////////////////////////////
//
// A breakpoint at uiAddress, enabled or not.  Returns false if there
// already was one (it is left as it was).
//
//////////////////////////////////////////////////////////////////////////
bool CBreakpoints::Add(uint32 uiAddress, bool bEnabled)
{
	SBreakpointBank*	psBank;
	uint32				uiOffset;
	uint8				uiBit;

	if (mapsBanks == NULL)
	{
		mapsBanks = (SBreakpointBank**)malloc(BREAKPOINT_BANKS * sizeof(SBreakpointBank*));
		memset(mapsBanks, 0, BREAKPOINT_BANKS * sizeof(SBreakpointBank*));
	}

	psBank = mapsBanks[uiAddress >> 16];
	if (psBank == NULL)
	{
		psBank = (SBreakpointBank*)malloc(sizeof(SBreakpointBank));
		memset(psBank, 0, sizeof(SBreakpointBank));
		mapsBanks[uiAddress >> 16] = psBank;
	}

	uiOffset = uiAddress & 0xFFFF;
	uiBit = (uint8)(1 << (uiOffset & 7));
	if (psBank->auiPresent[uiOffset >> 3] & uiBit)
	{
		return false;
	}

	psBank->auiPresent[uiOffset >> 3] |= uiBit;
	muiNumBreakpoints++;
	if (bEnabled)
	{
		psBank->auiEnabled[uiOffset >> 3] |= uiBit;
		muiNumEnabled++;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Returns false if there was no breakpoint at uiAddress.
//
//////////////////////////////////////////////////////////////////////////
bool CBreakpoints::Remove(uint32 uiAddress)
{
	SBreakpointBank*	psBank;
	uint32				uiOffset;
	uint8				uiBit;

	psBank = GetBank(uiAddress);
	if (psBank == NULL)
	{
		return false;
	}

	uiOffset = uiAddress & 0xFFFF;
	uiBit = (uint8)(1 << (uiOffset & 7));
	if (!(psBank->auiPresent[uiOffset >> 3] & uiBit))
	{
		return false;
	}

	psBank->auiPresent[uiOffset >> 3] &= (uint8)~uiBit;
	muiNumBreakpoints--;
	if (psBank->auiEnabled[uiOffset >> 3] & uiBit)
	{
		psBank->auiEnabled[uiOffset >> 3] &= (uint8)~uiBit;
		muiNumEnabled--;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Enables or disables the breakpoint at uiAddress, which is kept either
// way.  Returns false if there is no breakpoint there.
//
//////////////////////////////////////////////////////////////////////////
bool CBreakpoints::Enable(uint32 uiAddress, bool bEnabled)
{
	SBreakpointBank*	psBank;
	uint32				uiOffset;
	uint8				uiBit;
	bool				bWasEnabled;

	psBank = GetBank(uiAddress);
	if (psBank == NULL)
	{
		return false;
	}

	uiOffset = uiAddress & 0xFFFF;
	uiBit = (uint8)(1 << (uiOffset & 7));
	if (!(psBank->auiPresent[uiOffset >> 3] & uiBit))
	{
		return false;
	}

	bWasEnabled = (psBank->auiEnabled[uiOffset >> 3] & uiBit) != 0;
	if (bEnabled && !bWasEnabled)
	{
		psBank->auiEnabled[uiOffset >> 3] |= uiBit;
		muiNumEnabled++;
	}
	else if (!bEnabled && bWasEnabled)
	{
		psBank->auiEnabled[uiOffset >> 3] &= (uint8)~uiBit;
		muiNumEnabled--;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Enables or disables every breakpoint.
//
//////////////////////////////////////////////////////////////////////////
void CBreakpoints::EnableAll(bool bEnabled)
{
	size				uiBank;
	SBreakpointBank*	psBank;

	if (mapsBanks == NULL)
	{
		return;
	}

	for (uiBank = 0; uiBank < BREAKPOINT_BANKS; uiBank++)
	{
		psBank = mapsBanks[uiBank];
		if (psBank != NULL)
		{
			if (bEnabled)
			{
				memcpy(psBank->auiEnabled, psBank->auiPresent, BREAKPOINT_BANK_BYTES);
			}
			else
			{
				memset(psBank->auiEnabled, 0, BREAKPOINT_BANK_BYTES);
			}
		}
	}
	muiNumEnabled = bEnabled ? muiNumBreakpoints : 0;
}


//////////////////////////////////////////////////////////////////////////
//
// Removes every breakpoint.
//
//////////////////////////////////////////////////////////////////////////
void CBreakpoints::Clear(void)
{
	size	uiBank;

	if (mapsBanks != NULL)
	{
		for (uiBank = 0; uiBank < BREAKPOINT_BANKS; uiBank++)
		{
			if (mapsBanks[uiBank] != NULL)
			{
				free(mapsBanks[uiBank]);
			}
		}
		free(mapsBanks);
		mapsBanks = NULL;
	}
	muiNumBreakpoints = 0;
	muiNumEnabled = 0;
}


//////////////////////////////////////////////////////////////////////////
//
// A breakpoint at uiAddress, enabled or disabled.
//
//////////////////////////////////////////////////////////////////////////
bool CBreakpoints::Contains(uint32 uiAddress)
{
	SBreakpointBank*	psBank;
	uint32				uiOffset;

	psBank = GetBank(uiAddress);
	if (psBank == NULL)
	{
		return false;
	}
	uiOffset = uiAddress & 0xFFFF;
	return (psBank->auiPresent[uiOffset >> 3] & (1 << (uiOffset & 7))) != 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CBreakpoints::IsEnabled(uint32 uiAddress)
{
	SBreakpointBank*	psBank;
	uint32				uiOffset;

	psBank = GetBank(uiAddress);
	if (psBank == NULL)
	{
		return false;
	}
	uiOffset = uiAddress & 0xFFFF;
	return (psBank->auiEnabled[uiOffset >> 3] & (1 << (uiOffset & 7))) != 0;
}


//////////////////////////////////////////////////////////////////////////
//
// Enabled and disabled.
//
//////////////////////////////////////////////////////////////////////////
size CBreakpoints::NumBreakpoints(void)
{
	return muiNumBreakpoints;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CBreakpoints::NumEnabled(void)
{
	return muiNumEnabled;
}
