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
#include <string.h>
#ifdef _MSC_VER
#include <intrin.h>
#endif // _MSC_VER
#include "TrueMallocator.h"


//////////////////////////////////////////////////////////////////////////
//
// The highest set bit of ui, which must not be 0.
//
//////////////////////////////////////////////////////////////////////////
static int TrueMallocatorHighestBit(uint32 ui)
{
#ifdef _MSC_VER
	unsigned long	uiIndex;

	_BitScanReverse(&uiIndex, ui);
	return (int)uiIndex;
#elif defined(__W65C816__)
	return 31 - __builtin_clzl(ui);		//An int is 16 bits.
#else
	return 31 - __builtin_clz(ui);
#endif // _MSC_VER
}


//////////////////////////////////////////////////////////////////////////
//
// The highest set bit of the 16 bit ui, which must not be 0 (a second level
// bitmap, or a size under 65536: on the W65C816 a 16 bit count is cheaper
// than a 32 bit one).
//
//////////////////////////////////////////////////////////////////////////
static int TrueMallocatorHighestBit16(uint16 ui)
{
#if defined(__W65C816__)
	return 15 - __builtin_clz(ui);		//An int is 16 bits.
#else
	return TrueMallocatorHighestBit(ui);
#endif // __W65C816__
}


//////////////////////////////////////////////////////////////////////////
//
// The lowest set bit of ui, which must not be 0.
//
//////////////////////////////////////////////////////////////////////////
static int TrueMallocatorLowestBit(uint32 ui)
{
#ifdef _MSC_VER
	unsigned long	uiIndex;

	_BitScanForward(&uiIndex, ui);
	return (int)uiIndex;
#elif defined(__W65C816__)
	return __builtin_ctzl(ui);		//An int is 16 bits.
#else
	return __builtin_ctz(ui);
#endif // _MSC_VER
}


//////////////////////////////////////////////////////////////////////////
//
// The lowest set bit of the 16 bit ui, which must not be 0.
//
//////////////////////////////////////////////////////////////////////////
static int TrueMallocatorLowestBit16(uint16 ui)
{
#if defined(__W65C816__)
	return __builtin_ctz(ui);		//An int is 16 bits.
#else
	return TrueMallocatorLowestBit(ui);
#endif // __W65C816__
}


//////////////////////////////////////////////////////////////////////////
//
// The highest set bit of a block's size, which must not be 0 (and is under
// TRUE_MALLOCATOR_MAX_SIZE): counted in 16 bits, from its high word when it
// is 65536 or more.
//
//////////////////////////////////////////////////////////////////////////
static int TrueMallocatorSizeHighestBit(lsize uiSize)
{
	if (uiSize <= 0xFFFF)
	{
		return TrueMallocatorHighestBit16((uint16)uiSize);
	}
	return 16 + TrueMallocatorHighestBit16((uint16)(uiSize >> 16));
}


//////////////////////////////////////////////////////////////////////////
//
// Manages the memory from pvMemory for uiBytes (at most TRUE_MALLOCATOR_MAX_SIZE of it).  It starts as one free
// block and a used block of no size that ends it.  Returns false if there is too little memory for a block.
//
//////////////////////////////////////////////////////////////////////////
bool CTrueMallocator::Init(void* pvMemory, lsize uiBytes)
{
	lsize					uiStart;
	lsize					uiEnd;
	lsize					uiSize;
	STrueMallocatorBlock*	psBlock;
	size					i;
	size					j;

	muiFirstBitmap = 0;
	for (i = 0; i < TRUE_MALLOCATOR_FIRST_LEVELS; i++)
	{
		mauiSecondBitmaps[i] = 0;
		for (j = 0; j < TRUE_MALLOCATOR_SECOND_LEVELS; j++)
		{
			mapsFree[TRUE_MALLOCATOR_LIST(i, j)] = NULL;
		}
	}
	muiNumAllocations = 0;
	muiAllocatedBytes = 0;
	mpuiStart = NULL;
	mpsEnd = NULL;

	uiStart = ((lsize)pvMemory + TRUE_MALLOCATOR_ALIGNMENT - 1) & ~(TRUE_MALLOCATOR_ALIGNMENT - 1);
	uiEnd = ((lsize)pvMemory + uiBytes) & ~(TRUE_MALLOCATOR_ALIGNMENT - 1);
	if ((uiEnd <= uiStart) || (uiEnd - uiStart < TRUE_MALLOCATOR_MIN_BLOCK + TRUE_MALLOCATOR_HEADER))
	{
		return false;
	}
	if (uiEnd - uiStart > TRUE_MALLOCATOR_MAX_SIZE + TRUE_MALLOCATOR_HEADER)
	{
		uiEnd = uiStart + TRUE_MALLOCATOR_MAX_SIZE + TRUE_MALLOCATOR_HEADER;
	}

	mpuiStart = (uint8*)uiStart;
	mpsEnd = (STrueMallocatorBlock*)(uiEnd - TRUE_MALLOCATOR_HEADER);
	mpsEnd->uiPreviousSize = 0;
	mpsEnd->uiSize = 0;

	uiSize = (lsize)((uint8*)mpsEnd - mpuiStart);
	psBlock = (STrueMallocatorBlock*)mpuiStart;
	psBlock->uiPreviousSize = 0;
	psBlock->uiSize = 0;
	MarkFree(psBlock, uiSize);
	InsertFree(psBlock);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTrueMallocator::Kill(void)
{
	mpuiStart = NULL;
	mpsEnd = NULL;
	muiFirstBitmap = 0;
	muiNumAllocations = 0;
	muiAllocatedBytes = 0;
}


//////////////////////////////////////////////////////////////////////////
//
// uiBytes of memory, aligned to TRUE_MALLOCATOR_ALIGNMENT, or NULL if there is no free block big enough.  The
// smallest free block that is sure to fit is used and what is left of it is a free block again.
//
//////////////////////////////////////////////////////////////////////////
void* CTrueMallocator::Malloc(lsize uiBytes)
{
	STrueMallocatorBlock*	psBlock;
	lsize					uiSize;

	uiSize = BlockSize(uiBytes);
	if (uiSize == 0)
	{
		return NULL;
	}
	psBlock = FindFree(uiSize);
	if (!psBlock)
	{
		return NULL;
	}

	RemoveFree(psBlock);
	psBlock->uiSize &= ~(lsize)TRUE_MALLOCATOR_FREE;
	Next(psBlock)->uiSize &= ~(lsize)TRUE_MALLOCATOR_PREVIOUS_FREE;
	Split(psBlock, uiSize);

	muiNumAllocations++;
	muiAllocatedBytes += psBlock->uiSize & ~(lsize)TRUE_MALLOCATOR_FLAGS;
	return (uint8*)psBlock + TRUE_MALLOCATOR_HEADER;
}


//////////////////////////////////////////////////////////////////////////
//
// Changes the size of pv's memory to uiBytes, keeping what is in it.  It shrinks in place, and grows in place when
// the block after it is free and big enough; otherwise the memory moves.  NULL pv is Malloc.  Returns NULL, leaving
// pv as it was, if there is no room.
//
//////////////////////////////////////////////////////////////////////////
void* CTrueMallocator::Realloc(void* pv, lsize uiBytes)
{
	STrueMallocatorBlock*	psBlock;
	STrueMallocatorBlock*	psNext;
	lsize					uiSize;
	lsize					uiCurrent;
	void*					pvNew;

	if (!pv)
	{
		return Malloc(uiBytes);
	}
	uiSize = BlockSize(uiBytes);
	if (uiSize == 0)
	{
		return NULL;
	}

	psBlock = (STrueMallocatorBlock*)((uint8*)pv - TRUE_MALLOCATOR_HEADER);
	uiCurrent = psBlock->uiSize & ~(lsize)TRUE_MALLOCATOR_FLAGS;
	if (uiSize <= uiCurrent)
	{
		Split(psBlock, uiSize);
		muiAllocatedBytes -= uiCurrent - (psBlock->uiSize & ~(lsize)TRUE_MALLOCATOR_FLAGS);
		return pv;
	}

	psNext = Next(psBlock);
	if ((psNext->uiSize & TRUE_MALLOCATOR_FREE) && (uiCurrent + (psNext->uiSize & ~(lsize)TRUE_MALLOCATOR_FLAGS) >= uiSize))
	{
		RemoveFree(psNext);
		psBlock->uiSize += psNext->uiSize & ~(lsize)TRUE_MALLOCATOR_FLAGS;
		Next(psBlock)->uiSize &= ~(lsize)TRUE_MALLOCATOR_PREVIOUS_FREE;
		Split(psBlock, uiSize);
		muiAllocatedBytes += (psBlock->uiSize & ~(lsize)TRUE_MALLOCATOR_FLAGS) - uiCurrent;
		return pv;
	}

	pvNew = Malloc(uiBytes);
	if (!pvNew)
	{
		return NULL;
	}
	memcpy(pvNew, pv, uiCurrent - TRUE_MALLOCATOR_HEADER);
	Free(pv);
	return pvNew;
}


//////////////////////////////////////////////////////////////////////////
//
// Gives pv's memory back.  Its block is joined to the free blocks either side of it.  NULL is ignored.
//
//////////////////////////////////////////////////////////////////////////
void CTrueMallocator::Free(void* pv)
{
	STrueMallocatorBlock*	psBlock;
	STrueMallocatorBlock*	psNext;
	STrueMallocatorBlock*	psPrevious;
	lsize					uiSize;

	if (!pv)
	{
		return;
	}
	psBlock = (STrueMallocatorBlock*)((uint8*)pv - TRUE_MALLOCATOR_HEADER);
	uiSize = psBlock->uiSize & ~(lsize)TRUE_MALLOCATOR_FLAGS;
	muiNumAllocations--;
	muiAllocatedBytes -= uiSize;

	psNext = Next(psBlock);
	if (psNext->uiSize & TRUE_MALLOCATOR_FREE)
	{
		RemoveFree(psNext);
		uiSize += psNext->uiSize & ~(lsize)TRUE_MALLOCATOR_FLAGS;
	}
	if (psBlock->uiSize & TRUE_MALLOCATOR_PREVIOUS_FREE)
	{
		psPrevious = (STrueMallocatorBlock*)((uint8*)psBlock - psBlock->uiPreviousSize);
		RemoveFree(psPrevious);
		uiSize += psPrevious->uiSize & ~(lsize)TRUE_MALLOCATOR_FLAGS;
		psBlock = psPrevious;
	}
	MarkFree(psBlock, uiSize);
	InsertFree(psBlock);
}


//////////////////////////////////////////////////////////////////////////
//
// The bytes pv's memory can hold: what it was asked for, rounded up.
//
//////////////////////////////////////////////////////////////////////////
lsize CTrueMallocator::Size(void* pv)
{
	STrueMallocatorBlock*	psBlock;

	psBlock = (STrueMallocatorBlock*)((uint8*)pv - TRUE_MALLOCATOR_HEADER);
	return (psBlock->uiSize & ~(lsize)TRUE_MALLOCATOR_FLAGS) - TRUE_MALLOCATOR_HEADER;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
lsize CTrueMallocator::NumAllocations(void)
{
	return muiNumAllocations;
}


//////////////////////////////////////////////////////////////////////////
//
// The bytes the used blocks take, with their headers.
//
//////////////////////////////////////////////////////////////////////////
lsize CTrueMallocator::AllocatedBytes(void)
{
	return muiAllocatedBytes;
}


//////////////////////////////////////////////////////////////////////////
//
// The most bytes one Malloc could get now.
//
//////////////////////////////////////////////////////////////////////////
lsize CTrueMallocator::LargestFree(void)
{
	STrueMallocatorBlock*	psBlock;
	lsize					uiLargest;
	lsize					uiSize;
	int						iFirst;
	int						iSecond;

	if (muiFirstBitmap == 0)
	{
		return 0;
	}
	iFirst = TrueMallocatorHighestBit(muiFirstBitmap);
	iSecond = TrueMallocatorHighestBit16(mauiSecondBitmaps[iFirst]);
	uiLargest = 0;
	for (psBlock = mapsFree[TRUE_MALLOCATOR_LIST(iFirst, iSecond)]; psBlock; psBlock = psBlock->psNextFree)
	{
		uiSize = psBlock->uiSize & ~(lsize)TRUE_MALLOCATOR_FLAGS;
		uiLargest = (uiSize > uiLargest) ? uiSize : uiLargest;
	}
	return uiLargest - TRUE_MALLOCATOR_HEADER;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether pv is in the memory this manages.
//
//////////////////////////////////////////////////////////////////////////
bool CTrueMallocator::Contains(void* pv)
{
	return ((uint8*)pv >= mpuiStart) && ((uint8*)pv < (uint8*)mpsEnd);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the blocks are consistent: each block's size and flags agree with its neighbours, no two free blocks are
// next to each other, every free block is on the right free list (and only those are), the bitmaps match the lists
// and the counts match the used blocks.
//
//////////////////////////////////////////////////////////////////////////
bool CTrueMallocator::IsValid(void)
{
	STrueMallocatorBlock*	psBlock;
	STrueMallocatorBlock*	psNext;
	STrueMallocatorBlock*	psListed;
	lsize					uiSize;
	lsize					uiNumUsed;
	lsize					uiUsedBytes;
	lsize					uiNumFree;
	lsize					uiNumListed;
	bool					bPreviousFree;
	int						iFirst;
	int						iSecond;
	size					i;
	size					j;

	if (!mpsEnd)
	{
		return false;
	}
	uiNumUsed = 0;
	uiUsedBytes = 0;
	uiNumFree = 0;
	bPreviousFree = false;
	for (psBlock = (STrueMallocatorBlock*)mpuiStart; psBlock != mpsEnd; psBlock = psNext)
	{
		uiSize = psBlock->uiSize & ~(lsize)TRUE_MALLOCATOR_FLAGS;
		if ((uiSize < TRUE_MALLOCATOR_MIN_BLOCK) || (uiSize & (TRUE_MALLOCATOR_ALIGNMENT - 1)) || (uiSize > (lsize)((uint8*)mpsEnd - (uint8*)psBlock)))
		{
			return false;
		}
		if (((psBlock->uiSize & TRUE_MALLOCATOR_PREVIOUS_FREE) != 0) != bPreviousFree)
		{
			return false;
		}
		psNext = Next(psBlock);
		if (psBlock->uiSize & TRUE_MALLOCATOR_FREE)
		{
			if (bPreviousFree || (psNext->uiPreviousSize != uiSize))
			{
				return false;
			}
			Mapping(uiSize, &iFirst, &iSecond);
			for (psListed = mapsFree[TRUE_MALLOCATOR_LIST(iFirst, iSecond)]; psListed && (psListed != psBlock); psListed = psListed->psNextFree)
			{
			}
			if (!psListed)
			{
				return false;
			}
			uiNumFree++;
			bPreviousFree = true;
		}
		else
		{
			uiNumUsed++;
			uiUsedBytes += uiSize;
			bPreviousFree = false;
		}
	}
	if ((((mpsEnd->uiSize & TRUE_MALLOCATOR_PREVIOUS_FREE) != 0) != bPreviousFree) || (mpsEnd->uiSize & ~(lsize)TRUE_MALLOCATOR_PREVIOUS_FREE))
	{
		return false;
	}
	if ((uiNumUsed != muiNumAllocations) || (uiUsedBytes != muiAllocatedBytes))
	{
		return false;
	}

	uiNumListed = 0;
	for (i = 0; i < TRUE_MALLOCATOR_FIRST_LEVELS; i++)
	{
		if (((muiFirstBitmap >> i) & 1) != (uint32)(mauiSecondBitmaps[i] != 0))
		{
			return false;
		}
		for (j = 0; j < TRUE_MALLOCATOR_SECOND_LEVELS; j++)
		{
			if (((mauiSecondBitmaps[i] >> j) & 1) != (uint32)(mapsFree[TRUE_MALLOCATOR_LIST(i, j)] != NULL))
			{
				return false;
			}
			for (psBlock = mapsFree[TRUE_MALLOCATOR_LIST(i, j)]; psBlock; psBlock = psBlock->psNextFree)
			{
				if (!(psBlock->uiSize & TRUE_MALLOCATOR_FREE) || (psBlock->psNextFree && (psBlock->psNextFree->psPreviousFree != psBlock)))
				{
					return false;
				}
				uiNumListed++;
			}
		}
	}
	return uiNumListed == uiNumFree;
}


//////////////////////////////////////////////////////////////////////////
//
// The size of a block for uiBytes of data, or 0 if it would be too big.
//
//////////////////////////////////////////////////////////////////////////
lsize CTrueMallocator::BlockSize(lsize uiBytes)
{
	lsize	uiSize;

	if (uiBytes > TRUE_MALLOCATOR_MAX_SIZE - TRUE_MALLOCATOR_HEADER - TRUE_MALLOCATOR_ALIGNMENT)
	{
		return 0;
	}
	uiSize = (uiBytes + TRUE_MALLOCATOR_HEADER + TRUE_MALLOCATOR_ALIGNMENT - 1) & ~(TRUE_MALLOCATOR_ALIGNMENT - 1);
	return (uiSize < TRUE_MALLOCATOR_MIN_BLOCK) ? TRUE_MALLOCATOR_MIN_BLOCK : uiSize;
}


//////////////////////////////////////////////////////////////////////////
//
// The free list a block of uiSize is on: the first level is the power of two of the size and the second which
// sixteenth of that power it is in.  Blocks smaller than TRUE_MALLOCATOR_SMALL_BLOCK are in the first level, on a
// list for each size.
//
//////////////////////////////////////////////////////////////////////////
void CTrueMallocator::Mapping(lsize uiSize, int* piFirst, int* piSecond)
{
	int		iHighest;
	uint16	uiHigh;

	if (uiSize < TRUE_MALLOCATOR_SMALL_BLOCK)
	{
		*piFirst = 0;
		*piSecond = (int)(uiSize >> TRUE_MALLOCATOR_ALIGNMENT_LOG2);
	}
	else if (uiSize <= 0xFFFF)
	{
		//Shifted in 16 bits (a 32 bit shift by a count is a runtime call on the W65C816).
		iHighest = TrueMallocatorHighestBit16((uint16)uiSize);
		*piFirst = iHighest - TRUE_MALLOCATOR_FIRST_SHIFT + 1;
		*piSecond = (int)((uint16)((uint16)uiSize >> (iHighest - TRUE_MALLOCATOR_SECOND_LOG2)) ^ TRUE_MALLOCATOR_SECOND_LEVELS);
	}
	else
	{
		//Its 16 bit high word's highest bit, and the 4 bits below the size's highest bit shifted in 16 bits: from the
		//high word when they are all in it (the highest bit 20 or more), else from the size >> 8 (which fits).
		uiHigh = (uint16)(uiSize >> 16);
		iHighest = 16 + TrueMallocatorHighestBit16(uiHigh);
		*piFirst = iHighest - TRUE_MALLOCATOR_FIRST_SHIFT + 1;
		if (iHighest >= 20)
		{
			*piSecond = (int)((uint16)(uiHigh >> (iHighest - 20)) ^ TRUE_MALLOCATOR_SECOND_LEVELS);
		}
		else
		{
			*piSecond = (int)((uint16)((uint16)(uiSize >> 8) >> (iHighest - 12)) ^ TRUE_MALLOCATOR_SECOND_LEVELS);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A free block of at least uiSize, or NULL.  The size is rounded up to the next list's so every block on the list
// that is found fits.  If no list from there on has a block, the blocks on uiSize's own list (which can be smaller)
// are looked through for one that is big enough.
//
//////////////////////////////////////////////////////////////////////////
STrueMallocatorBlock* CTrueMallocator::FindFree(lsize uiSize)
{
	STrueMallocatorBlock*	psBlock;
	uint32					uiMap;
	uint16					uiSecondMap;
	lsize					uiRounded;
	int						iFirst;
	int						iSecond;

	uiRounded = uiSize;
	if (uiSize >= TRUE_MALLOCATOR_SMALL_BLOCK)
	{
		uiRounded += ((lsize)1 << (TrueMallocatorSizeHighestBit(uiSize) - TRUE_MALLOCATOR_SECOND_LOG2)) - 1;
	}
	Mapping(uiRounded, &iFirst, &iSecond);
	if (iFirst < TRUE_MALLOCATOR_FIRST_LEVELS)
	{
		uiSecondMap = (uint16)(mauiSecondBitmaps[iFirst] & (uint16)(0xFFFF << iSecond));
		if (uiSecondMap == 0)
		{
			uiMap = muiFirstBitmap & (~(uint32)0 << (iFirst + 1));
			if (uiMap != 0)
			{
				iFirst = TrueMallocatorLowestBit(uiMap);
				uiSecondMap = mauiSecondBitmaps[iFirst];
			}
		}
		if (uiSecondMap != 0)
		{
			iSecond = TrueMallocatorLowestBit16(uiSecondMap);
			return mapsFree[TRUE_MALLOCATOR_LIST(iFirst, iSecond)];
		}
	}

	Mapping(uiSize, &iFirst, &iSecond);
	for (psBlock = mapsFree[TRUE_MALLOCATOR_LIST(iFirst, iSecond)]; psBlock; psBlock = psBlock->psNextFree)
	{
		if ((psBlock->uiSize & ~(lsize)TRUE_MALLOCATOR_FLAGS) >= uiSize)
		{
			return psBlock;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTrueMallocator::InsertFree(STrueMallocatorBlock* psBlock)
{
	STrueMallocatorBlock*	psHead;
	int						iFirst;
	int						iSecond;
	int						iList;

	Mapping(psBlock->uiSize & ~(lsize)TRUE_MALLOCATOR_FLAGS, &iFirst, &iSecond);
	iList = TRUE_MALLOCATOR_LIST(iFirst, iSecond);
	psHead = mapsFree[iList];
	psBlock->psNextFree = psHead;
	psBlock->psPreviousFree = NULL;
	if (psHead)
	{
		psHead->psPreviousFree = psBlock;
	}
	mapsFree[iList] = psBlock;
	muiFirstBitmap |= (uint32)1 << iFirst;
	mauiSecondBitmaps[iFirst] |= (uint16)(1 << iSecond);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTrueMallocator::RemoveFree(STrueMallocatorBlock* psBlock)
{
	int		iFirst;
	int		iSecond;
	uint16	uiSecondMap;

	Mapping(psBlock->uiSize & ~(lsize)TRUE_MALLOCATOR_FLAGS, &iFirst, &iSecond);
	if (psBlock->psNextFree)
	{
		psBlock->psNextFree->psPreviousFree = psBlock->psPreviousFree;
	}
	if (psBlock->psPreviousFree)
	{
		psBlock->psPreviousFree->psNextFree = psBlock->psNextFree;
	}
	else
	{
		mapsFree[TRUE_MALLOCATOR_LIST(iFirst, iSecond)] = psBlock->psNextFree;
		if (!psBlock->psNextFree)
		{
			uiSecondMap = (uint16)(mauiSecondBitmaps[iFirst] & ~(1 << iSecond));
			mauiSecondBitmaps[iFirst] = uiSecondMap;
			if (uiSecondMap == 0)
			{
				muiFirstBitmap &= ~((uint32)1 << iFirst);
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The block after psBlock in memory.
//
//////////////////////////////////////////////////////////////////////////
STrueMallocatorBlock* CTrueMallocator::Next(STrueMallocatorBlock* psBlock)
{
	return (STrueMallocatorBlock*)((uint8*)psBlock + (psBlock->uiSize & ~(lsize)TRUE_MALLOCATOR_FLAGS));
}


//////////////////////////////////////////////////////////////////////////
//
// Makes psBlock a free block of uiSize (keeping whether the block before it is free) and tells the block after it.
// It is not put on a free list.
//
//////////////////////////////////////////////////////////////////////////
void CTrueMallocator::MarkFree(STrueMallocatorBlock* psBlock, lsize uiSize)
{
	STrueMallocatorBlock*	psNext;

	psBlock->uiSize = uiSize | TRUE_MALLOCATOR_FREE | (psBlock->uiSize & TRUE_MALLOCATOR_PREVIOUS_FREE);
	psNext = Next(psBlock);
	psNext->uiPreviousSize = uiSize;
	psNext->uiSize |= TRUE_MALLOCATOR_PREVIOUS_FREE;
}


//////////////////////////////////////////////////////////////////////////
//
// Cuts the used block psBlock down to uiSize when what is left over is big enough to be a block of its own.  The
// rest is freed (joined to the block after it if that is free).
//
//////////////////////////////////////////////////////////////////////////
void CTrueMallocator::Split(STrueMallocatorBlock* psBlock, lsize uiSize)
{
	STrueMallocatorBlock*	psRest;
	STrueMallocatorBlock*	psNext;
	lsize					uiTotal;
	lsize					uiRest;

	uiTotal = psBlock->uiSize & ~(lsize)TRUE_MALLOCATOR_FLAGS;
	if (uiTotal - uiSize < TRUE_MALLOCATOR_MIN_BLOCK)
	{
		return;
	}
	psBlock->uiSize = uiSize | (psBlock->uiSize & TRUE_MALLOCATOR_FLAGS);
	psRest = Next(psBlock);
	uiRest = uiTotal - uiSize;
	psRest->uiSize = uiRest;
	psNext = Next(psRest);
	if (psNext->uiSize & TRUE_MALLOCATOR_FREE)
	{
		RemoveFree(psNext);
		uiRest += psNext->uiSize & ~(lsize)TRUE_MALLOCATOR_FLAGS;
	}
	MarkFree(psRest, uiRest);
	InsertFree(psRest);
}

