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
#ifndef __DATA_MEMORY_H__
#define __DATA_MEMORY_H__
#include "BaseMemory.h"


#define DATA_MEMORY_ALIGNMENT ((uint16)4)


extern CMemoryFreeListParams gcDataMemoryFreeListParams;


//CDataMemory is designed to sit behind multiple collections with similar allocation patterns. 
//  It finds it's use mostly in CUnknowns.
//  It It's not a replacement for malloc / free.
//
//CDataMemory is also useful as a debugging too.  When coupled with collections (from Data) it can be used to tell if their allocations are being unexpectedly sized / allocated.
class CDataMemory : public CBaseMemory
{
private:
	CLinkListFreeList			mcFreeLists;
	CLinkedListBlockAligned		mcLargeList;
	CArrayFreeListDesc			mcOrder;

	CMemoryFreeListParams*		mpcFreeListParams;

public:
	void					Init(void);
	void					Init(CMemoryFreeListParams*	pcFreeListParams);
	void					Kill(void);
	void					ReInit(void);

	void*					Add(size iSize);
	void					Remove(void* pv);
	bool					Remove(CArrayVoidPtr* pav);
	size					GetSize(void* pv);
	CFreeList*				GetFreeList(size iElementSize);
	void*					Grow(void* pvInitial, size iSize);
	size					NumElements(void);
	size					ByteSize(void);
	CMemoryFreeListParams*	GetFreeListParams(void);

	SMemory					StartIteration(SMemoryIterator* psIterator);
	SMemory					Iterate(SMemoryIterator* psIterator);

	uint64					GetTotalAllocatedMemory(void);
	uint64					GetTotalAllocations(void);

	CFreeList*				TestGetFreeListsHead(void);
	void*					TestGetLargeListsHead(void);
	size						NumFreeLists(void);

protected:
	size					RemoveNode(CArrayVoidPtr* pav, size i, SDataMemoryAllocation* psAlloc, SFNode* psNode, CFreeList* pcList);
	size					RemoveElements(CArrayVoidPtr* pav, size i, SFNode* psNode, CFreeList* pcList);

private:
	CFreeList*				GetOrAddFreeList(size iElementSize);
	void*					AllocateInFreeList(CFreeList* pcFreeList, size uiElementSize);
	void					DeallocateInFreeList(CFreeList* pcFreeList, SDataMemoryAllocation* psAlloc);
	void					FreeFreeList(CFreeList* pcFreeList);
	void*					AllocateInLargeList(size uiSize);
	void					DeallocateInLargeList(SDataMemoryAllocation* psAlloc);
	void					CopyAllocation(void* pvDest, void* pvSource, size uiDestSize, size uiSourceSize);
};


//This should be a method.
#define DATA_MEMORY_GET_ALLOCATION(p)	(SDataMemoryAllocation*)RemapSinglePointer(p, -(ptrdiff_t)sizeof(SDataMemoryAllocation))


void DataMemoryInit(void);
void DataMemoryKill(void);


#endif // __DATA_MEMORY_H__

