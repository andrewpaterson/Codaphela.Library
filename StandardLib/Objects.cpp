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

#include "BaseLib/DebugOutput.h"
#include "BaseLib/Logger.h"
#include "BaseObject.h"
#include "NamedObject.h"
#include "ObjectSingleSerialiser.h"
#include "ObjectWriterIndexed.h"
#include "ObjectReaderIndexed.h"
#include "IndexedDataObjectDeserialiser.h"
#include "ObjectAllocator.h"
#include "ObjectSource.h"
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

	mcSource.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjects::Kill(void)
{
	mcSource.Kill();
	mcMemory.Kill();
	mcDatabase.Kill();  //Also flushes.
	mcIndexGenerator.Kill();
	mpcUnknownsAllocatingFrom = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjects::Dump(void)
{
	CChars				sz;
	SIndexesIterator	sIter;
	CBaseObject*		pcBaseObject;
	int					iDistToRoot;

	sz.Init("-------------------------- Memory -------------------------- \n");
	pcBaseObject = mcMemory.StartIteration(&sIter);
	while (pcBaseObject)
	{
		iDistToRoot = pcBaseObject->DistToRoot();
		sz.Append(pcBaseObject->DistToRoot());
		if (iDistToRoot >= 0 && iDistToRoot <= 9)
		{
			sz.Append(":  ");
		}
		else
		{
			sz.Append(": ");
		}

		sz.Append(pcBaseObject->ClassName());
		sz.Append("(");
		sz.Append(pcBaseObject->ClassSize());
		sz.Append(") ");
		sz.Append(pcBaseObject->GetOI());
		if (pcBaseObject->IsNamed())
		{
			sz.Append(" ");
			sz.Append(pcBaseObject->GetName());
		}
		sz.Append("\n");
		pcBaseObject = mcMemory.Iterate(&sIter);
	}

	sz.Append("------------------------------------------------------------ \n");
	sz.Dump();
	sz.Kill();
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
		pcBaseObject = GetFromMemory(oi);
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
		pcBaseObject = GetFromMemory(oi);
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
BOOL CObjects::AddWithID(CBaseObject* pvObject, OIndex oi)
{
	return mcMemory.AddWithID(pvObject, oi);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjects::AddWithIDAndName(CBaseObject* pvObject, char* szObjectName, OIndex oi)
{
	if ((szObjectName != NULL) || (pvObject->IsNamed()))
	{
		return mcMemory.AddWithIDAndName(pvObject, oi, szObjectName);
	}
	else
	{
		return mcMemory.AddWithID(pvObject, oi);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjects::Dename(CBaseObject* pvObject)
{
	char* szName;

	if (pvObject->IsNamed())
	{
		szName = pvObject->GetName();
		if (!StrEmpty(szName))
		{
			mcMemory.RemoveName(szName);
			pvObject->SetName("");
		}
		return TRUE;
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjects::Deindex(CBaseObject* pvObject)
{
	return mcMemory.RemoveIndex(pvObject->GetOI());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CRoot> CObjects::AddRoot(void)
{
	Ptr<CRoot>	pRoot;

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
CPointerObject CObjects::Get(OIndex oi)
{
	CBaseObject*	pvObject;
	CPointerObject	pObject;

	pvObject = mcMemory.Get(oi);
	if (pvObject)
	{
		pObject.mpcObject = pvObject;
		return pObject;
	}

	pvObject = GetFromDatabase(oi);
	if (pvObject)
	{
		pObject.mpcObject = pvObject;
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
	CPointerObject	pObject;

	pvObject = GetFromMemory(szObjectName);
	if (pvObject)
	{
		pObject.mpcObject = pvObject;
		return pObject;
	}

	pvObject = GetFromDatabase(szObjectName);
	if (pvObject)
	{
		pObject.mpcObject = pvObject;
		return pObject;
	}

	pvObject = GetFromSources(szObjectName);
	if (pvObject)
	{
		pObject.mpcObject = pvObject;
		return pObject;
	}

	return Null();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CObjects::GetFromMemory(OIndex oi)
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
CBaseObject* CObjects::GetFromMemory(char* szObjectName)
{
	CBaseObject*	pvObject;

	pvObject = mcMemory.Get(szObjectName);
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
CBaseObject* CObjects::GetFromDatabase(OIndex oi)
{
	CIndexedDataObjectDeserialiser	cDeserialiser;
	CObjectAllocator				cAllocator;

	if (!mcDatabase.Contains(oi))
	{
		return NULL;
	}

	cAllocator.Init(this);
	cDeserialiser.Init(&cAllocator, &mcDatabase, &mcMemory);

	CPointerObject	pObject;

	pObject = cDeserialiser.Read(oi);
	cDeserialiser.Kill();

	if (pObject.GetIndex() != oi)
	{
		gcLogger.Error2("CObjects::GetFromDatabase requested object with index [", IndexToString(oi), "] but object had index [", IndexToString(pObject.GetIndex()), "].", NULL);
		return NULL;
	}

	return pObject.Object();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CObjects::GetFromDatabase(char* szObjectName)
{
	CIndexedDataObjectDeserialiser	cDeserialiser;
	CObjectAllocator				cAllocator;

	CPointerObject	pObject;

	if (mcDatabase.Contains(szObjectName))
	{
		cAllocator.Init(this); 
		cDeserialiser.Init(&cAllocator, &mcDatabase, &mcMemory);
		pObject = cDeserialiser.Read(szObjectName);
		cDeserialiser.Kill();
		return pObject.Object();
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
CBaseObject* CObjects::GetFromSources(char* szObjectName)
{
	CObjectSource*	pcSource;
	CPointerObject	pObject;

	pcSource = mcSource.GetSource(szObjectName);
	if (pcSource)
	{
		pObject = pcSource->Convert(szObjectName);
		return pObject.Object();
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
CPointerObject CObjects::TestGetFromMemory(OIndex oi)
{
	CPointerObject	pObject;

	pObject.mpcObject = GetFromMemory(oi);
	return pObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CObjects::Dehollow(OIndex oi)
{
	return GetFromDatabase(oi);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CObjects::Dehollow(char* szObjectName)
{
	CBaseObject*	pvObject;
	CPointerObject	pObject;

	pvObject = GetFromDatabase(szObjectName);
	if (pvObject)
	{
		return pvObject;
	}

	pvObject = GetFromSources(szObjectName);
	if (pvObject)
	{
		return pvObject;
	}

	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjects::Contains(char* szObjectName)
{
	CBaseObject*	pvObject;

	//This does not check mcSources intentionally.

	pvObject = mcMemory.Get(szObjectName);
	if (pvObject)
	{
		return TRUE;
	}
	else
	{
		return mcDatabase.Contains(szObjectName);
	}
	return FALSE;
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
	//This is a very slow method.  
	//It loads every descriptor from 0 to the LastOI and checks if it points to an object.
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
	Deindex(pvObject);
	Dename(pvObject);
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
CNamedIndexedObjects* CObjects::GetMemory(void)
{
	return &mcMemory;
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

