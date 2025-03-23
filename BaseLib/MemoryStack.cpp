/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

This file is part of The Codaphela Project: Codaphela BaseLib

Codaphela BaseLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela BaseLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela BaseLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#include <stdlib.h>
#include "PointerFunctions.h"
#include "PointerRemapper.h"
#include "MemoryStack.h" 


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CMemoryStack::Init(size iSize)
{
	mpvMemory = NULL;
	miMemorySize = iSize;
	miUsedMemory = 0;
	miElements = 0;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CMemoryStack::Allocate(size iSize)
{
	Init(iSize);
	mpvMemory = malloc(miMemorySize);
	return mpvMemory;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CMemoryStack::Clear(void)
{
	miUsedMemory = 0;
	miElements = 0;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CMemoryStack::Kill(void)
{
	SafeFree(mpvMemory);
	miMemorySize = 0;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CMemoryStack::Add(size iSize)
{
	void*	pvCurrent;

	if (mpvMemory == NULL)
	{
		mpvMemory = malloc(miMemorySize);
		if (mpvMemory == NULL)
		{
			return NULL;
		}
	}

	if ((miUsedMemory + iSize) > miMemorySize)
	{
		return NULL;
	}

	pvCurrent = RemapSinglePointer(mpvMemory, miUsedMemory);
	miUsedMemory += iSize;
	miElements++;
	return pvCurrent;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CMemoryStack::Remove(void)
{
	Remove(1);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CMemoryStack::Remove(size iNumToRemove)
{
	if (iNumToRemove > miElements)
	{
		iNumToRemove = miElements;
	}

	miElements -= iNumToRemove;
	if (miElements == 0)
	{
		Clear();
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SStackMark CMemoryStack::Mark(void)
{
	SStackMark	sMark;

	sMark.Init(miUsedMemory, miElements);
	return sMark;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CMemoryStack::Rollback(SStackMark* psMark)
{
	miUsedMemory = psMark->miUsedMemory;
	miElements = psMark->miElements;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
size CMemoryStack::GetTotalMemory(void)
{
	return miMemorySize;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
size CMemoryStack::GetUsedMemory(void)
{
	return miUsedMemory;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
size CMemoryStack::GetRemainingMemory(void)
{
	return miMemorySize - miUsedMemory;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CMemoryStack::GetData(void)
{
	return mpvMemory;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
size CMemoryStack::NumElements(void)
{
	return miElements;
}

