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
#include "BaseLib/ConstructorCall.h"
#include "BaseLib/Logger.h"
#include "BaseLib/LogString.h"
#include "ObjectsWorker.h"


static OBJECTS_PER_THREAD SObjectsWorker*	gpsObjectsWorker = NULL;	//The running thread's object system, NULL if it isn't a worker.


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectsWorkerSequence::Init(CObjects* pcMain)
{
	mpcMain = pcMain;
	moiNext = INVALID_O_INDEX;
	moiEnd = INVALID_O_INDEX;
}


//////////////////////////////////////////////////////////////////////////
//
// The next of the block in hand, or the first of a new block.
//
//////////////////////////////////////////////////////////////////////////
OIndex CObjectsWorkerSequence::GetNext(void)
{
	OIndex	oiFirst;

	if (moiNext == moiEnd)
	{
		oiFirst = mpcMain->ReserveIndexes(OBJECTS_WORKER_INDEXES);
		if (oiFirst == INVALID_O_INDEX)
		{
			return INVALID_O_INDEX;
		}
		moiNext = oiFirst;
		moiEnd = oiFirst + OBJECTS_WORKER_INDEXES;
	}
	return moiNext++;
}


//////////////////////////////////////////////////////////////////////////
//
// INVALID_O_INDEX when the next comes from a block not yet taken.
//
//////////////////////////////////////////////////////////////////////////
OIndex CObjectsWorkerSequence::PeekNext(void)
{
	return (moiNext != moiEnd) ? moiNext : INVALID_O_INDEX;
}


//////////////////////////////////////////////////////////////////////////
//
// The running thread, registered, is given an object system of its own.
//
//////////////////////////////////////////////////////////////////////////
bool ObjectsWorkerOwnInit(void)
{
	SObjectsWorker*	psWorker;

	if (gpsObjectsWorker != NULL)
	{
		return gcLogger.Error2(__METHOD__, " The thread has an object system of its own already.", NULL);
	}

	psWorker = NewMalloc<SObjectsWorker>();
	psWorker->cStackPointers.Init();
	psWorker->cSequence.Init(&gcObjects);
	psWorker->cObjects.InitWorker(&gcObjects, &psWorker->cStackPointers, &psWorker->cSequence);
	gpsObjectsWorker = psWorker;
	gpcObjectsOwn = &psWorker->cObjects;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The running thread's own object system goes, with every object left in
// it.  How many stack pointers it still held into it: they are cleared,
// so that they free nothing when they go.
//
//////////////////////////////////////////////////////////////////////////
size ObjectsWorkerOwnKill(void)
{
	SObjectsWorker*	psWorker;
	size			uiStackPointers;

	psWorker = gpsObjectsWorker;
	if (psWorker == NULL)
	{
		return 0;
	}

	//Nothing is saved: it has no database.
	uiStackPointers = psWorker->cStackPointers.NumElements();
	psWorker->cStackPointers.ClearAllPointers();
	psWorker->cObjects.Discard();
	psWorker->cObjects.Kill();
	psWorker->cStackPointers.Kill();

	gpcObjectsOwn = NULL;
	gpsObjectsWorker = NULL;
	free(psWorker);
	return uiStackPointers;
}


#endif // !__LIBC65__
