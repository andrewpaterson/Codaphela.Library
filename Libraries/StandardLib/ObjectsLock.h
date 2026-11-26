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
//A pointer's own ways in (its constructors, its destructor, its assignments) do their own locking, OBJECTS_LOCKING()
//saying whether there is any to do: with the locking on they first try to do what they do as a reader, without the
//lock (level 2; ObjectsThreads.h says what a reader is and when it can).
//ThreadSafetyPlan.txt's audit lists which each public method is.  The lock may be taken again by the thread that
//holds it, since a Free(), an Init() or a Save() is the user's code and comes back in.
//
//Nothing is locked until ObjectsLocking(true): a program with one thread pays a test of gbObjectsLocking at each way
//in and nothing else, and on the 65C64 and the W65C816, which run one thread, the three compile to nothing.
//
//A thread that isn't registered may still read objects it is handed, through plain pointers and operator ->, while
//the thread that handed them over holds them and waits (a bake's rows shared among threads).  What it must not do is
//come in by any of the ways in: copy, assign or let go of a Ptr.  It is told when it does, whether the locking is on
//(every build) or off (a Debug build).
//
//Threads.  The thread that calls ObjectsInit is the home thread.  Before any other thread touches an object the home
//thread switches the locking on, ObjectsLocking(true): it is the one thread that can know it isn't half way through
//something it began with the lock off.  Each other thread then calls ObjectsThreadInit before it touches an object
//and ObjectsThreadKill when it has finished, having let go of every Ptr it held (in a Debug build ObjectsThreadKill
//says if it hasn't).  When they have all finished the home thread may switch the locking off again.
//
//Freeing.  Only the home thread frees objects: a Free() is the user's code, and what it frees (a window, a texture)
//belongs to the thread the objects were made for.  When another thread lets go of the last thing holding some
//objects it still works out that they are dead, and makes them unreachable, but leaves the freeing to the home
//thread, which does it the next time it takes the lock, or when its tick calls ObjectsCollect.  So what a thread
//lets go of stays allocated until the home thread next uses an object: a home thread that waits for its threads
//without calling ObjectsCollect frees nothing while it waits.
//And because a Free() runs with the lock held, it must not wait for another thread that uses objects, nor take a
//lock that another thread holds while it uses objects: the two would each be waiting for what the other has.  A class
//with a lock of its own can check the second where it takes that lock: ObjectsAssertNoFree says (in a Debug build)
//when it is reached from inside a Free().
//
//Stored objects.  Any registered thread may use stored objects, loading them as it goes.  A hollow loaded while other
//threads are registered isn't freed at once, as another thread may have read it and not yet asked it for its object:
//it forwards to the object until the home thread's ObjectsCollect finds every other thread has come in since
//(CObjects::FreeForwarders), so a registered thread that does nothing with objects keeps a hollow or two waiting.  The
//Kill of a stored object (which leaves a hollow for what held it) is refused while another thread holds a Ptr to it,
//and EvictInMemory while another thread holds any.  Flush, and the save of a dirty stored object as it is freed, read
//its fields: a class whose fields other threads write saves under its own lock, or the program flushes between jobs.
//
//What it doesn't do: it keeps the object system right, and no object is freed while a thread holds a Ptr to it, but
//an object's own fields are the object's to guard (as are a collection's elements) when threads share it.

//A test that is seldom true, for the compilers that can be told.
#if defined(_MSC_VER) || defined(__LIBC65__)
#define OBJECTS_SELDOM(b)	(b)
#else
#define OBJECTS_SELDOM(b)	__builtin_expect(!!(b), 0)
#endif // _MSC_VER || __LIBC65__


#ifdef __LIBC65__


#define OBJECTS_WRITE()
#define OBJECTS_READ()
#define OBJECTS_INSIDE()
#define OBJECTS_LOCKING()	false


inline bool ObjectsLocking(bool bLocking) { return !bLocking; }
inline bool IsObjectsLocking(void) { return false; }
inline bool ObjectsThreadInit(void) { return false; }
inline bool ObjectsWorkerInit(void) { return false; }
inline bool ObjectsThreadKill(void) { return false; }
inline size ObjectsNumThreads(void) { return 0; }
inline void ObjectsHomeThread(bool bHome) {}
inline bool ObjectsIsHomeThread(void) { return true; }
inline void ObjectsFreesWaiting(bool bWaiting) {}
inline bool ObjectsAreFreesWaiting(void) { return false; }
inline bool ObjectsAssertNoFree(const char* szMethod) { return true; }
inline void ObjectsLock(void) {}
inline void ObjectsUnlock(void) {}
inline void ObjectsOwnEnters(void) {}
inline void ObjectsOwnLeaves(void) {}
inline size ObjectsNumWaysIn(void) { return 0; }
inline size ObjectsNumThreadErrors(void) { return 0; }


#else


#include "BaseLib/Atomic.h"
#include "BaseLib/RecursiveLock.h"
#include "BaseLib/SpinLock.h"


//A variable each thread has its own of: a plain one (no constructor) read in a couple of instructions.
#ifdef _MSC_VER
#define OBJECTS_PER_THREAD	__declspec(thread)
#else
#define OBJECTS_PER_THREAD	__thread
#endif // _MSC_VER


//Debug builds on the hosts remember which thread each stack pointer is on.
#ifdef _DEBUG
#define OBJECTS_THREAD_CHECKS
#endif // _DEBUG


extern bool					gbObjectsLocking;	//Changed by the home thread alone, while no other thread uses objects.
extern CRecursiveSpinLock	gcObjectsLock;		//The lock.  Here for what reads (ObjectsThreads.h) to look at.
extern int32				giObjectsFreesWaiting;	//Other threads have found objects dead.  Written with the lock.

bool	ObjectsLocking(bool bLocking);		//The home thread's.  False (and nothing changed) if it can't be done now.
bool	IsObjectsLocking(void);
bool	ObjectsThreadInit(void);			//Every other thread's, before it touches an object.
bool	ObjectsWorkerInit(void);			//Or, for a thread that makes objects of its own: ObjectsThreadInit and an object system of its own (ObjectsWorker.h).
bool	ObjectsThreadKill(void);			//And when it has finished (a worker's own object system, and what is left in it, goes).
size	ObjectsNumThreads(void);			//The other threads registered now.
void	ObjectsHomeThread(bool bHome);		//ObjectsInit's: the running thread is the home thread.  ObjectsKill's: no thread is.
bool	ObjectsIsHomeThread(void);			//Always, while the locking is off.
void	ObjectsFreesWaiting(bool bWaiting);	//CObjects': there are objects for the home thread to free.

bool	ObjectsAssertNoFree(const char* szMethod);	//False, and said, if a Free() is running on this thread (a Debug build knows).
void	ObjectsFreeBegins(void);			//CBaseObject's, round each Free().
void	ObjectsFreeEnds(void);

void	ObjectsLock(void);
void	ObjectsUnlock(void);
bool	ObjectsIsLockedByThisThread(void);
bool	ObjectsIsInside(void);				//The running thread has the lock, or is reading.
void	ObjectsNotInside(const char* szMethod);

void	ObjectsNotLocking(void);

//For the tests that every way in has its guard (TestObjectsThreadsEveryWayIn), and that threads which aren't to use
//objects don't.
size	ObjectsNumWaysIn(void);				//How many times the running thread has come into the object system from outside
//it: taken the lock when it didn't hold it, or read (ObjectsThreads.h).
size	ObjectsNumThreadErrors(void);		//How many times a thread has been told it used the object system wrongly: its
//machinery reached without the lock, or objects used by a thread that isn't registered.


class CObjects;
class CEmbeddedObject;


//What a guard holds: nothing (the locking is off), the lock, or a way into the running thread's own object system (a
//worker's), which takes no lock (ThreadSafetyPlan.txt, Level 3 as built).
#define OBJECTS_HELD_NOTHING	0
#define OBJECTS_HELD_LOCK		1
#define OBJECTS_HELD_OWN		2


//A way into pcObjects, or into pcObject's object system: the lock taken, or the thread's own.  And let go of.
int		ObjectsEnterFor(const CObjects* pcObjects);
int		ObjectsEnterForObject(const CEmbeddedObject* pcObject);
void	ObjectsLeave(int iHeld);
void	ObjectsOwnEnters(void);  //The running thread's own object system is used without a guard (a pointer's ways in).
void	ObjectsOwnLeaves(void);


//Holds the lock from where it is declared to the end of the block, when the locking is on.  It is given what the way
//in is a method of (`this`): of an object system or an object, it is the lock that object system needs (none for the
//running thread's own); of anything else (a pointer, a weak pointer), the main object system's.
class CObjectsGuard
{
protected:
	int		miHeld;

public:
	CObjectsGuard(const void* pvOther);
	CObjectsGuard(const CObjects* pcObjects);
	CObjectsGuard(const CEmbeddedObject* pcObject);
	~CObjectsGuard();

protected:
	void	NotLocking(void);
};


//////////////////////////////////////////////////////////////////////////
//
// The locking is out of line: what is here is the test every way in pays,
// which is all one thread pays.
//
//////////////////////////////////////////////////////////////////////////
inline CObjectsGuard::CObjectsGuard(const void* pvOther)
{
	miHeld = OBJECTS_HELD_NOTHING;
	if (OBJECTS_SELDOM(gbObjectsLocking))
	{
		ObjectsLock();
		miHeld = OBJECTS_HELD_LOCK;
	}
	else
	{
		NotLocking();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
inline CObjectsGuard::CObjectsGuard(const CObjects* pcObjects)
{
	miHeld = OBJECTS_HELD_NOTHING;
	if (OBJECTS_SELDOM(gbObjectsLocking))
	{
		miHeld = ObjectsEnterFor(pcObjects);
	}
	else
	{
		NotLocking();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
inline CObjectsGuard::CObjectsGuard(const CEmbeddedObject* pcObject)
{
	miHeld = OBJECTS_HELD_NOTHING;
	if (OBJECTS_SELDOM(gbObjectsLocking))
	{
		miHeld = ObjectsEnterForObject(pcObject);
	}
	else
	{
		NotLocking();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A Debug build checks the thread with the locking off.
//
//////////////////////////////////////////////////////////////////////////
inline void CObjectsGuard::NotLocking(void)
{
#ifdef OBJECTS_THREAD_CHECKS
	ObjectsNotLocking();
#endif // OBJECTS_THREAD_CHECKS
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
inline CObjectsGuard::~CObjectsGuard()
{
	if (OBJECTS_SELDOM(miHeld != OBJECTS_HELD_NOTHING))
	{
		ObjectsLeave(miHeld);
	}
}


#define OBJECTS_WRITE()		CObjectsGuard cObjectsGuard(this)
#define OBJECTS_READ()		CObjectsGuard cObjectsGuard(this)

#ifdef _DEBUG
#define OBJECTS_INSIDE()	do { if (gbObjectsLocking && !ObjectsIsInside()) { ObjectsNotInside(__METHOD__); } } while (false)
#else
#define OBJECTS_INSIDE()
#endif // _DEBUG


//For the home thread where it is about to read (ObjectsThreads.h): with frees waiting it takes the lock instead,
//which frees them.  Without this a home thread that only took and dropped pointers would never free what the others
//let go of.
inline bool ObjectsAreFreesWaiting(void)
{
	return OBJECTS_SELDOM(AtomicLoad(&giObjectsFreesWaiting) != 0);
}


//For a way in that does its own locking (a pointer's, which may read instead: ObjectsThreads.h): whether the locking
//is on.  With it off a Debug build checks the thread, as the guard does.
inline bool ObjectsLockingHere(void)
{
	if (OBJECTS_SELDOM(gbObjectsLocking))
	{
		return true;
	}
#ifdef OBJECTS_THREAD_CHECKS
	ObjectsNotLocking();
#endif // OBJECTS_THREAD_CHECKS
	return false;
}


#define OBJECTS_LOCKING()	ObjectsLockingHere()


#endif // __LIBC65__


#endif // __OBJECTS_LOCK_H__

