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
#ifndef __GENERAL_MEMORY_H__
#define __GENERAL_MEMORY_H__
#include "BaseMemory.h"


class CGeneralMemory : public CBaseMemory
{
private:
	CLinkListFreeList			mcFreeLists;
	CLinkedListBlockAligned		mcLargeList;
	CArrayFreeListAlignedDesc	mcOrder;

	CMemoryFreeListParams		mcFreeListParams;
	int							miDefaultAlignment;

	uint32				muiAllocCount;
	uint32				muiBreakAlloc;
	bool						mbBreakOnAlloc;

public:
	void					Init(void);
	void					Init(int iDefaultAlignment, bool bDefaultFreeListParams = true);
	void					Kill(void);
	void*					Add(uint32 iSize);
	void*					Add(uint32 iSize, int iAlignment, int iOffset = 0);
	bool					Remove(void* pv);
	int						RemoveMultiple(CArrayVoidPtr* pav);
	uint32			GetSize(void* pv);
	CFreeList*				GetFreeList(uint32 iElementSize, int iAlignment, int iOffset);
	CFreeList*				GetFreeList(uint32 iElementSize);
	void*					Grow(void* pvInitial, uint32 iSize);
	void					SetDebugName(void* pv, char (*pszDebug)[4]);
	void					BreakOnAdd(uint32 uiAllocCount);
	int						NumElements(void);
	int						ByteSize(void);
	CMemoryFreeListParams*	GetFreeListParams(void);
	int						GetDefaultAlignment(void);

	SMemory					StartIteration(SMemoryIterator* psIterator);
	SMemory					Iterate(SMemoryIterator* psIterator);

	uint64					GetTotalAllocatedMemory(void);
	uint64					GetTotalAllocations(void);

	CFreeList*				TestGetFreeListsHead(void);
	void*					TestGetLargeListsHead(void);
	int						NumFreeLists(void);

protected:
	int						RemoveNode(CArrayVoidPtr* pav, int i, SGeneralMemoryAllocation* psAlloc, SFNode* psNode, CFreeList* pcList);
	int						RemoveElements(CArrayVoidPtr* pav, int i, SFNode* psNode, CFreeList* pcList);

private:
	CFreeList*				GetOrAddFreeList(uint32 iElementSize, int iAlignment, int iOffset);
	void*					AllocateInFreeList(CFreeList* pcFreeList, uint32 uiElementSize);
	bool					DeallocateInFreeList(CFreeList* pcFreeList, SGeneralMemoryAllocation* psAlloc);
	void					FreeFreeList(CFreeList* pcFreeList);
	void*					AllocateInLargeList(uint32 uiSize, int iAlignment, int iOffset);
	bool					DeallocateInLargeList(SGeneralMemoryAllocation* psAlloc);
	void					CopyAllocation(void* pvDest, void* pvSource, uint32 uiDestSize, uint32 uiSourceSize);
};


//This should be a method.
#define GENERAL_MEMORY_GET_ALLOCATION(p)	(SGeneralMemoryAllocation*)RemapSinglePointer(p, -(ptrdiff_t)sizeof(SGeneralMemoryAllocation))


#endif // !__GENERAL_MEMORY_H__

