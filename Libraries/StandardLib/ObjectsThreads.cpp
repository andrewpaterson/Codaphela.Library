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
#ifndef __LIBC65__
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/Atomic.h"
#include "BaseLib/Chars.h"
#include "BaseLib/NativeThread.h"
#include "BaseObject.h"
#include "Objects.h"
#include "PointerObject.h"
#include "ObjectsThreads.h"


//One of a thread's stack pointers, and the object it points at.
struct SThreadStackFrom
{
	CPointer*			pcPointer;
	CEmbeddedObject*	pcObject;
};


typedef CArrayTemplate<SThreadStackFrom>	CArrayThreadStackFrom;


#define OBJECTS_CACHE_LINE	64


//What is kept for a thread that uses objects while the locking is on: what it reads by, with nothing else on its
//cache line, and the stack froms it keeps itself (the home thread keeps none).
struct SObjectsThread
{
	SObjectsReader			sReader;
	char					acRestOfLine[OBJECTS_CACHE_LINE - sizeof(SObjectsReader)];
	CArrayThreadStackFrom	asStackFroms;
	int						iThread;
	bool					bKeeping;	//Counted in giObjectsThreadsKeeping.
	void*					pvAllocated;
};


typedef CArrayTemplatePtr<SObjectsThread>	CArrayObjectsThreadPtr;


bool									gbObjectsThreads = false;
int32									giObjectsThreadsKeeping = 0;
OBJECTS_PER_THREAD SObjectsReader*		gpsObjectsReader = NULL;

static CArrayObjectsThreadPtr			gapsObjectsThreads;			//Every thread's, the home thread's among them.  Initialised while the locking is on.
static bool								gbObjectsReaders = false;	//Threads may read.  Changed by the home thread while it is the only one.

static OBJECTS_PER_THREAD SObjectsThread*	gpsObjectsThread = NULL;	//The running thread's, NULL if it has none.


//////////////////////////////////////////////////////////////////////////
//
// The running thread's, on a cache line boundary, and in with every thread's.
//
//////////////////////////////////////////////////////////////////////////
static SObjectsThread* ObjectsThreadNew(bool bHome)
{
	SObjectsThread*		psThread;
	void*				pvAllocated;

	pvAllocated = malloc(sizeof(SObjectsThread) + OBJECTS_CACHE_LINE);
	psThread = (SObjectsThread*)(((ptr)pvAllocated + OBJECTS_CACHE_LINE) & ~(ptr)(OBJECTS_CACHE_LINE - 1));
	psThread->pvAllocated = pvAllocated;
	psThread->sReader.iReading = 0;
	psThread->sReader.bHome = bHome;
	psThread->sReader.uiReads = 0;
	psThread->asStackFroms.Init();
	psThread->iThread = CurrentThreadId();
	psThread->bKeeping = false;

	gapsObjectsThreads.Add(psThread);
	gpsObjectsThread = psThread;
	gpsObjectsReader = gbObjectsReaders ? &psThread->sReader : NULL;
	return psThread;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void ObjectsThreadFree(SObjectsThread* psThread)
{
	if (psThread->bKeeping)
	{
		AtomicAdd(&giObjectsThreadsKeeping, -1);
	}
	gapsObjectsThreads.RemoveAt(gapsObjectsThreads.Find(psThread), false);
	psThread->asStackFroms.Kill();
	free(psThread->pvAllocated);
	gpsObjectsThread = NULL;
	gpsObjectsReader = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void ObjectsThreadsAddHome(bool bReaders)
{
	gapsObjectsThreads.Init();
	gbObjectsReaders = bReaders;
	ObjectsThreadNew(true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void ObjectsThreadsRemoveHome(void)
{
	ObjectsThreadFree(gpsObjectsThread);
	gapsObjectsThreads.Kill();
	gbObjectsReaders = false;
}


//////////////////////////////////////////////////////////////////////////
//
// With the lock.  Once every other thread's flag has been seen down none is reading, nor will be until this thread
// lets the lock go: one that would read sees the lock held first.  A reader's flag is up for a few instructions,
// unless its thread has been put aside in the middle of them, which is what the yield is for.
//
//////////////////////////////////////////////////////////////////////////
void ObjectsWriteBegins(void)
{
	SObjectsThread*		psThread;
	size				uiSpins;
	size				i;

	if (!gbObjectsReaders)
	{
		return;
	}

	for (i = 0; i < gapsObjectsThreads.NumElements(); i++)
	{
		psThread = gapsObjectsThreads.GetPtr(i);
		uiSpins = 0;
		while (AtomicLoad(&psThread->sReader.iReading) != 0)
		{
			uiSpins++;
			if (uiSpins < 2000)
			{
				AtomicPause();
			}
			else
			{
				ThreadYield();
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The thread set its flag and then saw the lock held.  It waits for the lock to be let go and tries again, as a
// reader, for as long as it takes; unless the lock is its own.
//
//////////////////////////////////////////////////////////////////////////
bool ObjectsReadBeginsAfterWriter(SObjectsReader* psReader)
{
	for (;;)
	{
		AtomicStore(&psReader->iReading, 0);
		if (gcObjectsLock.IsLockedByThisThread())
		{
			return false;
		}
		gcObjectsLock.WaitWhileLocked();
		AtomicExchange(&psReader->iReading, 1);
		if (!gcObjectsLock.IsLocked())
		{
			return true;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void ObjectsThreadsAdd(void)
{
	ObjectsThreadNew(false);
	gbObjectsThreads = true;
}


//////////////////////////////////////////////////////////////////////////
//
// A stack from the thread still keeps is of a pointer it hasn't let go of.  Each is let go of for it, as killing
// its object would: the pointer is made NULL, and what it alone held is freed.
//
//////////////////////////////////////////////////////////////////////////
size ObjectsThreadsRemove(void)
{
	SObjectsThread*		psThread;
	SThreadStackFrom	sFrom;
	size				uiStackFroms;

	psThread = gpsObjectsThread;
	if ((psThread == NULL) || psThread->sReader.bHome)
	{
		return 0;
	}

	uiStackFroms = psThread->asStackFroms.NumElements();
	while (psThread->asStackFroms.NumElements() > 0)
	{
		psThread->asStackFroms.Pop(&sFrom);
		sFrom.pcPointer->UnsafeClearObject();
		sFrom.pcObject->GetObjectsOrDefault()->FromRemoved(sFrom.pcObject, false);
	}

	ObjectsThreadFree(psThread);
	gbObjectsThreads = gapsObjectsThreads.NumElements() > 1;
	return uiStackFroms;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size ObjectsThreadsNum(void)
{
	return gbObjectsThreads ? (gapsObjectsThreads.NumElements() - 1) : 0;
}


//////////////////////////////////////////////////////////////////////////
//
// By a writer or, of an object it has seen attached to the root, a reader: it is the running thread's own table.
//
//////////////////////////////////////////////////////////////////////////
bool ThreadStackFromAddInside(CEmbeddedObject* pcObject, CPointer* pcPointer)
{
	SObjectsThread*		psThread;
	SThreadStackFrom*	psFrom;

	psThread = gpsObjectsThread;
	if ((psThread == NULL) || psThread->sReader.bHome || !pcObject->IsAttachedToRoot())
	{
		return false;
	}

	psFrom = psThread->asStackFroms.Add();
	psFrom->pcPointer = pcPointer;
	psFrom->pcObject = pcObject;
	if (!psThread->bKeeping)
	{
		psThread->bKeeping = true;
		AtomicAdd(&giObjectsThreadsKeeping, 1);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The uiThread'th registered thread if it keeps stack froms, NULL if it keeps none.  One that is counted as keeping
// them and has none left is uncounted: it counts itself again when it next keeps one.
//
//////////////////////////////////////////////////////////////////////////
static SObjectsThread* ObjectsThreadKeeping(size uiThread)
{
	SObjectsThread*		psThread;

	psThread = gapsObjectsThreads.GetPtr(uiThread);
	if (!psThread->bKeeping)
	{
		return NULL;
	}
	if (psThread->asStackFroms.NumElements() == 0)
	{
		psThread->bKeeping = false;
		AtomicAdd(&giObjectsThreadsKeeping, -1);
		return NULL;
	}
	return psThread;
}


//////////////////////////////////////////////////////////////////////////
//
// Takes pcPointer's from out of the thread's, if it is one of them: the last looked at first.  What was last takes
// its place.
//
//////////////////////////////////////////////////////////////////////////
static bool ObjectsThreadRemoveStackFrom(SObjectsThread* psThread, CPointer* pcPointer)
{
	SThreadStackFrom*	pasFroms;
	size				i;

	pasFroms = psThread->asStackFroms.GetData();
	i = psThread->asStackFroms.NumElements();
	while (i > 0)
	{
		i--;
		if (pasFroms[i].pcPointer == pcPointer)
		{
			psThread->asStackFroms.RemoveAt(i, false);
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// By a writer or a reader: it is the running thread's own table.
//
//////////////////////////////////////////////////////////////////////////
bool ThreadStackFromRemoveInside(CPointer* pcPointer)
{
	SObjectsThread*		psThread;

	psThread = gpsObjectsThread;
	return (psThread != NULL) && ObjectsThreadRemoveStackFrom(psThread, pcPointer);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool ThreadStackFromRemoveAnywhereInside(CPointer* pcPointer)
{
	SObjectsThread*		psThread;
	size				i;

	for (i = 0; i < gapsObjectsThreads.NumElements(); i++)
	{
		psThread = ObjectsThreadKeeping(i);
		if ((psThread != NULL) && ObjectsThreadRemoveStackFrom(psThread, pcPointer))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// How many of the thread's stack froms are of pcObject.  With bAny, 1 at the first found.
//
//////////////////////////////////////////////////////////////////////////
static size ObjectsThreadNumStackFroms(SObjectsThread* psThread, CEmbeddedObject* pcObject, bool bAny)
{
	SThreadStackFrom*	pasFroms;
	size				uiNumFroms;
	size				uiNum;
	size				i;

	pasFroms = psThread->asStackFroms.GetData();
	uiNumFroms = psThread->asStackFroms.NumElements();
	uiNum = 0;
	for (i = 0; i < uiNumFroms; i++)
	{
		if (pasFroms[i].pcObject == pcObject)
		{
			uiNum++;
			if (bAny)
			{
				break;
			}
		}
	}
	return uiNum;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool ThreadStackFromsHasInside(CEmbeddedObject* pcObject)
{
	SObjectsThread*		psThread;
	size				i;

	for (i = 0; i < gapsObjectsThreads.NumElements(); i++)
	{
		psThread = ObjectsThreadKeeping(i);
		if ((psThread != NULL) && (ObjectsThreadNumStackFroms(psThread, pcObject, true) != 0))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size ThreadStackFromsNumInside(CEmbeddedObject* pcObject)
{
	SObjectsThread*		psThread;
	size				uiNum;
	size				i;

	uiNum = 0;
	for (i = 0; i < gapsObjectsThreads.NumElements(); i++)
	{
		psThread = ObjectsThreadKeeping(i);
		if (psThread != NULL)
		{
			uiNum += ObjectsThreadNumStackFroms(psThread, pcObject, false);
		}
	}
	return uiNum;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool ThreadStackFromsHasOnAnotherThreadInside(CEmbeddedObject* pcObject)
{
	SObjectsThread*		psThread;
	size				i;

	for (i = 0; i < gapsObjectsThreads.NumElements(); i++)
	{
		psThread = ObjectsThreadKeeping(i);
		if ((psThread != NULL) && (psThread != gpsObjectsThread) && (ObjectsThreadNumStackFroms(psThread, pcObject, true) != 0))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size ThreadStackFromsNumOnOtherThreadsInside(void)
{
	SObjectsThread*		psThread;
	size				uiNum;
	size				i;

	uiNum = 0;
	for (i = 0; i < gapsObjectsThreads.NumElements(); i++)
	{
		psThread = ObjectsThreadKeeping(i);
		if ((psThread != NULL) && (psThread != gpsObjectsThread))
		{
			uiNum += psThread->asStackFroms.NumElements();
		}
	}
	return uiNum;
}


//////////////////////////////////////////////////////////////////////////
//
// The pointers are other threads': each is on its thread's stack, and is changed here as the home thread's are
// changed where they are, by whichever thread is freeing or loading the object.
//
//////////////////////////////////////////////////////////////////////////
size ThreadStackFromsClearInside(CEmbeddedObject* pcObject)
{
	SObjectsThread*		psThread;
	SThreadStackFrom*	pasFroms;
	size				uiNum;
	size				i;
	size				j;

	uiNum = 0;
	for (i = 0; i < gapsObjectsThreads.NumElements(); i++)
	{
		psThread = ObjectsThreadKeeping(i);
		if (psThread == NULL)
		{
			continue;
		}
		pasFroms = psThread->asStackFroms.GetData();
		j = psThread->asStackFroms.NumElements();
		while (j > 0)
		{
			j--;
			if (pasFroms[j].pcObject == pcObject)
			{
				pasFroms[j].pcPointer->UnsafeClearObject();
				psThread->asStackFroms.RemoveAt(j, false);
				pasFroms = psThread->asStackFroms.GetData();
				uiNum++;
			}
		}
	}
	return uiNum;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size ThreadStackFromsRemapInside(CEmbeddedObject* pcOld, CEmbeddedObject* pcNew)
{
	SObjectsThread*		psThread;
	SThreadStackFrom*	pasFroms;
	size				uiNumFroms;
	size				uiNum;
	size				i;
	size				j;

	uiNum = 0;
	for (i = 0; i < gapsObjectsThreads.NumElements(); i++)
	{
		psThread = ObjectsThreadKeeping(i);
		if (psThread == NULL)
		{
			continue;
		}
		pasFroms = psThread->asStackFroms.GetData();
		uiNumFroms = psThread->asStackFroms.NumElements();
		for (j = 0; j < uiNumFroms; j++)
		{
			if (pasFroms[j].pcObject == pcOld)
			{
				pasFroms[j].pcPointer->UnsafePointTo(pcNew);
				pasFroms[j].pcObject = pcNew;
				uiNum++;
			}
		}
	}
	return uiNum;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void ThreadStackFromsAddressesInside(CArrayVoidPtr* papvAddresses)
{
	SObjectsThread*		psThread;
	size				i;
	size				j;

	for (i = 0; i < gapsObjectsThreads.NumElements(); i++)
	{
		psThread = gapsObjectsThreads.GetPtr(i);
		for (j = 0; j < psThread->asStackFroms.NumElements(); j++)
		{
			papvAddresses->Add(psThread->asStackFroms.Get(j)->pcPointer);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void ThreadStackFromsPrintInside(CChars* psz, bool bPointerMemory)
{
	SObjectsThread*		psThread;
	SThreadStackFrom*	psFrom;
	size				i;
	size				j;

	for (i = 0; i < gapsObjectsThreads.NumElements(); i++)
	{
		psThread = gapsObjectsThreads.GetPtr(i);
		for (j = 0; j < psThread->asStackFroms.NumElements(); j++)
		{
			psFrom = psThread->asStackFroms.Get(j);
			if (bPointerMemory)
			{
				psz->AppendPointer(psFrom->pcPointer);
				psz->Append(" ");
			}
			psz->Append(j);
			psz->Append(": Pointer [");
			psFrom->pcObject->PrintObject(psz);
			psz->Append("] kept by thread ");
			psz->Append(psThread->iThread);
			psz->AppendNewLine();
		}
	}
}


#endif // !__LIBC65__

