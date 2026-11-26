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

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#ifndef __OBJECTS_THREADS_H__
#define __OBJECTS_THREADS_H__
#include "BaseLib/ArrayVoidPtr.h"
#include "BaseLib/Atomic.h"
#include "ObjectsLock.h"


class CChars;
class CPointer;
class CEmbeddedObject;


//The stack froms of the threads that have registered to use objects (ObjectsThreadInit; ThreadSafetyPlan.txt, level
//2).
//
//A stack pointer to an object is a from of the object: it holds it.  The home thread's are kept on the objects, a
//list each, as they are when there is one thread.  A registered thread's are kept by the thread, in a table of its
//own, when the object is attached to the root as the pointer is pointed at it, which is nearly always.  Nothing is
//then written to the object, so threads that take pointers to the same object write nothing in common.  (An object
//that isn't attached to the root is held by its stack froms alone: taking and letting go of those is the walks'
//business, and they are kept on the object whoever's they are.)
//
//A thread's table is in the order its pointers were taken, and they are nearly always let go in the reverse of it,
//so the last is looked at first.
//
//What asks an object for its stack froms asks here too: whether it has any (which the walks ask of an object found
//detached from the root), how many, and for them to be cleared when the object is freed, or pointed at what the
//object has become when a hollow is loaded.  Each looks through the tables of the threads that keep any, which are
//counted: threads that are registered and hold nothing cost the asking nothing.  (A thread is counted from when it
//first keeps one until something that looks finds its table empty, not each time its table empties, which for a
//thread taking one pointer at a time is every time.)
//
//Readers and writers.  Taking a stack pointer to an object that is attached to the root, or letting go of one,
//frees nothing and changes no distance: it is most of what threads do with pointers, and it is done without the
//object system's lock, by a thread that is reading.  Everything else (a pointer in an object changed, an object
//made, freed or loaded, a stack pointer to an object that isn't attached) is done with the lock, by a thread that is
//writing, alone.
//  A thread that would read sets its own flag (ObjectsReadBegins) and then looks at the lock: if it is held it
//  drops its flag, waits for the lock to be let go and tries again.  (It doesn't take the lock itself at once:
//  readers that did would each be a writer to the rest, and under load nobody would ever read.  It does after a few
//  tries, or writers that follow each other closely would keep it waiting for good.  And if the lock is its own it
//  is inside the object system already, and goes on as the writer it is.)  While its flag is up it may read whether
//  objects are attached, and change its own table of stack froms (or, the home thread, the stack froms on the
//  objects, which only it and writers touch).  It never looks at another thread's table.
//  A writer takes the lock and then waits for every other thread's flag to drop (ObjectsWriteBegins).  Each has set
//  its own (the lock is the writers') before it looks at the other's, so one of the two always sees the other.
//Each thread's flag is on a cache line of its own: threads that only read write nothing in common.  A writer costs
//each reader that line once, which is why writers should be the fewer.
//It is switched on with the locking (ObjectsLocking), unless the objects' database is written through to, where
//pointing a pointer at an object can save the object.
//
//Everything else here is called with the object system's lock held, by a writer.  None of it is built for the 65C64
//or the W65C816, which run one thread.

#ifdef __LIBC65__


inline bool ObjectsThreadsRegistered(void) { return false; }
inline bool ObjectsThreadsKeeping(void) { return false; }
inline bool ObjectsReadBegins(void) { return false; }
inline void ObjectsReadEnds(bool bDone) {}
inline bool ObjectsIsReading(void) { return false; }
inline bool ObjectsReaderIsHome(void) { return true; }
inline size ObjectsNumReads(void) { return 0; }
inline size ObjectsThreadsNumWaysIn(void) { return 0; }
inline void ObjectsThreadsMarkWaysIn(void) {}
inline bool ObjectsThreadsAllInSinceMark(void) { return true; }
inline bool ThreadStackFromAdd(CEmbeddedObject* pcObject, CPointer* pcPointer) { return false; }
inline bool ThreadStackFromRemove(CEmbeddedObject* pcObject, CPointer* pcPointer) { return false; }
inline bool ThreadStackFromRemoveAnywhere(CEmbeddedObject* pcObject, CPointer* pcPointer) { return false; }
inline bool ThreadStackFromsHas(CEmbeddedObject* pcObject) { return false; }
inline size ThreadStackFromsNum(CEmbeddedObject* pcObject) { return 0; }
inline size ThreadStackFromsClear(CEmbeddedObject* pcObject) { return 0; }
inline size ThreadStackFromsRemap(CEmbeddedObject* pcOld, CEmbeddedObject* pcNew) { return 0; }
inline bool ThreadStackFromsHasOnAnotherThread(CEmbeddedObject* pcObject) { return false; }
inline size ThreadStackFromsNumOnOtherThreads(void) { return 0; }
inline void ThreadStackFromsAddresses(CArrayVoidPtr* papvAddresses) {}
inline void ThreadStackFromsPrint(CChars* psz, bool bPointerMemory) {}


#else


//What a thread reads by: its flag, on a cache line no other thread writes.
struct SObjectsReader
{
	int32	iReading;	//The thread is reading: writers wait.
	bool	bHome;		//It is the home thread, whose stack froms are on the objects.
	int32	iReads;		//How many times it has read and done what it came for,
	int32	iLocks;		//and taken the lock from outside.  Its ways in, written by the thread and read by the tests of the
	//ways in and by the home thread, which frees forwarding hollows by them (CObjects::FreeForwarders).
};


extern lsize								guiObjectsThreads;			//A thread other than the home thread is registered.  Changed by writers; read whole, as a worker's own object system asks without the lock.
extern int32								giObjectsThreadsKeeping;	//How many registered threads are counted as keeping stack froms.
extern OBJECTS_PER_THREAD SObjectsReader*	gpsObjectsReader;			//The running thread's, NULL if it can't read (it isn't registered, or nothing reads).


//The home thread's, as the locking is switched on and off (ObjectsLocking's): with bReaders threads may read.
void	ObjectsThreadsAddHome(bool bReaders);
void	ObjectsThreadsRemoveHome(void);

//A writer's, as it takes the lock from outside (ObjectsLock's).
void	ObjectsWriteBegins(void);

bool	ObjectsReadBeginsAfterWriter(SObjectsReader* psReader);

//The registration of the running thread (ObjectsThreadInit's and ObjectsThreadKill's).  ObjectsThreadsRemove gives
//back how many stack froms the thread was still keeping: they are let go of for it.
void	ObjectsThreadsAdd(void);
size	ObjectsThreadsRemove(void);
size	ObjectsThreadsNum(void);

bool	ThreadStackFromAddInside(CEmbeddedObject* pcObject, CPointer* pcPointer);
bool	ThreadStackFromRemoveInside(CEmbeddedObject* pcObject, CPointer* pcPointer);
bool	ThreadStackFromRemoveAnywhereInside(CEmbeddedObject* pcObject, CPointer* pcPointer);
bool	ThreadStackFromsHasInside(CEmbeddedObject* pcObject);
size	ThreadStackFromsNumInside(CEmbeddedObject* pcObject);
size	ThreadStackFromsClearInside(CEmbeddedObject* pcObject);
size	ThreadStackFromsRemapInside(CEmbeddedObject* pcOld, CEmbeddedObject* pcNew);
bool	ThreadStackFromsHasOnAnotherThreadInside(CEmbeddedObject* pcObject);
size	ThreadStackFromsNumOnOtherThreadsInside(void);
void	ThreadStackFromsAddressesInside(CArrayVoidPtr* papvAddresses);
void	ThreadStackFromsPrintInside(CChars* psz, bool bPointerMemory);


//True if the running thread is now reading, and must call ObjectsReadEnds; false if it must take the lock instead.
inline bool ObjectsReadBegins(void)
{
	SObjectsReader*		psReader;

	psReader = gpsObjectsReader;
	if (psReader == NULL)
	{
		return false;
	}
	AtomicExchange(&psReader->iReading, 1);
	if (OBJECTS_SELDOM(gcObjectsLock.IsLocked()))
	{
		return ObjectsReadBeginsAfterWriter(psReader);
	}
	return true;
}


//bDone if what it read for was done (it is counted), not left for a writer.
inline void ObjectsReadEnds(bool bDone)
{
	SObjectsReader*		psReader;

	psReader = gpsObjectsReader;
	AtomicStore(&psReader->iReads, psReader->iReads + (bDone ? 1 : 0));
	AtomicStore(&psReader->iReading, 0);
}


inline bool ObjectsIsReading(void)
{
	return (gpsObjectsReader != NULL) && (gpsObjectsReader->iReading != 0);
}


//Of a thread that is reading.
inline bool ObjectsReaderIsHome(void)
{
	return gpsObjectsReader->bHome;
}


inline size ObjectsNumReads(void)
{
	return (gpsObjectsReader != NULL) ? (size)gpsObjectsReader->iReads : 0;
}


//The ways in of the running thread, and (the home thread's, for forwarding hollows) of every other.  A thread can't
//come in between reading a pointer to a hollow and asking the hollow for its object (CHollowObject::Dehollow, which
//takes the lock): every thread having come in since the mark means none can still be about to ask a hollow that was
//forwarding by then.  A thread registered since the mark couldn't have read the pointer.
size	ObjectsThreadsNumWaysIn(void);
void	ObjectsThreadsMarkWaysIn(void);
bool	ObjectsThreadsAllInSinceMark(void);
void	ObjectsThreadLocked(void);		//ObjectsLock's, as the running thread takes the lock from outside.


//What the object system calls.  With no thread registered but the home thread, or (for what asks about an object's
//froms) none that keeps any, each is one test.

inline bool ObjectsThreadsRegistered(void)
{
	return OBJECTS_SELDOM(AtomicLoadSize(&guiObjectsThreads) != 0);
}


inline bool ObjectsThreadsKeeping(void)
{
	return OBJECTS_SELDOM(AtomicLoad(&giObjectsThreadsKeeping) != 0);
}


//pcPointer has been pointed at pcObject.  True if the running thread keeps the from itself: it is a registered
//thread and pcObject is attached to the root.
inline bool ThreadStackFromAdd(CEmbeddedObject* pcObject, CPointer* pcPointer)
{
	return ObjectsThreadsRegistered() && ThreadStackFromAddInside(pcObject, pcPointer);
}


//pcPointer no longer points at pcObject.  True if the running thread was keeping the from, which it no longer is.
inline bool ThreadStackFromRemove(CEmbeddedObject* pcObject, CPointer* pcPointer)
{
	return ObjectsThreadsRegistered() && ThreadStackFromRemoveInside(pcObject, pcPointer);
}


//The same, of every thread: for a pointer let go of by a thread that didn't take it.
inline bool ThreadStackFromRemoveAnywhere(CEmbeddedObject* pcObject, CPointer* pcPointer)
{
	return ObjectsThreadsKeeping() && ThreadStackFromRemoveAnywhereInside(pcObject, pcPointer);
}


//Whether any thread keeps a stack from of pcObject.
inline bool ThreadStackFromsHas(CEmbeddedObject* pcObject)
{
	return ObjectsThreadsKeeping() && ThreadStackFromsHasInside(pcObject);
}


//How many stack froms of pcObject the threads keep.
inline size ThreadStackFromsNum(CEmbeddedObject* pcObject)
{
	return ObjectsThreadsKeeping() ? ThreadStackFromsNumInside(pcObject) : 0;
}


//pcObject is being freed: each pointer the threads keep as a from of it is made NULL, and forgotten.  How many.
inline size ThreadStackFromsClear(CEmbeddedObject* pcObject)
{
	return ObjectsThreadsKeeping() ? ThreadStackFromsClearInside(pcObject) : 0;
}


//pcOld has become pcNew (a hollow loaded): each pointer the threads keep as a from of pcOld is pointed at pcNew, and
//is a from of it.  How many.
inline size ThreadStackFromsRemap(CEmbeddedObject* pcOld, CEmbeddedObject* pcNew)
{
	return ObjectsThreadsKeeping() ? ThreadStackFromsRemapInside(pcOld, pcNew) : 0;
}


//Whether a thread other than the running one keeps a stack from of pcObject.
inline bool ThreadStackFromsHasOnAnotherThread(CEmbeddedObject* pcObject)
{
	return ObjectsThreadsKeeping() && ThreadStackFromsHasOnAnotherThreadInside(pcObject);
}


//How many stack froms the threads other than the running one keep.
inline size ThreadStackFromsNumOnOtherThreads(void)
{
	return ObjectsThreadsKeeping() ? ThreadStackFromsNumOnOtherThreadsInside() : 0;
}


//The address of every pointer the threads keep as a from.
inline void ThreadStackFromsAddresses(CArrayVoidPtr* papvAddresses)
{
	if (ObjectsThreadsKeeping())
	{
		ThreadStackFromsAddressesInside(papvAddresses);
	}
}


//Each thread's stack froms, a line each, after the home thread's (CObjects::PrintStackPointers).
inline void ThreadStackFromsPrint(CChars* psz, bool bPointerMemory)
{
	if (ObjectsThreadsKeeping())
	{
		ThreadStackFromsPrintInside(psz, bPointerMemory);
	}
}


#endif // __LIBC65__


#endif // __OBJECTS_THREADS_H__

