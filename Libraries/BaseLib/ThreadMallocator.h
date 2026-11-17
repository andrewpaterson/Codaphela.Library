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
#ifndef __THREAD_MALLOCATOR_H__
#define __THREAD_MALLOCATOR_H__
#ifndef __LIBC65__
#include "Atomic.h"
#include "SpinLock.h"
#include "TrueMallocator.h"


//malloc, realloc and free for any number of threads, built on CTrueMallocator.  gcSystemAllocator uses it.
//
//Every thread that allocates has an arena: a CTrueMallocator over chunks of memory from the operating system, another
//chunk added when they are full.  Only its thread allocates from an arena or frees into it, so nothing there is
//locked.
//In front of each arena is a list of free blocks for each size (the front lists: 16 sizes to each power of two, up to
//THREAD_MALLOCATOR_FRONT_BLOCK).  A block freed goes onto the front of its list and the next of that size wanted
//comes off it: a push and a pop, with no joining of blocks or splitting them.  What the front lists hold
//is limited: past THREAD_MALLOCATOR_FRONT_BYTES lists are emptied back into the arena, in turn, so sizes that are no
//longer wanted don't stay there.
//A block's arena is found from its address: chunks start on THREAD_MALLOCATOR_CHUNK boundaries and a chunk begins by
//naming its arena.  A block freed by another thread than its arena's is pushed onto the arena's remote list (a
//compare and exchange, no lock); the arena's thread takes that whole list back when one of its front lists is empty.
//Blocks over THREAD_MALLOCATOR_ARENA_BYTES are each a piece of memory of their own from the operating system.  The
//last few freed are kept (THREAD_MALLOCATOR_KEPT_BLOCKS of them, THREAD_MALLOCATOR_KEPT_BYTES at most) and handed out
//again to whoever next wants about as much: asking the operating system for memory, and its handing it over a page
//at a time as each page is first written, costs far more than anything else here.
//A thread that ends leaves its arena, with whatever is still allocated in it, to the next thread that starts.
//
//Memory from malloc is not this allocator's and must go back to free (and this allocator's must come back here):
//Free reports a pointer it didn't allocate, when it can tell.
//
//All zeros is its starting state, so it works before anything has run and needs no Init.  It is never killed: its
//memory lasts as long as the process.  It isn't built for the 65C64 or the W65C816, which use a CTrueMallocator alone.

#define THREAD_MALLOCATOR_CHUNK_LOG2	22
#define THREAD_MALLOCATOR_CHUNK			((lsize)1 << THREAD_MALLOCATOR_CHUNK_LOG2)
#define THREAD_MALLOCATOR_ARENA_BYTES	(THREAD_MALLOCATOR_CHUNK / 4)		//The most bytes asked for that come from an arena.
#define THREAD_MALLOCATOR_FRONT_LOG2	16
#define THREAD_MALLOCATOR_FRONT_BLOCK	((lsize)1 << THREAD_MALLOCATOR_FRONT_LOG2)	//The biggest block (with its header) a front list keeps.
#define THREAD_MALLOCATOR_MIN_LOG2		(TRUE_MALLOCATOR_ALIGNMENT_LOG2 + 1)		//TRUE_MALLOCATOR_MIN_BLOCK's power of two.
#define THREAD_MALLOCATOR_FRONT_LISTS	(((THREAD_MALLOCATOR_FRONT_LOG2 - THREAD_MALLOCATOR_MIN_LOG2) << 4) + 1)
#define THREAD_MALLOCATOR_FRONT_BYTES	((lsize)2 << 20)					//The most an arena's front lists hold between them.
#define THREAD_MALLOCATOR_KEPT_BLOCKS	8
#define THREAD_MALLOCATOR_KEPT_BYTES	((lsize)64 << 20)					//The most the kept blocks are between them.


#ifdef _MSC_VER
#define THREAD_MALLOCATOR_PER_THREAD	__declspec(thread)
#else
#define THREAD_MALLOCATOR_PER_THREAD	__thread
#endif // _MSC_VER


class CThreadMallocatorArena;

//What a chunk starts with.  A block bigger than an arena gives out starts with one too (whose arena is
//THREAD_MALLOCATOR_NO_ARENA) and then a block's header, so its size is found as any block's is.
struct SThreadMallocatorChunk
{
	CThreadMallocatorArena*		pcArena;
	lsize						uiBytes;		//All of it, this included: what is given back to the operating system.
	lsize						uiMagic;		//THREAD_MALLOCATOR_MAGIC: what tells memory from here from memory given to Free by mistake.
	lsize						uiUnused;		//So that what follows is aligned as blocks are, in 32 bits and in 64.
};


#define THREAD_MALLOCATOR_MAGIC			((lsize)0x7EADC0DA)


//Never a thread's arena, so a block of its own is never taken for one of the freeing thread's.
#define THREAD_MALLOCATOR_NO_ARENA		((CThreadMallocatorArena*)1)


class CThreadMallocatorArena : protected CTrueMallocator
{
friend class CThreadMallocator;
protected:
	void*						mapvFrontLists[THREAD_MALLOCATOR_FRONT_LISTS];	//Each a list of free blocks through their first word.
	lsize						muiFrontBytes;		//The blocks on the front lists, with their headers.
	int							miFlushList;		//Where emptying the front lists stopped last time.
	lsize						muiNumChunks;
	SThreadMallocatorChunk*		mpsFreeChunk;		//A chunk with nothing allocated in it, kept; NULL if there isn't one.
	CThreadMallocatorArena*		mpcNext;			//Every arena there is.  Under CThreadMallocator's lock, as is mbInUse.
	bool						mbInUse;			//A thread has it.
	void*						mpvRemote;			//Blocks freed by other threads, a list through their first word.

public:
	void	Init(void);

protected:
	void*	Malloc(lsize uiBytes, int iList);
	void	Free(void* pv);
	void*	Realloc(void* pv, lsize uiBytes);

	void	PushFront(void* pv, lsize uiBlock, int iList);
	void*	PopFront(int iList);
	void	FreeRemote(void* pv);
	bool	TakeRemote(void);
	void	FlushFront(lsize uiBytesLeft);
	bool	AddChunk(void);
	void	FreeToChunk(void* pv);
	bool	IsValid(void);
};


class CThreadMallocator
{
protected:
	CSpinLock					mcLock;			//Over the list of arenas.
	CThreadMallocatorArena*		mpcArenas;
	CSpinLock					mcKeptLock;		//Over the kept blocks.
	SThreadMallocatorChunk*		mapsKept[THREAD_MALLOCATOR_KEPT_BLOCKS];	//Freed blocks of their own; NULL where there is none.
	lsize						muiKeptBytes;
	size						muiKeptNext;	//The one that goes next when room is wanted, in turn.

public:
	void*	Malloc(lsize uiBytes);
	void*	Realloc(void* pv, lsize uiBytes);
	void	Free(void* pv);
	lsize	Size(void* pv);

	void	Flush(void);
	lsize	NumThreadAllocations(void);
	lsize	NumThreadChunks(void);
	bool	IsThreadArenaValid(void);
	lsize	NumArenas(void);
	lsize	NumArenasInUse(void);
	lsize	KeptBytes(void);

	void	ReleaseArena(CThreadMallocatorArena* pcArena);

protected:
	CThreadMallocatorArena*		TakeArena(void);
	void*						MallocWithoutArena(lsize uiBytes);
	void*						MallocBlockOfItsOwn(lsize uiBytes);
	void						FreeBlockOfItsOwn(SThreadMallocatorChunk* psChunk);
	SThreadMallocatorChunk*		TakeKept(lsize uiBytes);
	void						NotAllocatedHere(void* pv);
};


extern CThreadMallocator										gcThreadMallocator;
extern THREAD_MALLOCATOR_PER_THREAD CThreadMallocatorArena*		gpcThreadMallocatorArena;	//The running thread's; NULL until it allocates.


//////////////////////////////////////////////////////////////////////////
//
// The chunk pv's block is in (or, for a block of its own, what it starts with).
//
//////////////////////////////////////////////////////////////////////////
inline SThreadMallocatorChunk* ThreadMallocatorChunk(void* pv)
{
	return (SThreadMallocatorChunk*)((lsize)pv & ~(THREAD_MALLOCATOR_CHUNK - 1));
}


//////////////////////////////////////////////////////////////////////////
//
// The size of pv's block with its header.  Only for the thread of the block's arena, which is the one that changes
// the flags beside it: another thread asks CThreadMallocator::Size.
//
//////////////////////////////////////////////////////////////////////////
inline lsize ThreadMallocatorBlock(void* pv)
{
	return ((STrueMallocatorBlock*)((uint8*)pv - TRUE_MALLOCATOR_HEADER))->uiSize & ~(lsize)TRUE_MALLOCATOR_FLAGS;
}


//////////////////////////////////////////////////////////////////////////
//
// The highest set bit of ui, which isn't 0.
//
//////////////////////////////////////////////////////////////////////////
inline int ThreadMallocatorHighestBit(lsize ui)
{
#if defined(_MSC_VER)
	unsigned long	uiIndex;

#ifdef BUILD_64
	_BitScanReverse64(&uiIndex, ui);
#else
	_BitScanReverse(&uiIndex, ui);
#endif // BUILD_64
	return (int)uiIndex;
#elif defined(BUILD_64)
	return 63 - __builtin_clzll(ui);
#else
	return 31 - __builtin_clz(ui);
#endif // _MSC_VER
}


//////////////////////////////////////////////////////////////////////////
//
// The front list a block of uiBlock bytes (with its header) is kept on: which power of two its size is in and which
// sixteenth of that power, so a block is at least its list's size and less than a sixteenth more.  (The smallest
// powers have fewer sizes than sixteen: their other lists stay empty.)  There is no test of the size in this or in
// ThreadMallocatorBlockFor: sizes come in no order, and a test the processor can't guess costs more than all the rest.
//
//////////////////////////////////////////////////////////////////////////
inline int ThreadMallocatorFrontList(lsize uiBlock)
{
	int		iHighest;

	iHighest = ThreadMallocatorHighestBit(uiBlock);
	return ((iHighest - THREAD_MALLOCATOR_MIN_LOG2) << 4) + (int)((uiBlock >> (iHighest - 4)) & 15);
}


//////////////////////////////////////////////////////////////////////////
//
// The block (with its header) that uiBytes are given: the size of the front list it will come from and go back to,
// so that whatever is on that list is big enough.  Over THREAD_MALLOCATOR_FRONT_BLOCK it needn't be any list's size.
//
//////////////////////////////////////////////////////////////////////////
inline lsize ThreadMallocatorBlockFor(lsize uiBytes)
{
	lsize	uiBlock;
	lsize	uiStep;

	uiBlock = (uiBytes + TRUE_MALLOCATOR_HEADER + TRUE_MALLOCATOR_ALIGNMENT - 1) & ~(TRUE_MALLOCATOR_ALIGNMENT - 1);
	uiBlock = (uiBlock < TRUE_MALLOCATOR_MIN_BLOCK) ? TRUE_MALLOCATOR_MIN_BLOCK : uiBlock;

	//A sixteenth of its power of two, which for the smallest blocks is less than they are aligned to and rounds nothing.
	uiStep = (lsize)1 << (ThreadMallocatorHighestBit(uiBlock) - 4);
	return (uiBlock + uiStep - 1) & ~(uiStep - 1);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
inline void* CThreadMallocatorArena::PopFront(int iList)
{
	void*	pv;

	pv = mapvFrontLists[iList];
	if (pv)
	{
		mapvFrontLists[iList] = *(void**)pv;
		muiFrontBytes -= ThreadMallocatorBlock(pv);
	}
	return pv;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
inline void CThreadMallocatorArena::PushFront(void* pv, lsize uiBlock, int iList)
{
	*(void**)pv = mapvFrontLists[iList];
	mapvFrontLists[iList] = pv;
	muiFrontBytes += uiBlock;
}


//////////////////////////////////////////////////////////////////////////
//
// uiBytes of memory, aligned to TRUE_MALLOCATOR_ALIGNMENT, or NULL if there is none.  Never NULL for no bytes.
//
//////////////////////////////////////////////////////////////////////////
inline void* CThreadMallocator::Malloc(lsize uiBytes)
{
	CThreadMallocatorArena*		pcArena;
	lsize						uiBlock;
	void*						pv;
	int							iList;

	pcArena = gpcThreadMallocatorArena;
	if (!pcArena || (uiBytes > THREAD_MALLOCATOR_ARENA_BYTES))
	{
		return MallocWithoutArena(uiBytes);
	}

	uiBlock = ThreadMallocatorBlockFor(uiBytes);
	if (uiBlock <= THREAD_MALLOCATOR_FRONT_BLOCK)
	{
		iList = ThreadMallocatorFrontList(uiBlock);
		pv = pcArena->PopFront(iList);
		if (pv)
		{
			return pv;
		}
		return pcArena->Malloc(uiBlock - TRUE_MALLOCATOR_HEADER, iList);
	}
	return pcArena->Malloc(uiBytes, -1);
}


//////////////////////////////////////////////////////////////////////////
//
// Gives pv's memory back, from any thread.  NULL is ignored.
//
//////////////////////////////////////////////////////////////////////////
inline void CThreadMallocator::Free(void* pv)
{
	SThreadMallocatorChunk*		psChunk;
	CThreadMallocatorArena*		pcArena;

	if (!pv)
	{
		return;
	}

	psChunk = ThreadMallocatorChunk(pv);
	pcArena = psChunk->pcArena;
	if (pcArena == gpcThreadMallocatorArena)
	{
		pcArena->Free(pv);
	}
	else if (psChunk->uiMagic != THREAD_MALLOCATOR_MAGIC)
	{
		NotAllocatedHere(pv);
	}
	else if (pcArena == THREAD_MALLOCATOR_NO_ARENA)
	{
		FreeBlockOfItsOwn(psChunk);
	}
	else
	{
		pcArena->FreeRemote(pv);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A block of this arena's freed by its own thread.
//
//////////////////////////////////////////////////////////////////////////
inline void CThreadMallocatorArena::Free(void* pv)
{
	lsize	uiBlock;

	uiBlock = ThreadMallocatorBlock(pv);
	if (uiBlock <= THREAD_MALLOCATOR_FRONT_BLOCK)
	{
		if (muiFrontBytes >= THREAD_MALLOCATOR_FRONT_BYTES)
		{
			FlushFront(THREAD_MALLOCATOR_FRONT_BYTES / 2);
		}
		PushFront(pv, uiBlock, ThreadMallocatorFrontList(uiBlock));
	}
	else
	{
		FreeToChunk(pv);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A block of this arena's freed by another thread: onto the remote list, for this arena's thread to take back.
//
//////////////////////////////////////////////////////////////////////////
inline void CThreadMallocatorArena::FreeRemote(void* pv)
{
	void*	pvHead;

	pvHead = AtomicLoadPointer(&mpvRemote);
	do
	{
		*(void**)pv = pvHead;
	}
	while (!AtomicCompareExchangePointer(&mpvRemote, &pvHead, pv));
}


#endif // !__LIBC65__
#endif // __THREAD_MALLOCATOR_H__

