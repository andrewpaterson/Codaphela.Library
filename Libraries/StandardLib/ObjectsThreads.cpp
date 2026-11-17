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


//What is kept for a registered thread: the stack froms it keeps itself.
struct SObjectsThread
{
	CArrayThreadStackFrom	asStackFroms;
	int						iThread;
	bool					bKeeping;	//Counted in giObjectsThreadsKeeping.
};


typedef CArrayTemplatePtr<SObjectsThread>	CArrayObjectsThreadPtr;


bool								gbObjectsThreads = false;
int32								giObjectsThreadsKeeping = 0;

static CArrayObjectsThreadPtr		gapsObjectsThreads;		//Every registered thread's.  Initialised while there are any.

#ifdef _MSC_VER
static __declspec(thread) SObjectsThread*	gpsObjectsThread = NULL;	//The running thread's, NULL if it isn't registered.
#else
static __thread SObjectsThread*				gpsObjectsThread = NULL;
#endif // _MSC_VER


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void ObjectsThreadsAdd(void)
{
	SObjectsThread*		psThread;

	if (!gbObjectsThreads)
	{
		gapsObjectsThreads.Init();
		gbObjectsThreads = true;
	}

	psThread = (SObjectsThread*)malloc(sizeof(SObjectsThread));
	psThread->asStackFroms.Init();
	psThread->iThread = CurrentThreadId();
	psThread->bKeeping = false;
	gapsObjectsThreads.Add(psThread);
	gpsObjectsThread = psThread;
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
	if (psThread == NULL)
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

	if (psThread->bKeeping)
	{
		AtomicAdd(&giObjectsThreadsKeeping, -1);
	}
	gapsObjectsThreads.RemoveAt(gapsObjectsThreads.Find(psThread), false);
	psThread->asStackFroms.Kill();
	free(psThread);
	gpsObjectsThread = NULL;

	if (gapsObjectsThreads.NumElements() == 0)
	{
		gapsObjectsThreads.Kill();
		gbObjectsThreads = false;
	}
	return uiStackFroms;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size ObjectsThreadsNum(void)
{
	return gbObjectsThreads ? gapsObjectsThreads.NumElements() : 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool ThreadStackFromAddInside(CEmbeddedObject* pcObject, CPointer* pcPointer)
{
	SObjectsThread*		psThread;
	SThreadStackFrom*	psFrom;

	psThread = gpsObjectsThread;
	if ((psThread == NULL) || (pcObject->GetEmbeddingContainer()->GetDistToRoot() < ROOT_DIST_TO_ROOT))
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

