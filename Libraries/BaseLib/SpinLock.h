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
#ifndef __SPIN_LOCK_H__
#define __SPIN_LOCK_H__
#include "Atomic.h"


//A lock for work that takes a few instructions (a list changed, a count read): a thread that finds it held waits by
//trying again, not by sleeping, because by the time it had gone to sleep the lock would be free.  If the wait goes on
//(the holder was switched out) it gives up the rest of its time slice each try.  A CMutex put to that use spends
//microseconds sleeping and waking for every few nanoseconds of work.  Not for anything that blocks or runs long while
//holding it.  It is not recursive (CRecursiveSpinLock is).
//
//All zeros is unlocked, so one in static memory is ready before anything has run, without Init.
class CSpinLock
{
protected:
	int32	miLocked;

public:
	void	Init(void);
	void	Kill(void);

	void	Lock(void);
	bool	TryLock(void);
	void	Unlock(void);

	bool	IsLocked(void);			//By any thread, as it was when it was looked at.
	void	WaitWhileLocked(void);	//Returns once it has been seen free, which it may no longer be.

protected:
	void	LockContended(void);
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
inline bool CSpinLock::IsLocked(void)
{
	return AtomicLoad(&miLocked) != 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
inline void CSpinLock::Lock(void)
{
	if (AtomicExchange(&miLocked, 1) != 0)
	{
		LockContended();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
inline bool CSpinLock::TryLock(void)
{
	return AtomicExchange(&miLocked, 1) == 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
inline void CSpinLock::Unlock(void)
{
	AtomicStore(&miLocked, 0);
}


#endif // __SPIN_LOCK_H__

