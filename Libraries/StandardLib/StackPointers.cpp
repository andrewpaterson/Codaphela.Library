/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela StandardLib

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
#include "BaseLib/PrimitiveTypes.h"
#include "BaseLib/PointerFunctions.h"
#include "BaseLib/PointerRemapper.h"
#include "StackPointers.h"


CStackPointers	gcStackPointers;


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CStackPointers::Init(void)
{
	mcFreeList.Init(sizeof(CStackPointer));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CStackPointers::Kill(void)
{
	ClearAllPointers();
	mcFreeList.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CStackPointer* CStackPointers::Add(CPointer* pcPointer)
{
	CStackPointer*	pcStackPointer;

	pcStackPointer = (CStackPointer*)mcFreeList.Add();
	if (pcStackPointer)
	{
		pcStackPointer->Init(pcPointer);
	}
	return pcStackPointer;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CStackPointer* CStackPointers::Add(CCollection* pcCollection)
{
	CStackPointer*	pcStackPointer;

	pcStackPointer = (CStackPointer*)mcFreeList.Add();
	if (pcStackPointer)
	{
		pcStackPointer->Init(pcCollection);
	}
	return pcStackPointer;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CStackPointers::SetLastNext(CStackPointer* pcStackPointer, CStackPointer* pcFirst)
{
	pcFirst->FindLast()->SetNext(pcStackPointer);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CStackPointers::NumElements(void)
{
	return mcFreeList.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CStackPointer* CStackPointers::Get(size uiIndex)
{
	return (CStackPointer*)mcFreeList.Get(uiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CStackPointers::ClearAllPointers(void)
{
	CStackPointer*		pcStackPointer;
	SFreeListIterator	sIter;

	pcStackPointer = (CStackPointer*)mcFreeList.StartIteration(&sIter);
	while (pcStackPointer)
	{
		pcStackPointer->ClearPointer();
		pcStackPointer = (CStackPointer*)mcFreeList.Iterate(&sIter);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CStackPointers::RemoveAll(CStackPointer* pcFirst)
{
	pcFirst->RemoveAll(&mcFreeList);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CStackPointer* CStackPointers::Remove(CStackPointer* pcFirst, CPointer* pcPointer, bool* pbRemoved)
{
	return pcFirst->Remove(pcPointer, &mcFreeList, pbRemoved);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CStackPointer* CStackPointers::Remove(CStackPointer* pcFirst, CCollection* pcPointer, bool* pbRemoved)
{
	return pcFirst->Remove(pcPointer, &mcFreeList, pbRemoved);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CStackPointer* CStackPointers::StartIteration(SFreeListIterator* psIter)
{
	return (CStackPointer*)mcFreeList.StartIteration(psIter);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CStackPointer* CStackPointers::Iterate(SFreeListIterator* psIter)
{
	return (CStackPointer*)mcFreeList.Iterate(psIter);
}

