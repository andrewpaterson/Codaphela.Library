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
#include "BaseLib/GlobalMemory.h"
#include "BaseLib/StackMemory.h"
#include "BaseLib/DataConnection.h"
#include "BaseLib/NativeThread.h"
#include "BaseLib/TransientSequence.h"
#include "BaseObject.h"
#include "InternalObjectSerialiser.h"
#include "InternalObjectDeserialiser.h"
#include "ObjectSource.h"
#include "ObjectRemapFrom.h"
#include "ObjectReader.h"
#include "BaseLib/MemoryFile.h"
#include "HollowObject.h"
#include "HollowEmbeddedObject.h"
#include "FreeObjects.h"
#include "StackPointers.h"
#include "KillOrderCalculator.h"
#include "Objects.h"
#include "ObjectsLock.h"
#include "ObjectsThreads.h"


CObjects	gcObjects;
bool		gbObjects = false;
size		guiObjectsForwarders = 0;
#ifndef __LIBC65__
OBJECTS_PER_THREAD CObjects*	gpcObjectsOwn = NULL;
#endif // __LIBC65__


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void LogObjectAllocation(CBaseObject* pcObject, const char* szMethod)
{
#ifdef DEBUG_OBJECT_ALLOCATION
#ifdef _DEBUG
	char*	szClass;
	char*	szName;
	char*	szIndex;
	char*	szAddress;

	if (pcObject != NULL)
	{
		szClass = pcObject->ClassName();
		szIndex = IndexToString(pcObject->GetIndex());
		szName = pcObject->GetName();
		szAddress = PointerToString(pcObject);
		gcLogger.Debug2(szMethod, " Allocate [", szClass, ": ", szIndex, " ", szName, " (", szAddress, ")]", NULL);
	}
	else
	{
		gcLogger.Debug2(szMethod, " Allocate [NULL]", NULL);
	}
#endif // _DEBUG
#endif // DEBUG_OBJECT_ALLOCATION
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void LogObjectAllocationFailure(const char* szClass, OIndex oi, const char* szName, const char* szMethod)
{
#ifdef _DEBUG
	char* szIndex;

	szIndex = IndexToString(oi);
	gcLogger.Debug2(szMethod, " Allocate FAILED! [", szClass, ": ", szIndex, " ", szName, "]", NULL);
#endif // _DEBUG
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void LogObjectDestruction(CBaseObject* pcObject, const char* szMethod)
{
#ifdef DEBUG_OBJECT_ALLOCATION
#ifdef _DEBUG
	char*	szClass;
	char*	szName;
	char*	szIndex;
	char*	szAddress;

	if (pcObject != NULL)
	{
		szClass = pcObject->ClassName();
		szIndex = IndexToString(pcObject->GetIndex());
		szName = pcObject->GetName();
		szAddress = PointerToString(pcObject);
		gcLogger.Debug2(szMethod, " Kill [", szClass, ": ", szIndex, " ", szName, " (", szAddress, ")]", NULL);
	}
	else
	{
		gcLogger.Debug2(szMethod, " Kill [NULL]", NULL);
	}
#endif // _DEBUG
#endif // DEBUG_OBJECT_ALLOCATION
}


//////////////////////////////////////////////////////////////////////////
//
//
////////////////////////////////////////////////////////////////////////////
void ObjectCallbackNotSet(CBaseObject* pcObject)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
////////////////////////////////////////////////////////////////////////////
CObjects::CObjects()
{
	mbInitialised = false;
	mpcUnknownsAllocatingFrom = NULL;
	mpcDataConnection = NULL;
	mpcSequenceConnection = NULL;
	muiWalkStamp = 0;
	muiForwardersMarked = 0;
	mbWorker = false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjects::Init(CUnknowns* pcUnknownsAllocatingFrom, CStackPointers* pcStackPointers, CDataConnection* pcDataConnection, CSequenceConnection* pcSequenceConnection)
{
	mpcUnknownsAllocatingFrom = pcUnknownsAllocatingFrom;
	mpcStackPointers = pcStackPointers;

	mpcDataConnection = pcDataConnection;
	mpcSequenceConnection = pcSequenceConnection;

	mAllocationCallback = &ObjectCallbackNotSet;
	mHollowAllocationCallback = &ObjectCallbackNotSet;
	mDestructionCallback = &ObjectCallbackNotSet;

	mcMemory.Init();
	mcSource.Init(this);
	mcClasses.Init(this);

	mbInitialised = true;
	mbValidateConsistency = true;
	mcRemovalContext.Init();
	mcWeakFroms.Init();
	mapcWaitingToBeFreed.Init();
	masForwarders.Init();
	muiForwardersMarked = 0;
	mbWorker = false;
}


//////////////////////////////////////////////////////////////////////////
//
// A worker thread's own object system (ThreadSafetyPlan.txt, Level 3 as
// built): no database, its indexes from the main object system's
// sequence, and the worker its home.  Its unknowns allocate from what the
// main one's do (CUnknowns::InitSharing), and it validates as main does.
//
//////////////////////////////////////////////////////////////////////////
void CObjects::InitWorker(CObjects* pcMain, CUnknowns* pcUnknowns, CStackPointers* pcStackPointers, CSequenceConnection* pcSequenceConnection)
{
	Init(pcUnknowns, pcStackPointers, NULL, pcSequenceConnection);
	mbValidateConsistency = pcMain->mbValidateConsistency;
	mbWorker = true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjects::Kill(void)
{
	OBJECTS_WRITE();

	mbInitialised = false;

	//The objects' memory is given back one by one: what allocates them (the global unknowns, from gcSystemAllocator)
	//outlives this, and doesn't give its memory back as a whole.  No other thread uses objects now.
	Collect();
	FreeForwardersInside(true);
	ValidateNoDirty();
	FreeAllInKillOrder(true);

	masForwarders.Kill();
	mapcWaitingToBeFreed.Kill();
	mcRemovalContext.Kill();
	mcWeakFroms.Kill();

	mpcDataConnection = NULL;
	mpcSequenceConnection = NULL;

	mcClasses.Kill();
	mcSource.Kill();
	mcMemory.Kill();

	mpcUnknownsAllocatingFrom = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// A from of pcTo (a heap from if bHeapFrom, otherwise a stack from) has
// been unlinked.  Frees what can no longer be reached: the one place that
// decides.
//
//////////////////////////////////////////////////////////////////////////
EFreeResult CObjects::FromRemoved(CEmbeddedObject* pcTo, bool bHeapFrom)
{
	OBJECTS_INSIDE();

	CBaseObject*			pcObject;
	CDistCalculator			cDistCalculator;
	CArrayBaseObjectPtr*	papcKilled;
	EFreeResult				eResult;

	pcObject = pcTo->GetEmbeddingContainer();
	if (pcObject->IsRoot())
	{
		return FR_NotFreed;
	}

	//Already in the batch being freed (a pointer changed in a Free()).  Working out what this detaches would find the
	//whole batch again, for every such change.
	if (pcObject->GetFlags() & OBJECT_FLAGS_FREED)
	{
		return FR_NotFreed;
	}

	//Only a stack from went and it is still attached to the root.
	if ((pcObject->GetDistToRoot() > ROOT_DIST_TO_ROOT) && !bHeapFrom)
	{
		return FR_NotFreed;
	}

	cDistCalculator.Init(this);
	papcKilled = cDistCalculator.Calculate(pcObject, bHeapFrom);

	//Only objects allocated in Objects are ever detached: an object on the stack is killed when it goes out of scope.
	if (papcKilled->NumElements() == 0)
	{
		eResult = FR_NotFreed;
	}
	else
	{
		eResult = FR_Freed;
		RemoveInKillOrder(papcKilled);
	}

	cDistCalculator.Kill();
	return eResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Objects with Nes pointers to them are removed after the objects that
// depend on them, in a batch of their own, each after the others in it
// that hold Nes pointers to it.
//
//////////////////////////////////////////////////////////////////////////
void CObjects::RemoveInKillOrder(CArrayBaseObjectPtr* papcKilled)
{
	OBJECTS_INSIDE();

	CKillOrderCalculator	cKillOrder;
	CArrayBaseObjectPtr*	papcKilledNesObjects;

	//Calculate removes Nes objects from papcKilled and puts them into papcKilledNesObjects.
	cKillOrder.Init(papcKilled);
	papcKilledNesObjects = cKillOrder.Calculate();
	Remove(papcKilled, papcKilledNesObjects);
	cKillOrder.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A new walk: the objects it visits are stamped with what this returns
// (and the steps after it, see WALK_STAMP_STEP), so it can tell them from
// those it hasn't.  A walk started while another is still under way
// restamps what it visits, and the first then takes those for unvisited:
// a walk that calls code that walks (a validator, CanFindRoot) must not
// rely on its stamps afterwards.  Validation uses flags (DUMPED,
// TESTED_FOR_SANITY) for that reason.  Nothing ends a walk, so this can't
// be checked.
//
//////////////////////////////////////////////////////////////////////////
uint32 CObjects::StartWalk(void)
{
	OBJECTS_INSIDE();

	muiWalkStamp += WALK_STAMP_STEP;
	if (muiWalkStamp == 0)
	{
		ClearWalkStamps();
	}
	return muiWalkStamp;
}


//////////////////////////////////////////////////////////////////////////
//
// The stamps have come round to 0: every object's is cleared, so that
// none looks visited to the walks to come.  Apart from StartWalk, which
// every walk calls: this is seldom.
//
//////////////////////////////////////////////////////////////////////////
NO_INLINE void CObjects::ClearWalkStamps(void)
{
	SIndexesIterator	sIter;
	CBaseObject*		pcBaseObject;

	if (mbInitialised)
	{
		pcBaseObject = mcMemory.StartIteration(&sIter);
		while (pcBaseObject)
		{
			pcBaseObject->SetWalkStamp(0);
			pcBaseObject = mcMemory.Iterate(&sIter);
		}
	}
	muiWalkStamp = WALK_STAMP_STEP;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjects::TestSetWalkStamp(uint32 uiWalkStamp)
{
	muiWalkStamp = uiWalkStamp;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjects::SetAllocationCallback(ObjectCallback fCallback)
{
	mAllocationCallback = fCallback;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjects::SetHollowAllocationCallback(ObjectCallback fCallback)
{
	mHollowAllocationCallback = fCallback;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjects::SetDestructionCallback(ObjectCallback fCallback)
{
	mDestructionCallback = fCallback;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CObjects::Flush(void)
{
	OBJECTS_WRITE();

	SIndexesIterator	sIter;
	OIndex				oi;
	bool				bResult;
	CBaseObject*		pcBaseObject;
	bool				bCanFindRoot;
	bool				bDirty;

	if (mpcDataConnection)
	{
		bResult = true;
		oi = StartMemoryIteration(&sIter);
		while (oi != INVALID_O_INDEX)
		{
			pcBaseObject = GetFromMemory(oi);
			bResult &= pcBaseObject->Flush();
			oi = IterateMemory(&sIter);
		}

		bResult &= mpcDataConnection->Flush(false);
		return bResult;
	}
	else
	{
		//No backing data connection so just discard everything.  I.e. mark it as saved.
		oi = StartMemoryIteration(&sIter);
		while (oi != INVALID_O_INDEX)
		{
			pcBaseObject = GetFromMemory(oi);
			bCanFindRoot = pcBaseObject->GetDistToRoot() != UNATTACHED_DIST_TO_ROOT;
			bDirty = pcBaseObject->IsDirty();

			if (bDirty && bCanFindRoot)
			{
				pcBaseObject->SetDirty(false);
			}
			oi = IterateMemory(&sIter);
		}

		return true;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CObjects::ValidateNoDirty(void)
{
	OBJECTS_READ();

	//ObjectsFlush or ObjectsDiscard must be called before Objects is killed.
	return mcMemory.ValidateNoDirty();
}


//////////////////////////////////////////////////////////////////////////
//
// Throws away every change in memory: each object is marked clean, so
// nothing is saved and Kill finds nothing dirty.
//
//////////////////////////////////////////////////////////////////////////
void CObjects::Discard(void)
{
	OBJECTS_WRITE();

	SIndexesIterator	sIter;
	OIndex				oi;

	oi = StartMemoryIteration(&sIter);
	while (oi != INVALID_O_INDEX)
	{
		GetFromMemory(oi)->SetDirty(false);
		oi = IterateMemory(&sIter);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Collects the holders a serialised object was saved with (their
// OIndexes) instead of allocating anything.
//
//////////////////////////////////////////////////////////////////////////
class CStoredHeapFromsCollector : public CDependentReadObjects
{
public:
	CArrayOIndex*		mpaoiHolders;
	CDataConnection*	mpcDataConnection;

	bool AddDependent(CObjectIdentifier* pcHeader, CEmbeddedObject** ppcPtrToBeUpdated, CBaseObject* pcObjectContainingPtrToBeUpdated, uint16 iNumEmbedded, uint16 iEmbeddedIndex) override
	{
		return true;
	}

	bool AddReverseDependent(CObjectIdentifier* pcHeader, CEmbeddedObject** ppcPtrToBeUpdated, CBaseObject* pcObjectContainingHeapFrom, uint16 iNumEmbedded, uint16 iEmbeddedIndex) override
	{
		OIndex	oi;

		oi = pcHeader->moi;
		if ((oi == INVALID_O_INDEX) && pcHeader->IsNamed())
		{
			oi = mpcDataConnection->GetIndex(pcHeader->mszObjectName.Text());
		}
		if ((oi != INVALID_O_INDEX) && (oi != NULL_O_INDEX))
		{
			mpaoiHolders->Add(oi);
		}
		return true;
	}

	CBaseObject* AllocateForDeserialisation(CObjectHeader* pcHeader) override
	{
		return NULL;
	}
};


//////////////////////////////////////////////////////////////////////////
//
// The holders pcObject's record in the database was saved with.
//
//////////////////////////////////////////////////////////////////////////
bool CObjects::GetStoredHeapFroms(CBaseObject* pcObject, CArrayOIndex* paoiHolders)
{
	OBJECTS_WRITE();

	CStoredHeapFromsCollector	cCollector;
	CObjectReader				cReader;
	CMemoryFile					cMemoryFile;
	CRecordMemory				cTemp;
	void*						pvSerialised;
	lsize						uiDataSize;
	bool						bResult;

	pvSerialised = CInternalObjectDeserialiser::GetRecord(mpcDataConnection, pcObject->GetIndex(), &cTemp, &uiDataSize);
	if (pvSerialised == NULL)
	{
		return gcLogger.Error2(__METHOD__, " Object [", IndexToString(pcObject->GetIndex()), "] is not in the database.", NULL);
	}

	cCollector.mpaoiHolders = paoiHolders;
	cCollector.mpcDataConnection = mpcDataConnection;
	cMemoryFile.Init(pvSerialised, uiDataSize);
	cMemoryFile.Open(EFM_Read);
	cReader.Init(&cMemoryFile, &cCollector);
	bResult = cReader.ReadStoredHeapFroms(pcObject);
	cMemoryFile.Close();
	cReader.Kill();
	cMemoryFile.Kill();
	cTemp.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// The object oi in memory, loaded if it is not or is a hollow.
//
//////////////////////////////////////////////////////////////////////////
bool CObjects::LoadForDestroy(OIndex oi)
{
	OBJECTS_INSIDE();

	CBaseObject*	pcObject;

	pcObject = GetFromMemory(oi);
	if ((pcObject != NULL) && !pcObject->IsHollow())
	{
		return true;
	}
	if (pcObject != NULL)
	{
		return pcObject->Dehollow() != NULL;
	}
	return Dehollow(oi) != NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Deletes pcObject from the database as well as from memory (see
// Destroy(CArrayOIndex*)).  An object that was never stored is freed with
// the pointers to it cleared.
//
//////////////////////////////////////////////////////////////////////////
bool CObjects::Destroy(CBaseObject* pcObject)
{
	OBJECTS_WRITE();

	CArrayOIndex	aoiDestroyed;
	bool			bResult;

	if (pcObject == NULL)
	{
		return gcLogger.Error2(__METHOD__, " Cannot destroy object [NULL].", NULL);
	}
	if (pcObject->IsEmbedded())
	{
		return gcLogger.Error2(__METHOD__, " Cannot destroy an embedded object.  Destroy the object it is embedded in.", NULL);
	}
	if (!pcObject->IsStored() || (mpcDataConnection == NULL))
	{
		if (pcObject->IsNamed(ROOT_NAME))
		{
			return gcLogger.Error2(__METHOD__, " Cannot destroy the root.", NULL);
		}
		pcObject->Kill();
		return true;
	}

	aoiDestroyed.Init();
	aoiDestroyed.Add(pcObject->GetIndex());
	bResult = Destroy(&aoiDestroyed);
	aoiDestroyed.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Adds oi to paoi if it is not there.
//
//////////////////////////////////////////////////////////////////////////
static void AddUniqueIndex(CArrayOIndex* paoi, OIndex oi)
{
	if (paoi->Find(oi) == ARRAY_LARGE_ELEMENT_NOT_FOUND)
	{
		paoi->Add(oi);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Deletes the objects paoiDestroyed lists from the database as well as
// from memory.  Everything that points to them (their stored holders,
// read from their records) and everything they point to is loaded; the
// pointers from outside the set to them are cleared and those objects are
// saved without them; then their records and names are removed.
// Pointers between the objects destroyed are not saved: they all go.  The
// objects they held that are not destroyed are left in the database for
// the database's garbage collection (objects that are not loaded may
// still hold them).
//
//////////////////////////////////////////////////////////////////////////
bool CObjects::Destroy(CArrayOIndex* paoiDestroyed)
{
	OBJECTS_WRITE();

	CArrayOIndex				aoiHolders;
	CArrayOIndex				aoiChanged;
	CArrayEmbeddedObjectPtr		apcTos;
	CEmbeddedObject*			pcEmbedded;
	CBaseObject*				pcObject;
	CBaseObject*				pcChanged;
	OIndex						oi;
	OIndex						oiOther;
	size						i;
	size						j;
	size						k;
	bool						bResult;
	bool						bValidateConsistency;

	if (mpcDataConnection == NULL)
	{
		return gcLogger.Error2(__METHOD__, " Cannot destroy objects without a database.", NULL);
	}

	//The objects destroyed, everything that points to them, as the database has it, and everything they point to, in memory.
	for (i = 0; i < paoiDestroyed->NumElements(); i++)
	{
		oi = paoiDestroyed->GetValue(i);
		if (!LoadForDestroy(oi))
		{
			return gcLogger.Error2(__METHOD__, " Could not load object [", IndexToString(oi), "] to destroy it.", NULL);
		}
		pcObject = GetFromMemory(oi);
		if (pcObject->IsNamed(ROOT_NAME))
		{
			return gcLogger.Error2(__METHOD__, " Cannot destroy the root.", NULL);
		}
	}
	bResult = true;
	for (i = 0; bResult && (i < paoiDestroyed->NumElements()); i++)
	{
		pcObject = GetFromMemory(paoiDestroyed->GetValue(i));
		if (pcObject->IsStored())
		{
			aoiHolders.Init();
			bResult = GetStoredHeapFroms(pcObject, &aoiHolders);
			for (j = 0; bResult && (j < aoiHolders.NumElements()); j++)
			{
				bResult = LoadForDestroy(aoiHolders.GetValue(j));
			}
			aoiHolders.Kill();
		}

		apcTos.Init();
		pcObject->GetPointerTos(&apcTos);
		for (j = 0; bResult && (j < apcTos.NumElements()); j++)
		{
			bResult = LoadForDestroy(apcTos.GetPtr(j)->GetEmbeddingContainer()->GetIndex());
		}
		apcTos.Kill();
	}
	if (!bResult)
	{
		return gcLogger.Error2(__METHOD__, " Could not load the objects pointing to or from the objects destroyed.", NULL);
	}

	//The objects outside the set whose pointers to them, or holders, change.
	aoiChanged.Init();
	for (i = 0; i < paoiDestroyed->NumElements(); i++)
	{
		pcObject = GetFromMemory(paoiDestroyed->GetValue(i));
		for (j = 0; j < pcObject->NumEmbedded(); j++)
		{
			pcEmbedded = pcObject->GetEmbeddedObject(j);
			for (k = 0; k < pcEmbedded->NumHeapFroms(); k++)
			{
				oiOther = pcEmbedded->GetHeapFrom(k)->GetEmbeddingContainer()->GetIndex();
				if (paoiDestroyed->Find(oiOther) == ARRAY_LARGE_ELEMENT_NOT_FOUND)
				{
					AddUniqueIndex(&aoiChanged, oiOther);
				}
			}
		}
		apcTos.Init();
		pcObject->GetPointerTos(&apcTos);
		for (j = 0; j < apcTos.NumElements(); j++)
		{
			oiOther = apcTos.GetPtr(j)->GetEmbeddingContainer()->GetIndex();
			if (paoiDestroyed->Find(oiOther) == ARRAY_LARGE_ELEMENT_NOT_FOUND)
			{
				AddUniqueIndex(&aoiChanged, oiOther);
			}
		}
		apcTos.Kill();
	}
	for (i = 0; i < aoiChanged.NumElements(); i++)
	{
		pcChanged = GetFromMemory(aoiChanged.GetValue(i));
		if (pcChanged != NULL)
		{
			pcChanged->SetDirty(true);
		}
	}

	//The objects destroyed are not saved again.  Their own pointers are cleared first, so what they point to is saved
	//without them as holders.  Nothing is freed while they are cleared: the objects loaded here may have no path to the
	//root (they were loaded for this) and a free could take ones still being worked on.  So until FromRemoved has been
	//asked of each changed object the graph isn't consistent (an object that lost its only holder still has its old
	//distance to the root), and it isn't validated: Kill would otherwise do so after each destroyed object.
	bValidateConsistency = mbValidateConsistency;
	mbValidateConsistency = false;
	for (i = 0; i < paoiDestroyed->NumElements(); i++)
	{
		pcObject = GetFromMemory(paoiDestroyed->GetValue(i));
		pcObject->SetDirty(false);
		pcObject->SetStored(false);
	}
	for (i = 0; i < paoiDestroyed->NumElements(); i++)
	{
		pcObject = GetFromMemory(paoiDestroyed->GetValue(i));
		pcObject->ClearPointerTos(false);
	}
	for (i = 0; i < paoiDestroyed->NumElements(); i++)
	{
		pcObject = GetFromMemory(paoiDestroyed->GetValue(i));
		if (pcObject != NULL)
		{
			pcObject->SetDirty(false);
			pcObject->Kill();
		}
	}

	for (i = 0; i < aoiChanged.NumElements(); i++)
	{
		pcChanged = GetFromMemory(aoiChanged.GetValue(i));
		if ((pcChanged != NULL) && !pcChanged->IsHollow() && pcChanged->IsDirty())
		{
			bResult &= ForceSave(pcChanged);
			pcChanged->SetDirty(false);
		}
	}

	//Saved, they are freed if nothing reaches them any more (the objects only the destroyed ones held).
	for (i = 0; i < aoiChanged.NumElements(); i++)
	{
		pcChanged = GetFromMemory(aoiChanged.GetValue(i));
		if (pcChanged != NULL)
		{
			FromRemoved(pcChanged, true);
		}
	}
	aoiChanged.Kill();

	mbValidateConsistency = bValidateConsistency;
#ifdef _DEBUG
	ValidateObjectsConsistency();
#endif // _DEBUG

	for (i = 0; i < paoiDestroyed->NumElements(); i++)
	{
		oi = paoiDestroyed->GetValue(i);
		if (mpcDataConnection->Contains(oi))
		{
			bResult &= mpcDataConnection->Remove(oi);
		}
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Loads everything pcStart reaches through its pointers, following the
// pointers of what it loads in turn, and adds each object's OIndex
// (pcStart's first) to paoiLoaded.  The walk stops with an error if it
// would load more than uiMaxObjects objects: it could pull in the whole
// database.
//
//////////////////////////////////////////////////////////////////////////
bool CObjects::LoadForward(CBaseObject* pcStart, CArrayOIndex* paoiLoaded, size uiMaxObjects)
{
	OBJECTS_WRITE();

	CArrayEmbeddedObjectPtr		apcTos;
	CBaseObject*				pcObject;
	OIndex						oi;
	OIndex						oiTo;
	lsize						uiNext;
	size						j;

	if (pcStart == NULL)
	{
		return gcLogger.Error2(__METHOD__, " Cannot load forward from object [NULL].", NULL);
	}

	uiNext = paoiLoaded->NumElements();
	AddUniqueIndex(paoiLoaded, pcStart->GetEmbeddingContainer()->GetIndex());
	while (uiNext < paoiLoaded->NumElements())
	{
		oi = paoiLoaded->GetValue(uiNext);
		uiNext++;
		if (!LoadForDestroy(oi))
		{
			return gcLogger.Error2(__METHOD__, " Could not load object [", IndexToString(oi), "].", NULL);
		}
		pcObject = GetFromMemory(oi);

		apcTos.Init();
		pcObject->GetPointerTos(&apcTos);
		for (j = 0; j < apcTos.NumElements(); j++)
		{
			oiTo = apcTos.GetPtr(j)->GetEmbeddingContainer()->GetIndex();
			if (paoiLoaded->Find(oiTo) == ARRAY_LARGE_ELEMENT_NOT_FOUND)
			{
				if (paoiLoaded->NumElements() >= uiMaxObjects)
				{
					apcTos.Kill();
					return gcLogger.Error2(__METHOD__, " Loading forward from object [", IndexToString(pcStart->GetIndex()), "] reaches more than [", SizeToString(uiMaxObjects), "] objects.", NULL);
				}
				paoiLoaded->Add(oiTo);
			}
		}
		apcTos.Kill();
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CObjects::EvictInMemory(void)
{
	OBJECTS_WRITE();

	bool	bResult;

	if (!ValidateNoOtherThreadHolds(__METHOD__))
	{
		return false;
	}

	Collect();
	bResult = SaveDirtyInMemory();
	FreeAllInKillOrder(true);

	mcMemory.ReInit();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Flush before free, for everything in memory: every dirty object that can
// find the root or is stored is saved.  Unreachable objects that never
// reached the database are left to be freed without saving.
//
//////////////////////////////////////////////////////////////////////////
bool CObjects::SaveDirtyInMemory(void)
{
	SIndexesIterator	sIter;
	OIndex				oi;
	CBaseObject*		pcBaseObject;
	bool				bResult;

	if (!mpcDataConnection)
	{
		return true;
	}

	bResult = true;
	oi = StartMemoryIteration(&sIter);
	while (oi != INVALID_O_INDEX)
	{
		pcBaseObject = GetFromMemory(oi);
		if (pcBaseObject->NeedsSaving())
		{
			bResult &= pcBaseObject->Flush();
		}
		oi = IterateMemory(&sIter);
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjects::FreeAllInKillOrder(bool bDeallocate)
{
	//Frees every Object in memory the way a garbage collected batch is freed: Objects with Nes pointers to them are freed
	//after the Objects that depend on them and nothing is deallocated until every Object has been freed.
	//If bDeallocate is false the Objects are left allocated for Kill (they are released with the Unknowns).

	SIndexesIterator		sIter;
	OIndex					oi;
	CArrayBaseObjectPtr		apcAll;
	CArrayBaseObjectPtr		apcDependents;
	CArrayBaseObjectPtr*	papcNes;
	CArrayBaseObjectPtr*	papcRemaining;
	CKillOrderCalculator	cKillOrder;

	if (mcRemovalContext.IsBusy())
	{
		gcLogger.Error2(__METHOD__, " Cannot free all Objects whilst performing a removal.", NULL);
		return;
	}

	apcAll.Init();
	oi = StartMemoryIteration(&sIter);
	while (oi != INVALID_O_INDEX)
	{
		apcAll.Add(GetFromMemory(oi));
		oi = IterateMemory(&sIter);
	}

	if (apcAll.IsEmpty())
	{
		apcAll.Kill();
		return;
	}

	//Removals started while the Objects are freed (e.g. a Free that clears a pointer) are pushed onto the context instead of
	//being performed.  Objects in apcAll are skipped when the context is drained so none are freed twice.
	mcRemovalContext.Start(&apcAll);

	//The kill order must be calculated before anything is freed; freeing clears the Nes pointers it looks for.
	apcDependents.Init();
	apcDependents.InsertArrayAfterEnd(&apcAll);
	cKillOrder.Init(&apcDependents);
	papcNes = cKillOrder.Calculate();

	//Every Object is going, so no Free() may reach one through a weak reference.
	ClearWeakFroms(&apcAll);
	FreeInternal(&apcDependents);
	FreeInternal(papcNes);
	DoneKill(&apcAll);

	cKillOrder.Kill();
	apcDependents.Kill();

	if (bDeallocate)
	{
		KillObjects(&apcAll);
	}
	else
	{
		mcMemory.FreeObjects(&apcAll);
	}

	papcRemaining = mcRemovalContext.Done();
	while (papcRemaining != NULL)
	{
		if (papcRemaining->IsNotEmpty())
		{
			RemoveBatch(papcRemaining);
		}
		papcRemaining = mcRemovalContext.Done();
	}

	apcAll.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The weak references to each object (and to each object embedded in it)
// become NULL, before any Free() of the batch runs.
//
//////////////////////////////////////////////////////////////////////////
void CObjects::ClearWeakFroms(CArrayBaseObjectPtr* papcObjectPts)
{
	size	i;
	size	iNumElements;

	iNumElements = papcObjectPts->NumElements();
	for (i = 0; i < iNumElements; i++)
	{
		ClearWeakFroms(*papcObjectPts->Get(i));
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjects::ClearWeakFroms(CBaseObject* pcObject)
{
	OBJECTS_INSIDE();

	size	i;
	size	uiNumEmbedded;

	if ((pcObject->GetFlags() & OBJECT_FLAGS_WEAKLY_HELD) == 0)
	{
		return;
	}
	uiNumEmbedded = pcObject->NumEmbedded();
	for (i = 0; i < uiNumEmbedded; i++)
	{
		mcWeakFroms.Clear(pcObject->GetEmbeddedObject(i));
	}
	pcObject->SetFlag(OBJECT_FLAGS_WEAKLY_HELD, false);
}


//////////////////////////////////////////////////////////////////////////
//
// The flag is the embedding container's, so the free path finds it.
//
//////////////////////////////////////////////////////////////////////////
void CObjects::AddWeakFrom(CEmbeddedObject* pcObject, CWeakHolder* pcHolder, size uiSlot)
{
	OBJECTS_INSIDE();

	mcWeakFroms.Add(pcObject, pcHolder, uiSlot);
	pcObject->GetEmbeddingContainer()->SetFlag(OBJECT_FLAGS_WEAKLY_HELD, true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjects::MoveWeakFroms(CEmbeddedObject* pcFrom, CEmbeddedObject* pcTo)
{
	OBJECTS_INSIDE();

	if (mcWeakFroms.NumWeakFroms(pcFrom) > 0)
	{
		mcWeakFroms.Move(pcFrom, pcTo);
		pcTo->GetEmbeddingContainer()->SetFlag(OBJECT_FLAGS_WEAKLY_HELD, true);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CObjects::RemoveWeakFrom(CEmbeddedObject* pcObject, CWeakHolder* pcHolder, size uiSlot)
{
	OBJECTS_INSIDE();

	return mcWeakFroms.Remove(pcObject, pcHolder, uiSlot);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CWeakFroms* CObjects::GetWeakFroms(void)
{
	return &mcWeakFroms;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjects::DoneKill(CArrayBaseObjectPtr* papcObjectPts)
{
	size			i;
	CBaseObject*	pcBaseObject;
	size			iNumElements;

	iNumElements = papcObjectPts->NumElements();
	for (i = 0; i < iNumElements; i++)
	{
		pcBaseObject = (CBaseObject*)(*papcObjectPts->Get(i));
		pcBaseObject->DoneKill(true);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjects::FreeInternal(CArrayBaseObjectPtr* papcObjectPts)
{
	size			i;
	CBaseObject*	pcBaseObject;
	size			iNumElements;

	iNumElements = papcObjectPts->NumElements();
	for (i = 0; i < iNumElements; i++)
	{
		pcBaseObject = (CBaseObject*)(*papcObjectPts->Get(i));
		pcBaseObject->FreeInternal();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjects::RemoveAllPointerTos(CArrayBaseObjectPtr* papcObjectPts)
{
	size			i;
	size			iNumElements;
	CBaseObject*	pcKilled;

	iNumElements = papcObjectPts->NumElements();
	for (i = 0; i < iNumElements; i++)
	{
		pcKilled = *papcObjectPts->Get(i);
		pcKilled->ClearPointerTos(false);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjects::RemoveAllHeapFroms(CArrayBaseObjectPtr* papcObjectPts)
{
	//Must be called after RemoveAllPointerTos.  Any heap froms left are from objects not being killed with these, whose
	//pointers to these must be cleared, or from objects killed with these (they don't unlink from each other), whose
	//pointers are cleared already.

	size			i;
	size			iNumElements;
	CBaseObject*	pcKilled;

	iNumElements = papcObjectPts->NumElements();
	for (i = 0; i < iNumElements; i++)
	{
		pcKilled = *papcObjectPts->Get(i);
		pcKilled->RemoveAllHeapFroms();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjects::KillObjects(CArrayBaseObjectPtr* papcObjectPts)
{
	size				i;
	CBaseObject*		pcBaseObject;
	size				iNumElements;
	CArrayUnknownPtr	cArray;
	CUnknown**			pvData;

	if (papcObjectPts->IsNotEmpty())
	{
		iNumElements = papcObjectPts->NumElements();
		for (i = 0; i < iNumElements; i++)
		{
			pcBaseObject = (*papcObjectPts->Get(i));
			mcMemory.FreeObject(pcBaseObject);
		}

		pvData = (CUnknown**)papcObjectPts->GetData();
		cArray.Fake(pvData, iNumElements);
		mpcUnknownsAllocatingFrom->RemoveInKill(&cArray);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Flush before free: a dirty stored object is saved before it is freed,
// so freeing never loses a change the database can still reach (its
// holders may be in the database only).  Objects that never reached the
// database are freed without saving.
//
//////////////////////////////////////////////////////////////////////////
void CObjects::SaveStoredDirty(CArrayBaseObjectPtr* papcObjectPts)
{
	size			i;
	size			iNumElements;
	CBaseObject*	pcBaseObject;

	if (!mpcDataConnection)
	{
		return;
	}

	iNumElements = papcObjectPts->NumElements();
	for (i = 0; i < iNumElements; i++)
	{
		pcBaseObject = *papcObjectPts->Get(i);
		if (pcBaseObject->IsDirty() && pcBaseObject->IsStored())
		{
			if (!ForceSave(pcBaseObject))
			{
				gcLogger.Error2(__METHOD__, " Could not save object [", IndexToString(pcBaseObject->GetIndex()), "] before freeing it.", NULL);
			}
			pcBaseObject->SetDirty(false);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjects::RemoveBatch(CArrayBaseObjectPtr* papcObjectPts)
{
	OBJECTS_INSIDE();

	//Done (marked freed) before Free, unlike a single Kill: the pointers the objects' Free changes then don't start
	//distance walks through objects that are going anyway (the walks skip freed objects).  With a batch of thousands
	//sharing an object that's the difference between linear and quadratic (TestDistToRootUpdateFreeBatchSpeed).  The
	//destruction callback is called before Free.
	SaveStoredDirty(papcObjectPts);
	DoneKill(papcObjectPts);
	ClearWeakFroms(papcObjectPts);
	FreeInternal(papcObjectPts);
	RemoveAllPointerTos(papcObjectPts);
	RemoveAllHeapFroms(papcObjectPts);
	KillObjects(papcObjectPts);
}


//////////////////////////////////////////////////////////////////////////
//
// The objects were found dead by a thread that isn't the home thread.  They are made unreachable now, which is all
// that can't wait: marked freed (the distance walks pass over them, and over their pointers to what is still alive),
// their weak references NULLed, saved if they are dirty and stored, and taken out of memory's indexes so that nothing
// is given them by index or by name.  Their Free(), and the rest of freeing them, is the home thread's (Collect).
// They are freed as one batch however many batches they were found in: marked as they are, they are each other's
// "freed with this".
//
//////////////////////////////////////////////////////////////////////////
void CObjects::WaitToBeFreed(CArrayBaseObjectPtr* papcObjectPts)
{
	OBJECTS_INSIDE();

	size			i;
	size			uiNumElements;
	CBaseObject*	pcBaseObject;

	SaveStoredDirty(papcObjectPts);
	uiNumElements = papcObjectPts->NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		pcBaseObject = papcObjectPts->GetPtr(i);
		pcBaseObject->SetFlag(OBJECT_FLAGS_FREED, true);
	}
	ClearWeakFroms(papcObjectPts);
	for (i = 0; i < uiNumElements; i++)
	{
		mcMemory.RemoveIdentifiers(papcObjectPts->GetPtr(i));
	}

	mapcWaitingToBeFreed.InsertArrayAfterEnd(papcObjectPts);
	ObjectsFreesWaiting(true);
}


//////////////////////////////////////////////////////////////////////////
//
// Frees the objects other threads found dead.  The home thread does this whenever it takes the object system's lock
// (so by the time it is in here the taking of the lock has usually done it), and its tick can call it (ObjectsCollect)
// so that they don't wait on its next use of an object.
//
//////////////////////////////////////////////////////////////////////////
void CObjects::Collect(void)
{
	OBJECTS_WRITE();

	CArrayBaseObjectPtr		apcDead;
	size					i;

	if (mapcWaitingToBeFreed.IsEmpty() || !IsHomeThread() || mcRemovalContext.IsBusy())
	{
		return;
	}

	//More can be found dead while these are freed (a Free() runs anything): they wait for the next time.
	apcDead.Init();
	apcDead.InsertArrayAfterEnd(&mapcWaitingToBeFreed);
	mapcWaitingToBeFreed.ReInit();
	ObjectsFreesWaiting(false);

	//They are freed as any batch is, which marks each object freed as its turn comes (those that others need to be
	//there are freed in a batch after them, and must not look freed while the others unlink from them).  The mark
	//they waited under has done its work: this thread holds the lock until they are gone.
	for (i = 0; i < apcDead.NumElements(); i++)
	{
		apcDead.GetPtr(i)->SetFlag(OBJECT_FLAGS_FREED, false);
	}

	RemoveInKillOrder(&apcDead);
	apcDead.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The main object system's home is the thread that called ObjectsInit; a
// worker's own object system's is the worker.
//
//////////////////////////////////////////////////////////////////////////
bool CObjects::IsHomeThread(void)
{
	return mbWorker ? (ObjectsOwn() == this) : ObjectsIsHomeThread();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CObjects::IsWorker(void)
{
	return mbWorker;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a hollow loaded now forwards (ThreadSafetyPlan.txt, Forwarding
// hollows): another thread may have read a pointer to it in operator->
// and not yet asked it for its object.  With no other thread registered
// it is freed at once, as on one thread.
//
//////////////////////////////////////////////////////////////////////////
bool CObjects::MustForward(void)
{
	return IsObjectsLocking() && (ObjectsNumThreads() > 0);
}


//////////////////////////////////////////////////////////////////////////
//
// pcHollow, loaded as pcObject (and everything that pointed to it now
// pointing to pcObject), is kept until no thread can still ask it for its
// object: it answers with pcObject.  A pointer to one of its embedded
// hollows that pcObject has no place for is cleared now, as the hollow's
// Kill would clear it.
//
//////////////////////////////////////////////////////////////////////////
void CObjects::Forward(CHollowObject* pcHollow, CBaseObject* pcObject)
{
	CBaseObject*		pcBaseHollow;
	SForwardingHollow*	psForwarder;

	pcBaseHollow = pcHollow;
	pcBaseHollow->RemoveAllStackFroms();
	pcBaseHollow->RemoveAllHeapFroms();

	//Not attached to the root (nothing points to it), so a pointer pointed at it once it forwards can't be by a
	//reader: it is a writer's, which points it at pcObject instead (Forwarded).
	pcBaseHollow->SetDistToRoot(UNATTACHED_DIST_TO_ROOT);

	psForwarder = masForwarders.Add();
	psForwarder->pcHollow = pcHollow;
	psForwarder->pcObject = pcObject;
	guiObjectsForwarders++;
}


//////////////////////////////////////////////////////////////////////////
//
// What pcHollow was loaded as, if it forwards; otherwise NULL.
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CObjects::GetForwarded(CHollowObject* pcHollow)
{
	SForwardingHollow*	psForwarder;
	size				i;

	for (i = 0; i < masForwarders.NumElements(); i++)
	{
		psForwarder = masForwarders.Get(i);
		if (psForwarder->pcHollow == pcHollow)
		{
			return psForwarder->pcObject;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// What pcObject was loaded as, if it is a forwarding hollow (or a hollow
// embedded in one); otherwise pcObject.  A pointer is about to be pointed
// at it by a thread that read it before the load, from a collection's
// element say, and must be pointed at what it stands for instead.
//
//////////////////////////////////////////////////////////////////////////
CEmbeddedObject* CObjects::Forwarded(CEmbeddedObject* pcObject)
{
	OBJECTS_INSIDE();

	CBaseObject*	pcContainer;
	CBaseObject*	pcLoaded;

	if (masForwarders.IsEmpty() || !pcObject->IsHollow())
	{
		return pcObject;
	}
	pcContainer = pcObject->GetEmbeddingContainer();
	pcLoaded = GetForwarded((CHollowObject*)pcContainer);
	if (pcLoaded == NULL)
	{
		return pcObject;
	}
	return pcLoaded->GetEmbeddedObject(pcContainer->GetEmbeddedIndex(pcObject));
}


//////////////////////////////////////////////////////////////////////////
//
// A thread that read a pointer to a hollow before it was loaded asks it
// for its object without coming into the object system in between, and
// asks it under the lock (Dehollow(CHollowObject*)).  So once every other
// thread has come in since a mark (taken the lock, or read) none can still
// be about to ask a hollow that was forwarding at the mark: those go.  The
// rest are marked now, for the next time.  With no other thread registered
// they all go.
//
//////////////////////////////////////////////////////////////////////////
void CObjects::FreeForwarders(void)
{
	OBJECTS_WRITE();

	if (!IsHomeThread() || mcRemovalContext.IsBusy())
	{
		return;
	}
	FreeForwardersInside(ObjectsNumThreads() == 0);
}


//////////////////////////////////////////////////////////////////////////
//
// bAll: every one, as no other thread uses objects.
//
//////////////////////////////////////////////////////////////////////////
void CObjects::FreeForwardersInside(bool bAll)
{
	CArrayBaseObjectPtr		apcHollows;
	size					uiFree;
	size					i;

	//The count of every object system's forwarders isn't touched by one that has none (a worker's never has).
	if (masForwarders.IsEmpty())
	{
		return;
	}

	if (bAll)
	{
		uiFree = masForwarders.NumElements();
	}
	else if (muiForwardersMarked == 0)
	{
		uiFree = 0;
	}
	else if (ObjectsThreadsAllInSinceMark())
	{
		uiFree = muiForwardersMarked;
	}
	else
	{
		return;
	}

	//Taken out first: a Kill runs callbacks.  Each is as a loaded hollow was freed before it forwarded.
	apcHollows.Init();
	for (i = 0; i < uiFree; i++)
	{
		apcHollows.Add(masForwarders.Get(i)->pcHollow);
	}
	masForwarders.RemoveRange(0, uiFree);
	guiObjectsForwarders -= uiFree;
	muiForwardersMarked = masForwarders.NumElements();
	if (muiForwardersMarked > 0)
	{
		ObjectsThreadsMarkWaysIn();
	}

	for (i = 0; i < uiFree; i++)
	{
#ifdef _DEBUG
		//Nothing may point to a forwarder (Forwarded): its Kill would leave such a pointer NULL.
		if (apcHollows.GetPtr(i)->HasHeapFroms() || apcHollows.GetPtr(i)->HasStackFroms())
		{
			gcLogger.Error2(__METHOD__, " A forwarding hollow is pointed to as it is freed.", NULL);
		}
#endif // _DEBUG
		apcHollows.GetPtr(i)->KillInternal(true, true);
	}
	apcHollows.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CObjects::NumForwarders(void)
{
	OBJECTS_READ();

	return masForwarders.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
// Evicting everything in memory puts hollows in place of objects under any thread using them: it is done between the
// other threads' work, and refused while any of them holds a stack pointer (in its table, or, as a Debug build knows,
// on an object).
//
//////////////////////////////////////////////////////////////////////////
bool CObjects::ValidateNoOtherThreadHolds(const char* szMethod)
{
	size	uiHeld;

	if (!IsObjectsLocking())
	{
		return true;
	}

	uiHeld = mpcStackPointers->NumOnOtherThreads(CurrentThreadId()) + ThreadStackFromsNumOnOtherThreads();
	if (uiHeld > 0)
	{
		return gcLogger.Error2(szMethod, " Cannot put hollows in place of objects while other threads hold [", SizeToString(uiHeld), "] stack pointers to objects.", NULL);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Putting a hollow in place of pcObject (the Kill of a stored object that
// is still held) takes it from under any other thread holding it: refused
// while one does.  Another thread's stack pointers to an object attached
// to the root are in that thread's table; those on the object are the
// home thread's, so are another's unless this is the home thread.
//
//////////////////////////////////////////////////////////////////////////
bool CObjects::ValidateNoOtherThreadHolds(const char* szMethod, CBaseObject* pcObject)
{
	CEmbeddedObject*	pcEmbedded;
	size				i;
	bool				bHome;

	if (!IsObjectsLocking())
	{
		return true;
	}

	bHome = IsHomeThread();
	for (i = 0; i < pcObject->NumEmbedded(); i++)
	{
		pcEmbedded = pcObject->GetEmbeddedObject(i);
		if (ThreadStackFromsHasOnAnotherThread(pcEmbedded) || (!bHome && (pcEmbedded->GetFirstStackFrom() != NULL)))
		{
			return gcLogger.Error2(szMethod, " Cannot put a hollow in place of {", ObjectToString(pcObject), "}: other threads hold stack pointers to it.", NULL);
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CObjects::NumWaitingToBeFreed(void)
{
	OBJECTS_READ();

	return mapcWaitingToBeFreed.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CObjects::ForceSave(CBaseObject* pcObject)
{
	OBJECTS_WRITE();

	bool							bResult;
	CInternalObjectSerialiser		cSerialiser;
	CBaseObject*					pcEmbeddingContainer;

	if (!pcObject)
	{
		return gcLogger.Error2(__METHOD__, " Cannot Save object [NULL].", NULL);
	}
	if (!mpcDataConnection)
	{
		CChars	sz;

		sz.Init();
		pcObject->PrintIdentifier(&sz);
		gcLogger.Error2(__METHOD__, " Cannot save object [", sz.Text(), "], Objects has data connection [NULL].", NULL);
		sz.Kill();
		return false;
	}

	pcEmbeddingContainer = pcObject->GetEmbeddingContainer();

	cSerialiser.Init(mpcDataConnection);
	bResult = cSerialiser.Write(pcEmbeddingContainer);
	cSerialiser.Kill();

	if (bResult)
	{
		pcEmbeddingContainer->SetStored(true);
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Into memory by index, and by name too if it has one (see
// CNamedIndexedObjects::Add).
//
//////////////////////////////////////////////////////////////////////////
bool CObjects::AddIntoMemory(CBaseObject* pcObject)
{
	OBJECTS_INSIDE();

	bool	bResult;

	bResult = mcMemory.Add(pcObject);
	if (bResult)
	{
		LOG_OBJECT_ALLOCATION(pcObject);
		mAllocationCallback(pcObject);
	}
	else
	{
		LOG_OBJECT_ALLOCATION_FAILURE(pcObject->ClassName(), pcObject->GetIndex(), pcObject->GetName());
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CRoot> CObjects::Root(void)
{
	OBJECTS_WRITE();

	Ptr<CRoot>	pRoot;

	pRoot = GetRoot();
	if (pRoot.IsNull())
	{
		pRoot = Malloc<CRoot>(ROOT_NAME);
		if (pRoot.IsNull())
		{
			gcLogger.Error2(__METHOD__, " Could not allocate the root.", NULL);
			return pRoot;
		}
		pRoot->Init();
	}
	return pRoot;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CFreeObjects> CObjects::FreeObjects(void)
{
	OBJECTS_WRITE();

	Ptr<CFreeObjects>	pFree;
	Ptr<CRoot>			pRoot;

	pRoot = Root();
	if (pRoot.IsNull())
	{
		return NULL;
	}

	pFree = Get(FREE_OBJECTS_NAME);
	if (pFree.IsNull())
	{
		pFree = Malloc<CFreeObjects>(FREE_OBJECTS_NAME);
		if (pFree.IsNull())
		{
			gcLogger.Error2(__METHOD__, " Could not allocate the free objects.", NULL);
			return pFree;
		}
		pFree->Init();

		pRoot->Add(pFree);
	}
	return pFree;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CObjects::AddFreeObject(CPointer& p)
{
	OBJECTS_WRITE();

	Ptr<CFreeObjects>	pFree;

	pFree = FreeObjects();
	if (pFree.IsNull())
	{
		return NULL;
	}
	return pFree->Add(p);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CObjects::RemoveFreeObject(CBaseObject* pcObject)
{
	OBJECTS_WRITE();

	return FreeObjects()->Remove(pcObject);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CObjects::NumFreeObjects(void)
{
	OBJECTS_READ();

	Ptr<CFreeObjects>	pFree;

	pFree = Get(FREE_OBJECTS_NAME);
	if (pFree.IsNull())
	{
		return 0;
	}
	else
	{
		return pFree->NumObjects();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjects::LogicalDirty(CBaseObject* pcObject)
{
	OBJECTS_WRITE();

	if (IsWriteThrough())
	{
		pcObject->Flush();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Whether an object is saved as soon as it is done being changed (LogicalDirty, which every pointer pointed at it
// calls).  It doesn't change once the objects are initialised.
//
//////////////////////////////////////////////////////////////////////////
bool CObjects::IsWriteThrough(void)
{
	return (mpcDataConnection != NULL) && mpcDataConnection->IsWriteThrough();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CRoot> CObjects::GetRoot(void)
{
	if (!mbInitialised)
	{
		gcLogger.Error2(__METHOD__, " CObjects is not initialised.", NULL);
		return Null();
	}
	return Get(ROOT_NAME);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CObjects::HasRoot(void)
{
	Ptr<CRoot>	pRoot;

	pRoot = GetRoot();
	return pRoot.IsNotNull();
}


//////////////////////////////////////////////////////////////////////////
//
// The object with index oi, loaded: from memory (a hollow there is
// loaded) or from the database.  NULL if there is none.
//
//////////////////////////////////////////////////////////////////////////
CPointer CObjects::Get(OIndex oi)
{
	OBJECTS_WRITE();

	CPointer	pObject;

	pObject = GetPossiblyHollow(oi);
	pObject.Dereference();
	return pObject;
}


//////////////////////////////////////////////////////////////////////////
//
// The object with index oi as it is: a hollow in memory is left hollow
// (looking at it doesn't load it; using it through a Ptr does).
//
//////////////////////////////////////////////////////////////////////////
CPointer CObjects::GetPossiblyHollow(OIndex oi)
{
	OBJECTS_WRITE();

	CBaseObject*	pcObject;
	CPointer		pObject;

	pcObject = mcMemory.Get(oi);
	if (pcObject == NULL)
	{
		pcObject = GetFromDatabase(oi);
	}
	pObject.AssignObjectInside(pcObject);
	return pObject;
}


//////////////////////////////////////////////////////////////////////////
//
// The object named szObjectName, loaded: from memory (a hollow there is
// loaded), the database or the sources.  NULL if there is none.
//
//////////////////////////////////////////////////////////////////////////
CPointer CObjects::Get(const char* szObjectName)
{
	OBJECTS_WRITE();

	CPointer	pObject;

	pObject = GetPossiblyHollow(szObjectName);
	pObject.Dereference();
	return pObject;
}


//////////////////////////////////////////////////////////////////////////
//
// The object named szObjectName as it is: a hollow in memory is left
// hollow (looking at it doesn't load it; using it through a Ptr does).
//
//////////////////////////////////////////////////////////////////////////
CPointer CObjects::GetPossiblyHollow(const char* szObjectName)
{
	OBJECTS_WRITE();

	CBaseObject*	pcObject;
	CPointer		pObject;

	pcObject = GetFromMemory(szObjectName);
	if (pcObject == NULL)
	{
		pcObject = GetFromDatabase(szObjectName);
	}
	if (pcObject == NULL)
	{
		pcObject = GetFromSources(szObjectName);
	}
	pObject.AssignObjectInside(pcObject);
	return pObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CObjects::GetFromMemory(OIndex oi)
{
	return mcMemory.Get(oi);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CObjects::GetFromMemory(const char* szObjectName)
{
	return mcMemory.Get(szObjectName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CObjects::GetFromDatabase(OIndex oi)
{
	CInternalObjectDeserialiser		cDeserialiser;
	CBaseObject*					pcBaseObject;

	if ((mpcDataConnection == NULL) || !mpcDataConnection->Contains(oi))
	{
		return NULL;
	}

	cDeserialiser.Init(this, mpcDataConnection, &mcMemory);
	pcBaseObject = cDeserialiser.Read(oi);
	cDeserialiser.Kill();

	if ((pcBaseObject != NULL) && (pcBaseObject->GetIndex() != oi))
	{
		//The record is corrupt.  What was read is in memory under its own index: it goes again.
		gcLogger.Error2(__METHOD__, " Requested object with index [", IndexToString(oi), "] but object had index [", IndexToString(pcBaseObject->GetIndex()), "].", NULL);
		pcBaseObject->Kill();
		return NULL;
	}
	return pcBaseObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CObjects::GetFromDatabase(const char* szObjectName)
{
	CInternalObjectDeserialiser		cDeserialiser;
	CBaseObject*					pcObject;

	if (!mpcDataConnection)
	{
		return NULL;
	}

	cDeserialiser.Init(this, mpcDataConnection, &mcMemory);
	pcObject = cDeserialiser.Read(szObjectName);
	cDeserialiser.Kill();
	return pcObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CObjects::GetFromSources(const char* szObjectName)
{
	CObjectSource*	pcSource;
	CBaseObject*	pcObject;
	CChars			szName;

	pcSource = mcSource.GetSource(szObjectName);
	if (pcSource)
	{
		//szObjectName can be a hollow's name, freed when the hollow is replaced during the conversion.
		szName.Init(szObjectName);
		pcObject = pcSource->Convert(szName.Text());

		//Every converted object must be named what it was asked for: objects in the Codabase reference it by that name.
		if ((pcObject != NULL) && !pcObject->IsNamed(szName.Text()))
		{
			gcLogger.Error2(__METHOD__, " Object converted from source [", szName.Text(), "] is not named [", szName.Text(), "].", NULL);
			pcObject->Kill();
			pcObject = NULL;
		}
		szName.Kill();
		return pcObject;
	}
	else
	{
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Under the index lock alone: a worker taking a block doesn't wait for
// the object system's writers, only for another index being handed out.
// With it held no other index comes between them: they are in a row.
//
//////////////////////////////////////////////////////////////////////////
OIndex CObjects::ReserveIndexes(size uiCount)
{
	OIndex	oiFirst;
	size	i;

	mcIndexLock.Lock();
	oiFirst = NextIndex();
	for (i = 1; i < uiCount; i++)
	{
		NextIndex();
	}
	mcIndexLock.Unlock();
	return oiFirst;
}


//////////////////////////////////////////////////////////////////////////
//
// While the locking is on, workers take blocks from the main object
// system's sequence too.  A worker's own sequence is its alone.
//
//////////////////////////////////////////////////////////////////////////
OIndex CObjects::GetNextIndex(void)
{
	if (OBJECTS_SELDOM(IsObjectsLocking()) && !mbWorker)
	{
		return ReserveIndexes(1);
	}
	return NextIndex();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
OIndex CObjects::NextIndex(void)
{
	if (mpcSequenceConnection)
	{
		return mpcSequenceConnection->GetNext();
	}
	else
	{
		return INVALID_O_INDEX;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPointer CObjects::TestGetFromMemory(const char* szName)
{
	OBJECTS_WRITE();

	CPointer	pObject;

	pObject.AssignObjectInside(GetFromMemory(szName));
	return pObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPointer CObjects::TestGetFromMemory(OIndex oi)
{
	OBJECTS_WRITE();

	CPointer	pObject;

	pObject.AssignObjectInside(GetFromMemory(oi));
	return pObject;
}


//////////////////////////////////////////////////////////////////////////
//
// The object (szObject: its index or name) a pointer refers to isn't in
// szWhere.  It should be: a programming error somewhere, say an object
// destroyed by hand.
//
//////////////////////////////////////////////////////////////////////////
static void LogPointedToMissing(const char* szMethod, const char* szObject, const char* szWhere)
{
	gcLogger.Error2(szMethod, " Object [", szObject, "] a pointer refers to is not in ", szWhere, ".", NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CObjects::Dehollow(OIndex oi)
{
	OBJECTS_WRITE();

	CBaseObject*	pcObject;

	pcObject = GetFromDatabase(oi);
	if ((pcObject == NULL) && (mpcDataConnection != NULL) && !mpcDataConnection->Contains(oi))
	{
		LogPointedToMissing(__METHOD__, IndexToString(oi), "the database");
	}
	return pcObject;
}


//////////////////////////////////////////////////////////////////////////
//
// The object pcHollow stands for (CHollowObject::Dehollow, for a pointer
// to it): loaded now, or if it has been loaded already while another
// thread read the pointer, what it was loaded as.  Nothing of pcHollow's
// that a load changes is read before the lock is held.
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CObjects::Dehollow(CHollowObject* pcHollow)
{
	OBJECTS_WRITE();

	CBaseObject*	pcObject;

	if (!masForwarders.IsEmpty())
	{
		pcObject = GetForwarded(pcHollow);
		if (pcObject != NULL)
		{
#ifdef _DEBUG
			//The pointer read is the one the load pointed at pcObject, which holds it: still in memory.
			if ((pcHollow->IsNamed() ? GetFromMemory(pcHollow->GetName()) : GetFromMemory(pcHollow->GetIndex())) != pcObject)
			{
				gcLogger.Error2(__METHOD__, " A hollow was asked for the object it was loaded as once that object had gone.", NULL);
			}
#endif // _DEBUG
			return pcObject;
		}
	}

	if (pcHollow->IsNamed())
	{
		return Dehollow(pcHollow->GetName());
	}
	return Dehollow(pcHollow->GetIndex());
}


//////////////////////////////////////////////////////////////////////////
//
// Loads the object a hollow named szObjectName stands for, from the
// database or a source.  An object already in memory isn't looked for.
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CObjects::Dehollow(const char* szObjectName)
{
	OBJECTS_WRITE();

	CBaseObject*	pcObject;
	CChars			szName;

	//szObjectName is the hollow's name: the hollow is freed when it is replaced by the loaded object.
	szName.Init(szObjectName);
	pcObject = GetFromDatabase(szName.Text());
	if (pcObject == NULL)
	{
		pcObject = GetFromSources(szName.Text());
	}

	if ((pcObject == NULL) && (mpcDataConnection != NULL) && !mpcDataConnection->Contains(szName.Text()))
	{
		LogPointedToMissing(__METHOD__, szName.Text(), "the database or a source");
	}
	szName.Kill();
	return pcObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CObjects::Contains(const char* szObjectName)
{
	OBJECTS_READ();

	//This does not check mcSources intentionally.
	if (mcMemory.Get(szObjectName) != NULL)
	{
		return true;
	}
	if (mpcDataConnection == NULL)
	{
		return false;
	}
	return mpcDataConnection->Contains(szObjectName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CObjects::Contains(OIndex oi)
{
	OBJECTS_READ();

	//This does not check mcSources intentionally.
	if (mcMemory.Get(oi) != NULL)
	{
		return true;
	}
	if (mpcDataConnection == NULL)
	{
		return false;
	}
	return mpcDataConnection->Contains(oi);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CObjects::Remove(CArrayBaseObjectPtr* papcKilled)
{
	return Remove(papcKilled, NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CObjects::Remove(CArrayBaseObjectPtr* papcKilled, CArrayBaseObjectPtr* papcKilledAfter)
{
	OBJECTS_WRITE();

	//Both batches are registered with the removal context before anything is freed.
	//Freeing the objects in papcKilled can cause other objects to be removed (e.g. RemoveFreeObject) and the removal context must already know about the objects in papcKilledAfter so they are not killed twice.

	CArrayBaseObjectPtr*	papcBatch;
	bool					bKilledAfter;

	bKilledAfter = (papcKilledAfter != NULL) && papcKilledAfter->IsNotEmpty();
	if (papcKilled->IsEmpty() && !bKilledAfter)
	{
		return true;
	}

	//Only the home thread frees: an object's Free(), and what it frees, belong to the thread the objects were made
	//for.  Another thread leaves what it found dead for it.
	if (!IsHomeThread())
	{
		WaitToBeFreed(papcKilled);
		if (bKilledAfter)
		{
			WaitToBeFreed(papcKilledAfter);
		}
		return true;
	}

	//A removal already under way frees these when it gets to them.
	if (mcRemovalContext.IsBusy())
	{
		if (papcKilled->IsNotEmpty())
		{
			mcRemovalContext.Push(papcKilled);
		}
		if (bKilledAfter)
		{
			mcRemovalContext.Push(papcKilledAfter);
		}
		return true;
	}

	papcBatch = mcRemovalContext.Start(papcKilled);
	if (bKilledAfter)
	{
		mcRemovalContext.Push(papcKilledAfter);
	}
	while (papcBatch != NULL)
	{
		if (papcBatch->IsNotEmpty())
		{
			if (!ValidateNoEmbedded(papcBatch))
			{
				mcRemovalContext.Clear();
				return false;
			}
			RemoveBatch(papcBatch);
		}
		papcBatch = mcRemovalContext.Done();
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPointer CObjects::Null(void)
{
	CPointer	pObject;
	return pObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CObjects::NumMemoryIndexes(void) const
{
	OBJECTS_READ();

	return mcMemory.NumIndexed();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CObjects::NumMemoryNames(void) const
{
	OBJECTS_READ();

	return mcMemory.NumNames();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int64 CObjects::NumDatabaseIndexes(void) const
{
	OBJECTS_READ();

	SIndexTreeFileIterator	sIter;
	OIndex					oi;
	int64					iCount;

	if (mpcDataConnection == NULL)
	{
		return 0;
	}

	iCount = 0;
	oi = mpcDataConnection->StartIndexIteration(&sIter, NULL, NULL, 0);
	while (oi != INVALID_O_INDEX)
	{
		iCount++;
		oi = mpcDataConnection->IndexIterate(&sIter, NULL, NULL, 0);
	}

	return iCount;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int64 CObjects::NumDatabaseNames(void) const
{
	OBJECTS_READ();

	SIndexTreeFileIterator	sIter;
	OIndex					oi;
	int64					iCount;

	if (mpcDataConnection == NULL)
	{
		return 0;
	}

	iCount = 0;
	mpcDataConnection->StartNameIteration(&sIter, NULL, &oi);
	while (oi != INVALID_O_INDEX)
	{
		iCount++;
		mpcDataConnection->NameIterate(&sIter, NULL, &oi);
	}

	return iCount;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CSequenceConnection* CObjects::GetIndexGenerator(void) const
{
	return mpcSequenceConnection;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CObjects::ValidateCanAllocate(const char* szClassName)
{
	if (StrEmpty(szClassName))
	{
		gcLogger.Error2(__METHOD__, " Cannot allocate an object of class with empty name.", NULL);
		return false;
	}

	if (!mbInitialised)
	{
		gcLogger.Error2(__METHOD__, " Cannot allocate object of class [", szClassName, "].  CObjects has not been initialised.", NULL);
		return false;
	}

	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CObjects::ValidateCanAllocate(void)
{
	if (!mbInitialised)
	{
		gcLogger.Error2(__METHOD__, " Cannot allocate object.  CObjects has not been initialised.", NULL);
		return false;
	}

	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CObjects::AllocateUninitialisedByClassName(const char* szClassName, const char* szObjectName, OIndex oi)
{
	CBaseObject*	pcObject;
	bool			bResult;

	bResult = ValidateCanAllocate(szClassName);
	if (!bResult)
	{
		return NULL;
	}

	pcObject = (CBaseObject*)mpcUnknownsAllocatingFrom->Add(szClassName);
	if (pcObject)
	{
		pcObject->Allocate(this);
		pcObject->InitIdentifiers(szObjectName, oi);

	}
	return pcObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
OIndex CObjects::StartMemoryIteration(SIndexesIterator* psIter)
{
	OBJECTS_READ();

	CIndexedObjects*	pcIndexedObjects;

	pcIndexedObjects = mcMemory.GetObjects();
	return pcIndexedObjects->StartIteration(psIter);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
OIndex CObjects::IterateMemory(SIndexesIterator* psIter)
{
	OBJECTS_READ();

	CIndexedObjects* pcIndexedObjects;

	pcIndexedObjects = mcMemory.GetObjects();
	return pcIndexedObjects->Iterate(psIter);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CNamedIndexedObjects* CObjects::GetMemory(void)
{
	return &mcMemory;
}


//////////////////////////////////////////////////////////////////////////
//
// A hollow with its embedded hollows after it.  An empty name is an
// unnamed hollow.
//
//////////////////////////////////////////////////////////////////////////
CHollowObject* CObjects::AllocateHollow(size uiNumEmbedded, const char* szObjectName, OIndex oi)
{
	CHollowObject*	pcHollow;
	void*			pvEmbedded;

	if (uiNumEmbedded == 0)
	{
		return NULL;
	}

	pcHollow = AllocateUninitialisedByTemplate<CHollowObject>(szObjectName, oi, sizeof(CHollowEmbeddedObject) * (uiNumEmbedded - 1));
	if (pcHollow == NULL)
	{
		return NULL;
	}
	pcHollow->Init(uiNumEmbedded);

	pvEmbedded = RemapSinglePointer(pcHollow, sizeof(CHollowObject));
	AppendHollowEmbeddedObjects(pcHollow, uiNumEmbedded, pvEmbedded);
	return pcHollow;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjects::AppendHollowEmbeddedObjects(CBaseObject* pcHollow, size uiNumEmbedded, void* pvEmbedded)
{
	size					i;
	CHollowEmbeddedObject*	pcEmbeddedObject;

	for (i = 0; i < uiNumEmbedded - 1; i++)
	{
		pcEmbeddedObject = new (pvEmbedded) CHollowEmbeddedObject();
		pcEmbeddedObject->SetEmbedding(pcHollow);
		pvEmbedded = RemapSinglePointer(pvEmbedded, sizeof(CHollowEmbeddedObject));
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CStackPointers* CObjects::GetStackPointers(void) const
{
	return mpcStackPointers;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CClasses* CObjects::GetClasses(void)
{
	return &mcClasses;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CConstructors* CObjects::GetConstructors(void) const
{
	return mpcUnknownsAllocatingFrom->GetConstructors();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CUnknowns* CObjects::GetUnknowns(void) const
{
	return mpcUnknownsAllocatingFrom;
}


//////////////////////////////////////////////////////////////////////////
//
// A new object of class szClassName in memory, with the next index.  An
// empty name is an unnamed object; a name must not be taken already.
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CObjects::AllocateUninitialisedByClassNameAndAddIntoMemory(const char* szClassName, const char* szObjectName)
{
	OBJECTS_WRITE();

	CBaseObject*	pcObject;
	OIndex			oi;

	if (!StrEmpty(szObjectName) && Contains(szObjectName))
	{
		gcLogger.Error2(__METHOD__, " Cannot allocate object named [", szObjectName, "] class [", szClassName, "].  It already exists.", NULL);
		return NULL;
	}

	oi = GetNextIndex();
	pcObject = AllocateUninitialisedByClassName(szClassName, szObjectName, oi);
	if (pcObject == NULL)
	{
		gcLogger.Error2(__METHOD__, " Cannot allocate object named [", szObjectName, "] class [", szClassName, "].", NULL);
		return NULL;
	}

	if (!AddIntoMemory(pcObject))
	{
		mpcUnknownsAllocatingFrom->RemoveInKill(pcObject);
		return NULL;
	}
	return pcObject;
}


//////////////////////////////////////////////////////////////////////////
//
// A new uninitialised object named szObjectName that takes the place of
// the object with that name (in memory or in the database) if there is
// one: everything pointing to the old one points to the new one.  The
// name is the identity, so the new one keeps the old one's index: the
// database still has the name under it, and what points to it there
// still finds it.  That's so even if the old one can't be read.
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CObjects::AllocateReplacingNamed(const char* szClassName, const char* szObjectName)
{
	OBJECTS_WRITE();

	CBaseObject*	pvOldObject;
	CBaseObject*	pcObject;
	OIndex			oi;
	bool			bResult;

	pvOldObject = GetFromMemory(szObjectName);
	if (!pvOldObject)
	{
		pvOldObject = GetFromDatabase(szObjectName);
	}
	if (pvOldObject != NULL)
	{
		oi = pvOldObject->GetIndex();
	}
	else
	{
		oi = (mpcDataConnection != NULL) ? mpcDataConnection->GetIndex(szObjectName) : INVALID_O_INDEX;
		if (oi == INVALID_O_INDEX)
		{
			oi = GetNextIndex();
		}
	}

	pcObject = AllocateUninitialisedByClassName(szClassName, szObjectName, oi);
	if (!pcObject)
	{
		gcLogger.Error2(__METHOD__, " Cannot allocate object named [", szObjectName, "] class [", szClassName, "].", NULL);
		return NULL;
	}

	if (pvOldObject)
	{
		bResult = ReplaceBaseObject(pvOldObject, pcObject);
		if (!bResult)
		{
			mpcUnknownsAllocatingFrom->RemoveInKill(pcObject);
			return NULL;
		}
	}
	else
	{
		bResult = AddIntoMemory(pcObject);
		if (!bResult)
		{
			mpcUnknownsAllocatingFrom->RemoveInKill(pcObject);
			return NULL;
		}
	}

	return pcObject;
}


//////////////////////////////////////////////////////////////////////////
//
// The object the InternalObjectDeserialiser is about to read: a hollow
// already in memory for it is replaced.  The hollow is found by index, or for a
// named object by name: a hollow made for a name alone (by an external
// read) before the database had the name has an index of its own, and the
// object loaded for it takes the database's.  An empty name is an unnamed
// object; a name taken by an object that isn't hollow fails to go into
// memory.
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CObjects::AllocateForInternalDeserialisation(const char* szClassName, const char* szObjectName, OIndex oi)
{
	CBaseObject*	pcObject;
	CHollowObject*	pcHollowObject;

	pcObject = GetFromMemory(oi);
	if ((pcObject == NULL) && !StrEmpty(szObjectName))
	{
		pcObject = GetFromMemory(szObjectName);
		if ((pcObject != NULL) && !pcObject->IsHollow())
		{
			pcObject = NULL;
		}
	}

	if (pcObject == NULL)
	{
		pcObject = AllocateUninitialisedByClassName(szClassName, szObjectName, oi);
		if (pcObject == NULL)
		{
			return NULL;
		}
		if (!AddIntoMemory(pcObject))
		{
			mpcUnknownsAllocatingFrom->RemoveInKill(pcObject);
			return NULL;
		}
		return pcObject;
	}

	if (!pcObject->IsHollow())
	{
		gcLogger.Error2(__METHOD__, " Cannot allocate object {", ObjectToString(pcObject), "} as class [", szClassName, "].  It already exists in memory (and is not hollow).", NULL);
		return NULL;
	}

	pcHollowObject = (CHollowObject*)pcObject;
	pcObject = AllocateUninitialisedByClassName(szClassName, szObjectName, oi);
	if (pcObject == NULL)
	{
		return NULL;
	}
	if (!ReplaceBaseObject(pcHollowObject, pcObject))
	{
		mpcUnknownsAllocatingFrom->RemoveInKill(pcObject);
		return NULL;
	}
	return pcObject;
}


//////////////////////////////////////////////////////////////////////////
//
// pcObject takes pcExisting's place: what points to pcExisting (or the
// objects embedded in it) points to pcObject instead, pcObject is put in
// memory and pcExisting is freed.  Returns false, with nothing changed,
// when pcObject can't take its place.
//
//////////////////////////////////////////////////////////////////////////
bool CObjects::ReplaceBaseObject(CBaseObject* pcExisting, CBaseObject* pcObject)
{
	OBJECTS_INSIDE();

	CObjectRemapFrom	cRemapper;

	if ((pcExisting == NULL) || (pcObject == NULL))
	{
		return false;
	}
	if (pcObject->HasHeapFroms())
	{
		return gcLogger.Error2(__METHOD__, " Cannot remap.  Object has heap froms already.", NULL);
	}

	//pcObject usually has pcExisting's identifiers, so they are taken out of memory before asking whether it can go in.
	//Remap frees pcExisting, so it is the last thing that can't be undone.
	mcMemory.RemoveIdentifiers(pcExisting);
	if (!mcMemory.CanAdd(pcObject))
	{
		mcMemory.Add(pcExisting);
		return false;
	}

	if (pcExisting->IsHollow() && !pcObject->IsHollow() && MustForward())
	{
		cRemapper.RemapFroms(pcExisting, pcObject);
		Forward((CHollowObject*)pcExisting, pcObject);
	}
	else
	{
		cRemapper.Remap(pcExisting, pcObject);
	}
	return AddIntoMemory(pcObject);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CObjects::TestRemoveMemoryIdentifiers(CBaseObject* pvObject)
{
	return mcMemory.RemoveIdentifiers(pvObject);
}


//////////////////////////////////////////////////////////////////////////
//
// A hollow for an object the InternalObjectDeserialiser found a pointer
// to, before reading it.  An empty name is an unnamed hollow.
//
//////////////////////////////////////////////////////////////////////////
CHollowObject* CObjects::AllocateInternalHollow(const char* szObjectName, OIndex oi, size uiNumEmbedded)
{
	OBJECTS_INSIDE();

	CHollowObject*	pcHollow;

	if ((oi == INVALID_O_INDEX) || (oi == NULL_O_INDEX))
	{
		gcLogger.Error2(__METHOD__, " Cannot allocate a hollow object with index [", IndexToString(oi), "].", NULL);
		return NULL;
	}

	pcHollow = AllocateHollow(uiNumEmbedded, szObjectName, oi);
	if (pcHollow == NULL)
	{
		LOG_OBJECT_ALLOCATION_FAILURE("CHollowObject", oi, szObjectName);
		return NULL;
	}

	if (!AddIntoMemory(pcHollow))
	{
		mpcUnknownsAllocatingFrom->RemoveInKill(pcHollow);
		return NULL;
	}
	mHollowAllocationCallback(pcHollow);
	return pcHollow;
}


//////////////////////////////////////////////////////////////////////////
//
// The object named szObjectName in memory, or a hollow for it to be loaded
// later.  The hollow has the database's index for the name, which the
// object loaded for it keeps (so it keeps its place in a set), or else the
// next index.
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CObjects::GetNamedOrAllocateHollow(const char* szObjectName, size uiNumEmbedded)
{
	OBJECTS_INSIDE();

	CHollowObject*	pcHollow;
	bool			bResult;
	CBaseObject*	pvExisting;
	OIndex			oi;

	if (StrEmpty(szObjectName))
	{
		return NULL;
	}

	pvExisting = GetFromMemory(szObjectName);
	if (pvExisting)
	{
		return pvExisting;
	}

	oi = (mpcDataConnection != NULL) ? mpcDataConnection->GetIndex(szObjectName) : INVALID_O_INDEX;
	if (oi == INVALID_O_INDEX)
	{
		oi = GetNextIndex();
	}
	pcHollow = AllocateHollow(uiNumEmbedded, szObjectName, oi);
	if (!pcHollow)
	{
		return NULL;
	}

	bResult = AddIntoMemory(pcHollow);
	if (!bResult)
	{
		mpcUnknownsAllocatingFrom->RemoveInKill(pcHollow);
		return NULL;
	}

	return pcHollow;
}


//////////////////////////////////////////////////////////////////////////
//
// Frees pcObject from memory but not from the database (Kill of a stored
// object, or one that couldn't be read): what points to it (or its
// embedded objects) points to a hollow with its index and name instead,
// so using it reads it again.  If nothing points to it the hollow is
// freed too.
//
//////////////////////////////////////////////////////////////////////////
void CObjects::ReplaceWithHollow(CBaseObject* pcObject)
{
	OBJECTS_INSIDE();

	CHollowObject*	pcHollow;

	pcHollow = AllocateHollow(pcObject->NumEmbedded(), pcObject->GetName(), pcObject->GetIndex());
	if ((pcHollow != NULL) && !ReplaceBaseObject(pcObject, pcHollow))
	{
		mpcUnknownsAllocatingFrom->RemoveInKill(pcHollow);
		pcHollow = NULL;
	}

	if (pcHollow == NULL)
	{
		//Nothing stands in for it, so it is freed as an object not in the database is.  Not by Kill: this is reached from
		//Kill, which has already marked it killed.
		pcObject->KillInternal(pcObject->HasHeapFroms(), true);
		return;
	}

	//Both ask about the hollow and the hollows embedded in it.
	if (!pcHollow->HasHeapFroms() && !pcHollow->HasStackFroms())
	{
		pcHollow->Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void ObjectsInit(void)
{
	ObjectsInit(NULL, &gcTransientSequence);
}


//////////////////////////////////////////////////////////////////////////
//
// The global objects, allocating from the global unknowns.
//
//////////////////////////////////////////////////////////////////////////
void ObjectsInit(CDataConnection* pcDataConnection, CSequenceConnection* pcSequenceConnection)
{
	ObjectsInit(&gcUnknowns, pcDataConnection, pcSequenceConnection);
}


//////////////////////////////////////////////////////////////////////////
//
// The global objects, allocating from pcUnknowns.  The global unknowns are
// Init'd either way: ObjectsKill Kills them.
//
//////////////////////////////////////////////////////////////////////////
void ObjectsInit(CUnknowns* pcUnknowns, CDataConnection* pcDataConnection, CSequenceConnection* pcSequenceConnection)
{
	if (gbObjects)
	{
		gcLogger.Error2(__METHOD__, " Objects has already been initialised.", NULL);
	}

	UnknownsInit();
	TransientSequenceInit();
	gcStackPointers.Init();
	gcObjects.Init(pcUnknowns, &gcStackPointers, pcDataConnection, pcSequenceConnection);
	gbObjects = true;
	ObjectsHomeThread(true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void ObjectsKill(bool bValidate)
{
	if (ObjectsNumThreads() > 0)
	{
		//Their objects would be freed under them.
		gcLogger.Error2(__METHOD__, " Objects cannot be killed while [", SizeToString(ObjectsNumThreads()), "] other threads are registered (ObjectsThreadKill).", NULL);
		return;
	}

	if (bValidate)
	{
		if (!gbObjects)
		{
			gcLogger.Error2(__METHOD__, " Objects has already been Killed.", NULL);
		}

		if (gcStackPointers.NumElements() > 0)
		{
			gcObjects.DumpStackPointers();
			gcLogger.Error2(__METHOD__, " Found unfreed stack pointers.", NULL);
		}
	}

	gcObjects.Kill();
	gcStackPointers.Kill();
	TransientSequenceKill();
	UnknownsKill();

	if (IsObjectsLocking())
	{
		ObjectsLocking(false);
	}
	gbObjects = false;
	ObjectsHomeThread(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool ObjectsValidate(void)
{
	if (!gbObjects)
	{
		return gcLogger.Error2(__METHOD__, " Objects have not been initialised.  Call ObjectsInit().", NULL);
	}
	else
	{
		return true;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void ObjectsCollect(void)
{
	if (ObjectsValidate())
	{
		gcObjects.Collect();
		gcObjects.FreeForwarders();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void ObjectsDiscard(void)
{
	if (!ObjectsValidate())
	{
		return;
	}

	gcObjects.Discard();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool ObjectsFlush(void)
{
	if (!ObjectsValidate())
	{
		return false;
	}

	return gcObjects.Flush();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void ObjectsDump(void)
{
	if (!ObjectsValidate())
	{
		return;
	}

	gcObjects.DumpMemoryUseIteration();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CRoot> ORoot(void)
{
	return ObjectsCurrent()->Root();
}

