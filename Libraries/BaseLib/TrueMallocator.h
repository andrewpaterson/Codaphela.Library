/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

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
#ifndef __TRUE_MALLOCATOR_H__
#define __TRUE_MALLOCATOR_H__
#include "PrimitiveTypes.h"


//A general purpose allocator over regions of memory it is given, used by Libc65's malloc (one region) and by
//CThreadMallocator's arenas (a region for each chunk).  It is a two level segregated fit (TLSF) allocator: a free
//block is on one of a list for each size range, found from two levels of bitmaps (the power of two of its size, then
//which of 16 parts of that power), so Malloc and Free take the same time however many blocks there are.  Freed blocks
//are joined to free blocks either side of them at once.  It uses nothing else from BaseLib but Atomic.h (which is
//only its header), so it can be linked into programs that only use Libc65.
//
//It isn't safe between threads.  One thing is, for CThreadMallocator: Size may be asked of a used block by another
//thread than the one allocating and freeing.

#ifdef BUILD_64
#define TRUE_MALLOCATOR_ALIGNMENT_LOG2		4
#else
#define TRUE_MALLOCATOR_ALIGNMENT_LOG2		3
#endif // BUILD_64
#define TRUE_MALLOCATOR_ALIGNMENT			((lsize)1 << TRUE_MALLOCATOR_ALIGNMENT_LOG2)	//Two sizes: blocks and the data in them start on these boundaries.
#define TRUE_MALLOCATOR_SECOND_LOG2			4
#define TRUE_MALLOCATOR_SECOND_LEVELS		(1 << TRUE_MALLOCATOR_SECOND_LOG2)
#define TRUE_MALLOCATOR_FIRST_SHIFT			(TRUE_MALLOCATOR_SECOND_LOG2 + TRUE_MALLOCATOR_ALIGNMENT_LOG2)
#define TRUE_MALLOCATOR_SMALL_BLOCK			((lsize)1 << TRUE_MALLOCATOR_FIRST_SHIFT)	//Smaller blocks are all in the first level, a list for each size.
#define TRUE_MALLOCATOR_FIRST_LEVELS		(32 - TRUE_MALLOCATOR_FIRST_SHIFT + 1)
#define TRUE_MALLOCATOR_LIST(FIRST, SECOND)	(((FIRST) << TRUE_MALLOCATOR_SECOND_LOG2) + (SECOND))	//One index for the free lists: on the W65C816 an index of 16 bits is kept in Y.
#define TRUE_MALLOCATOR_MAX_SIZE			((lsize)0x40000000)		//The biggest block, so sizes and their rounding fit in 32 bits.

//A block's flags, in the low bits of its size.
#define TRUE_MALLOCATOR_FREE				1
#define TRUE_MALLOCATOR_PREVIOUS_FREE		2
#define TRUE_MALLOCATOR_FLAGS				3


//A block is its header then its data.  The next block in memory is uiSize bytes on.  While a block is free its data
//holds its links in its free list and the next block's uiPreviousSize is its size, so it can be joined to a block freed
//after it.
struct STrueMallocatorBlock
{
	lsize					uiPreviousSize;		//The size of the block before, while that block is free.
	lsize					uiSize;				//This block's size, with its header, and its flags.
	STrueMallocatorBlock*	psNextFree;			//A used block's data starts here.
	STrueMallocatorBlock*	psPreviousFree;
};


//A region is its memory's start, aligned, then its blocks, then a used block of no size that ends it.
struct STrueMallocatorRegion
{
	STrueMallocatorRegion*	psNext;
	STrueMallocatorBlock*	psEnd;
};


#define TRUE_MALLOCATOR_HEADER				(2 * sizeof(lsize))
#define TRUE_MALLOCATOR_MIN_BLOCK			(sizeof(STrueMallocatorBlock))


class CTrueMallocator
{
protected:
	STrueMallocatorRegion*	mpsRegions;
	uint32					muiFirstBitmap;		//A bit for each first level with a free block.
	uint16					mauiSecondBitmaps[TRUE_MALLOCATOR_FIRST_LEVELS];	//A bit for each second level (16) with a free block.
	STrueMallocatorBlock*	mapsFree[TRUE_MALLOCATOR_FIRST_LEVELS * TRUE_MALLOCATOR_SECOND_LEVELS];	//By TRUE_MALLOCATOR_LIST(first, second).
	lsize					muiNumAllocations;
	lsize					muiAllocatedBytes;	//The used blocks' sizes, with their headers.
	lsize					muiPeakBytes;		//The most muiAllocatedBytes has been.

public:
	void					Init(void);
	bool					Init(void* pvMemory, lsize uiBytes);
	void					Kill(void);

	bool					AddRegion(void* pvMemory, lsize uiBytes);
	bool					RemoveRegion(void* pvMemory);
	bool					IsRegionFree(void* pvMemory);

	void*					Malloc(lsize uiBytes);
	void*					Realloc(void* pv, lsize uiBytes);
	void					Free(void* pv);
	lsize					Size(void* pv);

	lsize					NumAllocations(void) const;
	lsize					AllocatedBytes(void);
	lsize					PeakBytes(void);
	lsize					LargestFree(void);
	bool					Contains(void* pv);
	bool					IsValid(void);

protected:
	STrueMallocatorRegion*	GetRegion(void* pvMemory);
	STrueMallocatorBlock*	FirstBlock(STrueMallocatorRegion* psRegion);
	bool					IsValidRegion(STrueMallocatorRegion* psRegion, lsize* puiNumUsed, lsize* puiUsedBytes, lsize* puiNumFree);
	bool					IsValidLists(lsize uiNumFree);
	lsize					BlockSize(lsize uiBytes);
	void					Mapping(lsize uiSize, int* piFirst, int* piSecond);
	STrueMallocatorBlock*	FindFree(lsize uiSize);
	void					InsertFree(STrueMallocatorBlock* psBlock);
	void					RemoveFree(STrueMallocatorBlock* psBlock);
	STrueMallocatorBlock*	Next(STrueMallocatorBlock* psBlock);
	void					MarkFree(STrueMallocatorBlock* psBlock, lsize uiSize);
	void					Split(STrueMallocatorBlock* psBlock, lsize uiSize);
	void					Allocated(lsize uiBytes);
};


#endif // __TRUE_MALLOCATOR_H__

