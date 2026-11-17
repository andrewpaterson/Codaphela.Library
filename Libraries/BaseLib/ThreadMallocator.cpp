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
#ifndef __LIBC65__
#include <string.h>
#include "SystemMemory.h"
#include "ThreadMallocator.h"


CThreadMallocator										gcThreadMallocator;
THREAD_MALLOCATOR_PER_THREAD CThreadMallocatorArena*	gpcThreadMallocatorArena = NULL;

//Set when the running thread has given its arena back: it is ending.
static THREAD_MALLOCATOR_PER_THREAD bool				gbThreadMallocatorEnded = false;


//////////////////////////////////////////////////////////////////////////
//
// Each thread has one, made the first time it allocates and destroyed when it ends, which is when its arena is left
// for another thread.
//
//////////////////////////////////////////////////////////////////////////
struct SThreadMallocatorArenaHolder
{
	~SThreadMallocatorArenaHolder();
};


static thread_local SThreadMallocatorArenaHolder	gsThreadMallocatorArenaHolder;


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SThreadMallocatorArenaHolder::~SThreadMallocatorArenaHolder()
{
	CThreadMallocatorArena*		pcArena;

	pcArena = gpcThreadMallocatorArena;
	gpcThreadMallocatorArena = NULL;
	gbThreadMallocatorEnded = true;
	if (pcArena)
	{
		gcThreadMallocator.ReleaseArena(pcArena);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The memory is zeroed already (it has just come from the operating system).
//
//////////////////////////////////////////////////////////////////////////
void CThreadMallocatorArena::Init(void)
{
	CTrueMallocator::Init();
	memset(mapvFrontLists, 0, sizeof(mapvFrontLists));
	muiFrontBytes = 0;
	miFlushList = 0;
	muiNumChunks = 0;
	mpsFreeChunk = NULL;
	mpcNext = NULL;
	mbInUse = false;
	mpvRemote = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// uiBytes for this arena's thread when the front list they would come from (iList, -1 if they are too many for one)
// is empty.  First what other threads freed is taken back, which may put something on that list; then the arena's
// own blocks are tried, then another chunk.
//
//////////////////////////////////////////////////////////////////////////
void* CThreadMallocatorArena::Malloc(lsize uiBytes, int iList)
{
	void*	pv;

	if (TakeRemote() && (iList >= 0))
	{
		pv = PopFront(iList);
		if (pv)
		{
			return pv;
		}
	}

	pv = CTrueMallocator::Malloc(uiBytes);
	if (pv)
	{
		return pv;
	}

	//The front lists may be holding what would make a block this big.
	if (muiFrontBytes > 0)
	{
		FlushFront(0);
		pv = CTrueMallocator::Malloc(uiBytes);
		if (pv)
		{
			return pv;
		}
	}

	if (!AddChunk())
	{
		return NULL;
	}
	return CTrueMallocator::Malloc(uiBytes);
}


//////////////////////////////////////////////////////////////////////////
//
// The blocks other threads freed, onto the front lists (those that go there).  Returns whether there were any.
//
//////////////////////////////////////////////////////////////////////////
bool CThreadMallocatorArena::TakeRemote(void)
{
	void*	pv;
	void*	pvNext;

	if (!AtomicLoadPointer(&mpvRemote))
	{
		return false;
	}

	pv = AtomicExchangePointer(&mpvRemote, NULL);
	while (pv)
	{
		pvNext = *(void**)pv;
		Free(pv);
		pv = pvNext;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Empties front lists back into the arena, from where the last flush stopped, until they hold uiBytesLeft or less.
//
//////////////////////////////////////////////////////////////////////////
void CThreadMallocatorArena::FlushFront(lsize uiBytesLeft)
{
	void*	pv;
	void*	pvNext;
	int		i;

	for (i = 0; (i < THREAD_MALLOCATOR_FRONT_LISTS) && (muiFrontBytes > uiBytesLeft); i++)
	{
		pv = mapvFrontLists[miFlushList];
		mapvFrontLists[miFlushList] = NULL;
		while (pv)
		{
			pvNext = *(void**)pv;
			muiFrontBytes -= ThreadMallocatorBlock(pv);
			FreeToChunk(pv);
			pv = pvNext;
		}

		miFlushList++;
		if (miFlushList == THREAD_MALLOCATOR_FRONT_LISTS)
		{
			miFlushList = 0;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Frees pv into the arena's own lists, where it is joined to the free blocks either side of it.  If that leaves its
// chunk with nothing allocated the chunk is kept, unless one is being kept already: then it goes back to the
// operating system.  (Keeping one stops a thread whose memory rises and falls across a chunk's end from mapping and
// unmapping a chunk each time.)
//
//////////////////////////////////////////////////////////////////////////
void CThreadMallocatorArena::FreeToChunk(void* pv)
{
	SThreadMallocatorChunk*		psChunk;

	CTrueMallocator::Free(pv);

	psChunk = ThreadMallocatorChunk(pv);
	if (IsRegionFree(psChunk + 1) && (psChunk != mpsFreeChunk))
	{
		if (mpsFreeChunk && IsRegionFree(mpsFreeChunk + 1))
		{
			RemoveRegion(psChunk + 1);
			muiNumChunks--;
			SystemMemoryUnmap(psChunk, psChunk->uiBytes);
		}
		else
		{
			mpsFreeChunk = psChunk;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CThreadMallocatorArena::AddChunk(void)
{
	SThreadMallocatorChunk*		psChunk;

	psChunk = (SThreadMallocatorChunk*)SystemMemoryMap(THREAD_MALLOCATOR_CHUNK, THREAD_MALLOCATOR_CHUNK);
	if (!psChunk)
	{
		return false;
	}

	psChunk->pcArena = this;
	psChunk->uiBytes = THREAD_MALLOCATOR_CHUNK;
	muiNumChunks++;
	return AddRegion(psChunk + 1, THREAD_MALLOCATOR_CHUNK - sizeof(SThreadMallocatorChunk));
}


//////////////////////////////////////////////////////////////////////////
//
// pv, a block of this arena's, with room for uiBytes, for this arena's thread: where it is if it has the room or the
// block after it is free and makes the room, else moved within the arena.  NULL if it can't be done here (the caller
// allocates afresh and copies).  A block much bigger than uiBytes is cut down.
//
//////////////////////////////////////////////////////////////////////////
void* CThreadMallocatorArena::Realloc(void* pv, lsize uiBytes)
{
	lsize	uiHas;

	uiHas = ThreadMallocatorBlock(pv) - TRUE_MALLOCATOR_HEADER;
	if ((uiBytes <= uiHas) && (uiBytes >= uiHas / 2))
	{
		return pv;
	}
	return CTrueMallocator::Realloc(pv, uiBytes);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the arena's blocks and lists are consistent, and the front lists hold what they say, each block on the
// list for its size.  Only while no other thread is freeing into it.
//
//////////////////////////////////////////////////////////////////////////
bool CThreadMallocatorArena::IsValid(void)
{
	void*	pv;
	lsize	uiFrontBytes;
	lsize	uiBlock;
	int		i;

	if (muiNumChunks == 0)
	{
		return muiFrontBytes == 0;
	}
	if (!CTrueMallocator::IsValid())
	{
		return false;
	}

	uiFrontBytes = 0;
	for (i = 0; i < THREAD_MALLOCATOR_FRONT_LISTS; i++)
	{
		for (pv = mapvFrontLists[i]; pv; pv = *(void**)pv)
		{
			uiBlock = ThreadMallocatorBlock(pv);
			if ((ThreadMallocatorFrontList(uiBlock) != i) || (ThreadMallocatorChunk(pv)->pcArena != this))
			{
				return false;
			}
			uiFrontBytes += uiBlock;
		}
	}
	return uiFrontBytes == muiFrontBytes;
}


//////////////////////////////////////////////////////////////////////////
//
// The running thread has no arena, or wants more than an arena gives.
//
//////////////////////////////////////////////////////////////////////////
void* CThreadMallocator::MallocWithoutArena(lsize uiBytes)
{
	CThreadMallocatorArena*		pcArena;
	void*						pv;

	if (uiBytes > THREAD_MALLOCATOR_ARENA_BYTES)
	{
		return MallocBlockOfItsOwn(uiBytes);
	}

	pcArena = TakeArena();
	if (!pcArena)
	{
		return NULL;
	}

	if (gbThreadMallocatorEnded)
	{
		//The thread is ending and has given its arena back already (something destroyed after the arena's holder is
		//allocating).  It borrows one for this allocation only: nothing would give back one it kept.
		gpcThreadMallocatorArena = pcArena;
		pv = Malloc(uiBytes);
		gpcThreadMallocatorArena = NULL;
		ReleaseArena(pcArena);
		return pv;
	}

	//Touching the holder is what makes it, and so what has it destroyed when the thread ends.
	(void)&gsThreadMallocatorArenaHolder;
	gpcThreadMallocatorArena = pcArena;
	return Malloc(uiBytes);
}


//////////////////////////////////////////////////////////////////////////
//
// An arena no thread has, or a new one.
//
//////////////////////////////////////////////////////////////////////////
CThreadMallocatorArena* CThreadMallocator::TakeArena(void)
{
	CThreadMallocatorArena*		pcArena;

	mcLock.Lock();
	for (pcArena = mpcArenas; pcArena; pcArena = pcArena->mpcNext)
	{
		if (!pcArena->mbInUse)
		{
			break;
		}
	}
	if (!pcArena)
	{
		pcArena = (CThreadMallocatorArena*)SystemMemoryMap(SystemMemoryPages(sizeof(CThreadMallocatorArena)), SYSTEM_MEMORY_PAGE);
		if (pcArena)
		{
			pcArena->Init();
			pcArena->mpcNext = mpcArenas;
			mpcArenas = pcArena;
		}
	}
	if (pcArena)
	{
		pcArena->mbInUse = true;
	}
	mcLock.Unlock();
	return pcArena;
}


//////////////////////////////////////////////////////////////////////////
//
// The thread that had pcArena is ending.  What is allocated in it stays where it is, and is freed into it (its
// remote list) by whichever threads free it, for the thread that takes the arena next.
//
//////////////////////////////////////////////////////////////////////////
void CThreadMallocator::ReleaseArena(CThreadMallocatorArena* pcArena)
{
	mcLock.Lock();
	pcArena->mbInUse = false;
	mcLock.Unlock();
}


//////////////////////////////////////////////////////////////////////////
//
// More than an arena gives: a kept block that is about the size, or memory of its own from the operating system.
//
//////////////////////////////////////////////////////////////////////////
void* CThreadMallocator::MallocBlockOfItsOwn(lsize uiBytes)
{
	SThreadMallocatorChunk*		psChunk;
	STrueMallocatorBlock*		psBlock;
	lsize						uiTotal;

	if (uiBytes > (lsize)0 - 2 * THREAD_MALLOCATOR_CHUNK)
	{
		return NULL;
	}

	uiTotal = SystemMemoryPages(sizeof(SThreadMallocatorChunk) + TRUE_MALLOCATOR_HEADER + uiBytes);
	psChunk = TakeKept(uiTotal);
	if (!psChunk)
	{
		psChunk = (SThreadMallocatorChunk*)SystemMemoryMap(uiTotal, THREAD_MALLOCATOR_CHUNK);
		if (!psChunk)
		{
			return NULL;
		}
		psChunk->pcArena = THREAD_MALLOCATOR_NO_ARENA;
		psChunk->uiBytes = uiTotal;
	}

	//A block's header, so its size is found as any block's is.
	psBlock = (STrueMallocatorBlock*)(psChunk + 1);
	psBlock->uiSize = psChunk->uiBytes - sizeof(SThreadMallocatorChunk);
	return (uint8*)psBlock + TRUE_MALLOCATOR_HEADER;
}


//////////////////////////////////////////////////////////////////////////
//
// The smallest kept block of at least uiBytes in all, if it is less than twice that; NULL if there is none.
//
//////////////////////////////////////////////////////////////////////////
SThreadMallocatorChunk* CThreadMallocator::TakeKept(lsize uiBytes)
{
	SThreadMallocatorChunk*		psChunk;
	size						uiBest;
	size						i;

	uiBest = THREAD_MALLOCATOR_KEPT_BLOCKS;
	mcKeptLock.Lock();
	for (i = 0; i < THREAD_MALLOCATOR_KEPT_BLOCKS; i++)
	{
		psChunk = mapsKept[i];
		if (psChunk && (psChunk->uiBytes >= uiBytes) && (psChunk->uiBytes / 2 < uiBytes))
		{
			if ((uiBest == THREAD_MALLOCATOR_KEPT_BLOCKS) || (psChunk->uiBytes < mapsKept[uiBest]->uiBytes))
			{
				uiBest = i;
			}
		}
	}

	psChunk = NULL;
	if (uiBest != THREAD_MALLOCATOR_KEPT_BLOCKS)
	{
		psChunk = mapsKept[uiBest];
		mapsKept[uiBest] = NULL;
		muiKeptBytes -= psChunk->uiBytes;
	}
	mcKeptLock.Unlock();
	return psChunk;
}


//////////////////////////////////////////////////////////////////////////
//
// A block of its own is freed: it is kept, in place of the kept blocks whose turn it is to go when there are too
// many or they are too much.  One too big to keep goes straight back to the operating system.
//
//////////////////////////////////////////////////////////////////////////
void CThreadMallocator::FreeBlockOfItsOwn(SThreadMallocatorChunk* psChunk)
{
	SThreadMallocatorChunk*		apsGoing[THREAD_MALLOCATOR_KEPT_BLOCKS];
	size						uiNumGoing;
	size						uiSlot;
	size						i;

	if (psChunk->uiBytes > THREAD_MALLOCATOR_KEPT_BYTES / 2)
	{
		SystemMemoryUnmap(psChunk, psChunk->uiBytes);
		return;
	}

	uiNumGoing = 0;
	uiSlot = THREAD_MALLOCATOR_KEPT_BLOCKS;
	mcKeptLock.Lock();
	for (i = 0; i < THREAD_MALLOCATOR_KEPT_BLOCKS; i++)
	{
		if (!mapsKept[i])
		{
			uiSlot = i;
		}
	}
	while ((uiSlot == THREAD_MALLOCATOR_KEPT_BLOCKS) || (muiKeptBytes + psChunk->uiBytes > THREAD_MALLOCATOR_KEPT_BYTES))
	{
		if (mapsKept[muiKeptNext])
		{
			apsGoing[uiNumGoing] = mapsKept[muiKeptNext];
			uiNumGoing++;
			muiKeptBytes -= mapsKept[muiKeptNext]->uiBytes;
			mapsKept[muiKeptNext] = NULL;
			uiSlot = muiKeptNext;
		}
		muiKeptNext = (muiKeptNext + 1) % THREAD_MALLOCATOR_KEPT_BLOCKS;
	}
	mapsKept[uiSlot] = psChunk;
	muiKeptBytes += psChunk->uiBytes;
	mcKeptLock.Unlock();

	for (i = 0; i < uiNumGoing; i++)
	{
		SystemMemoryUnmap(apsGoing[i], apsGoing[i]->uiBytes);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
lsize CThreadMallocator::KeptBytes(void)
{
	lsize	uiKeptBytes;

	mcKeptLock.Lock();
	uiKeptBytes = muiKeptBytes;
	mcKeptLock.Unlock();
	return uiKeptBytes;
}


//////////////////////////////////////////////////////////////////////////
//
// The bytes pv's memory can hold: what was asked for, rounded up.  From any thread.
//
//////////////////////////////////////////////////////////////////////////
lsize CThreadMallocator::Size(void* pv)
{
	STrueMallocatorBlock*	psBlock;

	//The block's flags can be changing under this (its arena's thread, as the block before it is freed and used).
	psBlock = (STrueMallocatorBlock*)((uint8*)pv - TRUE_MALLOCATOR_HEADER);
	return (AtomicLoadSize(&psBlock->uiSize) & ~(lsize)TRUE_MALLOCATOR_FLAGS) - TRUE_MALLOCATOR_HEADER;
}


//////////////////////////////////////////////////////////////////////////
//
// pv's memory with room for uiBytes, keeping what is in it (as much as fits), from any thread.  It stays where it is
// when it can.  NULL pv is Malloc; no bytes is Free, and returns NULL.  Returns NULL, leaving pv as it was, if there
// is no room.
//
//////////////////////////////////////////////////////////////////////////
void* CThreadMallocator::Realloc(void* pv, lsize uiBytes)
{
	CThreadMallocatorArena*		pcArena;
	lsize						uiHas;
	void*						pvNew;

	if (!pv)
	{
		return Malloc(uiBytes);
	}
	if (uiBytes == 0)
	{
		Free(pv);
		return NULL;
	}

	pcArena = ThreadMallocatorChunk(pv)->pcArena;
	if ((pcArena == gpcThreadMallocatorArena) && (uiBytes <= THREAD_MALLOCATOR_ARENA_BYTES))
	{
		pvNew = pcArena->Realloc(pv, uiBytes);
		if (pvNew)
		{
			return pvNew;
		}
	}

	//Another thread's block, a block of its own, or one its arena has no room to grow: as it is if that is big enough
	//and not much too big, else moved.
	uiHas = Size(pv);
	if ((uiBytes <= uiHas) && (uiBytes >= uiHas / 2))
	{
		return pv;
	}
	pvNew = Malloc(uiBytes);
	if (pvNew)
	{
		memcpy(pvNew, pv, (uiBytes < uiHas) ? uiBytes : uiHas);
		Free(pv);
	}
	return pvNew;
}


//////////////////////////////////////////////////////////////////////////
//
// Empties the running thread's front lists, and what other threads have freed into its arena, into the arena, so
// that its counts are of what is really allocated.
//
//////////////////////////////////////////////////////////////////////////
void CThreadMallocator::Flush(void)
{
	CThreadMallocatorArena*		pcArena;

	pcArena = gpcThreadMallocatorArena;
	if (pcArena)
	{
		pcArena->TakeRemote();
		pcArena->FlushFront(0);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The blocks allocated in the running thread's arena: what its front lists hold counts, and what other threads have
// freed and it hasn't taken back, until Flush.
//
//////////////////////////////////////////////////////////////////////////
lsize CThreadMallocator::NumThreadAllocations(void)
{
	CThreadMallocatorArena*		pcArena;

	pcArena = gpcThreadMallocatorArena;
	return pcArena ? pcArena->NumAllocations() : 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
lsize CThreadMallocator::NumThreadChunks(void)
{
	CThreadMallocatorArena*		pcArena;

	pcArena = gpcThreadMallocatorArena;
	return pcArena ? pcArena->muiNumChunks : 0;
}


//////////////////////////////////////////////////////////////////////////
//
// Only while no other thread is freeing what this thread allocated.
//
//////////////////////////////////////////////////////////////////////////
bool CThreadMallocator::IsThreadArenaValid(void)
{
	CThreadMallocatorArena*		pcArena;

	pcArena = gpcThreadMallocatorArena;
	return pcArena ? pcArena->IsValid() : true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
lsize CThreadMallocator::NumArenas(void)
{
	CThreadMallocatorArena*		pcArena;
	lsize						uiNumArenas;

	uiNumArenas = 0;
	mcLock.Lock();
	for (pcArena = mpcArenas; pcArena; pcArena = pcArena->mpcNext)
	{
		uiNumArenas++;
	}
	mcLock.Unlock();
	return uiNumArenas;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
lsize CThreadMallocator::NumArenasInUse(void)
{
	CThreadMallocatorArena*		pcArena;
	lsize						uiNumArenas;

	uiNumArenas = 0;
	mcLock.Lock();
	for (pcArena = mpcArenas; pcArena; pcArena = pcArena->mpcNext)
	{
		if (pcArena->mbInUse)
		{
			uiNumArenas++;
		}
	}
	mcLock.Unlock();
	return uiNumArenas;
}


#endif // !__LIBC65__

