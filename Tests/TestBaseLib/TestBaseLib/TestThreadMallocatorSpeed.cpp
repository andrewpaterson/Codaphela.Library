//CThreadMallocator isn't built for the 65C64 or the W65C816, whose compiler is given every test's source.
#ifndef __LIBC65__
#include <stdlib.h>
#include <string.h>
#include "BaseLib/Logger.h"
#include "BaseLib/LogString.h"
#include "BaseLib/NativeThread.h"
#include "BaseLib/Atomic.h"
#include "BaseLib/ThreadMallocator.h"
#include "BaseLib/Timer.h"
#include "TestLib/Assert.h"


//How long CThreadMallocator takes beside malloc, in the ways allocators get used.  It asserts only that each ran;
//set THREAD_MALLOCATOR_SPEED (to how many times longer to run, 1 or more) and it logs the nanoseconds each
//operation took, which are the numbers in StandardLib/ThreadSafetyPlan.txt's allocator tables.  Measure a Release
//build.

#define TEST_THREAD_MALLOCATOR_SPEED_THREADS	12


struct STestMallocatorSpeed
{
	const char*		szName;
	void*			(*Malloc)(lsize uiBytes);
	void*			(*Realloc)(void* pv, lsize uiBytes);
	void			(*Free)(void* pv);
};


static void* TestMallocatorSpeedSystemMalloc(lsize uiBytes) { return malloc(uiBytes); }
static void* TestMallocatorSpeedSystemRealloc(void* pv, lsize uiBytes) { return realloc(pv, uiBytes); }
static void  TestMallocatorSpeedSystemFree(void* pv) { free(pv); }
static void* TestMallocatorSpeedThreadMalloc(lsize uiBytes) { return gcThreadMallocator.Malloc(uiBytes); }
static void* TestMallocatorSpeedThreadRealloc(void* pv, lsize uiBytes) { return gcThreadMallocator.Realloc(pv, uiBytes); }
static void  TestMallocatorSpeedThreadFree(void* pv) { gcThreadMallocator.Free(pv); }


static STestMallocatorSpeed	gasTestMallocatorSpeed[2] =
{
	{ "malloc",				TestMallocatorSpeedSystemMalloc, TestMallocatorSpeedSystemRealloc, TestMallocatorSpeedSystemFree },
	{ "CThreadMallocator",	TestMallocatorSpeedThreadMalloc, TestMallocatorSpeedThreadRealloc, TestMallocatorSpeedThreadFree },
};


//////////////////////////////////////////////////////////////////////////
//
// How many times longer than the quick run that is the test: 1 unless THREAD_MALLOCATOR_SPEED says.
//
//////////////////////////////////////////////////////////////////////////
static size TestMallocatorSpeedScale(void)
{
	char*	szScale;
	int		iScale;

	szScale = getenv("THREAD_MALLOCATOR_SPEED");
	iScale = szScale ? atoi(szScale) : 1;
	return (iScale < 1) ? 1 : (size)iScale;
}


static uint32 TestMallocatorSpeedNext(uint32* puiRandom)
{
	uint32	ui;

	ui = *puiRandom;
	ui ^= ui << 13;
	ui ^= ui >> 17;
	ui ^= ui << 5;
	*puiRandom = ui;
	return ui;
}


//////////////////////////////////////////////////////////////////////////
//
// Each of these returns the nanoseconds one operation took.
//
//////////////////////////////////////////////////////////////////////////
static float TestMallocatorSpeedPairs(STestMallocatorSpeed* psMallocator, lsize uiBytes, size uiCount)
{
	void*	pv;
	int64	iStart;
	size	i;

	iStart = GetDebugMicroseconds();
	for (i = 0; i < uiCount; i++)
	{
		pv = psMallocator->Malloc(uiBytes);
		*(volatile uint8*)pv = 1;
		psMallocator->Free(pv);
	}
	return (float)(GetDebugMicroseconds() - iStart) * 1000.0f / (float)uiCount;
}


static float TestMallocatorSpeedChurn(STestMallocatorSpeed* psMallocator, size uiLive, lsize uiMaxBytes, size uiCount, uint32 uiSeed)
{
	void**	apv;
	int64	iStart;
	uint32	uiRandom;
	size	uiSlot;
	size	i;
	float	fNanoseconds;

	uiRandom = uiSeed;
	apv = (void**)malloc(uiLive * sizeof(void*));
	for (i = 0; i < uiLive; i++)
	{
		apv[i] = psMallocator->Malloc(16 + TestMallocatorSpeedNext(&uiRandom) % uiMaxBytes);
	}

	iStart = GetDebugMicroseconds();
	for (i = 0; i < uiCount; i++)
	{
		uiSlot = TestMallocatorSpeedNext(&uiRandom) % uiLive;
		psMallocator->Free(apv[uiSlot]);
		apv[uiSlot] = psMallocator->Malloc(16 + TestMallocatorSpeedNext(&uiRandom) % uiMaxBytes);
		*(volatile uint8*)apv[uiSlot] = 1;
	}
	fNanoseconds = (float)(GetDebugMicroseconds() - iStart) * 1000.0f / (float)uiCount;

	for (i = 0; i < uiLive; i++)
	{
		psMallocator->Free(apv[i]);
	}
	free(apv);
	return fNanoseconds;
}


static float TestMallocatorSpeedBatches(STestMallocatorSpeed* psMallocator, size uiBatch, lsize uiBytes, size uiRounds)
{
	void**	apv;
	int64	iStart;
	size	uiRound;
	size	i;
	float	fNanoseconds;

	apv = (void**)malloc(uiBatch * sizeof(void*));
	iStart = GetDebugMicroseconds();
	for (uiRound = 0; uiRound < uiRounds; uiRound++)
	{
		for (i = 0; i < uiBatch; i++)
		{
			apv[i] = psMallocator->Malloc(uiBytes);
			*(volatile uint8*)apv[i] = 1;
		}
		for (i = 0; i < uiBatch; i++)
		{
			psMallocator->Free(apv[i]);
		}
	}
	fNanoseconds = (float)(GetDebugMicroseconds() - iStart) * 1000.0f / ((float)uiBatch * (float)uiRounds);
	free(apv);
	return fNanoseconds;
}


//An array that grows from uiFirst bytes to uiLast (doubling, or by uiStep bytes each time when that isn't 0) with
//another allocation made behind it: the object system's scratch arrays, and an index tree's nodes gaining children.
static float TestMallocatorSpeedGrowing(STestMallocatorSpeed* psMallocator, lsize uiFirst, lsize uiLast, lsize uiStep, size uiCount)
{
	void*	pv;
	void*	pvBehind;
	lsize	uiBytes;
	int64	iStart;
	size	uiReallocs;
	size	i;

	uiReallocs = 0;
	iStart = GetDebugMicroseconds();
	for (i = 0; i < uiCount; i++)
	{
		pv = psMallocator->Malloc(uiFirst);
		pvBehind = psMallocator->Malloc(48);
		for (uiBytes = uiStep ? uiFirst + uiStep : uiFirst * 2; uiBytes <= uiLast; uiBytes = uiStep ? uiBytes + uiStep : uiBytes * 2)
		{
			pv = psMallocator->Realloc(pv, uiBytes);
			*(volatile uint8*)pv = 1;
			uiReallocs++;
		}
		psMallocator->Free(pvBehind);
		psMallocator->Free(pv);
	}
	return (float)(GetDebugMicroseconds() - iStart) * 1000.0f / (float)uiReallocs;
}


struct STestMallocatorSpeedThread
{
	CNativeThread			cThread;
	STestMallocatorSpeed*	psMallocator;
	void**					apvShared;		//NULL: the thread churns blocks of its own.
	size					uiCount;
	uint32					uiSeed;
	float					fNanoseconds;
};


#define TEST_THREAD_MALLOCATOR_SPEED_SHARED		4096


static void TestMallocatorSpeedThread(void* pvThread)
{
	STestMallocatorSpeedThread*		psThread;
	void*							pvNew;
	uint32							uiRandom;
	int64							iStart;
	size							i;

	psThread = (STestMallocatorSpeedThread*)pvThread;
	if (!psThread->apvShared)
	{
		psThread->fNanoseconds = TestMallocatorSpeedChurn(psThread->psMallocator, 2000, 512, psThread->uiCount, psThread->uiSeed);
		return;
	}

	//Every block is freed by whichever thread next swaps one into its slot.
	uiRandom = psThread->uiSeed;
	iStart = GetDebugMicroseconds();
	for (i = 0; i < psThread->uiCount; i++)
	{
		pvNew = psThread->psMallocator->Malloc(16 + TestMallocatorSpeedNext(&uiRandom) % 512);
		*(volatile uint8*)pvNew = 1;
		psThread->psMallocator->Free(AtomicExchangePointer(&psThread->apvShared[TestMallocatorSpeedNext(&uiRandom) % TEST_THREAD_MALLOCATOR_SPEED_SHARED], pvNew));
	}
	psThread->fNanoseconds = (float)(GetDebugMicroseconds() - iStart) * 1000.0f / (float)psThread->uiCount;
}


//The nanoseconds an operation took on each thread, averaged: the same on any number of threads if the allocator
//doesn't get in its own way.
static float TestMallocatorSpeedThreads(STestMallocatorSpeed* psMallocator, size uiThreads, bool bShared, size uiCount)
{
	static void*				apvShared[TEST_THREAD_MALLOCATOR_SPEED_SHARED];
	STestMallocatorSpeedThread	asThreads[TEST_THREAD_MALLOCATOR_SPEED_THREADS];
	float						fNanoseconds;
	size						i;

	memset(apvShared, 0, sizeof(apvShared));
	for (i = 0; i < uiThreads; i++)
	{
		asThreads[i].psMallocator = psMallocator;
		asThreads[i].apvShared = bShared ? apvShared : NULL;
		asThreads[i].uiCount = uiCount;
		asThreads[i].uiSeed = 12345 + (uint32)i * 7919;
		asThreads[i].fNanoseconds = 0.0f;
		asThreads[i].cThread.Init();
		asThreads[i].cThread.Start(TestMallocatorSpeedThread, &asThreads[i]);
	}

	fNanoseconds = 0.0f;
	for (i = 0; i < uiThreads; i++)
	{
		asThreads[i].cThread.Join();
		asThreads[i].cThread.Kill();
		fNanoseconds += asThreads[i].fNanoseconds;
	}
	for (i = 0; i < TEST_THREAD_MALLOCATOR_SPEED_SHARED; i++)
	{
		psMallocator->Free(apvShared[i]);
	}
	return fNanoseconds / (float)uiThreads;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void TestMallocatorSpeedLog(const char* szWhat, float* afNanoseconds)
{
	if (getenv("THREAD_MALLOCATOR_SPEED"))
	{
		//In tenths, as whole numbers: printing a float needs the number control, which TestBaseLib doesn't start.
		gcLogger.Info2(szWhat, ": ", gasTestMallocatorSpeed[0].szName, " ", IntToString((int)afNanoseconds[0]), ".", IntToString((int)(afNanoseconds[0] * 10.0f) % 10), " ns, ",
				gasTestMallocatorSpeed[1].szName, " ", IntToString((int)afNanoseconds[1]), ".", IntToString((int)(afNanoseconds[1] * 10.0f) % 10), " ns", NULL);
	}
	AssertTrue(afNanoseconds[0] >= 0.0f);
	AssertTrue(afNanoseconds[1] >= 0.0f);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestThreadMallocatorSpeed(void)
{
	static const size	auiThreads[5] = { 1, 2, 4, 8, TEST_THREAD_MALLOCATOR_SPEED_THREADS };
	float				afNanoseconds[2];
	char				szWhat[64];
	size				uiScale;
	size				uiThreads;
	size				i;
	size				j;

	BeginTests();

	uiScale = TestMallocatorSpeedScale();
	for (i = 0; i < 2; i++) { afNanoseconds[i] = TestMallocatorSpeedPairs(&gasTestMallocatorSpeed[i], 64, 20000 * uiScale); }
	TestMallocatorSpeedLog("malloc + free, 64 bytes", afNanoseconds);
	for (i = 0; i < 2; i++) { afNanoseconds[i] = TestMallocatorSpeedPairs(&gasTestMallocatorSpeed[i], 256, 20000 * uiScale); }
	TestMallocatorSpeedLog("malloc + free, 256 bytes", afNanoseconds);
	for (i = 0; i < 2; i++) { afNanoseconds[i] = TestMallocatorSpeedPairs(&gasTestMallocatorSpeed[i], 16384, 20000 * uiScale); }
	TestMallocatorSpeedLog("malloc + free, 16 KB", afNanoseconds);
	for (i = 0; i < 2; i++) { afNanoseconds[i] = TestMallocatorSpeedPairs(&gasTestMallocatorSpeed[i], 2 * THREAD_MALLOCATOR_ARENA_BYTES, 200 * uiScale); }
	TestMallocatorSpeedLog("malloc + free, 2 MB", afNanoseconds);
	for (i = 0; i < 2; i++) { afNanoseconds[i] = TestMallocatorSpeedChurn(&gasTestMallocatorSpeed[i], 2000, 512, 20000 * uiScale, 777); }
	TestMallocatorSpeedLog("churn, 2000 live, 16 to 528 bytes", afNanoseconds);
	for (i = 0; i < 2; i++) { afNanoseconds[i] = TestMallocatorSpeedChurn(&gasTestMallocatorSpeed[i], 20000, 4096, 20000 * uiScale, 778); }
	TestMallocatorSpeedLog("churn, 20000 live, 16 to 4112 bytes", afNanoseconds);
	for (i = 0; i < 2; i++) { afNanoseconds[i] = TestMallocatorSpeedBatches(&gasTestMallocatorSpeed[i], 10000, 100, uiScale); }
	TestMallocatorSpeedLog("10000 of 100 bytes, then all freed", afNanoseconds);
	for (i = 0; i < 2; i++) { afNanoseconds[i] = TestMallocatorSpeedGrowing(&gasTestMallocatorSpeed[i], 16, 4096, 0, 2000 * uiScale); }
	TestMallocatorSpeedLog("realloc doubling, 16 to 4096 bytes", afNanoseconds);
	for (i = 0; i < 2; i++) { afNanoseconds[i] = TestMallocatorSpeedGrowing(&gasTestMallocatorSpeed[i], 8, 256, 8, 1000 * uiScale); }
	TestMallocatorSpeedLog("realloc by 8 bytes, 8 to 256", afNanoseconds);

	for (j = 0; j < 5; j++)
	{
		uiThreads = auiThreads[j];
		for (i = 0; i < 2; i++) { afNanoseconds[i] = TestMallocatorSpeedThreads(&gasTestMallocatorSpeed[i], uiThreads, false, 5000 * uiScale); }
		strcpy(szWhat, "churn, each thread its own blocks, threads ");
		strcat(szWhat, IntToString((int)uiThreads));
		TestMallocatorSpeedLog(szWhat, afNanoseconds);
	}
	for (j = 0; j < 5; j++)
	{
		uiThreads = auiThreads[j];
		for (i = 0; i < 2; i++) { afNanoseconds[i] = TestMallocatorSpeedThreads(&gasTestMallocatorSpeed[i], uiThreads, true, 5000 * uiScale); }
		strcpy(szWhat, "every block freed by another thread, threads ");
		strcat(szWhat, IntToString((int)uiThreads));
		TestMallocatorSpeedLog(szWhat, afNanoseconds);
	}

	TestStatistics();
}


#endif // !__LIBC65__

