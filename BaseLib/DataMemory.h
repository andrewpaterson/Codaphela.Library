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
#ifndef __DATA_MEMORY_H__
#define __DATA_MEMORY_H__
#include "BaseMemory.h"


#define DATA_MEMORY_ALIGNMENT 4


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
	void					Init(CMemoryFreeListParams*	pcFreeListParams);
	void					Kill(void);
	void					ReInit(void);

	void*					Add(unsigned int iSize);
	void					Remove(void* pv);
	BOOL					Remove(CArrayVoidPtr* pav);
	size_t					GetSize(void* pv);
	CFreeList*				GetFreeList(size_t iElementSize);
	void*					Grow(void* pvInitial, unsigned int iSize);
	int						NumElements(void);
	int						ByteSize(void);
	CMemoryFreeListParams*	GetFreeListParams(void);

	SMemory					StartIteration(SMemoryIterator* psIterator);
	SMemory					Iterate(SMemoryIterator* psIterator);

	uint64					GetTotalAllocatedMemory(void);
	uint64					GetTotalAllocations(void);

	CFreeList*				TestGetFreeListsHead(void);
	void*					TestGetLargeListsHead(void);
	int						NumFreeLists(void);

protected:
	int						RemoveNode(CArrayVoidPtr* pav, int i, SDataMemoryAllocation* psAlloc, SFNode* psNode, CFreeList* pcList);
	int						RemoveElements(CArrayVoidPtr* pav, int i, SFNode* psNode, CFreeList* pcList);

private:
	CFreeList*				GetOrAddFreeList(size_t iElementSize);
	void*					AllocateInFreeList(CFreeList* pcFreeList, unsigned int uiElementSize);
	void					DeallocateInFreeList(CFreeList* pcFreeList, SDataMemoryAllocation* psAlloc);
	void					FreeFreeList(CFreeList* pcFreeList);
	void*					AllocateInLargeList(unsigned int uiSize);
	void					DeallocateInLargeList(SDataMemoryAllocation* psAlloc);
	void					CopyAllocation(void* pvDest, void* pvSource, size_t uiDestSize, size_t uiSourceSize);
};


//This should be a method.
#define DATA_MEMORY_GET_ALLOCATION(p)	(SDataMemoryAllocation*)RemapSinglePointer(p, -(ptrdiff_t)sizeof(SDataMemoryAllocation))


void DataMemoryInit(void);
void DataMemoryKill(void);


#endif // !__DATA_MEMORY_H__

