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
	void*			pvData;
	unsigned int	iSize;
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
	void*	Add(size_t iElementSize);
	void*	Add(void* pvElement, size_t iElementSize);
	bool	Get(int iIndex, void** pvData);
	void*	Get(int iIndex);
	void*	Get(int iIndex, size_t* puiElementSize);
	void	Set(int iIndex, void* pvData, int iElementSize);
	void*	Resize(int iIndex, int iElementSize = 0);
	void	RemoveAt(int iIndex, int bPreserveOrder = true);
	int		IndexOf(void* pvElement);
	bool	Remove(void* pvElement);
	int		NumElements(void);
	bool	FindInSorted(void* pvElement, DataCompare fCompare, int* piIndex);
	void*	InsertAt(int iIndex, int iElementSize);
	int		InsertIntoSorted(DataCompare fCompare, void* pvElement, bool bOverwriteExisting, int iElementSize);
	int		GetSize(int iIndex);

private:
	void	PrivateFree(SPointerAndSize* psType);
	void*	PrivateMalloc(int iElementSize);
};


#endif // !__LIST_VARIABLE_H__

