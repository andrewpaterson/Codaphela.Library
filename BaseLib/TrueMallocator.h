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


//A general purpose allocator over one region of memory, used by Libc65's malloc.  It is a two level segregated fit
//(TLSF) allocator: a free block is on one of a list for each size range, found from two levels of bitmaps (the power
//of two of its size, then which of 16 parts of that power), so Malloc and Free take the same time however many blocks
//there are.  Freed blocks are joined to free blocks either side of them at once.  It uses nothing else from BaseLib,
//so it can be linked into programs that only use Libc65.

#ifdef BUILD_64
#define TRUE_MALLOCATOR_ALIGNMENT_LOG2		4
#else
#define TRUE_MALLOCATOR_ALIGNMENT_LOG2		3
#endif // BUILD_64
#define TRUE_MALLOCATOR_ALIGNMENT			((size)1 << TRUE_MALLOCATOR_ALIGNMENT_LOG2)	//Two sizes: blocks and the data in them start on these boundaries.
#define TRUE_MALLOCATOR_SECOND_LOG2			4
#define TRUE_MALLOCATOR_SECOND_LEVELS		(1 << TRUE_MALLOCATOR_SECOND_LOG2)
#define TRUE_MALLOCATOR_FIRST_SHIFT			(TRUE_MALLOCATOR_SECOND_LOG2 + TRUE_MALLOCATOR_ALIGNMENT_LOG2)
#define TRUE_MALLOCATOR_SMALL_BLOCK			((size)1 << TRUE_MALLOCATOR_FIRST_SHIFT)	//Smaller blocks are all in the first level, a list for each size.
#define TRUE_MALLOCATOR_FIRST_LEVELS		(32 - TRUE_MALLOCATOR_FIRST_SHIFT + 1)
#define TRUE_MALLOCATOR_MAX_SIZE			((size)0x40000000)		//The biggest block, so sizes and their rounding fit in 32 bits.

//A block's flags, in the low bits of its size.
#define TRUE_MALLOCATOR_FREE				1
#define TRUE_MALLOCATOR_PREVIOUS_FREE		2
#define TRUE_MALLOCATOR_FLAGS				3


//A block is its header then its data.  The next block in memory is uiSize bytes on.  While a block is free its data
//holds its links in its free list and the next block's uiPreviousSize is its size, so it can be joined to a block freed
//after it.
struct STrueMallocatorBlock
{
	size					uiPreviousSize;		//The size of the block before, while that block is free.
	size					uiSize;				//This block's size, with its header, and its flags.
	STrueMallocatorBlock*	psNextFree;			//A used block's data starts here.
	STrueMallocatorBlock*	psPreviousFree;
};


#define TRUE_MALLOCATOR_HEADER				(2 * sizeof(size))
#define TRUE_MALLOCATOR_MIN_BLOCK			(sizeof(STrueMallocatorBlock))


class CTrueMallocator
{
protected:
	uint8*					mpuiStart;
	STrueMallocatorBlock*	mpsEnd;				//A used block of no size after the last block.
	uint32					muiFirstBitmap;		//A bit for each first level with a free block.
	uint32					mauiSecondBitmaps[TRUE_MALLOCATOR_FIRST_LEVELS];
	STrueMallocatorBlock*	mapsFree[TRUE_MALLOCATOR_FIRST_LEVELS][TRUE_MALLOCATOR_SECOND_LEVELS];
	size					muiNumAllocations;
	size					muiAllocatedBytes;	//The used blocks' sizes, with their headers.

public:
	bool					Init(void* pvMemory, size uiBytes);
	void					Kill(void);

	void*					Malloc(size uiBytes);
	void*					Realloc(void* pv, size uiBytes);
	void					Free(void* pv);
	size					Size(void* pv);

	size					NumAllocations(void);
	size					AllocatedBytes(void);
	size					LargestFree(void);
	bool					Contains(void* pv);
	bool					IsValid(void);

protected:
	size					BlockSize(size uiBytes);
	void					Mapping(size uiSize, int* piFirst, int* piSecond);
	STrueMallocatorBlock*	FindFree(size uiSize);
	void					InsertFree(STrueMallocatorBlock* psBlock);
	void					RemoveFree(STrueMallocatorBlock* psBlock);
	STrueMallocatorBlock*	Next(STrueMallocatorBlock* psBlock);
	void					MarkFree(STrueMallocatorBlock* psBlock, size uiSize);
	void					Split(STrueMallocatorBlock* psBlock, size uiSize);
};


#endif // __TRUE_MALLOCATOR_H__

