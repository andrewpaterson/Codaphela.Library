/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela EmulatorDebugLib

Codaphela EmulatorDebugLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela EmulatorDebugLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela EmulatorDebugLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include <string.h>
#include "DebugMemoryCache.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDebugMemoryCache::Init(void)
{
	masPages.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDebugMemoryCache::Kill(void)
{
	masPages.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Forgets every page: the target has run, or memory was written.
//
//////////////////////////////////////////////////////////////////////////
void CDebugMemoryCache::Clear(void)
{
	masPages.ReInit();
}


//////////////////////////////////////////////////////////////////////////
//
// Copies uiSize bytes from uiAddress in uiSpace if they are all here;
// false if not, the pages not here yet wanted.  Bytes the target said
// aren't memory are never here.
//
//////////////////////////////////////////////////////////////////////////
bool CDebugMemoryCache::Read(uint32 uiSpace, uint32 uiAddress, void* pvDest, size uiSize)
{
	SDebugMemoryPage*	psPage;
	uint32				uiPage;
	uint32				uiOffset;
	size				uiLength;
	size				uiDone;
	bool				bAll;

	bAll = true;
	for (uiDone = 0; uiDone < uiSize; uiDone += uiLength)
	{
		uiPage = (uint32)(uiAddress + uiDone) & ~(uint32)(DEBUG_MEMORY_PAGE - 1);
		uiOffset = (uint32)(uiAddress + uiDone) - uiPage;
		uiLength = DEBUG_MEMORY_PAGE - uiOffset;
		uiLength = (uiLength > uiSize - uiDone) ? uiSize - uiDone : uiLength;

		psPage = Find(uiSpace, uiPage);
		if (!psPage)
		{
			psPage = masPages.Add();
			psPage->uiSpace = uiSpace;
			psPage->uiAddress = uiPage;
			psPage->uiState = DMP_Wanted;
			psPage->uiValid = 0;
		}
		if ((psPage->uiState != DMP_Read) || (uiOffset + uiLength > psPage->uiValid))
		{
			bAll = false;
			continue;
		}
		memcpy(&((uint8*)pvDest)[uiDone], &psPage->auiBytes[uiOffset], uiLength);
	}
	return bAll;
}


//////////////////////////////////////////////////////////////////////////
//
// Up to uiMax of the pages wanted, as ranges to ask for (now asked).
// Returns how many.
//
//////////////////////////////////////////////////////////////////////////
size CDebugMemoryCache::TakeWanted(SDebugMemoryRange* asRanges, size uiMax)
{
	SDebugMemoryPage*	psPage;
	size				ui;
	size				uiCount;

	uiCount = 0;
	for (ui = 0; (ui < masPages.NumElements()) && (uiCount < uiMax); ui++)
	{
		psPage = masPages.Get(ui);
		if (psPage->uiState == DMP_Wanted)
		{
			psPage->uiState = DMP_Asked;
			asRanges[uiCount].uiAddress = psPage->uiAddress;
			asRanges[uiCount].uiSize = DEBUG_MEMORY_PAGE;
			asRanges[uiCount].uiSpace = psPage->uiSpace;
			asRanges[uiCount].uiPadding = 0;
			uiCount++;
		}
	}
	return uiCount;
}


//////////////////////////////////////////////////////////////////////////
//
// A range of a DM_Memory reply, puiBytes its bytes: the page asked for.
//
//////////////////////////////////////////////////////////////////////////
void CDebugMemoryCache::Fill(SDebugMemoryRange* psRange, uint8* puiBytes)
{
	SDebugMemoryPage*	psPage;

	psPage = Find(psRange->uiSpace, psRange->uiAddress);
	if (!psPage || (psRange->uiSize > DEBUG_MEMORY_PAGE))
	{
		return;
	}
	psPage->uiState = DMP_Read;
	psPage->uiValid = psRange->uiSize;
	memcpy(psPage->auiBytes, puiBytes, psRange->uiSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SDebugMemoryPage* CDebugMemoryCache::Find(uint32 uiSpace, uint32 uiAddress)
{
	SDebugMemoryPage*	psPage;
	size				ui;

	for (ui = 0; ui < masPages.NumElements(); ui++)
	{
		psPage = masPages.Get(ui);
		if ((psPage->uiAddress == uiAddress) && (psPage->uiSpace == uiSpace))
		{
			return psPage;
		}
	}
	return NULL;
}

