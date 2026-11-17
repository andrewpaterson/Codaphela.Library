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
#include "NativeThread.h"
#include "SpinLock.h"


//How many times a waiter looks at the lock before it starts giving up its time slice between looks.
#define SPIN_LOCK_SPINS		2000


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSpinLock::Init(void)
{
	miLocked = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSpinLock::Kill(void)
{
	miLocked = 0;
}


//////////////////////////////////////////////////////////////////////////
//
// Trying again for a while, then giving up the rest of the time slice each try: the holder has been switched out.
//
//////////////////////////////////////////////////////////////////////////
void CSpinLock::WaitWhileLocked(void)
{
	size	uiSpins;

	uiSpins = 0;
	while (AtomicLoad(&miLocked) != 0)
	{
		if (uiSpins < SPIN_LOCK_SPINS)
		{
			uiSpins++;
			AtomicPause();
		}
		else
		{
			ThreadYield();
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The lock was held.  It is only looked at (a read, which doesn't take the cache line from its holder) until it is
// seen free, then tried for.
//
//////////////////////////////////////////////////////////////////////////
void CSpinLock::LockContended(void)
{
	for (;;)
	{
		WaitWhileLocked();
		if (AtomicExchange(&miLocked, 1) == 0)
		{
			return;
		}
	}
}

