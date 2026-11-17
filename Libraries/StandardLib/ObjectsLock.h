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
#ifndef __OBJECTS_LOCK_H__
#define __OBJECTS_LOCK_H__
#include "BaseLib/Define.h"
#include "BaseLib/PrimitiveTypes.h"


//The object system's lock (ThreadSafetyPlan.txt).  The object system keeps things about objects that any Ptr changes:
//what points at each, how far each is from the root, which are being freed.  When more than one thread uses objects
//those are changed by one thread at a time, each way into the object system taking this lock for as long as it is
//in.  It is one lock for the process.
//
//A way in declares what it is by its first statement:
//  OBJECTS_WRITE()   it changes what the object system keeps, or frees, loads or saves.
//  OBJECTS_READ()    it only reads what the object system keeps.
//  OBJECTS_INSIDE()  it is the object system's own machinery, only reached from one of the two above: in a Debug
//                    build it says so if it is reached with the lock not held.
//ThreadSafetyPlan.txt's audit lists which each public method is.  The lock may be taken again by the thread that
//holds it, since a Free(), an Init() or a Save() is the user's code and comes back in.
//
//Nothing is locked until ObjectsLocking(true): a program with one thread pays a test of gbObjectsLocking at each way
//in and nothing else, and on the 65C64 and the W65C816, which run one thread, the three compile to nothing.
//
//Threads.  The thread that calls ObjectsInit is the home thread.  Before any other thread touches an object the home
//thread switches the locking on, ObjectsLocking(true): it is the one thread that can know it isn't half way through
//something it began with the lock off.  Each other thread then calls ObjectsThreadInit before it touches an object
//and ObjectsThreadKill when it has finished, having let go of every Ptr it held (in a Debug build ObjectsThreadKill
//says if it hasn't).  When they have all finished the home thread may switch the locking off again.
//
//What it doesn't do: it keeps the object system right, and no object is freed while a thread holds a Ptr to it, but
//an object's own fields are the object's to guard (as are a collection's elements) when threads share it.

#ifdef __LIBC65__


#define OBJECTS_WRITE()
#define OBJECTS_READ()
#define OBJECTS_INSIDE()


inline bool ObjectsLocking(bool bLocking) { return !bLocking; }
inline bool IsObjectsLocking(void) { return false; }
inline bool ObjectsThreadInit(void) { return false; }
inline bool ObjectsThreadKill(void) { return false; }
inline size ObjectsNumThreads(void) { return 0; }
inline void ObjectsHomeThread(void) {}


#else


//Debug builds on the hosts remember which thread each stack pointer is on.
#ifdef _DEBUG
#define OBJECTS_THREAD_CHECKS
#endif // _DEBUG


#ifdef _MSC_VER
#define OBJECTS_SELDOM(b)	(b)
#else
#define OBJECTS_SELDOM(b)	__builtin_expect(!!(b), 0)
#endif // _MSC_VER


extern bool		gbObjectsLocking;	//Changed by the home thread alone, while no other thread uses objects.

bool	ObjectsLocking(bool bLocking);		//The home thread's.  False (and nothing changed) if it can't be done now.
bool	IsObjectsLocking(void);
bool	ObjectsThreadInit(void);			//Every other thread's, before it touches an object.
bool	ObjectsThreadKill(void);			//And when it has finished.
size	ObjectsNumThreads(void);			//The other threads registered now.
void	ObjectsHomeThread(void);			//ObjectsInit's: the running thread is the home thread.

void	ObjectsLock(void);
void	ObjectsUnlock(void);
bool	ObjectsIsLockedByThisThread(void);
void	ObjectsNotInside(const char* szMethod);


//Holds the lock from where it is declared to the end of the block, when the locking is on.
class CObjectsGuard
{
protected:
	bool	mbLocked;

public:
	CObjectsGuard();
	~CObjectsGuard();
};


//////////////////////////////////////////////////////////////////////////
//
// The locking of it is out of line: what is here is the test every way in pays.
//
//////////////////////////////////////////////////////////////////////////
inline CObjectsGuard::CObjectsGuard()
{
	mbLocked = gbObjectsLocking;
	if (OBJECTS_SELDOM(mbLocked))
	{
		ObjectsLock();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
inline CObjectsGuard::~CObjectsGuard()
{
	if (OBJECTS_SELDOM(mbLocked))
	{
		ObjectsUnlock();
	}
}


#define OBJECTS_WRITE()		CObjectsGuard cObjectsGuard
#define OBJECTS_READ()		CObjectsGuard cObjectsGuard

#ifdef _DEBUG
#define OBJECTS_INSIDE()	do { if (gbObjectsLocking && !ObjectsIsLockedByThisThread()) { ObjectsNotInside(__METHOD__); } } while (false)
#else
#define OBJECTS_INSIDE()
#endif // _DEBUG


#endif // __LIBC65__


#endif // __OBJECTS_LOCK_H__

