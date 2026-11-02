/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela ThreadLib

Codaphela ThreadLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela ThreadLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela ThreadLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#include <stdlib.h>
#include "BaseLib/NativeThread.h"
#include "WorkerThreadJobs.h"


//A job given to the pool: what to run and whose it is.
struct SWorkerThreadJob
{
	JobFunction			fJob;
	void*				pvData;
	CWorkerThreadJobs*	pcJobs;
};


//////////////////////////////////////////////////////////////////////////
//
// A pool thread runs the job, then it is counted finished.
//
//////////////////////////////////////////////////////////////////////////
static void RunWorkerThreadJob(int iThreadId, void* pvData)
{
	SWorkerThreadJob*	psJob;
	CWorkerThreadJobs*	pcJobs;

	psJob = (SWorkerThreadJob*)pvData;
	psJob->fJob(psJob->pvData);
	pcJobs = psJob->pcJobs;
	free(psJob);
	pcJobs->Finished();
}


//////////////////////////////////////////////////////////////////////////
//
// A thread for each the hardware runs at once.
//
//////////////////////////////////////////////////////////////////////////
void CWorkerThreadJobs::Init(void)
{
	Init(NumHardwareThreads());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWorkerThreadJobs::Init(int iNumThreads)
{
	mcMutex.Init();
	mcFinished.Init();
	muiUnfinished = 0;
	mcPool.Init(iNumThreads);
}


//////////////////////////////////////////////////////////////////////////
//
// The jobs given are finished first.
//
//////////////////////////////////////////////////////////////////////////
void CWorkerThreadJobs::Kill(void)
{
	Wait();
	mcPool.Kill();
	mcFinished.Kill();
	mcMutex.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A pool that has stopped can't take the job, so it runs now.
//
//////////////////////////////////////////////////////////////////////////
void CWorkerThreadJobs::Run(JobFunction fJob, void* pvData)
{
	SWorkerThreadJob*	psJob;

	psJob = (SWorkerThreadJob*)malloc(sizeof(SWorkerThreadJob));
	psJob->fJob = fJob;
	psJob->pvData = pvData;
	psJob->pcJobs = this;

	mcMutex.Lock();
	muiUnfinished++;
	mcMutex.Unlock();
	if (!mcPool.Push(RunWorkerThreadJob, psJob))
	{
		RunWorkerThreadJob(-1, psJob);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWorkerThreadJobs::Wait(void)
{
	mcMutex.Lock();
	while (muiUnfinished > 0)
	{
		mcFinished.Wait(&mcMutex);
	}
	mcMutex.Unlock();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CWorkerThreadJobs::NumThreads(void)
{
	return mcPool.Size();
}


//////////////////////////////////////////////////////////////////////////
//
// A job has finished (RunWorkerThreadJob).
//
//////////////////////////////////////////////////////////////////////////
void CWorkerThreadJobs::Finished(void)
{
	mcMutex.Lock();
	muiUnfinished--;
	if (muiUnfinished == 0)
	{
		mcFinished.WakeAll();
	}
	mcMutex.Unlock();
}
