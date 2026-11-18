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
#include "BaseLib/Logger.h"
#include "BaseLib/LogString.h"
#include "Collection.h"
#include "Object.h"
#include "Objects.h"
#include "StackPointer.h"


//A worker's objects adopted by the main object system (ThreadSafetyPlan.txt, Level 3 as built, step 25d).  The graph
//is the object and every object of the worker's it reaches; it is moved, not copied, under the main lock, by the
//worker (it is the worker's object system that it leaves).


//////////////////////////////////////////////////////////////////////////
//
// The running thread, a worker, gives pTop's graph to the main object
// system.  False, with nothing moved, if it can't: it isn't a worker,
// pTop isn't its, or a name in the graph is main's already.  pTop then
// points at a main object, which isn't attached to the main root until
// something attached points at it.
//
//////////////////////////////////////////////////////////////////////////
bool ObjectsAdopt(CPointer& pTop)
{
	return gcObjects.Adopt(pTop);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether pcObject is in the graph: its container has the graph's walk
// stamp.
//
//////////////////////////////////////////////////////////////////////////
static bool IsAdopted(CEmbeddedObject* pcObject, uint32 uiStamp)
{
	return pcObject->GetEmbeddingContainer()->GetWalkStamp() == uiStamp;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether pvAddress is inside an object of the graph (a weak holder is a
// field or a collection of the object it is in, which it doesn't know).
// papcGraph is sorted by address.
//
//////////////////////////////////////////////////////////////////////////
static bool IsInAdoptedMemory(void* pvAddress, CArrayBaseObjectPtr* papcGraph)
{
	size			uiLow;
	size			uiHigh;
	size			uiMiddle;
	CBaseObject*	pcObject;

	uiLow = 0;
	uiHigh = papcGraph->NumElements();
	while (uiLow < uiHigh)
	{
		uiMiddle = uiLow + (uiHigh - uiLow) / 2;
		pcObject = papcGraph->GetPtr(uiMiddle);
		if ((char*)pvAddress < (char*)pcObject)
		{
			uiHigh = uiMiddle;
		}
		else if ((char*)pvAddress >= ((char*)pcObject + pcObject->ClassSize()))
		{
			uiLow = uiMiddle + 1;
		}
		else
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
bool CObjects::Adopt(CPointer& pTop)
{
	OBJECTS_WRITE();

	CObjects*			pcWorker;
	CBaseObject*		pcTop;
	CArrayBaseObjectPtr	apcGraph;
	CArrayAdoptFrom		asFroms;
	uint32				uiStamp;
	size				i;
	SAdoptFrom*			psFrom;
	bool				bResult;

	pcWorker = ObjectsOwn();
	pcTop = (pTop.Object() != NULL) ? pTop.Object()->GetEmbeddingContainer() : NULL;
	if ((pcWorker == NULL) || (pcWorker == this) || (pcTop == NULL) || (pcTop->GetObjectsThisIn() != pcWorker))
	{
		return gcLogger.Error2(__METHOD__, " A worker adopts an object of its own.", NULL);
	}

	apcGraph.Init();
	uiStamp = AdoptGraph(pcWorker, pcTop, &apcGraph);
	bResult = CanAdoptNames(&apcGraph);
	if (bResult)
	{
		asFroms.Init();
		for (i = 0; i < apcGraph.NumElements(); i++)
		{
			TakeAdoptedFroms(apcGraph.GetPtr(i), uiStamp, &asFroms);
		}
		apcGraph.QuickSort();
		AdoptWeakFroms(pcWorker, &apcGraph, uiStamp);
		for (i = 0; i < apcGraph.NumElements(); i++)
		{
			MoveAdopted(pcWorker, apcGraph.GetPtr(i));
		}
		for (i = 0; i < apcGraph.NumElements(); i++)
		{
			HoldAdoptedTos(apcGraph.GetPtr(i), uiStamp);
		}
		for (i = 0; i < asFroms.NumElements(); i++)
		{
			psFrom = asFroms.Get(i);
			if (psFrom->pcPointer != NULL)
			{
				psFrom->pcTo->AddStackFrom(psFrom->pcPointer, false);
			}
			else
			{
				psFrom->pcTo->AddStackFrom(psFrom->pcCollection, false);
			}
		}
		asFroms.Kill();
	}

	for (i = 0; i < apcGraph.NumElements(); i++)
	{
		apcGraph.GetPtr(i)->SetWalkStamp(0);
	}
	apcGraph.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// pcTop and every object of the worker's it reaches (a worker's walk
// doesn't go into main objects), stamped.  The stamp.
//
//////////////////////////////////////////////////////////////////////////
uint32 CObjects::AdoptGraph(CObjects* pcWorker, CBaseObject* pcTop, CArrayBaseObjectPtr* papcGraph)
{
	uint32					uiStamp;
	CArrayEmbeddedObjectPtr	apcTos;
	CBaseObject*			pcObject;
	CBaseObject*			pcTo;
	size					uiNext;
	size					i;

	uiStamp = pcWorker->StartWalk();
	pcTop->SetWalkStamp(uiStamp);
	papcGraph->Add(pcTop);
	apcTos.Init();
	for (uiNext = 0; uiNext < papcGraph->NumElements(); uiNext++)
	{
		pcObject = papcGraph->GetPtr(uiNext);
		apcTos.ReInit();
		pcObject->GetPointerTos(&apcTos);
		for (i = 0; i < apcTos.NumElements(); i++)
		{
			pcTo = apcTos.GetPtr(i)->GetEmbeddingContainer();
			if (pcTo->GetWalkStamp() != uiStamp)
			{
				pcTo->SetWalkStamp(uiStamp);
				papcGraph->Add(pcTo);
			}
		}
	}
	apcTos.Kill();
	return uiStamp;
}


//////////////////////////////////////////////////////////////////////////
//
// No name in the graph is the main object system's already, in memory or
// in its database (decision 14): the whole graph is refused otherwise.
//
//////////////////////////////////////////////////////////////////////////
bool CObjects::CanAdoptNames(CArrayBaseObjectPtr* papcGraph)
{
	CBaseObject*	pcObject;
	size			i;

	for (i = 0; i < papcGraph->NumElements(); i++)
	{
		pcObject = papcGraph->GetPtr(i);
		if (pcObject->IsNamed() && Contains(pcObject->GetName()))
		{
			return gcLogger.Error2(__METHOD__, " Cannot adopt a graph with an object named [", pcObject->GetName(), "]: the main object system has one.  Nothing is adopted.", NULL);
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The froms of pcObject (and of its embedded objects) that aren't in the
// graph come off while it moves, kept in pasFroms: the worker's stack
// pointers and collections, and the heap froms of the worker's objects
// outside the graph (a field or a collection of a worker's object, which
// holds a main object once the graph is main's).  Unlinked: nothing is
// freed.
//
//////////////////////////////////////////////////////////////////////////
void CObjects::TakeAdoptedFroms(CBaseObject* pcObject, uint32 uiStamp, CArrayAdoptFrom* pasFroms)
{
	CEmbeddedObject*		pcEmbedded;
	CStackPointer*			pcStackPointer;
	SStackPointer*			psStackPointer;
	SAdoptFrom*				psFrom;
	CArrayBaseObjectPtr		apcOutside;
	CArrayHeapFromPointer	asFields;
	CBaseObject*			pcFrom;
	size					uiFirst;
	size					i;
	size					j;

	apcOutside.Init();
	asFields.Init();
	for (i = 0; i < pcObject->NumEmbedded(); i++)
	{
		pcEmbedded = pcObject->GetEmbeddedObject(i);

		uiFirst = pasFroms->NumElements();
		for (pcStackPointer = pcEmbedded->GetFirstStackFrom(); pcStackPointer != NULL; pcStackPointer = pcStackPointer->GetNext())
		{
			psStackPointer = pcStackPointer->Get();
			psFrom = pasFroms->Add();
			psFrom->pcTo = pcEmbedded;
			psFrom->pcPointer = (psStackPointer->eType == SPT_Pointer) ? psStackPointer->u.pcPointer : NULL;
			psFrom->pcCollection = (psStackPointer->eType == SPT_Collection) ? psStackPointer->u.pcCollection : NULL;
		}
		for (j = uiFirst; j < pasFroms->NumElements(); j++)
		{
			psFrom = pasFroms->Get(j);
			if (psFrom->pcPointer != NULL)
			{
				pcEmbedded->UnlinkStackFrom(psFrom->pcPointer);
			}
			else
			{
				pcEmbedded->UnlinkStackFrom(psFrom->pcCollection);
			}
		}

		//Each heap from of an object outside the graph is one of its fields, or a place in it if it is a collection.
		apcOutside.ReInit();
		for (j = 0; j < pcEmbedded->CEmbeddedObject::NumHeapFroms(); j++)
		{
			pcFrom = pcEmbedded->GetHeapFrom(j);
			if (!IsAdopted(pcFrom, uiStamp))
			{
				apcOutside.Add(pcFrom);
			}
		}
		for (j = 0; j < apcOutside.NumElements(); j++)
		{
			pcFrom = apcOutside.GetPtr(j);
			pcEmbedded->UnlinkHeapFrom(pcFrom);
			psFrom = pasFroms->Add();
			psFrom->pcTo = pcEmbedded;
			psFrom->pcPointer = NULL;
			psFrom->pcCollection = pcFrom->IsCollection() ? (CCollection*)pcFrom : NULL;
		}
		RecordAdoptedFields(pcEmbedded, &apcOutside, pasFroms, &asFields);
	}
	asFields.Kill();
	apcOutside.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The froms TakeAdoptedFroms took of the objects (not collections) in
// papcOutside are fields: which fields, in the order their heap froms
// were, each of them once.
//
//////////////////////////////////////////////////////////////////////////
void CObjects::RecordAdoptedFields(CEmbeddedObject* pcEmbedded, CArrayBaseObjectPtr* papcOutside, CArrayAdoptFrom* pasFroms, CArrayHeapFromPointer* pasFields)
{
	CBaseObject*		pcFrom;
	SAdoptFrom*			psFrom;
	size				uiTaken;
	size				i;
	size				j;
	size				k;

	//The collections' are recorded; the fields' are filled in here, each object's fields once.
	uiTaken = pasFroms->NumElements() - papcOutside->NumElements();
	for (i = 0; i < papcOutside->NumElements(); i++)
	{
		pcFrom = papcOutside->GetPtr(i);
		psFrom = pasFroms->Get(uiTaken + i);
		if ((psFrom->pcCollection != NULL) || (psFrom->pcPointer != NULL))
		{
			continue;
		}

		pasFields->ReInit();
		pcFrom->GetPointerFieldsTo(pcEmbedded, pasFields);
		k = 0;
		for (j = i; (j < papcOutside->NumElements()) && (k < pasFields->NumElements()); j++)
		{
			if (papcOutside->GetPtr(j) == pcFrom)
			{
				pasFroms->Get(uiTaken + j)->pcPointer = pasFields->Get(k)->pcPointer;
				k++;
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The worker's weak references to objects of the graph from inside it go
// to the main object system with them.  Those across the edge of the graph
// (in or out) are cleared: their holders would be cleared by one object
// system's thread while another used them.  papcGraph is sorted by
// address.
//
//////////////////////////////////////////////////////////////////////////
void CObjects::AdoptWeakFroms(CObjects* pcWorker, CArrayBaseObjectPtr* papcGraph, uint32 uiStamp)
{
	CArrayEmbeddedObjectPtr	apcHeld;
	CEmbeddedObject*		pcHeld;
	SWeakFrom*				psFrom;
	CWeakHolder*			pcHolder;
	size					uiSlot;
	bool					bHeldAdopted;
	size					i;
	size					j;

	apcHeld.Init();
	pcWorker->mcWeakFroms.GetObjects(&apcHeld);
	for (i = 0; i < apcHeld.NumElements(); i++)
	{
		pcHeld = apcHeld.GetPtr(i);
		bHeldAdopted = IsAdopted(pcHeld, uiStamp);
		j = pcWorker->mcWeakFroms.NumWeakFroms(pcHeld);
		while (j > 0)
		{
			j--;
			psFrom = pcWorker->mcWeakFroms.GetWeakFrom(pcHeld, j);
			pcHolder = psFrom->pcHolder;
			uiSlot = psFrom->uiSlot;
			if (bHeldAdopted != IsInAdoptedMemory(pcHolder, papcGraph))
			{
				pcHolder->ClearWeak(uiSlot);
				pcWorker->mcWeakFroms.Remove(pcHeld, pcHolder, uiSlot);
			}
			else if (bHeldAdopted)
			{
				mcWeakFroms.Add(pcHeld, pcHolder, uiSlot);
				pcWorker->mcWeakFroms.Remove(pcHeld, pcHolder, uiSlot);
			}
		}
	}
	apcHeld.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// pcObject leaves the worker's memory for main's: its index and name, its
// object system, its classes (main's, completed now if main has none
// yet), not attached to the main root, and to be saved.
//
//////////////////////////////////////////////////////////////////////////
void CObjects::MoveAdopted(CObjects* pcWorker, CBaseObject* pcObject)
{
	CBaseObject*	pcEmbedded;
	size			i;

	pcWorker->mcMemory.RemoveIndex(pcObject->GetIndex());
	if (pcObject->IsNamed())
	{
		pcWorker->mcMemory.RemoveName(pcObject->GetName());
	}

	//The embedded objects' classes first: a class completed now has its embedded fields' classes.
	pcObject->mpcObjectsThisIn = this;
	i = pcObject->NumEmbedded();
	while (i > 0)
	{
		i--;
		pcEmbedded = (CBaseObject*)pcObject->GetEmbeddedObject(i);
		pcEmbedded->mpcClass = NULL;
		pcEmbedded->CompleteClass(&mcClasses);
	}
	pcObject->SetDistToRoot(UNATTACHED_DIST_TO_ROOT);
	pcObject->SetFlag(OBJECT_FLAGS_DIRTY, true);
	mcMemory.Add(pcObject);
}


//////////////////////////////////////////////////////////////////////////
//
// pcObject, main's now, points at main objects outside the graph that it
// held as a worker's object does (a stack from of its field or collection,
// IsAcross): held by heap froms now, as any main object's are.
//
//////////////////////////////////////////////////////////////////////////
void CObjects::HoldAdoptedTos(CBaseObject* pcObject, uint32 uiStamp)
{
	CBaseObject*			pcEmbedded;
	CArrayEmbeddedObjectPtr	apcTos;
	CEmbeddedObject*		pcTo;
	CObject*				pcFields;
	CPointer*				pcPointer;
	size					i;
	size					j;

	apcTos.Init();
	for (i = 0; i < pcObject->NumEmbedded(); i++)
	{
		pcEmbedded = (CBaseObject*)pcObject->GetEmbeddedObject(i);
		if (pcEmbedded->IsCollection())
		{
			apcTos.ReInit();
			pcEmbedded->BaseGetPointerTos(&apcTos);
			for (j = 0; j < apcTos.NumElements(); j++)
			{
				pcTo = apcTos.GetPtr(j);
				if (!IsAdopted(pcTo, uiStamp))
				{
					pcTo->UnlinkStackFrom((CCollection*)pcEmbedded);
					pcTo->AddHeapFrom(pcEmbedded, false);
				}
			}
		}
		else if (pcEmbedded->IsObject())
		{
			pcFields = (CObject*)pcEmbedded;
			for (j = 0; j < pcFields->NumPointerFields(); j++)
			{
				pcPointer = pcFields->GetPointerField(j);
				pcTo = pcPointer->Object();
				if ((pcTo != NULL) && !IsAdopted(pcTo, uiStamp))
				{
					pcTo->UnlinkStackFrom(pcPointer);
					pcTo->AddHeapFrom(pcFields, false);
				}
			}
		}
	}
	apcTos.Kill();
}
