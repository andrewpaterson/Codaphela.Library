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
#include "FreeListBlock.h"
#include "PointerRemapper.h"
#include "MemoryStats.h"
#include "ArrayTemplate.h"
#include "LinkListAligned.h"


#define MEMORY_FREE_LIST_SIZE_LIMIT	2048


struct SMemoryAllocation
{
	unsigned int	uiAllocCount;
	unsigned int	uiSize;
	SFNode*			psFreeListNode;  //This is only valid when in a free list.
	char			szDebug[4];
};


struct SFreeListParams
{
	unsigned int	iMaxListSize;  //This is the size of the actual element in the freelist including SMemoryAllocation
	unsigned int	iMinListSize;  //This is the size of the previous element in the freelist including SMemoryAllocation +1.
								   //These exist for some Binary Search Compare function stuffs.

	unsigned int	iMaxElementSize;
	unsigned int	iMinElementSize;

	int				iChunkSize;

	SFreeListParams*	Init(unsigned int iFreeListSize, int iPrevSize, int iChunkSize);
};


typedef CArrayTemplate<SFreeListParams>		CArrayFreeListParams;
int CompareFreeListParam(const void* arg1, const void* arg2);


struct SFreeListDesc
{
	CFreeListBlock*		pcFreeList;
	unsigned int		iStride;
	int					iAlignment;
	int					iOffset;

	//int					iCompareHash;

	void Init(unsigned int iStride, int iAlignment, int iOffset);
	void Init(CFreeListBlock* pcFreeList, int iStride, int iAlignment, int iOffset);
};


typedef CArrayTemplate<SFreeListDesc>		CArrayFreeListDesc;
int CompareFreeListDesc(const void* arg1, const void* arg2);


class CMemory
{
private:
	CFreeListBlock*		GetOrAddFreeList(unsigned int iElementSize, int iAlignment, int iOffset);
	SFreeListParams*	GetParamsForSize(unsigned int iElementSize);
	void				InitFreeListParams(void);
	void*				AllocateInFreeList(CFreeListBlock* pcFreeList, unsigned int uiElementSize);
	void				DeallocateInFreeList(CFreeListBlock* pcFreeList, SMemoryAllocation* psAlloc);
	void*				AllocateInLargeList(unsigned int uiSize, int iAlignment, int iOffset);
	void				DeallocateInLargeList(SMemoryAllocation* psAlloc);
	void				CopyAllocation(void* pvDest, void* pvSource, unsigned int uiDestSize, unsigned int uiSourceSize);

public:
	CLinkListFreeListBlock		mcFreeLists;  
	CLinkListAligned			mcLargeList;
	int							miDefaultAlignment;
	CMemoryStats				mcStats;
	CArrayFreeListDesc			mcOrder;
	CArrayFreeListParams		mcParams;
	unsigned int				muiAdds;
	unsigned int				muiBreakAlloc;

	void				Init(void);
	void				Init(int iDefaultAlignment);
	void				Kill(void);
	void*				Add(unsigned int iSize);
	void*				Add(unsigned int iSize, int iAlignment, int iOffset = 0);
	void				Remove(void* pv);
	CFreeListBlock*		GetFreeList(unsigned int iElementSize, int iAlignment, int iOffset);
	CFreeListBlock*		GetFreeList(unsigned int iElementSize);
	void*				Grow(void* pvInitial, unsigned int iSize);
	void				SetDebugName(void* pv, char (*pszDebug)[4]);
	void				BreakOnAdd(unsigned int uiAllocCount);
	int					NumElements(void);
	int					ByteSize(void);
};



#define MEMORY_GET_ALLOCATION(p)	(SMemoryAllocation*)RemapSinglePointer(p, -(int)(sizeof(SMemoryAllocation)))


#endif // __MEMORY_H__

