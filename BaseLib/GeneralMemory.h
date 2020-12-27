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
#ifndef __GENERAL_MEMORY_H__
#define __GENERAL_MEMORY_H__
#include "BaseMemory.h"


//CGeneralMemory is designed to sit behind multiple collections with similar allocation patterns. 
//  It finds it's use mostly in CUnknowns.
//  It It's not a replacement for malloc / free.
//
//CGeneralMemory is also useful as a debugging too.  When coupled with collections (from Data) it can be used to tell if their allocations are being unexpectedly sized / allocated.
class CGeneralMemory : public CBaseMemory
{
private:
	CLinkListFreeList			mcFreeLists;
	CLinkedListBlockAligned		mcLargeList;
	CArrayFreeListAlignedDesc	mcOrder;

	CMemoryFreeListParams		mcFreeListParams;
	int							miDefaultAlignment;

	unsigned int				muiAllocCount;
	unsigned int				muiBreakAlloc;
	BOOL						mbBreakOnAlloc;

public:
	void					Init(void);
	void					Init(int iDefaultAlignment, BOOL bDefaultFreeListParams = TRUE);
	void					Kill(void);
	void*					Add(unsigned int iSize);
	void*					Add(unsigned int iSize, int iAlignment, int iOffset = 0);
	void					Remove(void* pv);
	BOOL					Remove(CArrayVoidPtr* pav);
	unsigned int			GetSize(void* pv);
	CFreeList*				GetFreeList(unsigned int iElementSize, int iAlignment, int iOffset);
	CFreeList*				GetFreeList(unsigned int iElementSize);
	void*					Grow(void* pvInitial, unsigned int iSize);
	void					SetDebugName(void* pv, char (*pszDebug)[4]);
	void					BreakOnAdd(unsigned int uiAllocCount);
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
	CFreeList*				GetOrAddFreeList(unsigned int iElementSize, int iAlignment, int iOffset);
	void*					AllocateInFreeList(CFreeList* pcFreeList, unsigned int uiElementSize);
	void					DeallocateInFreeList(CFreeList* pcFreeList, SGeneralMemoryAllocation* psAlloc);
	void					FreeFreeList(CFreeList* pcFreeList);
	void*					AllocateInLargeList(unsigned int uiSize, int iAlignment, int iOffset);
	void					DeallocateInLargeList(SGeneralMemoryAllocation* psAlloc);
	void					CopyAllocation(void* pvDest, void* pvSource, unsigned int uiDestSize, unsigned int uiSourceSize);
};


//This should be a method.
#define GENERAL_MEMORY_GET_ALLOCATION(p)	(SGeneralMemoryAllocation*)RemapSinglePointer(p, -(ptrdiff_t)sizeof(SGeneralMemoryAllocation))


#endif // __GENERAL_MEMORY_H__

