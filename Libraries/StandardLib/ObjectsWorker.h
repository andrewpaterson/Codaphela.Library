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
#ifndef __OBJECTS_WORKER_H__
#define __OBJECTS_WORKER_H__
#ifndef __LIBC65__
#include "BaseLib/SequenceConnection.h"
#include "Objects.h"
#include "StackPointers.h"


//A worker thread's object system (ThreadSafetyPlan.txt, Level 3 as built).  A thread that makes objects in parallel
//with others calls ObjectsWorkerInit in place of ObjectsThreadInit: OMalloc, ONMalloc and ORoot on it then make and
//find objects in an object system of its own, whose ways in take no lock.  It is the worker's alone: the worker is its
//home thread (Free() runs there) and nothing outside it may point into it.  It has no database; its objects take their
//indexes from the main object system's sequence, so they are unique in the process.  ObjectsThreadKill frees what is
//left in it.


#define OBJECTS_WORKER_INDEXES	256		//The indexes a worker takes from the main object system at a time: one lock for so many objects.


//A worker's indexes: blocks taken from the main object system's sequence, handed out without the lock.
class CObjectsWorkerSequence : public CSequenceConnection
{
protected:
	CObjects*	mpcMain;
	OIndex		moiNext;
	OIndex		moiEnd;

public:
	void	Init(CObjects* pcMain);
	OIndex	GetNext(void) override;
	OIndex	PeekNext(void) override;
};


//What a worker's object system is made of.
struct SObjectsWorker
{
	CObjects				cObjects;
	CStackPointers			cStackPointers;
	CObjectsWorkerSequence	cSequence;
};


bool	ObjectsWorkerOwnInit(void);
size	ObjectsWorkerOwnKill(void);


#endif // !__LIBC65__
#endif // __OBJECTS_WORKER_H__
