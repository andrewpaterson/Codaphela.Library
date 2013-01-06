/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

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
#include "BaseObject.h"
#include "NamedObject.h"
#include "HollowObject.h"
#include "NamedHollowObject.h"
#include "ObjectSingleSerialiser.h"
#include "ObjectIndexedDataDeserialiser.h"
#include "ObjectWriterIndexed.h"
#include "Objects.h"


CObjects gcObjects;


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjects::Init(CUnknowns* pcUnknownsAllocatingFrom, char* szWorkingDirectory)
{
	CIndexedConfig	cConfig;

	cConfig.OptimiseForGameGraph(szWorkingDirectory);

	Init(pcUnknownsAllocatingFrom, &cConfig);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjects::Init(CUnknowns* pcUnknownsAllocatingFrom, CIndexedConfig* pcConfig)
{
	mpcUnknownsAllocatingFrom = pcUnknownsAllocatingFrom;
	mcIndexGenerator.Init();

	mcDatabase.Init(pcConfig);
	mcMemory.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjects::Kill(void)
{
	mcMemory.Kill();
	mcDatabase.Kill();  //Also flushes.
	mcIndexGenerator.Kill();
	mpcUnknownsAllocatingFrom = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjects::Close(void)
{
	//xxx
	////Need to put more thought into Durable Files and Closing.
	return mcDatabase.Close();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjects::Flush(BOOL bClearMemory, BOOL bClearCache)
{
	SIndexesIterator	sIter;
	OIndex				oi;
	BOOL				bResult;
	CBaseObject*		pcBaseObject;

	bResult = TRUE;
	oi = StartMemoryIteration(&sIter);
	while (oi != INVALID_O_INDEX)
	{
		pcBaseObject = GetInMemoryObject(oi);
		bResult &= Save(pcBaseObject);
		oi = IterateMemory(&sIter);
	}

	if (bClearMemory)
	{
		bResult &= ClearMemory();
	}

	bResult &= mcDatabase.Flush(bClearCache);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjects::ClearMemory(void)
{
	SIndexesIterator		sIter;
	OIndex					oi;
	CBaseObject*			pcBaseObject;
	CArrayBaseObjectPtr		apcBaseObjects;
	int						iCount;

	apcBaseObjects.Init(CLEAR_MEMORY_CHUNK_SIZE);
	oi = StartMemoryIteration(&sIter);

	iCount = 0;
	while (oi != INVALID_O_INDEX)
	{
		pcBaseObject = GetInMemoryObject(oi);
		apcBaseObjects.Add(&pcBaseObject);
		iCount++;

		if (iCount == CLEAR_MEMORY_CHUNK_SIZE)
		{
			KillDontFreeObjects(&apcBaseObjects);
			FreeObjects(&apcBaseObjects);

			iCount = 0;
			apcBaseObjects.ReInit();
		}

		oi = IterateMemory(&sIter);
	}

	KillDontFreeObjects(&apcBaseObjects);
	FreeObjects(&apcBaseObjects);

	apcBaseObjects.Kill();

	mcMemory.ReInit();
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjects::KillDontFreeObjects(CArrayBaseObjectPtr* papcObjectPts)
{
	int				i;
	CBaseObject*	pcBaseObject;
	int				iNumElements;

	if (papcObjectPts->IsNotEmpty())
	{
		iNumElements = papcObjectPts->NumElements();
		for (i = 0; i < iNumElements; i++)
		{
			pcBaseObject = (CBaseObject*)(*papcObjectPts->Get(i));
			pcBaseObject->KillDontFree();
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjects::FreeObjects(CArrayBaseObjectPtr* papcObjectPts)
{
	int					i;
	CBaseObject*		pcBaseObject;
	int					iNumElements;
	CArrayUnknownPtr	cArray;
	CUnknown**			pvData;

	if (papcObjectPts->IsNotEmpty())
	{
		iNumElements = papcObjectPts->NumElements();
		for (i = 0; i < iNumElements; i++)
		{
			pcBaseObject = (*papcObjectPts->Get(i));
			RemoveInKill(pcBaseObject);
		}

		pvData = (CUnknown**)papcObjectPts->GetData();
		cArray.Fake(pvData, papcObjectPts->NumElements());
		mpcUnknownsAllocatingFrom->RemoveInKill(&cArray);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjects::Save(CBaseObject* pcObject)
{
	if (pcObject->IsDirty())
	{
		return ForceSave(pcObject);
	}
	else
	{
		return TRUE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjects::ForceSave(CBaseObject* pcObject)
{
	BOOL						bResult;
	CObjectSingleSerialiser		cGraphSerialiser;
	CObjectWriterIndexed		cWriter;

	cWriter.Init(&mcDatabase, 0);
	cGraphSerialiser.Init(&cWriter);
	bResult = cGraphSerialiser.Write(pcObject);
	cGraphSerialiser.Kill();
	cWriter.Kill();

	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjects::AddWithID(CBaseObject* pvObject)
{
	return mcMemory.AddWithID(pvObject, mcIndexGenerator.PopIndex(), NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjects::AddWithIDAndName(CBaseObject* pvObject, char* szObjectName)
{
	return mcMemory.AddWithIDAndName(pvObject, mcIndexGenerator.PopIndex(), szObjectName, NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPointerObject CObjects::Add(char* szClassName, OIndex oi, OIndex* oiExisting)
{
	CBaseObject*	pvObject;
	CBaseObject*	pvExisting;
	BOOL			bResult;

	pvObject = (CBaseObject*)mpcUnknownsAllocatingFrom->Add(szClassName);
	if (pvObject)
	{
		if (!pvObject->IsNamed())
		{
			CPointerObject	pObject;

			pvExisting = NULL;
			mcMemory.AddWithID(pvObject, oi, &pvExisting);
			if (pvExisting)
			{
				bResult = mcMemory.AddWithID(pvExisting, mcIndexGenerator.PopIndex(), NULL);
				if (!bResult)
				{
					pvObject->Kill();
					return ONull;
				}
				*oiExisting = pvExisting->GetOI();
			}
			else
			{
				*oiExisting = INVALID_O_INDEX;
			}

			pObject.mpcObject = pvObject;
			return pObject;
		}
		else
		{
			pvObject->Kill();
			return ONull;
		}
	}
	else
	{
		return ONull;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPointerObject CObjects::Add(char* szClassName, char* szObjectName, OIndex oi, OIndex* oiExisting)
{
	CBaseObject*	pvObject;
	CBaseObject*	pvExisting;
	BOOL			bResult;

	pvObject = Allocate(szClassName);
	if (pvObject)
	{
		if (pvObject->IsNamed())
		{
			CPointerObject	pObject;

			pvExisting = NULL;
			mcMemory.AddWithIDAndName(pvObject, oi, szObjectName, &pvExisting);
			if (pvExisting)
			{
				bResult = mcMemory.AddWithID(pvExisting, mcIndexGenerator.PopIndex(), NULL);
				if (!bResult)
				{
					pvObject->Kill();
					return ONull;
				}
				*oiExisting = pvExisting->GetOI();
			}
			else
			{
				*oiExisting = INVALID_O_INDEX;
			}

			pObject.mpcObject = pvObject;
			return pObject;
		}
		else
		{
			pvObject->Kill();
			return ONull;
		}
	}
	else
	{
		return ONull;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPointerObject CObjects::Add(char* szClassName, char* szObjectName)
{
	CBaseObject*	pvObject;

	pvObject = (CBaseObject*)mpcUnknownsAllocatingFrom->Add(szClassName);
	if (pvObject)
	{
		if (pvObject->IsNamed())
		{
			CPointerObject	pObject;

			mcMemory.AddWithIDAndName(pvObject, mcIndexGenerator.PopIndex(), szObjectName, NULL);
			pObject.mpcObject = pvObject;
			return pObject;
		}
		else
		{
			pvObject->Kill();
			return ONull;
		}
	}
	else
	{
		return ONull;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPointer<CRoot> CObjects::AddRoot(void)
{
	CPointer<CRoot>	pRoot;

	pRoot = Get(ROOT_NAME);
	if (!pRoot)
	{
		pRoot = Add<CRoot>(ROOT_NAME);
		pRoot->Init(this);
	}
	return pRoot;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPointerObject CObjects::AddHollow(OIndex oi)
{
	CPointer<CHollowObject>	pHollow;
	CHollowObject*			pcHollow;
	BOOL					bResult;

	pcHollow = Allocate<CHollowObject>();

	bResult = mcMemory.AddWithID(pcHollow, oi, NULL);
	if (bResult)
	{
		pHollow.mpcObject = pcHollow;
		return pHollow;
	}
	else
	{
		//If an object with this oi already existed in memory 
		//then a hollow version should not be added.
		return ONull;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPointerObject CObjects::AddHollow(char* szName, OIndex oi)
{
	CPointer<CNamedHollowObject>	pHollow;
	CNamedHollowObject*				pcHollow;
	BOOL							bResult;
	CBaseObject*					pcExisting;

	pcExisting = mcMemory.Get(szName);
	if (pcExisting)
	{
		pHollow.mpcObject = pcExisting;
		return pHollow;
	}

	pcHollow = Allocate<CNamedHollowObject>();
	pcHollow->InitName(szName);

	bResult = mcMemory.AddWithIDAndName(pcHollow, oi, szName, NULL);
	if (bResult)
	{
		pHollow.mpcObject = pcHollow;
		return pHollow;
	}
	else
	{
		//If an object with this oi already existed in memory 
		//then a hollow version should not be added.
		return ONull;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPointerObject CObjects::GetIfInMemory(OIndex oi)
{
	CBaseObject*	pvObject;

	pvObject = mcMemory.Get(oi);
	if (pvObject)
	{
		CPointerObject		pObject;

		pObject.mpcObject = pvObject;
		return pObject;
	}
	else
	{
		return Null();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPointerObject CObjects::Get(OIndex oi)
{
	CBaseObject*	pvObject;
	void*			pvData;

	pvObject = mcMemory.Get(oi);
	if (pvObject)
	{
		CPointerObject		pObject;

		pObject.mpcObject = pvObject;
		return pObject;
	}

	pvData = mcDatabase.Get(oi);
	if (pvData)
	{
		CPointerObject	pObject;

		pObject = GetSerialised(pvData);
		free(pvData);
		return pObject;
	}

	return Null();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPointerObject CObjects::Get(char* szObjectName)
{
	CBaseObject*	pvObject;

	pvObject = mcMemory.Get(szObjectName);
	if (pvObject)
	{
		CPointerObject		pObject;

		pObject.mpcObject = pvObject;
		return pObject;
	}
	else
	{
		return GetNotInMemory(szObjectName);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPointerObject CObjects::GetNotInMemory(char* szObjectName)
{
	void*			pvData;	

	pvData = mcDatabase.Get(szObjectName);
	if (pvData)
	{
		CPointerObject	pObject;

		pObject = GetSerialised(pvData);
		free(pvData);
		return pObject;
	}
	else
	{


		//mcSource.
		return ONull;		
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPointerObject CObjects::GetSerialised(void* pvData)
{
	CObjectIndexedDataDeserialiser	cDeserialiser;
	CSerialisedObject*				pcSerialised;

	pcSerialised = (CSerialisedObject*)pvData;
	if (pcSerialised)
	{
		CPointerObject						pObject;

		cDeserialiser.Init(pcSerialised);
		pObject = cDeserialiser.Load(pcSerialised->GetIndex());
		cDeserialiser.Kill();
		return pObject;
	}
	else
	{
		return ONull;		
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjects::Remove(CArrayBaseObjectPtr* papcKilled)
{
	int								i;
	int								iNumElements;
	CBaseObject*					pcKilled;
	CArrayEmbeddedBaseObjectPtr		apcFromsChanged;

	iNumElements = papcKilled->NumElements();
	apcFromsChanged.Init();

	for (i = 0; i < iNumElements; i++)
	{
		pcKilled = *papcKilled->Get(i);
		pcKilled->RemoveAllTos(&apcFromsChanged);  //Does not kill anything.
	}

	KillDontFreeObjects(papcKilled);
	FreeObjects(papcKilled);

	FixDistToRoot(&apcFromsChanged);

	apcFromsChanged.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjects::FixDistToRoot(CArrayEmbeddedBaseObjectPtr* papcFromsChanged)
{
	int				i;
	int				iNumElements;
	CBaseObject*	pcSubRoot;
	CBaseObject*	pcTemp;
	CBaseObject*	pcFromsChanged;

	//You could optimize this to stop one before the RootSet.
	pcSubRoot = NULL;
	iNumElements = papcFromsChanged->NumElements();
	for (i = 0; i < iNumElements; i++)
	{
		pcFromsChanged = *papcFromsChanged->Get(i);

		pcTemp = pcFromsChanged->ClearDistToSubRoot();
		if (!pcSubRoot)
		{
			//Theres a bug here, you need to collect all the SubRoots and call FixDistToRoot on all of them.
			pcSubRoot = pcTemp;
		}
	}

	if (pcSubRoot)
	{
		pcSubRoot->FixDistToRoot();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPointerObject CObjects::Null(void)
{
	CPointerObject		pvObject;
	return pvObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
long long int CObjects::NumMemoryIndexes(void)
{
	return mcMemory.NumIndexed();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CObjects::NumMemoryNames(void)
{
	return mcMemory.NumNames();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
long long int CObjects::NumDatabaseObjects(void)
{
	return mcDatabase.NumObjects();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CObjects::NumDatabaseObjectsCached(int iSize)
{
	return mcDatabase.NumCached(iSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CObjects::NumDatabaseObjectsCached(void)
{
	return mcDatabase.NumCached();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
long long int CObjects::NumDatabaseNames(void)
{
	return mcDatabase.NumNames();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CObjects::GetInMemoryObject(OIndex oi)
{
	CBaseObject*	pvObject;

	pvObject = mcMemory.Get(oi);
	if (pvObject)
	{
		return pvObject;
	}
	else
	{
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexGenerator* CObjects::GetIndexGenerator(void)
{
	return &mcIndexGenerator;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CObjects::Allocate(char* szClassName)
{
	CBaseObject*	pvObject;

	pvObject = (CBaseObject*)mpcUnknownsAllocatingFrom->Add(szClassName);
	if (pvObject)
	{
		pvObject->CBaseObject::PreInit(this);
	}
	return pvObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjects::RemoveInKill(CBaseObject* pvObject)
{
	mcMemory.Remove(pvObject->GetOI());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
OIndex CObjects::StartMemoryIteration(SIndexesIterator* psIter)
{
	return mcMemory.GetObjects()->StartIteration(psIter);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
OIndex CObjects::IterateMemory(SIndexesIterator* psIter)
{
	return mcMemory.GetObjects()->Iterate(psIter);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void ObjectsInit(void)
{
	UnknownsInit();
	gcObjects.Init(&gcUnknowns, (char*)NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void ObjectsInit(char* szWorkingDirectory)
{
	UnknownsInit();
	gcObjects.Init(&gcUnknowns, szWorkingDirectory);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void ObjectsInit(CIndexedConfig* pcConfig)
{
	UnknownsInit();
	gcObjects.Init(&gcUnknowns, pcConfig);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void ObjectsKill(void)
{
	gcObjects.Kill();
	UnknownsKill();
}

