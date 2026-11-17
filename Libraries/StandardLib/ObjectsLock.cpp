/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela StandardLib

Codaphela StandardLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela StandardLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela StandardLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __LIBC65__
#include "BaseLib/Atomic.h"
#include "BaseLib/Logger.h"
#include "BaseLib/LogString.h"
#include "BaseLib/NativeThread.h"
#include "BaseLib/RecursiveLock.h"
#include "Objects.h"
#include "ObjectsLock.h"
#include "StackPointers.h"


bool						gbObjectsLocking = false;

//Spinning: it is held for a few instructions at a time, thousands of times a second.  A mutex that is fought over
//puts its waiters to sleep, which measured twenty times slower than one thread doing all the work.
static CRecursiveSpinLock	gcObjectsLock;

static int					giObjectsHomeThread = 0;	//0 until ObjectsInit.
static size					guiObjectsThreads = 0;		//The other threads registered.  Under the lock.
static bool					gbObjectsFreesWaiting = false;	//Other threads have found objects dead.  Under the lock.
static size					guiObjectsLocksTaken = 0;	//Under the lock.
static int32				giObjectsNotInside = 0;

//Whether the running thread may use objects: it is the home thread, or has registered.
static thread_local bool	gbObjectsThread = false;

//How many Free()s the running thread is inside (a Free() frees other objects).
static thread_local int		giObjectsFrees = 0;


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void ObjectsHomeThread(void)
{
	giObjectsHomeThread = CurrentThreadId();
	gbObjectsThread = true;
}


//////////////////////////////////////////////////////////////////////////
//
// Switches the locking on or off.  Only the home thread can, between its own uses of objects, and it can't switch it
// off while another thread is registered.
//
//////////////////////////////////////////////////////////////////////////
bool ObjectsLocking(bool bLocking)
{
	if (CurrentThreadId() != giObjectsHomeThread)
	{
		return gcLogger.Error2(__METHOD__, " The locking of objects is switched on and off by the thread that called ObjectsInit.", NULL);
	}
	if (gcObjectsLock.IsLockedByThisThread())
	{
		return gcLogger.Error2(__METHOD__, " The locking of objects can't be switched from inside the object system (a Free or an Init).", NULL);
	}
	if (!bLocking && (ObjectsNumThreads() > 0))
	{
		return gcLogger.Error2(__METHOD__, " The locking of objects can't be switched off while [", SizeToString(ObjectsNumThreads()), "] other threads are registered.", NULL);
	}

	if (!bLocking && gbObjectsLocking)
	{
		//What the threads left to be freed is freed while there is still a lock to free it under.
		gcObjects.Collect();
	}
	gbObjectsLocking = bLocking;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// With the locking off there is one thread, which is the home thread whatever its number.
//
//////////////////////////////////////////////////////////////////////////
bool ObjectsIsHomeThread(void)
{
	return !gbObjectsLocking || (CurrentThreadId() == giObjectsHomeThread);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void ObjectsFreesWaiting(bool bWaiting)
{
	gbObjectsFreesWaiting = bWaiting;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool IsObjectsLocking(void)
{
	return gbObjectsLocking;
}


//////////////////////////////////////////////////////////////////////////
//
// The running thread is going to use objects.  The home thread has switched the locking on already.
//
//////////////////////////////////////////////////////////////////////////
bool ObjectsThreadInit(void)
{
	if (!gbObjectsLocking)
	{
		return gcLogger.Error2(__METHOD__, " The locking of objects is off: the thread that called ObjectsInit switches it on (ObjectsLocking) before another thread registers.", NULL);
	}
	if (gbObjectsThread)
	{
		return gcLogger.Error2(__METHOD__, " The thread is registered already.", NULL);
	}

	gbObjectsThread = true;
	gcObjectsLock.Lock();
	guiObjectsThreads++;
	gcObjectsLock.Unlock();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The running thread has finished with objects.  It should hold no Ptr: one it did hold would keep its object for
// good (in a Debug build they are counted, and it is said).  False if it wasn't registered or still holds some.
//
//////////////////////////////////////////////////////////////////////////
bool ObjectsThreadKill(void)
{
	size	uiStackPointers;

	if (!gbObjectsThread || (CurrentThreadId() == giObjectsHomeThread))
	{
		return gcLogger.Error2(__METHOD__, " The thread is not registered.", NULL);
	}

	gcObjectsLock.Lock();
	uiStackPointers = gcStackPointers.NumOnThread(CurrentThreadId());
	guiObjectsThreads--;
	gcObjectsLock.Unlock();
	gbObjectsThread = false;

	if (uiStackPointers > 0)
	{
		return gcLogger.Error2(__METHOD__, " The thread still holds [", SizeToString(uiStackPointers), "] stack pointers to objects.", NULL);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size ObjectsNumThreads(void)
{
	size	uiThreads;

	gcObjectsLock.Lock();
	uiThreads = guiObjectsThreads;
	gcObjectsLock.Unlock();
	return uiThreads;
}


//////////////////////////////////////////////////////////////////////////
//
// A thread that hasn't registered is told so (each time: it is a mistake in the program, not something to live
// with) and locks all the same, so that what it does is at least done alone.
//
//////////////////////////////////////////////////////////////////////////
void ObjectsLock(void)
{
	if (!gbObjectsThread)
	{
		gcLogger.Error2(__METHOD__, " A thread that has not called ObjectsThreadInit is using objects.", NULL);
	}
	gcObjectsLock.Lock();
	if (gcObjectsLock.Depth() == 1)
	{
		guiObjectsLocksTaken++;

		//The home thread, coming in from outside, first frees what the other threads have found dead.
		if (gbObjectsFreesWaiting && (CurrentThreadId() == giObjectsHomeThread))
		{
			gcObjects.Collect();
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void ObjectsUnlock(void)
{
	gcObjectsLock.Unlock();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool ObjectsIsLockedByThisThread(void)
{
	return gcObjectsLock.IsLockedByThisThread();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void ObjectsFreeBegins(void)
{
	giObjectsFrees++;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void ObjectsFreeEnds(void)
{
	giObjectsFrees--;
}


//////////////////////////////////////////////////////////////////////////
//
// For where a class takes a lock of its own.  A Free() runs with the object system's lock held, so a lock taken
// inside one is taken after it; any thread that holds that lock and then uses an object takes them the other way
// round, and the two stop for good.  Only a Debug build counts the Free()s: elsewhere this is always true.
//
//////////////////////////////////////////////////////////////////////////
bool ObjectsAssertNoFree(const char* szMethod)
{
	if (giObjectsFrees > 0)
	{
		return gcLogger.Error2(szMethod, " was reached from inside an object's Free(), which runs with the object system's lock held.", NULL);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// szMethod is the object system's own machinery and was reached with the lock not held: the way in that led to it
// is missing its OBJECTS_WRITE or OBJECTS_READ.
//
//////////////////////////////////////////////////////////////////////////
void ObjectsNotInside(const char* szMethod)
{
	AtomicAdd(&giObjectsNotInside, 1);
	gcLogger.Error2(szMethod, " was reached without the object system's lock held.", NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size ObjectsNumLocksTaken(void)
{
	size	uiLocksTaken;

	gcObjectsLock.Lock();
	uiLocksTaken = guiObjectsLocksTaken;
	gcObjectsLock.Unlock();
	return uiLocksTaken;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size ObjectsNumNotInside(void)
{
	return (size)AtomicLoad(&giObjectsNotInside);
}


#endif // !__LIBC65__

