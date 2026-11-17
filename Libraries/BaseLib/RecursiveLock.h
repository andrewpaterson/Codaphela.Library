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
#ifndef __RECURSIVE_LOCK_H__
#define __RECURSIVE_LOCK_H__
#include "Atomic.h"
#include "Mutex.h"
#include "NativeThread.h"
#include "SpinLock.h"


//A lock the thread that holds it may take again: what guards something whose work calls back into itself (a logger
//whose outputs log, an object system whose objects free objects).  It is LOCK (a CMutex or a CSpinLock, whichever
//suits how long it is held) with the thread that holds it and how many times.  Each Lock is undone by an Unlock, by
//the same thread.
template<class LOCK>
class CRecursiveLock
{
protected:
	LOCK	mcLock;
	int32	miThread;	//The thread that holds it, 0 if none does.  Read by threads that don't.
	int		miDepth;	//How many times it has locked it.  Only the holder's to read or write.

public:
	void	Init(void);
	void	Kill(void);

	void	Lock(void);
	void	Unlock(void);

	bool	IsLockedByThisThread(void);
	int		Depth(void);
};


typedef CRecursiveLock<CMutex>		CRecursiveMutex;
typedef CRecursiveLock<CSpinLock>	CRecursiveSpinLock;


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class LOCK>
void CRecursiveLock<LOCK>::Init(void)
{
	mcLock.Init();
	miThread = 0;
	miDepth = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class LOCK>
void CRecursiveLock<LOCK>::Kill(void)
{
	mcLock.Kill();
	miThread = 0;
	miDepth = 0;
}


//////////////////////////////////////////////////////////////////////////
//
// A thread only ever finds its own number in miThread while it holds the lock, so finding it there is holding it.
//
//////////////////////////////////////////////////////////////////////////
template<class LOCK>
void CRecursiveLock<LOCK>::Lock(void)
{
	int32	iThread;

	iThread = (int32)CurrentThreadId();
	if (AtomicLoad(&miThread) == iThread)
	{
		miDepth++;
		return;
	}

	mcLock.Lock();
	AtomicStore(&miThread, iThread);
	miDepth = 1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class LOCK>
void CRecursiveLock<LOCK>::Unlock(void)
{
	miDepth--;
	if (miDepth == 0)
	{
		AtomicStore(&miThread, 0);
		mcLock.Unlock();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class LOCK>
bool CRecursiveLock<LOCK>::IsLockedByThisThread(void)
{
	return AtomicLoad(&miThread) == (int32)CurrentThreadId();
}


//////////////////////////////////////////////////////////////////////////
//
// How many times the running thread has locked it and not unlocked it.
//
//////////////////////////////////////////////////////////////////////////
template<class LOCK>
int CRecursiveLock<LOCK>::Depth(void)
{
	return IsLockedByThisThread() ? miDepth : 0;
}


#endif // __RECURSIVE_LOCK_H__

