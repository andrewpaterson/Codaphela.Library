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
#ifndef __JOBS_H__
#define __JOBS_H__


typedef void (*JobFunction)(void* pvData);


//Runs jobs: each job's function is called once with its data.  Jobs may run at the same time as each other and as
//the caller, on other threads, but they start in the order Run was given them.  Wait returns when every job given
//so far has finished.
//
//CSerialJobs runs each job at once, on the caller's thread (as on the 65C64 and the W65C816, which run one
//thread).  ThreadLib's CWorkerThreadJobs runs them on a pool of threads.
class CJobs
{
public:
	virtual void	Run(JobFunction fJob, void* pvData) =0;
	virtual void	Wait(void) =0;
	virtual int		NumThreads(void) =0;	//How many jobs can run at the same time.
};


class CSerialJobs : public CJobs
{
public:
	void	Run(JobFunction fJob, void* pvData) override;
	void	Wait(void) override;
	int		NumThreads(void) override;
};


#endif // __JOBS_H__
