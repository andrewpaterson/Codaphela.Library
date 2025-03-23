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
#include "MemoryStackExtended.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMemoryStackExtended::Init(size iChunkSize)
{
	mcStacks.Init();
	muiElements = 0;
	muiChunkSize = iChunkSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMemoryStackExtended::Clear(void)
{
	size			i;
	CMemoryStack*	pcStack;
	CMemoryStack	cStack;
	size			uiNumElements;

	uiNumElements = mcStacks.NumElements();
	if (uiNumElements != 0)
	{
		for (i = 1; i < uiNumElements; i++)
		{
			pcStack = mcStacks.Get(i);
			pcStack->Kill();
		}

		pcStack = mcStacks.Get(0);
		memcpy(&cStack, pcStack, sizeof(CMemoryStack));
		mcStacks.Kill();
		mcStacks.Init();
		cStack.Clear();
		mcStacks.Add(&cStack);
	}
	muiElements = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMemoryStackExtended::Kill(void)
{
	size			i;
	CMemoryStack*	pcStack;
	size			uiNumElements;

	uiNumElements = mcStacks.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		pcStack = mcStacks.Get(i);
		pcStack->Kill();
	}
	mcStacks.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CMemoryStackExtended::Add(size uiSize)
{
	size			i;
	CMemoryStack*	pcStack;
	size			uiRemaining;

	if (uiSize > muiChunkSize)
	{
		pcStack = mcStacks.Add();
		pcStack->Init(uiSize);
		muiElements++;
		return pcStack->Add(uiSize);
	}

	i = mcStacks.NumElements();
	if (i != 0)
	{
		do
		{
			i--;
			pcStack = mcStacks.Get(i);
			uiRemaining = pcStack->GetRemainingMemory();
			if (uiSize <= uiRemaining)
			{
				muiElements++;
				return pcStack->Add(uiSize);
			}
		}
		while (i != 0);
	}

	pcStack = mcStacks.Add();
	pcStack->Init(muiChunkSize);
	muiElements++;
	return pcStack->Add(uiSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMemoryStackExtended::Remove(void)
{
	Remove(1);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMemoryStackExtended::Remove(size uiNumToRemove)
{
	if (uiNumToRemove > muiElements)
	{
		uiNumToRemove = muiElements;
	}

	muiElements -= uiNumToRemove;
	if (muiElements == 0)
	{
		Clear();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMemoryStackExtended::Mark(CStackMarkExtended* psMark)
{
	size			i;
	CMemoryStack*	pcStack;
	SStackMark		sMark;
	size			uiNumElements;

	uiNumElements = mcStacks.NumElements();
	psMark->Init(muiElements);

	for (i = 0; i < uiNumElements; i++)
	{
		pcStack = mcStacks.Get(i);
		sMark = pcStack->Mark();

		psMark->Add(sMark);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMemoryStackExtended::Rollback(CStackMarkExtended* pcMark)
{
	size			i;
	CMemoryStack*	pcStack;
	SStackMark*		psMark;

	if (pcMark->muiElements == 0)
	{
		Clear();
	}
	else
	{
		for (i = 0; i < pcMark->mcMarks.NumElements(); i++)
		{
			psMark = pcMark->mcMarks.Get(i);

			pcStack = mcStacks.Get(i);
			pcStack->Rollback(psMark);
		}

		for (; i < mcStacks.NumElements(); i++)
		{
			pcStack = mcStacks.Get(i);
			pcStack->Clear();
		}

		muiElements = pcMark->muiElements;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CMemoryStackExtended::GetTotalMemory(void)
{
	size			i;
	CMemoryStack*	pcStack;
	size			uiSize;
	size			uiNumElements;

	uiNumElements = mcStacks.NumElements();
	uiSize = 0;
	for (i = 0; i < uiNumElements; i++)
	{
		pcStack = mcStacks.Get(i);
		uiSize += pcStack->GetTotalMemory();
	}
	return uiSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CMemoryStackExtended::GetUsedMemory(void)
{
	size			i;
	CMemoryStack*	pcStack;
	size			uiSize;
	size			uiNumElements;

	uiNumElements = mcStacks.NumElements();
	uiSize = 0;
	for (i = 0; i < uiNumElements; i++)
	{
		pcStack = mcStacks.Get(i);
		uiSize += pcStack->GetUsedMemory();
	}
	return uiSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CMemoryStack* CMemoryStackExtended::GetStack(size iIndex)
{
	return  mcStacks.SafeGet(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CMemoryStackExtended::NumStacks(void)
{
	return mcStacks.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CMemoryStackExtended::NumElements(void)
{
	return muiElements;
}

