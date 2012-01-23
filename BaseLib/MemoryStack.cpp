/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

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
#include "MemoryStack.h" 
#include "PointerRemapper.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CMemoryStack::Init(int iSize)
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
void* CMemoryStack::Allocate(int iSize)
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
	if (mpvMemory)
	{
		free(mpvMemory);
		mpvMemory = NULL;
	}
	miMemorySize = 0;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CMemoryStack::Add(int iSize)
{
	void*	pvCurrent;

	if (mpvMemory == NULL)
	{
		mpvMemory = malloc(miMemorySize);
	}
	if (mpvMemory == NULL)
	{
		return NULL;
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
	miElements--;
	if (miElements == 0)
	{
		Clear();
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CMemoryStack::Remove(int iNumToRemove)
{
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
	miUsedMemory = psMark->iUsedMemory;
	miElements = psMark->iElements;
}

