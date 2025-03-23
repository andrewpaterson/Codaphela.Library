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
#ifndef __GENERAL_MEMORY_H__
#define __GENERAL_MEMORY_H__
#include "BaseMemory.h"


//CGeneralMemory is designed to sit behind multiple collections with similar allocation patterns. 
//  It finds it's use mostly in CUnknowns.
//  It It's not a replacement for malloc / free.
class CGeneralMemory : public CBaseMemory
{
private:
	CLinkListFreeList			mcFreeLists;
	CLinkedListBlockAligned		mcLargeList;
	CArrayFreeListAlignedDesc	mcOrder;

	CMemoryFreeListParams		mcFreeListParams;
	uint8						miDefaultAlignment;

	size						muiAllocCount;
	size						muiBreakAlloc;
	bool						mbBreakOnAlloc;

public:
	void					Init(void);
	void					Init(uint16 iDefaultAlignment, bool bDefaultFreeListParams = true);
	void					Kill(void);
	void*					Add(size iSize);
	void*					Add(size iSize, uint16 iAlignment, int16 iOffset = 0);
	bool					Remove(void* pv);
	size					RemoveMultiple(CArrayVoidPtr* pav);
	size					GetSize(void* pv);
	CFreeList*				GetFreeList(size iElementSize, uint16 iAlignment, int16 iOffset);
	CFreeList*				GetFreeList(size iElementSize);
	void*					Grow(void* pvInitial, size iSize);
	void					SetDebugName(void* pv, char (*pszDebug)[4]);
	void					BreakOnAdd(size uiAllocCount);
	size					NumElements(void);
	size					ByteSize(void);
	CMemoryFreeListParams*	GetFreeListParams(void);
	size					GetDefaultAlignment(void);

	SMemory					StartIteration(SMemoryIterator* psIterator);
	SMemory					Iterate(SMemoryIterator* psIterator);

	uint64					GetTotalAllocatedMemory(void);
	uint64					GetTotalAllocations(void);

	CFreeList*				TestGetFreeListsHead(void);
	void*					TestGetLargeListsHead(void);
	size					NumFreeLists(void);

protected:
	size					RemoveNode(CArrayVoidPtr* pav, size i, SGeneralMemoryAllocation* psAlloc, SFNode* psNode, CFreeList* pcList);
	size					RemoveElements(CArrayVoidPtr* pav, size i, SFNode* psNode, CFreeList* pcList);

private:
	CFreeList*				GetOrAddFreeList(size iElementSize, uint16 iAlignment, int16 iOffset);
	void*					AllocateInFreeList(CFreeList* pcFreeList, size uiElementSize);
	bool					DeallocateInFreeList(CFreeList* pcFreeList, SGeneralMemoryAllocation* psAlloc);
	void					FreeFreeList(CFreeList* pcFreeList);
	void*					AllocateInLargeList(size uiSize, uint16 iAlignment, int16 iOffset);
	bool					DeallocateInLargeList(SGeneralMemoryAllocation* psAlloc);
	void					CopyAllocation(void* pvDest, void* pvSource, size uiDestSize, size uiSourceSize);
};


//This should be a method.
#define GENERAL_MEMORY_GET_ALLOCATION(p)	(SGeneralMemoryAllocation*)RemapSinglePointer(p, -(ptrdiff_t)sizeof(SGeneralMemoryAllocation))


#endif // __GENERAL_MEMORY_H__

