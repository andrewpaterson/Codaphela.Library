#include "BaseLib/GlobalMemory.h"
#include "BaseLib/Jobs.h"
#include "ThreadLib/WorkerThreadJobs.h"
#include "ThreadLib/AtomicInt.h"
#include "TestLib/Assert.h"


#define TEST_WORKER_THREAD_JOBS_COUNT		200


//What a counting job adds to.
struct STestJobsCount
{
	CAtomicInt	iRun;
	int			aiTimes[TEST_WORKER_THREAD_JOBS_COUNT];
};


//A job that counts itself: its data is its entry in aiTimes (whose count is the job's index).
struct STestJob
{
	STestJobsCount*	psCount;
	int				iIndex;
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void TestJobsCount(void* pvJob)
{
	STestJob*	psJob;

	psJob = (STestJob*)pvJob;
	psJob->psCount->aiTimes[psJob->iIndex]++;
	psJob->psCount->iRun.Increment();
}


//////////////////////////////////////////////////////////////////////////
//
// Gives pcJobs iJobs counting jobs and waits for them: each ran once.
//
//////////////////////////////////////////////////////////////////////////
static void TestJobsRunAll(CJobs* pcJobs, int iJobs)
{
	STestJobsCount	sCount;
	STestJob		asJobs[TEST_WORKER_THREAD_JOBS_COUNT];
	int				i;

	sCount.iRun.Set(0);
	for (i = 0; i < iJobs; i++)
	{
		sCount.aiTimes[i] = 0;
		asJobs[i].psCount = &sCount;
		asJobs[i].iIndex = i;
		pcJobs->Run(TestJobsCount, &asJobs[i]);
	}
	pcJobs->Wait();
	AssertInt(iJobs, sCount.iRun.Get());
	for (i = 0; i < iJobs; i++)
	{
		AssertInt(1, sCount.aiTimes[i]);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// CSerialJobs runs each job before Run returns, on one thread.
//
//////////////////////////////////////////////////////////////////////////
void TestWorkerThreadJobsSerial(void)
{
	CSerialJobs		cJobs;
	STestJobsCount	sCount;
	STestJob		sJob;

	AssertInt(1, cJobs.NumThreads());
	sCount.iRun.Set(0);
	sCount.aiTimes[0] = 0;
	sJob.psCount = &sCount;
	sJob.iIndex = 0;
	cJobs.Run(TestJobsCount, &sJob);
	AssertInt(1, sCount.iRun.Get());
	AssertInt(1, sCount.aiTimes[0]);
	cJobs.Wait();

	TestJobsRunAll(&cJobs, TEST_WORKER_THREAD_JOBS_COUNT);
}


//////////////////////////////////////////////////////////////////////////
//
// Every job given runs once before Wait returns, and the jobs can be given
// again after.
//
//////////////////////////////////////////////////////////////////////////
void TestWorkerThreadJobsRunAll(void)
{
	CWorkerThreadJobs	cJobs;

	cJobs.Init(4);
	AssertInt(4, cJobs.NumThreads());
	TestJobsRunAll(&cJobs, TEST_WORKER_THREAD_JOBS_COUNT);
	TestJobsRunAll(&cJobs, 1);
	TestJobsRunAll(&cJobs, TEST_WORKER_THREAD_JOBS_COUNT);
	cJobs.Wait();
	cJobs.Kill();
}


//Two jobs that wait for each other: they only both finish if they run at the same time.
struct STestJobsMeet
{
	CAtomicInt	iArrived;
	CAtomicInt	iMet;
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void TestJobsMeet(void* pvMeet)
{
	STestJobsMeet*	psMeet;

	psMeet = (STestJobsMeet*)pvMeet;
	psMeet->iArrived.Increment();
	if (psMeet->iArrived.WaitUntilAtLeast(2, 10000))
	{
		psMeet->iMet.Increment();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Jobs run at the same time as each other, and Kill waits for jobs not yet
// finished.
//
//////////////////////////////////////////////////////////////////////////
void TestWorkerThreadJobsAtTheSameTime(void)
{
	CWorkerThreadJobs	cJobs;
	STestJobsMeet		sMeet;

	sMeet.iArrived.Set(0);
	sMeet.iMet.Set(0);
	cJobs.Init(2);
	cJobs.Run(TestJobsMeet, &sMeet);
	cJobs.Run(TestJobsMeet, &sMeet);
	cJobs.Kill();
	AssertInt(2, sMeet.iArrived.Get());
	AssertInt(2, sMeet.iMet.Get());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestWorkerThreadJobs(void)
{
	BeginTests();
	MemoryInit();

	TestWorkerThreadJobsSerial();
	TestWorkerThreadJobsRunAll();
	TestWorkerThreadJobsAtTheSameTime();

	MemoryKill();
	TestStatistics();
}
