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
#ifndef __MEMORY_H__
#define __MEMORY_H__
#include "MemoryHeader.h"


typedef CLinkedListTemplate<CFreeList> CLinkListFreeList;


class CMemory
{
private:
	CLinkListFreeList		mcFreeLists;  
	CLinkedListBlockAligned		mcLargeList;
	int							miDefaultAlignment;
	CArrayFreeListDesc			mcOrder;
	CArrayFreeListParams		mcParams;
	unsigned int				muiAllocCount;
	unsigned int				muiBreakAlloc;
	unsigned int				muiFreeListSizeLimit;
	BOOL						mbBreakOnAlloc;

public:
	void				Init(void);
	void				Init(int iDefaultAlignment, BOOL bDefaultFreeListParams = TRUE);
	void				Kill(void);
	void*				Add(unsigned int iSize);
	void*				Add(unsigned int iSize, int iAlignment, int iOffset = 0);
	void				Remove(void* pv);
	BOOL				Remove(CArrayVoidPtr* pav);
	CFreeList*		GetFreeList(unsigned int iElementSize, int iAlignment, int iOffset);
	CFreeList*		GetFreeList(unsigned int iElementSize);
	void*				Grow(void* pvInitial, unsigned int iSize);
	void				SetDebugName(void* pv, char (*pszDebug)[4]);
	void				BreakOnAdd(unsigned int uiAllocCount);
	int					NumElements(void);
	int					ByteSize(void);
	void				AddParamBlock(unsigned int iFreeListSize, int iPrevSize, int iChunkSize);

	void*				StartIteration(SMemoryIterator* psIterator);
	void*				Iterate(SMemoryIterator* psIterator);

	CFreeList*		TestGetFreeListsHead(void);
	void*				TestGetLargeListsHead(void);

protected:
	int					RemoveNode(CArrayVoidPtr* pav, int i, SMemoryAllocation* psAlloc, int iChunkSize, SFNode* psNode, CFreeList* pcList);
	int					RemoveElements(CArrayVoidPtr* pav, int i, int iChunkSize, SFNode* psNode, CFreeList* pcList);

private:
	CFreeList*		GetOrAddFreeList(unsigned int iElementSize, int iAlignment, int iOffset);
	SFreeListParams*	GetParamsForSize(unsigned int iElementSize);
	void				InitFreeListParams(void);
	void*				AllocateInFreeList(CFreeList* pcFreeList, unsigned int uiElementSize);
	void				DeallocateInFreeList(CFreeList* pcFreeList, SMemoryAllocation* psAlloc);
	void*				AllocateInLargeList(unsigned int uiSize, int iAlignment, int iOffset);
	void				DeallocateInLargeList(SMemoryAllocation* psAlloc);
	void				CopyAllocation(void* pvDest, void* pvSource, unsigned int uiDestSize, unsigned int uiSourceSize);
};


#define MEMORY_GET_ALLOCATION(p)	(SMemoryAllocation*)RemapSinglePointer(p, -(int)(sizeof(SMemoryAllocation)))


#endif // __MEMORY_H__

