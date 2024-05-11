/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2022 Andrew Paterson

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
	miElements = 0;
	miChunkSize = iChunkSize;
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
	size			iNumStacks;
	size			iNumElements;

	iNumStacks = mcStacks.NumElements();
	if (iNumStacks > 0)
	{
		iNumElements = mcStacks.NumElements();
		for (i = 1; i < iNumElements; i++)
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
	miElements = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMemoryStackExtended::Kill(void)
{
	size			i;
	CMemoryStack*	pcStack;

	for (i = 0; i < mcStacks.NumElements(); i++)
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
void* CMemoryStackExtended::Add(size iSize)
{
	size			i;
	CMemoryStack*	pcStack;
	size			iRemaining;

	if (iSize > miChunkSize)
	{
		pcStack = mcStacks.Add();
		pcStack->Init(iSize);
		miElements++;
		return pcStack->Add(iSize);
	}

	i = mcStacks.NumElements();
	if (i != 0)
	{
		do
		{
			i--;
			pcStack = mcStacks.Get(i);
			iRemaining = pcStack->GetRemainingMemory();
			if (iSize <= iRemaining)
			{
				miElements++;
				return pcStack->Add(iSize);
			}
		}
		while (i != 0);
	}

	pcStack = mcStacks.Add();
	pcStack->Init(miChunkSize);
	miElements++;
	return pcStack->Add(iSize);
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
void CMemoryStackExtended::Remove(size iNumToRemove)
{
	miElements -= iNumToRemove;
	if (miElements == 0)
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

	psMark->Init(miElements);

	for (i = 0; i < mcStacks.NumElements(); i++)
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

	if (pcMark->miElements == 0)
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

		miElements = pcMark->miElements;
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
	size				iSize;

	iSize = 0;
	for (i = 0; i < mcStacks.NumElements(); i++)
	{
		pcStack = mcStacks.Get(i);
		iSize += pcStack->GetTotalMemory();
	}
	return iSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CMemoryStackExtended::GetUsedMemory(void)
{
	size			i;
	CMemoryStack*	pcStack;
	size			iSize;

	iSize = 0;
	for (i = 0; i < mcStacks.NumElements(); i++)
	{
		pcStack = mcStacks.Get(i);
		iSize += pcStack->GetUsedMemory();
	}
	return iSize;
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
	return miElements;
}

