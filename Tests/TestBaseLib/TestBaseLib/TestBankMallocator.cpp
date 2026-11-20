#include <string.h>
#include "BaseLib/BankMallocator.h"
#include "BaseLib/StdRandom.h"
#include "TestLib/Assert.h"


#define TEST_BANK_SIZE			1024
#define TEST_BANKS				16
#define TEST_BANK_ALLOCATIONS	64


//The banks, each starting at a multiple of TEST_BANK_SIZE (the memory is a bank longer so they can be).
uint8	gauiTestBankMemory[(TEST_BANKS + 1) * TEST_BANK_SIZE];


//A source that hands out the banks the test lets it, and remembers which the heap has: asking for one it has, or
//giving back one it hasn't, is wrong.
class CTestBankSource : public CBankSource
{
public:
	uint8*	mpuiFirst;
	bool	mabHeld[TEST_BANKS];
	bool	mabRefused[TEST_BANKS];
	size	muiAcquires;
	size	muiReleases;
	bool	mbWrong;

	void	Init(uint8* puiFirst);
	bool	Acquire(void* pvBank, size uiCount) override;
	void	Release(void* pvBank, size uiCount) override;
	size	NumHeld(void) const;
	size	Index(void* pv) const;
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTestBankSource::Init(uint8* puiFirst)
{
	size	i;

	mpuiFirst = puiFirst;
	for (i = 0; i < TEST_BANKS; i++)
	{
		mabHeld[i] = false;
		mabRefused[i] = false;
	}
	muiAcquires = 0;
	muiReleases = 0;
	mbWrong = false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTestBankSource::Acquire(void* pvBank, size uiCount)
{
	size	uiFirst;
	size	i;

	uiFirst = Index(pvBank);
	if ((uiCount == 0) || (uiFirst + uiCount > TEST_BANKS) || ((uint8*)pvBank != &mpuiFirst[uiFirst * TEST_BANK_SIZE]))
	{
		mbWrong = true;
		return false;
	}
	for (i = uiFirst; i < uiFirst + uiCount; i++)
	{
		if (mabHeld[i])
		{
			mbWrong = true;
			return false;
		}
		if (mabRefused[i])
		{
			return false;
		}
	}
	for (i = uiFirst; i < uiFirst + uiCount; i++)
	{
		mabHeld[i] = true;
	}
	muiAcquires++;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTestBankSource::Release(void* pvBank, size uiCount)
{
	size	uiFirst;
	size	i;

	uiFirst = Index(pvBank);
	for (i = uiFirst; i < uiFirst + uiCount; i++)
	{
		if ((i >= TEST_BANKS) || !mabHeld[i])
		{
			mbWrong = true;
			return;
		}
		mabHeld[i] = false;
	}
	muiReleases++;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CTestBankSource::NumHeld(void) const
{
	size	uiHeld;
	size	i;

	uiHeld = 0;
	for (i = 0; i < TEST_BANKS; i++)
	{
		uiHeld += mabHeld[i] ? 1 : 0;
	}
	return uiHeld;
}


size CTestBankSource::Index(void* pv) const { return (size)(((uint8*)pv - mpuiFirst) / TEST_BANK_SIZE); }


//////////////////////////////////////////////////////////////////////////
//
// The test's first bank: the first multiple of TEST_BANK_SIZE in its memory.
//
//////////////////////////////////////////////////////////////////////////
static uint8* TestBankFirst(void)
{
	return (uint8*)(((lsize)gauiTestBankMemory + TEST_BANK_SIZE - 1) & ~((lsize)TEST_BANK_SIZE - 1));
}


//////////////////////////////////////////////////////////////////////////
//
// No banks are the heap's until it needs them.
//
//////////////////////////////////////////////////////////////////////////
void TestBankMallocatorInit(void)
{
	CBankMallocator	cMallocator;
	CTestBankSource	cSource;
	uint8*			puiFirst;

	puiFirst = TestBankFirst();
	cSource.Init(puiFirst);
	AssertTrue(cMallocator.Init(puiFirst, TEST_BANKS * TEST_BANK_SIZE, TEST_BANK_SIZE, &cSource, 1, false));
	AssertSize(TEST_BANKS, cMallocator.NumBanks());
	AssertSize(0, cMallocator.NumHeld());
	AssertSize(0, (size)cMallocator.NumAllocations());
	AssertSize(TEST_BANK_SIZE - BANK_MALLOCATOR_OVERHEAD, (size)cMallocator.MaxSmall());
	AssertSize(0, cSource.muiAcquires);
	AssertTrue(cMallocator.IsValid());
	cMallocator.Kill();

	AssertFalse(cMallocator.Init(puiFirst, TEST_BANKS * TEST_BANK_SIZE, 1000, &cSource, 1, false));
	AssertFalse(cMallocator.Init(puiFirst, TEST_BANKS * TEST_BANK_SIZE, 32, &cSource, 1, false));
	AssertFalse(cMallocator.Init(puiFirst, 0, TEST_BANK_SIZE, &cSource, 1, false));
	AssertFalse(cSource.mbWrong);
}


//////////////////////////////////////////////////////////////////////////
//
// Small blocks fill the lowest bank, then the next is asked for; emptied
// banks go back but for the spare.
//
//////////////////////////////////////////////////////////////////////////
void TestBankMallocatorSmall(void)
{
	CBankMallocator	cMallocator;
	CTestBankSource	cSource;
	uint8*			puiFirst;
	uint8*			apui[TEST_BANK_ALLOCATIONS];
	size			uiCount;
	size			i;

	puiFirst = TestBankFirst();
	cSource.Init(puiFirst);
	cMallocator.Init(puiFirst, TEST_BANKS * TEST_BANK_SIZE, TEST_BANK_SIZE, &cSource, 1, false);

	apui[0] = (uint8*)cMallocator.Malloc(100);
	AssertTrue(apui[0] >= puiFirst);
	AssertTrue(apui[0] < puiFirst + TEST_BANK_SIZE);
	AssertTrue(cSource.mabHeld[0]);
	AssertSize(1, cSource.NumHeld());
	AssertSize(BANK_MALLOCATOR_SMALL, cMallocator.GetBank(0));

	//Blocks until a second bank is needed: it is bank 1, and no block crosses into it.
	for (uiCount = 1; (uiCount < TEST_BANK_ALLOCATIONS) && (cSource.NumHeld() == 1); uiCount++)
	{
		apui[uiCount] = (uint8*)cMallocator.Malloc(100);
		AssertNotNull(apui[uiCount]);
	}
	AssertSize(2, cSource.NumHeld());
	AssertTrue(cSource.mabHeld[1]);
	for (i = 0; i < uiCount; i++)
	{
		AssertTrue(cSource.Index(apui[i]) == cSource.Index(apui[i] + 99));
	}
	AssertTrue(cMallocator.IsValid());
	AssertSize(uiCount, (size)cMallocator.NumAllocations());

	//All freed: the first bank emptied is kept as the spare, the other goes back.
	for (i = 0; i < uiCount; i++)
	{
		cMallocator.Free(apui[i]);
		AssertTrue(cMallocator.IsValid());
	}
	AssertSize(1, cSource.NumHeld());
	AssertSize(1, cMallocator.NumHeld());
	AssertSize(0, (size)cMallocator.NumAllocations());

	//The spare is used before another bank is asked for.
	i = cSource.muiAcquires;
	apui[0] = (uint8*)cMallocator.Malloc(100);
	AssertSize(i, cSource.muiAcquires);
	cMallocator.Free(apui[0]);

	cMallocator.Kill();
	AssertSize(0, cSource.NumHeld());
	AssertFalse(cSource.mbWrong);
}


//////////////////////////////////////////////////////////////////////////
//
// More than a bank is a run of whole banks from the highest, asked for in
// one go; freed, its banks go back but for one kept as the spare.
//
//////////////////////////////////////////////////////////////////////////
void TestBankMallocatorLarge(void)
{
	CBankMallocator	cMallocator;
	CTestBankSource	cSource;
	uint8*			puiFirst;
	uint8*			pui;

	puiFirst = TestBankFirst();
	cSource.Init(puiFirst);
	cMallocator.Init(puiFirst, TEST_BANKS * TEST_BANK_SIZE, TEST_BANK_SIZE, &cSource, 1, false);

	pui = (uint8*)cMallocator.Malloc(3000);
	AssertPointer(&puiFirst[13 * TEST_BANK_SIZE], pui);
	AssertSize(1, cSource.muiAcquires);
	AssertSize(3, cSource.NumHeld());
	AssertSize(BANK_MALLOCATOR_LARGE, cMallocator.GetBank(13));
	AssertSize(BANK_MALLOCATOR_MORE, cMallocator.GetBank(14));
	AssertSize(BANK_MALLOCATOR_MORE, cMallocator.GetBank(15));
	AssertSize(3 * TEST_BANK_SIZE, (size)cMallocator.Size(pui));
	memset(pui, 0x5A, 3000);
	AssertTrue(cMallocator.IsValid());

	cMallocator.Free(pui);
	AssertSize(1, cSource.NumHeld());
	AssertTrue(cSource.mabHeld[13]);
	AssertSize(BANK_MALLOCATOR_SPARE, cMallocator.GetBank(13));
	AssertTrue(cMallocator.IsValid());
	cMallocator.Kill();
	AssertFalse(cSource.mbWrong);

	//Without spares they all go back.
	cSource.Init(puiFirst);
	cMallocator.Init(puiFirst, TEST_BANKS * TEST_BANK_SIZE, TEST_BANK_SIZE, &cSource, 0, false);
	pui = (uint8*)cMallocator.Malloc(2 * TEST_BANK_SIZE);
	AssertPointer(&puiFirst[14 * TEST_BANK_SIZE], pui);
	cMallocator.Free(pui);
	AssertSize(0, cSource.NumHeld());

	//Too big for the heap.
	AssertNull(cMallocator.Malloc((TEST_BANKS + 1) * TEST_BANK_SIZE));
	cMallocator.Kill();
	AssertFalse(cSource.mbWrong);
}


//////////////////////////////////////////////////////////////////////////
//
// A bank the source won't give is passed over.
//
//////////////////////////////////////////////////////////////////////////
void TestBankMallocatorRefused(void)
{
	CBankMallocator	cMallocator;
	CTestBankSource	cSource;
	uint8*			puiFirst;
	uint8*			puiSmall;
	uint8*			puiLarge;
	size			i;

	puiFirst = TestBankFirst();
	cSource.Init(puiFirst);
	cSource.mabRefused[0] = true;
	cSource.mabRefused[14] = true;
	cMallocator.Init(puiFirst, TEST_BANKS * TEST_BANK_SIZE, TEST_BANK_SIZE, &cSource, 1, false);

	puiSmall = (uint8*)cMallocator.Malloc(100);
	AssertSize(1, cSource.Index(puiSmall));

	//Three banks from the top would be 13 to 15 and then 12 to 14, both with bank 14: 11 to 13.
	puiLarge = (uint8*)cMallocator.Malloc(3 * TEST_BANK_SIZE);
	AssertPointer(&puiFirst[11 * TEST_BANK_SIZE], puiLarge);
	AssertTrue(cMallocator.IsValid());

	//Nothing at all to be had.
	for (i = 0; i < TEST_BANKS; i++)
	{
		cSource.mabRefused[i] = true;
	}
	AssertNull(cMallocator.Malloc(4 * TEST_BANK_SIZE));
	AssertNull(cMallocator.Malloc(TEST_BANK_SIZE - BANK_MALLOCATOR_OVERHEAD));

	cMallocator.Free(puiLarge);
	cMallocator.Free(puiSmall);
	AssertTrue(cMallocator.IsValid());
	cMallocator.Kill();
	AssertSize(0, cSource.NumHeld());
	AssertFalse(cSource.mbWrong);
}


//////////////////////////////////////////////////////////////////////////
//
// Reserved banks are never the heap's, and a run doesn't cross them.
//
//////////////////////////////////////////////////////////////////////////
void TestBankMallocatorReserved(void)
{
	CBankMallocator	cMallocator;
	CTestBankSource	cSource;
	uint8*			puiFirst;
	uint8*			apui[TEST_BANK_ALLOCATIONS];
	uint8*			puiLarge;
	size			uiCount;
	size			i;

	puiFirst = TestBankFirst();
	cSource.Init(puiFirst);
	cMallocator.Init(puiFirst, TEST_BANKS * TEST_BANK_SIZE, TEST_BANK_SIZE, &cSource, 1, false);
	AssertTrue(cMallocator.Reserve(&puiFirst[1 * TEST_BANK_SIZE], 2 * TEST_BANK_SIZE));
	AssertTrue(cMallocator.Reserve(&puiFirst[13 * TEST_BANK_SIZE + 10], 10));
	AssertSize(BANK_MALLOCATOR_RESERVED, cMallocator.GetBank(1));
	AssertSize(BANK_MALLOCATOR_RESERVED, cMallocator.GetBank(2));
	AssertSize(BANK_MALLOCATOR_RESERVED, cMallocator.GetBank(13));

	//Three banks' worth of small blocks: banks 0, 3 and 4.
	for (uiCount = 0; (uiCount < TEST_BANK_ALLOCATIONS) && (cSource.NumHeld() < 3); uiCount++)
	{
		apui[uiCount] = (uint8*)cMallocator.Malloc(400);
	}
	AssertTrue(cSource.mabHeld[0]);
	AssertTrue(cSource.mabHeld[3]);
	AssertTrue(cSource.mabHeld[4]);
	AssertFalse(cSource.mabHeld[1]);
	AssertFalse(cSource.mabHeld[2]);

	//Three banks from the top: 14 and 15 are two, then 13 is reserved, so 10 to 12.
	puiLarge = (uint8*)cMallocator.Malloc(3 * TEST_BANK_SIZE);
	AssertPointer(&puiFirst[10 * TEST_BANK_SIZE], puiLarge);

	//A bank the heap has can't be reserved.
	AssertFalse(cMallocator.Reserve(&puiFirst[10 * TEST_BANK_SIZE], 1));

	cMallocator.Free(puiLarge);
	for (i = 0; i < uiCount; i++)
	{
		cMallocator.Free(apui[i]);
	}
	AssertTrue(cMallocator.IsValid());
	cMallocator.Kill();
	AssertSize(0, cSource.NumHeld());
	AssertFalse(cSource.mbWrong);
}


//////////////////////////////////////////////////////////////////////////
//
// Memory that starts and ends part way through a bank: those parts take
// small blocks, and only whole banks a large allocation.
//
//////////////////////////////////////////////////////////////////////////
void TestBankMallocatorPartial(void)
{
	CBankMallocator	cMallocator;
	CTestBankSource	cSource;
	uint8*			puiFirst;
	uint8*			pui;
	uint8*			puiLarge;

	puiFirst = TestBankFirst();
	cSource.Init(puiFirst);
	AssertTrue(cMallocator.Init(&puiFirst[100], 4 * TEST_BANK_SIZE - 150, TEST_BANK_SIZE, &cSource, 0, false));
	AssertSize(4, cMallocator.NumBanks());

	pui = (uint8*)cMallocator.Malloc(100);
	AssertTrue(pui >= &puiFirst[100]);
	AssertTrue(pui < &puiFirst[TEST_BANK_SIZE]);
	AssertTrue(cSource.mabHeld[0]);

	//Banks 1 and 2 are the only two whole ones.
	puiLarge = (uint8*)cMallocator.Malloc(2 * TEST_BANK_SIZE);
	AssertPointer(&puiFirst[1 * TEST_BANK_SIZE], puiLarge);
	AssertNull(cMallocator.Malloc(2 * TEST_BANK_SIZE));

	//The first bank's part has room for one more of these, then the last bank's part is asked for.
	AssertNotNull(cMallocator.Malloc(600));
	AssertFalse(cSource.mabHeld[3]);
	AssertNotNull(cMallocator.Malloc(600));
	AssertTrue(cSource.mabHeld[3]);
	AssertNull(cMallocator.Malloc(TEST_BANK_SIZE - BANK_MALLOCATOR_OVERHEAD));
	AssertTrue(cMallocator.IsValid());

	cMallocator.Kill();
	AssertSize(0, cSource.NumHeld());
	AssertFalse(cSource.mbWrong);
}


//////////////////////////////////////////////////////////////////////////
//
// Realloc keeps what was written, in place where it can.
//
//////////////////////////////////////////////////////////////////////////
void TestBankMallocatorRealloc(void)
{
	CBankMallocator	cMallocator;
	CTestBankSource	cSource;
	uint8*			puiFirst;
	uint8*			pui;
	uint8*			puiNew;
	size			i;
	bool			bKept;

	puiFirst = TestBankFirst();
	cSource.Init(puiFirst);
	cMallocator.Init(puiFirst, TEST_BANKS * TEST_BANK_SIZE, TEST_BANK_SIZE, &cSource, 1, false);

	pui = (uint8*)cMallocator.Realloc(NULL, 50);
	for (i = 0; i < 50; i++)
	{
		pui[i] = (uint8)i;
	}

	//Small to small, then small to large: copied.
	pui = (uint8*)cMallocator.Realloc(pui, 300);
	AssertNotNull(pui);
	puiNew = (uint8*)cMallocator.Realloc(pui, 2500);
	AssertSize(BANK_MALLOCATOR_LARGE, cMallocator.GetBank(cSource.Index(puiNew)));
	bKept = true;
	for (i = 0; i < 50; i++)
	{
		bKept &= puiNew[i] == (uint8)i;
	}
	AssertTrue(bKept);
	AssertTrue(cMallocator.IsValid());

	//Large and still more than a bank in its banks: stays.
	pui = (uint8*)cMallocator.Realloc(puiNew, 2100);
	AssertPointer(puiNew, pui);

	//Large to small: copied back into a bank of small blocks.
	puiNew = (uint8*)cMallocator.Realloc(pui, 40);
	AssertSize(BANK_MALLOCATOR_SMALL, cMallocator.GetBank(cSource.Index(puiNew)));
	bKept = true;
	for (i = 0; i < 40; i++)
	{
		bKept &= puiNew[i] == (uint8)i;
	}
	AssertTrue(bKept);
	AssertSize(1, (size)cMallocator.NumAllocations());
	AssertTrue(cMallocator.IsValid());

	cMallocator.Free(puiNew);
	AssertSize(0, (size)cMallocator.NumAllocations());
	AssertTrue(cMallocator.IsValid());
	cMallocator.Kill();
	AssertSize(0, cSource.NumHeld());
	AssertFalse(cSource.mbWrong);
}


//////////////////////////////////////////////////////////////////////////
//
// Small blocks' banks highest first (the HCP65 kernal's heap), and every
// bank there already without a source.
//
//////////////////////////////////////////////////////////////////////////
void TestBankMallocatorHighFirst(void)
{
	CBankMallocator	cMallocator;
	CTestBankSource	cSource;
	uint8*			puiFirst;
	uint8*			pui;

	puiFirst = TestBankFirst();
	cSource.Init(puiFirst);
	cMallocator.Init(puiFirst, TEST_BANKS * TEST_BANK_SIZE, TEST_BANK_SIZE, &cSource, 1, true);
	pui = (uint8*)cMallocator.Malloc(100);
	AssertSize(15, cSource.Index(pui));
	cMallocator.Free(pui);
	cMallocator.Kill();
	AssertFalse(cSource.mbWrong);

	cMallocator.Init(puiFirst, TEST_BANKS * TEST_BANK_SIZE, TEST_BANK_SIZE, NULL, 1, false);
	pui = (uint8*)cMallocator.Malloc(100);
	AssertTrue(pui < &puiFirst[TEST_BANK_SIZE]);
	AssertPointer(&puiFirst[12 * TEST_BANK_SIZE], cMallocator.Malloc(4 * TEST_BANK_SIZE));
	AssertSize(5, cMallocator.NumHeld());
	AssertTrue(cMallocator.IsValid());
	cMallocator.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Many mallocs, reallocs and frees, small and large: what is written is
// kept, and once everything is freed the heap has only its spare.
//
//////////////////////////////////////////////////////////////////////////
void TestBankMallocatorRandom(void)
{
	CBankMallocator	cMallocator;
	CTestBankSource	cSource;
	CRandom			cRandom;
	uint8*			puiFirst;
	uint8*			apui[TEST_BANK_ALLOCATIONS];
	size			auiSizes[TEST_BANK_ALLOCATIONS];
	uint8*			pui;
	size			uiSize;
	size			i;
	size			j;
	size			k;
	int				iAction;
	bool			bValid;
	bool			bKept;

	puiFirst = TestBankFirst();
	cSource.Init(puiFirst);
	cMallocator.Init(puiFirst, TEST_BANKS * TEST_BANK_SIZE, TEST_BANK_SIZE, &cSource, 1, false);
	cRandom.Init(4321);
	for (i = 0; i < TEST_BANK_ALLOCATIONS; i++)
	{
		apui[i] = NULL;
		auiSizes[i] = 0;
	}

	bValid = true;
	bKept = true;
	for (k = 0; k < 1500; k++)
	{
		i = (size)cRandom.Next(0, TEST_BANK_ALLOCATIONS - 1);
		iAction = cRandom.Next(0, 2);
		uiSize = (size)cRandom.Next(1, (cRandom.Next(0, 9) == 0) ? 3 * TEST_BANK_SIZE : 300);
		if (apui[i])
		{
			for (j = 0; j < auiSizes[i]; j++)
			{
				bKept &= apui[i][j] == (uint8)(i + j);
			}
		}

		if (iAction == 0)
		{
			cMallocator.Free(apui[i]);
			apui[i] = NULL;
			auiSizes[i] = 0;
		}
		else
		{
			pui = (uint8*)cMallocator.Realloc(apui[i], uiSize);
			if (pui)
			{
				apui[i] = pui;
				auiSizes[i] = uiSize;
				for (j = 0; j < uiSize; j++)
				{
					pui[j] = (uint8)(i + j);
				}
			}
		}
		if ((k % 50) == 0)
		{
			bValid &= cMallocator.IsValid();
		}
	}
	AssertTrue(bKept);
	AssertTrue(bValid);
	AssertSize(cSource.NumHeld(), cMallocator.NumHeld());

	for (i = 0; i < TEST_BANK_ALLOCATIONS; i++)
	{
		cMallocator.Free(apui[i]);
	}
	AssertTrue(cMallocator.IsValid());
	AssertSize(0, (size)cMallocator.NumAllocations());
	AssertSize(1, cMallocator.NumHeld());
	AssertSize(1, cSource.NumHeld());
	cRandom.Kill();
	cMallocator.Kill();
	AssertSize(0, cSource.NumHeld());
	AssertFalse(cSource.mbWrong);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestBankMallocator(void)
{
	BeginTests();

	TestBankMallocatorInit();
	TestBankMallocatorSmall();
	TestBankMallocatorLarge();
	TestBankMallocatorRefused();
	TestBankMallocatorReserved();
	TestBankMallocatorPartial();
	TestBankMallocatorRealloc();
	TestBankMallocatorHighFirst();
	TestBankMallocatorRandom();

	TestStatistics();
}

