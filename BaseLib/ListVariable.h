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
#ifndef __LIST_VARIABLE_H__
#define __LIST_VARIABLE_H__
#include "ArrayIntAndPointer.h"
#include "LinkedListTemplate.h"
#include "DataMemory.h"


struct SPointerAndSize
{
	void*	pvData;
	size	iSize;
};

typedef CArrayTemplate<SPointerAndSize> CArrayTypedPointerAndSize;


//Remember this class is an *ordered* array.  mcArray is an array of pointers into the freelists.  This is *not* a general memory manager.
//mcArray is the intended method of access into the array, not mcFreeLists.
class CListVariable
{
private:
	CArrayTypedPointerAndSize	mcArray;
	CDataMemory					mcFreeLists;

public:
	void	Init(void);
	void	ReInit(void);
	void	Kill(void);
	void*	Add(size iElementSize);
	void*	Add(void* pvElement, size iElementSize);
	bool	Get(size iIndex, void** pvData);
	void*	Get(size iIndex);
	void*	Get(size iIndex, size* puiElementSize);
	void	Set(size iIndex, void* pvData, size iElementSize);
	void*	Resize(size iIndex, size iElementSize = 0);
	void	RemoveAt(size iIndex, bool bPreserveOrder = true);
	size	IndexOf(void* pvElement);
	bool	Remove(void* pvElement);
	size	NumElements(void);
	bool	FindInSorted(void* pvElement, DataCompare fCompare, size* piIndex);
	void*	InsertAt(size iIndex, size iElementSize);
	size	InsertIntoSorted(DataCompare fCompare, void* pvElement, bool bOverwriteExisting, size iElementSize);
	size	GetSize(size iIndex);

private:
	void	PrivateFree(SPointerAndSize* psType);
	void*	PrivateMalloc(size iElementSize);
};


#endif // __LIST_VARIABLE_H__

