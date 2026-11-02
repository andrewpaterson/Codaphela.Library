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
#ifndef __WORKER_THREAD_JOBS_H__
#define __WORKER_THREAD_JOBS_H__
#include "BaseLib/Jobs.h"
#include "BaseLib/Mutex.h"
#include "BaseLib/ConditionVariable.h"
#include "WorkerThreadPool.h"


//BaseLib's CJobs on a CWorkerThreadPool: jobs run on its threads, started in the order Run was given them (the
//pool's order).  Wait counts the jobs that haven't finished.
class CWorkerThreadJobs : public CJobs
{
protected:
	CWorkerThreadPool	mcPool;
	CMutex				mcMutex;
	CConditionVariable	mcFinished;
	size				muiUnfinished;

public:
	void	Init(void);
	void	Init(int iNumThreads);
	void	Kill(void);

	void	Run(JobFunction fJob, void* pvData) override;
	void	Wait(void) override;
	int		NumThreads(void) override;

	void	Finished(void);
};


#endif // __WORKER_THREAD_JOBS_H__
