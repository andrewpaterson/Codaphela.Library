#include <string.h>
#include "BaseLib/TrueMallocator.h"
#include "BaseLib/StdRandom.h"
#include "TestLib/Assert.h"


#define TEST_TRUE_MALLOCATOR_MEMORY		0x10000
#define TEST_TRUE_MALLOCATOR_ALLOCATIONS	200
#define TEST_TRUE_MALLOCATOR_LARGE_MEMORY	0x30000


uint8	gauiTestTrueMallocatorMemory[TEST_TRUE_MALLOCATOR_MEMORY];
uint8	gauiTestTrueMallocatorLargeMemory[TEST_TRUE_MALLOCATOR_LARGE_MEMORY];


//////////////////////////////////////////////////////////////////////////
//
// All of the memory is one free block to start with, and is again when everything is freed.
//
//////////////////////////////////////////////////////////////////////////
void TestTrueMallocatorInit(void)
{
	CTrueMallocator	cMallocator;
	lsize			uiLargest;

	AssertTrue(cMallocator.Init(gauiTestTrueMallocatorMemory, TEST_TRUE_MALLOCATOR_MEMORY));
	AssertTrue(cMallocator.IsValid());
	AssertSize(0, cMallocator.NumAllocations());
	AssertSize(0, cMallocator.AllocatedBytes());
	uiLargest = cMallocator.LargestFree();
	AssertTrue(uiLargest >= TEST_TRUE_MALLOCATOR_MEMORY - 4 * TRUE_MALLOCATOR_ALIGNMENT);
	AssertTrue(uiLargest < TEST_TRUE_MALLOCATOR_MEMORY);
	AssertTrue(cMallocator.Contains(&gauiTestTrueMallocatorMemory[100]));
	AssertFalse(cMallocator.Contains(&gauiTestTrueMallocatorMemory[TEST_TRUE_MALLOCATOR_MEMORY - 1]));
	cMallocator.Kill();

	AssertFalse(cMallocator.Init(gauiTestTrueMallocatorMemory, 8));
}


//////////////////////////////////////////////////////////////////////////
//
// Memory is aligned, doesn't overlap, keeps what is written to it and is all given back.
//
//////////////////////////////////////////////////////////////////////////
void TestTrueMallocatorMallocFree(void)
{
	CTrueMallocator	cMallocator;
	uint8*			apui[10];
	size			uiLargest;
	size			i;
	size			j;
	bool			bAligned;
	bool			bKept;

	cMallocator.Init(gauiTestTrueMallocatorMemory, TEST_TRUE_MALLOCATOR_MEMORY);
	uiLargest = cMallocator.LargestFree();

	bAligned = true;
	for (i = 0; i < 10; i++)
	{
		apui[i] = (uint8*)cMallocator.Malloc((lsize)i * 13);
		AssertNotNull(apui[i]);
		bAligned &= ((ptr)apui[i] & (TRUE_MALLOCATOR_ALIGNMENT - 1)) == 0;
		AssertTrue(cMallocator.Size(apui[i]) >= i * 13);
		memset(apui[i], (int)i, (lsize)i * 13);
	}
	AssertTrue(bAligned);
	AssertSize(10, cMallocator.NumAllocations());
	AssertTrue(cMallocator.IsValid());

	bKept = true;
	for (i = 0; i < 10; i++)
	{
		for (j = 0; j < i * 13; j++)
		{
			bKept &= apui[i][j] == i;
		}
	}
	AssertTrue(bKept);

	//Every other one, then the rest, so freed blocks are joined to free blocks on both sides.
	for (i = 0; i < 10; i += 2)
	{
		cMallocator.Free(apui[i]);
		AssertTrue(cMallocator.IsValid());
	}
	for (i = 1; i < 10; i += 2)
	{
		cMallocator.Free(apui[i]);
		AssertTrue(cMallocator.IsValid());
	}
	cMallocator.Free(NULL);
	AssertSize(0, cMallocator.NumAllocations());
	AssertSize(0, cMallocator.AllocatedBytes());
	AssertSize(uiLargest, cMallocator.LargestFree());

	cMallocator.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Memory runs out, and what doesn't fit (or is too big to ask for) is NULL.
//
//////////////////////////////////////////////////////////////////////////
void TestTrueMallocatorFull(void)
{
	CTrueMallocator	cMallocator;
	void*			pv;
	void*			pvLast;
	size			uiCount;

	cMallocator.Init(gauiTestTrueMallocatorMemory, TEST_TRUE_MALLOCATOR_MEMORY);
	AssertNull(cMallocator.Malloc(TEST_TRUE_MALLOCATOR_MEMORY));
	AssertNull(cMallocator.Malloc((size)-1));
	AssertNull(cMallocator.Malloc((size)-1 - 4));
	AssertNull(cMallocator.Malloc(0x7FFFFFFF));
	AssertNotNull(cMallocator.Malloc(cMallocator.LargestFree()));
	AssertNull(cMallocator.Malloc(1));
	AssertTrue(cMallocator.IsValid());
	cMallocator.Kill();

	cMallocator.Init(gauiTestTrueMallocatorMemory, TEST_TRUE_MALLOCATOR_MEMORY);
	uiCount = 0;
	pvLast = NULL;
	for (;;)
	{
		pv = cMallocator.Malloc(100);
		if (!pv)
		{
			break;
		}
		pvLast = pv;
		uiCount++;
	}
	AssertTrue(uiCount > TEST_TRUE_MALLOCATOR_MEMORY / 160);
	AssertTrue(cMallocator.IsValid());
	cMallocator.Free(pvLast);
	AssertNotNull(cMallocator.Malloc(100));
	AssertTrue(cMallocator.IsValid());
	cMallocator.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Realloc shrinks in place, grows in place into a free block after it, moves when it has to and keeps what was in
// the memory.
//
//////////////////////////////////////////////////////////////////////////
void TestTrueMallocatorRealloc(void)
{
	CTrueMallocator	cMallocator;
	uint8*			pui;
	uint8*			puiAfter;
	uint8*			puiMoved;
	uint8*			puiBlocker;
	size			i;
	bool			bKept;

	cMallocator.Init(gauiTestTrueMallocatorMemory, TEST_TRUE_MALLOCATOR_MEMORY);
	pui = (uint8*)cMallocator.Realloc(NULL, 100);
	AssertNotNull(pui);
	for (i = 0; i < 100; i++)
	{
		pui[i] = (uint8)i;
	}

	//Shrinking stays where it is.
	AssertPointer(pui, cMallocator.Realloc(pui, 40));
	AssertTrue(cMallocator.IsValid());

	//Growing into the free memory after it stays where it is too.
	AssertPointer(pui, cMallocator.Realloc(pui, 1000));
	AssertTrue(cMallocator.IsValid());
	bKept = true;
	for (i = 0; i < 40; i++)
	{
		bKept &= pui[i] == i;
	}
	AssertTrue(bKept);

	//With a used block after it, growing moves it.
	puiBlocker = (uint8*)cMallocator.Malloc(16);
	puiMoved = (uint8*)cMallocator.Realloc(pui, 2000);
	AssertNotNull(puiMoved);
	AssertFalse(puiMoved == pui);
	bKept = true;
	for (i = 0; i < 40; i++)
	{
		bKept &= puiMoved[i] == i;
	}
	AssertTrue(bKept);
	AssertSize(2, cMallocator.NumAllocations());
	AssertTrue(cMallocator.IsValid());

	//The block freed by the move is used again.
	puiAfter = (uint8*)cMallocator.Malloc(500);
	AssertPointer(pui, puiAfter);

	//Too big leaves the memory as it was.
	AssertNull(cMallocator.Realloc(puiMoved, TEST_TRUE_MALLOCATOR_MEMORY));
	AssertTrue(puiMoved[39] == 39);
	AssertNull(cMallocator.Realloc(puiMoved, (size)-1));

	cMallocator.Free(puiAfter);
	cMallocator.Free(puiBlocker);
	cMallocator.Free(puiMoved);
	AssertSize(0, cMallocator.NumAllocations());
	AssertTrue(cMallocator.IsValid());
	cMallocator.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Random allocations, reallocations and frees keep the blocks consistent and the memory unchanged.  When everything
// is freed the memory is one block again.
//
//////////////////////////////////////////////////////////////////////////
void TestTrueMallocatorRandom(void)
{
	CTrueMallocator	cMallocator;
	CRandom			cRandom;
	uint8*			apui[TEST_TRUE_MALLOCATOR_ALLOCATIONS];
	size			auiSizes[TEST_TRUE_MALLOCATOR_ALLOCATIONS];
	size			uiLargest;
	size			uiSize;
	size			i;
	size			j;
	size			k;
	int				iAction;
	bool			bValid;
	bool			bKept;
	bool			bWritten;
	uint8*			pui;

	cMallocator.Init(gauiTestTrueMallocatorMemory, TEST_TRUE_MALLOCATOR_MEMORY);
	uiLargest = cMallocator.LargestFree();
	cRandom.Init(1234);
	for (i = 0; i < TEST_TRUE_MALLOCATOR_ALLOCATIONS; i++)
	{
		apui[i] = NULL;
		auiSizes[i] = 0;
	}

	bValid = true;
	bKept = true;
	for (k = 0; k < 3000; k++)
	{
		i = (size)cRandom.Next(0, TEST_TRUE_MALLOCATOR_ALLOCATIONS - 1);
		iAction = cRandom.Next(0, 2);
		uiSize = (size)cRandom.Next(0, (cRandom.Next(0, 9) == 0) ? 3000 : 200);
		if (apui[i])
		{
			for (j = 0; j < auiSizes[i]; j++)
			{
				bKept &= apui[i][j] == (uint8)(i + j);
			}
		}

		bWritten = false;
		if (!apui[i] || (iAction == 0))
		{
			cMallocator.Free(apui[i]);
			apui[i] = (uint8*)cMallocator.Malloc(uiSize);
			bWritten = apui[i] != NULL;
		}
		else if (iAction == 1)
		{
			pui = (uint8*)cMallocator.Realloc(apui[i], uiSize);
			if (pui)
			{
				apui[i] = pui;
				bWritten = true;
			}
		}
		else
		{
			cMallocator.Free(apui[i]);
			apui[i] = NULL;
		}

		if (bWritten)
		{
			for (j = 0; j < uiSize; j++)
			{
				apui[i][j] = (uint8)(i + j);
			}
		}
		auiSizes[i] = bWritten ? uiSize : (apui[i] ? auiSizes[i] : 0);
		if ((k % 50) == 0)
		{
			bValid &= cMallocator.IsValid();
		}
	}
	AssertTrue(bValid);
	AssertTrue(bKept);
	AssertTrue(cMallocator.IsValid());

	for (i = 0; i < TEST_TRUE_MALLOCATOR_ALLOCATIONS; i++)
	{
		cMallocator.Free(apui[i]);
	}
	AssertSize(0, cMallocator.NumAllocations());
	AssertSize(uiLargest, cMallocator.LargestFree());
	AssertTrue(cMallocator.IsValid());

	cRandom.Kill();
	cMallocator.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Blocks and free space bigger than 64KB.  Sizes are lsize: with the W65C816's 16 bit size they wrapped and
// the heap overwrote itself.
//
//////////////////////////////////////////////////////////////////////////
void TestTrueMallocatorLarge(void)
{
	CTrueMallocator	cMallocator;
	uint8*			pui1;
	uint8*			pui2;
	uint8*			pui3;
	lsize			uiLargest;

	AssertTrue(cMallocator.Init(gauiTestTrueMallocatorLargeMemory, TEST_TRUE_MALLOCATOR_LARGE_MEMORY));
	uiLargest = cMallocator.LargestFree();
	AssertTrue(uiLargest > 0x20000);

	pui1 = (uint8*)cMallocator.Malloc(100000);
	AssertNotNull(pui1);
	AssertTrue(cMallocator.Size(pui1) >= 100000);
	memset(pui1, 0x5A, 100000);
	pui2 = (uint8*)cMallocator.Malloc(70000);
	AssertNotNull(pui2);
	memset(pui2, 0xA5, 70000);
	AssertTrue((pui2 >= pui1 + 100000) || (pui2 + 70000 <= pui1));
	AssertTrue(cMallocator.IsValid());
	AssertLSize(2, cMallocator.NumAllocations());
	AssertTrue(cMallocator.AllocatedBytes() >= 170000);
	AssertInt(0x5A, pui1[99999]);
	AssertInt(0xA5, pui2[69999]);

	//Too big for what is left.
	AssertNull(cMallocator.Malloc(100000));

	//Freeing the first gives back its 100000 bytes; growing the second keeps what it had.
	cMallocator.Free(pui1);
	AssertTrue(cMallocator.LargestFree() >= 100000);
	pui3 = (uint8*)cMallocator.Malloc(90000);
	AssertNotNull(pui3);
	cMallocator.Free(pui3);
	pui2 = (uint8*)cMallocator.Realloc(pui2, 90000);
	AssertNotNull(pui2);
	AssertInt(0xA5, pui2[0]);
	AssertInt(0xA5, pui2[69999]);
	AssertTrue(cMallocator.IsValid());

	cMallocator.Free(pui2);
	AssertLSize(0, cMallocator.NumAllocations());
	AssertLSize(0, cMallocator.AllocatedBytes());
	AssertLSize(uiLargest, cMallocator.LargestFree());
	cMallocator.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestTrueMallocator(void)
{
	BeginTests();

	TestTrueMallocatorInit();
	TestTrueMallocatorMallocFree();
	TestTrueMallocatorFull();
	TestTrueMallocatorRealloc();
	TestTrueMallocatorRandom();
	TestTrueMallocatorLarge();

	TestStatistics();
}

