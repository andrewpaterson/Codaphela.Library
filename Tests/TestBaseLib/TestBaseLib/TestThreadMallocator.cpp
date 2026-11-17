//CThreadMallocator isn't built for the 65C64 or the W65C816, whose compiler is given every test's source.
#ifndef __LIBC65__
#include <string.h>
#include "BaseLib/Atomic.h"
#include "BaseLib/NativeThread.h"
#include "BaseLib/StdRandom.h"
#include "BaseLib/SystemMemory.h"
#include "BaseLib/ThreadMallocator.h"
#include "TestLib/Assert.h"


#define TEST_THREAD_MALLOCATOR_THREADS		8
#define TEST_THREAD_MALLOCATOR_SLOTS		1024
#define TEST_THREAD_MALLOCATOR_STEPS		20000


//What a test block starts with, so whoever frees it can tell it is whole.
struct STestThreadMallocatorBlock
{
	lsize	uiBytes;
	uint32	uiSeed;
	uint32	uiMagic;
};


#define TEST_THREAD_MALLOCATOR_MAGIC	0xC0DAFE1A


//////////////////////////////////////////////////////////////////////////
//
// Fills pv (uiBytes of it, at least a STestThreadMallocatorBlock) with bytes that follow from uiSeed.  Past the first
// 256 only some are written, enough to find a block that was cut short or written over.
//
//////////////////////////////////////////////////////////////////////////
static void TestThreadMallocatorFill(void* pv, lsize uiBytes, uint32 uiSeed)
{
	STestThreadMallocatorBlock*		psBlock;
	uint8*							pui;
	lsize							uiCount;
	lsize							i;

	psBlock = (STestThreadMallocatorBlock*)pv;
	psBlock->uiBytes = uiBytes;
	psBlock->uiSeed = uiSeed;
	psBlock->uiMagic = TEST_THREAD_MALLOCATOR_MAGIC;

	pui = (uint8*)(psBlock + 1);
	uiCount = uiBytes - sizeof(STestThreadMallocatorBlock);
	for (i = 0; i < uiCount; i += (i < 256) ? 1 : 61)
	{
		pui[i] = (uint8)(uiSeed + i);
	}
	if (uiCount > 0)
	{
		pui[uiCount - 1] = (uint8)(uiSeed + uiCount - 1);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Whether pv is as TestThreadMallocatorFill left it, as far as uiUpTo bytes of what follows its start (a block that
// was reallocated smaller keeps only that much).
//
//////////////////////////////////////////////////////////////////////////
static bool TestThreadMallocatorIsFilled(void* pv, lsize uiUpTo)
{
	STestThreadMallocatorBlock*		psBlock;
	uint8*							pui;
	lsize							uiCount;
	lsize							i;

	psBlock = (STestThreadMallocatorBlock*)pv;
	if ((psBlock->uiMagic != TEST_THREAD_MALLOCATOR_MAGIC) || (((ptr)pv & (TRUE_MALLOCATOR_ALIGNMENT - 1)) != 0))
	{
		return false;
	}

	pui = (uint8*)(psBlock + 1);
	uiCount = psBlock->uiBytes - sizeof(STestThreadMallocatorBlock);
	if (uiCount > uiUpTo)
	{
		uiCount = uiUpTo;
	}
	else if ((uiCount > 0) && (pui[uiCount - 1] != (uint8)(psBlock->uiSeed + uiCount - 1)))
	{
		return false;
	}
	for (i = 0; i < uiCount; i += (i < 256) ? 1 : 61)
	{
		if (pui[i] != (uint8)(psBlock->uiSeed + i))
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A size for a test block: mostly small, some up to 20 KB, a few up to 300 KB and now and then one too big for an
// arena.
//
//////////////////////////////////////////////////////////////////////////
static lsize TestThreadMallocatorRandomBytes(CRandom* pcRandom)
{
	uint32	uiKind;

	uiKind = (uint32)pcRandom->Next(0, 999);
	if (uiKind < 700)
	{
		return sizeof(STestThreadMallocatorBlock) + (lsize)pcRandom->Next(0, 599);
	}
	if (uiKind < 950)
	{
		return sizeof(STestThreadMallocatorBlock) + (lsize)pcRandom->Next(0, 19999);
	}
	if (uiKind < 998)
	{
		return sizeof(STestThreadMallocatorBlock) + (lsize)pcRandom->Next(0, 299999);
	}
	return sizeof(STestThreadMallocatorBlock) + THREAD_MALLOCATOR_ARENA_BYTES + (lsize)pcRandom->Next(0, (int32)(2 * THREAD_MALLOCATOR_ARENA_BYTES));
}


//////////////////////////////////////////////////////////////////////////
//
// A bigger block is never on an earlier front list, every block that is kept has a list, and the block a request is
// given is big enough and the smallest on its list (so whatever else is on that list is big enough too).
//
//////////////////////////////////////////////////////////////////////////
void TestThreadMallocatorFrontLists(void)
{
	lsize	uiBlock;
	lsize	uiBytes;
	int		iList;
	int		iPrevious;
	int		iOutOfOrder;
	int		iNotSmallest;
	int		iTooSmall;

	iOutOfOrder = 0;
	iPrevious = 0;
	for (uiBlock = TRUE_MALLOCATOR_MIN_BLOCK; uiBlock <= THREAD_MALLOCATOR_FRONT_BLOCK; uiBlock += TRUE_MALLOCATOR_ALIGNMENT)
	{
		iList = ThreadMallocatorFrontList(uiBlock);
		if ((iList < iPrevious) || (iList >= THREAD_MALLOCATOR_FRONT_LISTS))
		{
			iOutOfOrder++;
		}
		iPrevious = iList;
	}
	AssertInt(0, iOutOfOrder);
	AssertInt(0, ThreadMallocatorFrontList(TRUE_MALLOCATOR_MIN_BLOCK));
	AssertInt(THREAD_MALLOCATOR_FRONT_LISTS - 1, ThreadMallocatorFrontList(THREAD_MALLOCATOR_FRONT_BLOCK));

	iNotSmallest = 0;
	iTooSmall = 0;
	for (uiBytes = 0; uiBytes <= THREAD_MALLOCATOR_FRONT_BLOCK - TRUE_MALLOCATOR_HEADER; uiBytes += 7)
	{
		uiBlock = ThreadMallocatorBlockFor(uiBytes);
		if ((uiBlock < uiBytes + TRUE_MALLOCATOR_HEADER) || (uiBlock < TRUE_MALLOCATOR_MIN_BLOCK) || (uiBlock & (TRUE_MALLOCATOR_ALIGNMENT - 1)))
		{
			iTooSmall++;
		}
		if ((uiBlock > TRUE_MALLOCATOR_MIN_BLOCK) && (ThreadMallocatorFrontList(uiBlock - TRUE_MALLOCATOR_ALIGNMENT) == ThreadMallocatorFrontList(uiBlock)))
		{
			iNotSmallest++;
		}
	}
	AssertInt(0, iTooSmall);
	AssertInt(0, iNotSmallest);
}


//////////////////////////////////////////////////////////////////////////
//
// Memory is aligned, big enough, keeps what is written to it and is all given back; a block just freed is the next
// of its size handed out.
//
//////////////////////////////////////////////////////////////////////////
void TestThreadMallocatorMallocFree(void)
{
	void*	apv[64];
	void*	pv;
	lsize	uiBytes;
	lsize	uiAllocations;
	size	i;
	bool	bFilled;
	bool	bBigEnough;

	gcThreadMallocator.Flush();
	uiAllocations = gcThreadMallocator.NumThreadAllocations();

	gcThreadMallocator.Free(NULL);
	pv = gcThreadMallocator.Malloc(0);
	AssertNotNull(pv);
	gcThreadMallocator.Free(pv);

	bBigEnough = true;
	for (i = 0; i < 64; i++)
	{
		uiBytes = sizeof(STestThreadMallocatorBlock) + (lsize)i * i * 37;
		apv[i] = gcThreadMallocator.Malloc(uiBytes);
		AssertNotNull(apv[i]);
		bBigEnough &= gcThreadMallocator.Size(apv[i]) >= uiBytes;
		TestThreadMallocatorFill(apv[i], uiBytes, (uint32)i);
	}
	AssertTrue(bBigEnough);
	AssertTrue(gcThreadMallocator.IsThreadArenaValid());

	bFilled = true;
	for (i = 0; i < 64; i++)
	{
		bFilled &= TestThreadMallocatorIsFilled(apv[i], (lsize)-1);
	}
	AssertTrue(bFilled);

	//The last one freed of a size is the first one of that size handed out.
	gcThreadMallocator.Free(apv[10]);
	gcThreadMallocator.Free(apv[11]);
	AssertPointer(apv[11], gcThreadMallocator.Malloc(sizeof(STestThreadMallocatorBlock) + 11 * 11 * 37));
	AssertPointer(apv[10], gcThreadMallocator.Malloc(sizeof(STestThreadMallocatorBlock) + 10 * 10 * 37));

	for (i = 0; i < 64; i++)
	{
		gcThreadMallocator.Free(apv[i]);
	}
	AssertTrue(gcThreadMallocator.IsThreadArenaValid());
	gcThreadMallocator.Flush();
	AssertTrue(gcThreadMallocator.IsThreadArenaValid());
	AssertLSize(uiAllocations, gcThreadMallocator.NumThreadAllocations());
}


//////////////////////////////////////////////////////////////////////////
//
// Realloc keeps what is in the memory, stays where it is when the block has the room, cuts a block down that is much
// too big, and works across the size where a block becomes one of its own.
//
//////////////////////////////////////////////////////////////////////////
void TestThreadMallocatorRealloc(void)
{
	void*	pv;
	void*	pvSame;
	lsize	uiBytes;
	lsize	uiAllocations;
	uint32	uiSeed;

	gcThreadMallocator.Flush();
	uiAllocations = gcThreadMallocator.NumThreadAllocations();

	pv = gcThreadMallocator.Realloc(NULL, 100);
	AssertNotNull(pv);
	TestThreadMallocatorFill(pv, 100, 1);

	//Within what the block has.
	pvSame = gcThreadMallocator.Realloc(pv, 90);
	AssertPointer(pv, pvSame);
	pvSame = gcThreadMallocator.Realloc(pv, gcThreadMallocator.Size(pv));
	AssertPointer(pv, pvSame);
	AssertTrue(TestThreadMallocatorIsFilled(pv, (lsize)-1));

	//Growing, from a few bytes to more than an arena gives and on.
	uiSeed = 2;
	for (uiBytes = 200; uiBytes <= 4 * THREAD_MALLOCATOR_ARENA_BYTES; uiBytes = uiBytes * 2 + 3)
	{
		pv = gcThreadMallocator.Realloc(pv, uiBytes);
		AssertNotNull(pv);
		AssertTrue(gcThreadMallocator.Size(pv) >= uiBytes);
		AssertTrue(TestThreadMallocatorIsFilled(pv, (lsize)-1));
		TestThreadMallocatorFill(pv, uiBytes, uiSeed);
		uiSeed++;
	}

	//And shrinking back: what is kept is still there, and the block doesn't stay big.
	for (uiBytes = uiBytes / 4; uiBytes >= 64; uiBytes /= 4)
	{
		pv = gcThreadMallocator.Realloc(pv, uiBytes);
		AssertNotNull(pv);
		AssertTrue(gcThreadMallocator.Size(pv) >= uiBytes);
		AssertTrue(gcThreadMallocator.Size(pv) < uiBytes * 2 + 2 * TRUE_MALLOCATOR_MIN_BLOCK);
		AssertTrue(TestThreadMallocatorIsFilled(pv, uiBytes - sizeof(STestThreadMallocatorBlock)));
		TestThreadMallocatorFill(pv, uiBytes, uiSeed);
		uiSeed++;
	}
	AssertTrue(gcThreadMallocator.IsThreadArenaValid());

	//No bytes frees.
	AssertNull(gcThreadMallocator.Realloc(pv, 0));
	gcThreadMallocator.Flush();
	AssertLSize(uiAllocations, gcThreadMallocator.NumThreadAllocations());
}


//////////////////////////////////////////////////////////////////////////
//
// More than a chunk's worth is allocated from more chunks, and the chunks go back when what was in them is freed
// (but one that is kept).
//
//////////////////////////////////////////////////////////////////////////
void TestThreadMallocatorChunks(void)
{
	void*	apv[200];
	lsize	uiChunks;
	lsize	uiAllocations;
	size	i;

	gcThreadMallocator.Flush();
	uiAllocations = gcThreadMallocator.NumThreadAllocations();
	uiChunks = gcThreadMallocator.NumThreadChunks();

	//20 MB in 100 KB blocks.
	for (i = 0; i < 200; i++)
	{
		apv[i] = gcThreadMallocator.Malloc(100000);
		TestThreadMallocatorFill(apv[i], 100000, (uint32)i);
	}
	AssertTrue(gcThreadMallocator.NumThreadChunks() >= 5);
	AssertTrue(gcThreadMallocator.IsThreadArenaValid());

	for (i = 0; i < 200; i++)
	{
		AssertTrue(TestThreadMallocatorIsFilled(apv[i], (lsize)-1));
		gcThreadMallocator.Free(apv[i]);
	}
	gcThreadMallocator.Flush();
	AssertTrue(gcThreadMallocator.IsThreadArenaValid());
	AssertLSize(uiAllocations, gcThreadMallocator.NumThreadAllocations());
	AssertTrue(gcThreadMallocator.NumThreadChunks() <= uiChunks + 1);
	AssertTrue(gcThreadMallocator.NumThreadChunks() >= 1);

	//The front lists don't hold more than their limit: 16 MB of small blocks freed leaves the arena consistent and,
	//once flushed, with everything back.
	for (i = 0; i < 200; i++)
	{
		apv[i] = gcThreadMallocator.Malloc(80000);
	}
	for (i = 0; i < 200; i++)
	{
		gcThreadMallocator.Free(apv[i]);
	}
	AssertTrue(gcThreadMallocator.IsThreadArenaValid());
	gcThreadMallocator.Flush();
	AssertLSize(uiAllocations, gcThreadMallocator.NumThreadAllocations());
}


//////////////////////////////////////////////////////////////////////////
//
// A block too big for an arena is kept when it is freed and is the next of about its size handed out; only so many
// are kept, and only so much.
//
//////////////////////////////////////////////////////////////////////////
void TestThreadMallocatorBlocksOfTheirOwn(void)
{
	void*	apv[THREAD_MALLOCATOR_KEPT_BLOCKS * 2];
	void*	pv;
	void*	pvAgain;
	lsize	uiBytes;
	lsize	uiKept;
	size	i;

	uiBytes = 3 * THREAD_MALLOCATOR_ARENA_BYTES;
	pv = gcThreadMallocator.Malloc(uiBytes);
	AssertNotNull(pv);
	AssertTrue(gcThreadMallocator.Size(pv) >= uiBytes);
	AssertTrue(((ptr)pv & (TRUE_MALLOCATOR_ALIGNMENT - 1)) == 0);
	TestThreadMallocatorFill(pv, uiBytes, 5);
	AssertTrue(TestThreadMallocatorIsFilled(pv, (lsize)-1));

	uiKept = gcThreadMallocator.KeptBytes();
	gcThreadMallocator.Free(pv);
	AssertTrue(gcThreadMallocator.KeptBytes() >= uiKept + uiBytes);

	//A little less is given the same block; a lot less isn't (it would waste most of it).
	pvAgain = gcThreadMallocator.Malloc(uiBytes - uiBytes / 4);
	AssertPointer(pv, pvAgain);
	AssertTrue(gcThreadMallocator.Size(pvAgain) >= uiBytes);
	gcThreadMallocator.Free(pvAgain);
	pvAgain = gcThreadMallocator.Malloc(uiBytes / 3 + THREAD_MALLOCATOR_ARENA_BYTES / 2);
	AssertNotNull(pvAgain);
	AssertFalse(pvAgain == pv);
	gcThreadMallocator.Free(pvAgain);

	//More freed than are kept, then more than is kept.
	for (i = 0; i < THREAD_MALLOCATOR_KEPT_BLOCKS * 2; i++)
	{
		apv[i] = gcThreadMallocator.Malloc(uiBytes + i * SYSTEM_MEMORY_PAGE * 4);
		AssertNotNull(apv[i]);
	}
	for (i = 0; i < THREAD_MALLOCATOR_KEPT_BLOCKS * 2; i++)
	{
		gcThreadMallocator.Free(apv[i]);
	}
	AssertTrue(gcThreadMallocator.KeptBytes() <= THREAD_MALLOCATOR_KEPT_BYTES);
	AssertTrue(gcThreadMallocator.KeptBytes() >= THREAD_MALLOCATOR_KEPT_BLOCKS * uiBytes);

	for (i = 0; i < 4; i++)
	{
		apv[i] = gcThreadMallocator.Malloc(THREAD_MALLOCATOR_KEPT_BYTES / 3);
		AssertNotNull(apv[i]);
	}
	for (i = 0; i < 4; i++)
	{
		gcThreadMallocator.Free(apv[i]);
	}
	AssertTrue(gcThreadMallocator.KeptBytes() <= THREAD_MALLOCATOR_KEPT_BYTES);
	AssertTrue(gcThreadMallocator.KeptBytes() >= 2 * (THREAD_MALLOCATOR_KEPT_BYTES / 3));

	//Too big to keep at all.
	uiKept = gcThreadMallocator.KeptBytes();
	pv = gcThreadMallocator.Malloc(THREAD_MALLOCATOR_KEPT_BYTES);
	AssertNotNull(pv);
	gcThreadMallocator.Free(pv);
	AssertLSize(uiKept, gcThreadMallocator.KeptBytes());
}


struct STestThreadMallocatorThread
{
	CNativeThread	cThread;
	uint32			uiSeed;
	void**			apvSlots;
	int32*			piStarted;	//How many of the threads have an arena.
	int				iErrors;
	lsize			uiLeft;		//What it left allocated in its arena.
};


//////////////////////////////////////////////////////////////////////////
//
// The thread takes its arena and waits until every thread has one, so that each has an arena of its own however
// quickly the first of them would have finished.
//
//////////////////////////////////////////////////////////////////////////
static void TestThreadMallocatorWaitForThreads(STestThreadMallocatorThread* psThread)
{
	gcThreadMallocator.Free(gcThreadMallocator.Malloc(1));
	AtomicAdd(psThread->piStarted, 1);
	while (AtomicLoad(psThread->piStarted) < TEST_THREAD_MALLOCATOR_THREADS)
	{
		ThreadYield();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A thread allocating, reallocating and freeing its own blocks.
//
//////////////////////////////////////////////////////////////////////////
static void TestThreadMallocatorOwnBlocksThread(void* pvThread)
{
	STestThreadMallocatorThread*	psThread;
	CRandom							cRandom;
	void*							apv[64];
	lsize							uiBytes;
	lsize							uiAllocations;
	size							uiSlot;
	size							i;

	psThread = (STestThreadMallocatorThread*)pvThread;
	TestThreadMallocatorWaitForThreads(psThread);
	gcThreadMallocator.Flush();
	uiAllocations = gcThreadMallocator.NumThreadAllocations();

	cRandom.Init(psThread->uiSeed);
	memset(apv, 0, sizeof(apv));
	for (i = 0; i < TEST_THREAD_MALLOCATOR_STEPS; i++)
	{
		uiSlot = (size)cRandom.Next(0, 63);
		if (apv[uiSlot])
		{
			if (!TestThreadMallocatorIsFilled(apv[uiSlot], (lsize)-1))
			{
				psThread->iErrors++;
			}
			gcThreadMallocator.Free(apv[uiSlot]);
		}
		uiBytes = TestThreadMallocatorRandomBytes(&cRandom);
		apv[uiSlot] = gcThreadMallocator.Malloc(uiBytes);
		TestThreadMallocatorFill(apv[uiSlot], uiBytes, (uint32)cRandom.Next(0, MAX_INT32));
	}
	for (i = 0; i < 64; i++)
	{
		gcThreadMallocator.Free(apv[i]);
	}
	if (!gcThreadMallocator.IsThreadArenaValid())
	{
		psThread->iErrors++;
	}
	gcThreadMallocator.Flush();
	psThread->uiLeft = gcThreadMallocator.NumThreadAllocations() - uiAllocations;
	cRandom.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A thread swapping a new block into a slot every thread shares and freeing (or first reallocating) the block it
// took out, which another thread most likely allocated.
//
//////////////////////////////////////////////////////////////////////////
static void TestThreadMallocatorSharedBlocksThread(void* pvThread)
{
	STestThreadMallocatorThread*	psThread;
	STestThreadMallocatorBlock		sWas;
	CRandom							cRandom;
	void*							pvNew;
	void*							pvOld;
	lsize							uiBytes;
	lsize							uiKept;
	size							i;

	psThread = (STestThreadMallocatorThread*)pvThread;
	TestThreadMallocatorWaitForThreads(psThread);

	cRandom.Init(psThread->uiSeed);
	for (i = 0; i < TEST_THREAD_MALLOCATOR_STEPS; i++)
	{
		uiBytes = TestThreadMallocatorRandomBytes(&cRandom);
		pvNew = gcThreadMallocator.Malloc(uiBytes);
		TestThreadMallocatorFill(pvNew, uiBytes, (uint32)cRandom.Next(0, MAX_INT32));

		pvOld = AtomicExchangePointer(&psThread->apvSlots[cRandom.Next(0, TEST_THREAD_MALLOCATOR_SLOTS - 1)], pvNew);
		if (pvOld)
		{
			if (!TestThreadMallocatorIsFilled(pvOld, (lsize)-1))
			{
				psThread->iErrors++;
			}
			if (cRandom.Next(0, 7) == 0)
			{
				sWas = *(STestThreadMallocatorBlock*)pvOld;
				uiBytes = TestThreadMallocatorRandomBytes(&cRandom);
				pvOld = gcThreadMallocator.Realloc(pvOld, uiBytes);
				uiKept = ((uiBytes < sWas.uiBytes) ? uiBytes : sWas.uiBytes) - sizeof(STestThreadMallocatorBlock);
				if ((gcThreadMallocator.Size(pvOld) < uiBytes) || !TestThreadMallocatorIsFilled(pvOld, uiKept))
				{
					psThread->iErrors++;
				}
			}
			gcThreadMallocator.Free(pvOld);
		}
	}
	cRandom.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Starts TEST_THREAD_MALLOCATOR_THREADS threads on fThread and waits for them.  Returns the errors they counted.
//
//////////////////////////////////////////////////////////////////////////
static int TestThreadMallocatorRunThreads(NativeThreadFunction fThread, uint32 uiSeed, void** apvSlots, lsize* puiLeft)
{
	STestThreadMallocatorThread		asThreads[TEST_THREAD_MALLOCATOR_THREADS];
	int32							iStarted;
	int								iErrors;
	size							i;

	iStarted = 0;
	for (i = 0; i < TEST_THREAD_MALLOCATOR_THREADS; i++)
	{
		asThreads[i].uiSeed = uiSeed + (uint32)i * 7919;
		asThreads[i].apvSlots = apvSlots;
		asThreads[i].piStarted = &iStarted;
		asThreads[i].iErrors = 0;
		asThreads[i].uiLeft = 0;
		asThreads[i].cThread.Init();
		asThreads[i].cThread.Start(fThread, &asThreads[i]);
	}

	iErrors = 0;
	*puiLeft = 0;
	for (i = 0; i < TEST_THREAD_MALLOCATOR_THREADS; i++)
	{
		asThreads[i].cThread.Join();
		asThreads[i].cThread.Kill();
		iErrors += asThreads[i].iErrors;
		*puiLeft += asThreads[i].uiLeft;
	}
	return iErrors;
}


//////////////////////////////////////////////////////////////////////////
//
// Threads allocate at once without disturbing each other, each from an arena of its own, and their arenas are there
// for the threads that start after they have ended.
//
//////////////////////////////////////////////////////////////////////////
void TestThreadMallocatorThreads(void)
{
	lsize	uiArenas;
	lsize	uiArenasInUse;
	lsize	uiLeft;

	gcThreadMallocator.Malloc(1);
	uiArenasInUse = gcThreadMallocator.NumArenasInUse();

	AssertInt(0, TestThreadMallocatorRunThreads(TestThreadMallocatorOwnBlocksThread, 100, NULL, &uiLeft));
	AssertLSize(0, uiLeft);
	AssertLSize(uiArenasInUse, gcThreadMallocator.NumArenasInUse());
	uiArenas = gcThreadMallocator.NumArenas();
	AssertTrue(uiArenas >= uiArenasInUse + TEST_THREAD_MALLOCATOR_THREADS);

	//The same number of threads again take up the arenas the first lot left.
	AssertInt(0, TestThreadMallocatorRunThreads(TestThreadMallocatorOwnBlocksThread, 200, NULL, &uiLeft));
	AssertLSize(0, uiLeft);
	AssertLSize(uiArenas, gcThreadMallocator.NumArenas());
	AssertLSize(uiArenasInUse, gcThreadMallocator.NumArenasInUse());
}


//////////////////////////////////////////////////////////////////////////
//
// Blocks allocated on one thread are freed and reallocated on others, whole, and what the threads leave behind when
// they end is freed by this one.
//
//////////////////////////////////////////////////////////////////////////
void TestThreadMallocatorFreedByOtherThreads(void)
{
	static void*	apvSlots[TEST_THREAD_MALLOCATOR_SLOTS];
	lsize			uiAllocations;
	lsize			uiLeft;
	size			i;
	bool			bFilled;

	gcThreadMallocator.Flush();
	uiAllocations = gcThreadMallocator.NumThreadAllocations();
	memset(apvSlots, 0, sizeof(apvSlots));

	AssertInt(0, TestThreadMallocatorRunThreads(TestThreadMallocatorSharedBlocksThread, 300, apvSlots, &uiLeft));
	AssertInt(0, TestThreadMallocatorRunThreads(TestThreadMallocatorSharedBlocksThread, 400, apvSlots, &uiLeft));

	bFilled = true;
	for (i = 0; i < TEST_THREAD_MALLOCATOR_SLOTS; i++)
	{
		if (apvSlots[i])
		{
			bFilled &= TestThreadMallocatorIsFilled(apvSlots[i], (lsize)-1);
			gcThreadMallocator.Free(apvSlots[i]);
		}
	}
	AssertTrue(bFilled);

	//None of those were this thread's.
	gcThreadMallocator.Flush();
	AssertTrue(gcThreadMallocator.IsThreadArenaValid());
	AssertLSize(uiAllocations, gcThreadMallocator.NumThreadAllocations());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestThreadMallocator(void)
{
	BeginTests();

	TestThreadMallocatorFrontLists();
	TestThreadMallocatorMallocFree();
	TestThreadMallocatorRealloc();
	TestThreadMallocatorChunks();
	TestThreadMallocatorBlocksOfTheirOwn();
	TestThreadMallocatorThreads();
	TestThreadMallocatorFreedByOtherThreads();

	TestStatistics();
}


#endif // !__LIBC65__

