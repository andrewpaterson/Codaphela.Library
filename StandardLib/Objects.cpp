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

#include "BaseLib/GlobalMemory.h"
#include "BaseLib/DebugOutput.h"
#include "BaseLib/Log.h"
#include "BaseObject.h"
#include "NamedObject.h"
#include "ObjectSingleSerialiser.h"
#include "ObjectWriterIndexed.h"
#include "ObjectReaderIndexed.h"
#include "IndexedDataObjectDeserialiser.h"
#include "ObjectAllocator.h"
#include "ObjectSource.h"
#include "Objects.h"
#include "NamedHollowObject.h"
#include "HollowEmbeddedObject.h"
#include "StackPointers.h"


CObjects gcObjects;


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void LogObjectAllocation(CBaseObject* pcObject, char* szMethod)
{
#ifdef DEBUG_OBJECT_ALLOCATION
#ifdef DEBUG
	char*	szClass;
	char*	szName;
	char*	szIndex;
	char*	szAddress;

	if (pcObject != NULL)
	{
		szClass = pcObject->ClassName();
		szIndex = IndexToString(pcObject->GetOI());
		szName = pcObject->GetName();
		szAddress = PointerToString(pcObject);
		gcLogger.Debug2(szMethod, "Allocate ", szClass, ": ", szIndex, " ", szName, " (", szAddress, ")]", NULL);
	}
	else
	{
		gcLogger.Debug2(szMethod, "Allocate NULL]", NULL);
	}
#endif // DEBUG
#endif // DEBUG_OBJECT_ALLOCATION
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void LogObjectDestruction(CBaseObject* pcObject, char* szMethod)
{
#ifdef DEBUG_OBJECT_ALLOCATION
#ifdef DEBUG
	char*	szClass;
	char*	szName;
	char*	szIndex;
	char*	szAddress;

	if (pcObject != NULL)
	{
		szClass = pcObject->ClassName();
		szIndex = IndexToString(pcObject->GetOI());
		szName = pcObject->GetName();
		szAddress = PointerToString(pcObject);
		gcLogger.Debug2(szMethod, "Kill ", szClass, ": ", szIndex, " ", szName, " (", szAddress, ")]", NULL);
	}
	else
	{
		gcLogger.Debug2(szMethod, "Kill NULL]", NULL);
	}
#endif // DEBUG
#endif // DEBUG_OBJECT_ALLOCATION
}


//////////////////////////////////////////////////////////////////////////
//
//
////////////////////////////////////////////////////////////////////////////
CObjects::CObjects()
{
	mbInitialised = FALSE;
	mpcUnknownsAllocatingFrom = NULL;
	mpcStackPointers = NULL;
	mbDatabase = FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjects::Init(CUnknowns* pcUnknownsAllocatingFrom, CStackPointers* pcStackPointers, char* szWorkingDirectory)
{
	CIndexedConfig	cConfig;

	cConfig.OptimiseForGameGraph(szWorkingDirectory);

	Init(pcUnknownsAllocatingFrom, pcStackPointers, &cConfig);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjects::Init(CUnknowns* pcUnknownsAllocatingFrom, CStackPointers* pcStackPointers, CIndexedConfig* pcConfig)
{
	mpcUnknownsAllocatingFrom = pcUnknownsAllocatingFrom;
	mpcStackPointers = pcStackPointers;
	mcIndexGenerator.Init();

	if (pcConfig->mszWorkingDirectory)
	{
		mcDatabase.Init(pcConfig);
		mbDatabase = TRUE;
	}
	else
	{
		mbDatabase = FALSE;
	}

	mcMemory.Init();

	mcSource.Init();

	mbInitialised = TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjects::Kill(void)
{
	mbInitialised = FALSE;
	mcSource.Kill();
	mcMemory.Kill();
	if (mbDatabase)
	{
		mcDatabase.Kill();  //Also flushes.
	}
	mcIndexGenerator.Kill();
	mpcUnknownsAllocatingFrom = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjects::ValidateEmpty(void)
{
	OIndex	iNumIndexed;

	iNumIndexed = mcMemory.NumIndexed();
	if (iNumIndexed != 0)
	{
		CChars				sz;

		sz.Init("\n");
		sz.Append("Memory not empty.  ");
		sz.Append(iNumIndexed);
		sz.Append(" objects are still indexed.\n");
		PrintMemory(&sz);
		gcLogger.Error(sz.Text());
		sz.Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjects::PrintMemory(CChars* psz)
{
	SIndexesIterator	sIter;
	CBaseObject*		pcBaseObject;

	pcBaseObject = mcMemory.StartIteration(&sIter);
	while (pcBaseObject)
	{
		pcBaseObject->PrintObject(psz);

		psz->Append("\n");
		pcBaseObject = mcMemory.Iterate(&sIter);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjects::DumpIndex(void)
{
	CChars				sz;

	sz.Init("-------------------------- Indices -------------------------- \n");
	PrintMemory(&sz);
	sz.Append("------------------------------------------------------------ \n");
	sz.Dump();
	sz.Kill();
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjects::DumpNames(void)
{
	CChars				sz;

	sz.Init("-------------------------- Names -------------------------- \n");
	sz.Dump();
	sz.Kill();
//	mcMemory.GetNames()->DumpTree();
	sz.Init();
	sz.Append("------------------------------------------------------------ \n");
	sz.Dump();
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjects::DumpGraph(void)
{
	CChars				sz;
	Ptr<CRoot>			pRoot;

	sz.Init("-------------------------- Graph --------------------------- \n");


	pRoot = Get(ROOT_NAME);
	if (pRoot.IsNotNull())
	{
		RecurseDumpGraph(&sz, pRoot.Object(), 0, FALSE);
	}

	sz.Append("------------------------------------------------------------ \n");
	sz.Dump();
	sz.Kill();

	SIndexesIterator	sIter;
	CBaseObject*		pcBaseObject;
	CBaseObject*		pcEmbeddedObject;
	CObject*			pcObject;
	int					i;

	pcBaseObject = mcMemory.StartIteration(&sIter);
	while (pcBaseObject)
	{
		SetFlag(&pcBaseObject->miFlags, OBJECT_FLAGS_DUMPED, FALSE);

		if (pcBaseObject->IsObject())
		{
			pcObject = (CObject*)pcBaseObject;
			for (i = 0; i < pcObject->mapEmbedded.NumElements(); i++)
			{
				pcEmbeddedObject = *pcObject->mapEmbedded.Get(i);
				SetFlag(&pcEmbeddedObject->miFlags, OBJECT_FLAGS_DUMPED, FALSE);
			}
		}
		pcBaseObject = mcMemory.Iterate(&sIter);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjects::RecurseDumpGraph(CChars* psz, CEmbeddedObject* pcIncoming, int iLevel, BOOL bEmbedded)
{
	CObject*					pcObject;
	CArrayTemplateEmbeddedObjectPtr		apcTos;
	int							i;
	CEmbeddedObject*			pcToObject;
	CBaseObject*				pcEmbeddedObject;
	CBaseObject*				pcBaseObject;

	if (!pcIncoming->IsBaseObject())
	{
		return;
	}

	pcBaseObject = (CBaseObject*)pcIncoming;

	psz->Append(' ', iLevel * 3);
	if ((pcBaseObject->miFlags & OBJECT_FLAGS_DUMPED) || (pcBaseObject->miDistToRoot < iLevel))
	{
		psz->Append('*');
		pcBaseObject->PrintObject(psz, bEmbedded);
		psz->AppendNewLine();
		return;
	}
	else
	{
		psz->Append(' ');
		pcBaseObject->PrintObject(psz, bEmbedded);
		psz->AppendNewLine();
	}

	pcBaseObject->miFlags |= OBJECT_FLAGS_DUMPED;


	apcTos.Init(1);
	pcBaseObject->BaseGetPointerTos(&apcTos);
	for (i = 0; i < apcTos.NumElements(); i++)
	{
		pcToObject = *apcTos.Get(i);
		RecurseDumpGraph(psz, pcToObject, iLevel+1, FALSE);
	}

	if (pcBaseObject->IsObject())
	{
		pcObject = (CObject*)pcBaseObject;
		for (i = 0; i < pcObject->mapEmbedded.NumElements(); i++)
		{
			pcEmbeddedObject = *pcObject->mapEmbedded.Get(i);
			RecurseDumpGraph(psz, pcEmbeddedObject, iLevel, TRUE);
		}
	}

	apcTos.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjects::ValidateObjectsConsistency(void)
{
	//If this method is called from an Object - rather than a test file - the it should be wrapped with a #ifdef DEBUG
	//This is because it is still useful to have ValidateObjectsConsistency called in RELEASE from tests.

	ValidateSceneGraph();
	ValidateIndexedObjects();
	ClearValidationFlags();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjects::ValidateIndexedObjects(void)
{
	SIndexesIterator	sIter;
	CBaseObject*		pcBaseObject;

	pcBaseObject = mcMemory.StartIteration(&sIter);
	while (pcBaseObject)
	{
		if (!(pcBaseObject->miFlags & OBJECT_FLAGS_TESTED_FOR_SANITY))
		{
			pcBaseObject->SetFlag(OBJECT_FLAGS_TESTED_FOR_SANITY, TRUE);

			pcBaseObject->ValidateConsistency();
		}

		pcBaseObject = mcMemory.Iterate(&sIter);
	}

	pcBaseObject = mcMemory.StartIteration(&sIter);
	while (pcBaseObject)
	{
		pcBaseObject->SetFlag(OBJECT_FLAGS_TESTED_FOR_SANITY, FALSE);
		pcBaseObject = mcMemory.Iterate(&sIter);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjects::ClearValidationFlags(void)
{
	SIndexesIterator	sIter;
	CBaseObject*		pcBaseObject;

	pcBaseObject = mcMemory.StartIteration(&sIter);
	while (pcBaseObject)
	{
		pcBaseObject->SetFlag(OBJECT_FLAGS_TESTED_FOR_SANITY, FALSE);
		pcBaseObject = mcMemory.Iterate(&sIter);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjects::ValidateSceneGraph(void)
{
	CRoot*				pcRoot;

	pcRoot = (CRoot*)GetFromMemory(ROOT_NAME);
	if (pcRoot != NULL)
	{
		RecurseValidateSceneGraph(pcRoot);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjects::RecurseValidateSceneGraph(CBaseObject* pcBaseObject)
{
	CArrayTemplateEmbeddedObjectPtr		apcTos;
	int							i;
	CEmbeddedObject*			pcToObject;
	CBaseObject*				pcToContainerObject;

	pcBaseObject->ValidateNotEmbedded(__METHOD__);

	if (!pcBaseObject->TestedForSanity())
	{
		pcBaseObject->SetFlag(OBJECT_FLAGS_TESTED_FOR_SANITY, TRUE);

		pcBaseObject->ValidateConsistency();

		apcTos.Init(1);
		pcBaseObject->GetPointerTos(&apcTos);
		for (i = 0; i < apcTos.NumElements(); i++)
		{
			pcToObject = *apcTos.Get(i);
			pcToContainerObject = pcToObject->GetEmbeddingContainer();
			RecurseValidateSceneGraph(pcToContainerObject);
		}

		apcTos.Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjects::Close(void)
{
	if (mbDatabase)
	{
		//xxx
		////Need to put more thought into Durable Files and Closing.
		return mcDatabase.Close();
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
BOOL CObjects::Flush(BOOL bClearMemory, BOOL bClearCache)
{
	SIndexesIterator	sIter;
	OIndex				oi;
	BOOL				bResult;
	CBaseObject*		pcBaseObject;

	if (mbDatabase)
	{
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
	else
	{
		bResult = ClearMemory();
		return bResult;
	}
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
	CArrayBlockObjectPtr		apcBaseObjects;
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
void CObjects::KillDontFreeObjects(CArrayBlockObjectPtr* papcObjectPts)
{
	int				i;
	CBaseObject*	pcBaseObject;
	int				iNumElements;

	iNumElements = papcObjectPts->NumElements();
	for (i = 0; i < iNumElements; i++)
	{
		pcBaseObject = (CBaseObject*)(*papcObjectPts->Get(i));
		pcBaseObject->KillDontFree();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjects::FreeObjects(CArrayBlockObjectPtr* papcObjectPts)
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
	pcObject->ValidateNotEmbedded(__METHOD__);

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
	CBaseObject*				pcContainer;

	pcContainer = pcObject->GetEmbeddingContainer();

	cWriter.Init(&mcDatabase, 0);
	cGraphSerialiser.Init(&cWriter);
	bResult = cGraphSerialiser.Write(pcContainer);
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

	pRoot = GetRoot();
	if (!pRoot)
	{
		pRoot = Add<CRoot>(ROOT_NAME)->Init();
	}
	return pRoot;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CRoot> CObjects::GetRoot(void)
{
	if (!mbInitialised)
	{
		gcLogger.Error("CObjects is not initialised.");
		return ONull;
	}
	return Get(ROOT_NAME);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPointer CObjects::Get(OIndex oi)
{
	CBaseObject*	pvObject;
	CPointer	pObject;

	pvObject = mcMemory.Get(oi);
	if (pvObject)
	{
		pObject.AssignObject(pvObject);
		return pObject;
	}

	pvObject = GetFromDatabase(oi);
	if (pvObject)
	{
		pObject.AssignObject(pvObject);
		return pObject;
	}

	return Null();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPointer CObjects::Get(char* szObjectName)
{
	CBaseObject*	pvObject;
	CPointer		pObject;

	pvObject = GetFromMemory(szObjectName);
	if (pvObject)
	{
		pObject.AssignObject(pvObject);
		return pObject;
	}

	pvObject = GetFromDatabase(szObjectName);
	if (pvObject)
	{
		pObject.AssignObject(pvObject);
		return pObject;
	}

	pvObject = GetFromSources(szObjectName);
	if (pvObject)
	{
		pObject.AssignObject(pvObject);
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
	CBaseObject*					pvBaseObject;

	if (!mbDatabase)
	{
		return NULL;
	}

	if (!mcDatabase.Contains(oi))
	{
		return NULL;
	}

	cAllocator.Init(this);
	cDeserialiser.Init(&cAllocator, &mcDatabase, &mcMemory);

	pvBaseObject = cDeserialiser.Read(oi);
	cDeserialiser.Kill();

	if (pvBaseObject->GetOI() != oi)
	{
		gcLogger.Error2(__METHOD__, " Requested object with index [", IndexToString(oi), "] but object had index [", IndexToString(pvBaseObject->GetOI()), "].", NULL);
		return NULL;
	}

	return pvBaseObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CObjects::GetFromDatabase(char* szObjectName)
{
	CIndexedDataObjectDeserialiser	cDeserialiser;
	CObjectAllocator				cAllocator;
	CBaseObject*					pvObject;

	if (!mbDatabase)
	{
		return NULL;
	}

	if (mcDatabase.Contains(szObjectName))
	{
		cAllocator.Init(this); 
		cDeserialiser.Init(&cAllocator, &mcDatabase, &mcMemory);
		pvObject = cDeserialiser.Read(szObjectName);
		cDeserialiser.Kill();
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
CBaseObject* CObjects::GetFromSources(char* szObjectName)
{
	CObjectSource*	pcSource;
	CBaseObject*	pvObject;

	pcSource = mcSource.GetSource(szObjectName);
	if (pcSource)
	{
		pvObject = pcSource->Convert(szObjectName);
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
CPointer CObjects::TestGetFromMemory(char* szName)
{
	CPointer	pObject;

	pObject.AssignObject(GetFromMemory(szName));
	return pObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPointer CObjects::TestGetFromMemory(OIndex oi)
{
	CPointer	pObject;

	pObject.AssignObject(GetFromMemory(oi));
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
	CPointer	pObject;

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
		if (mbDatabase)
		{
			return mcDatabase.Contains(szObjectName);
		}
		else
		{
			return FALSE;
		}
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjects::Remove(CArrayBlockObjectPtr* papcKilled)
{
	int								i;
	int								iNumElements;
	CBaseObject*					pcKilled;
	CBaseObject*					pcContainer;

	//No embedded objects should be in the list papcKilled.

	iNumElements = papcKilled->NumElements();
	if (iNumElements == 0)
	{
		return TRUE;
	}

	for (i = 0; i < iNumElements; i++)
	{
		pcKilled = *papcKilled->Get(i);
		if (pcKilled->IsEmbedded())
		{
			pcContainer = pcKilled->GetEmbeddingContainer();
			gcLogger.Error2(__METHOD__, " Object of class [", pcKilled->ClassName(), "] is marked for killing but is embedded in object with index [", IndexToString(pcContainer->GetOI()),"] of class [", pcContainer->ClassName(), "].", NULL);
			return FALSE;
		}
		else if (!pcKilled->IsAllocatedInObjects())
		{
			gcLogger.Error2(__METHOD__, " Object of class [", pcKilled->ClassName(), "] is marked for killing but is not allocated in Objects [0x", PointerToString(this),"].", NULL);
			return FALSE;
		}
	}

	for (i = 0; i < iNumElements; i++)
	{
		pcKilled = *papcKilled->Get(i);
		pcKilled->RemoveAllPointerTosDontKill();
	}

	KillDontFreeObjects(papcKilled);
	FreeObjects(papcKilled);

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPointer CObjects::Null(void)
{
	CPointer		pvObject;
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
	if (mbDatabase)
	{
		//This is a very slow method.  
		//It loads every descriptor from 0 to the LastOI and checks if it points to an object.
		return mcDatabase.NumObjects();
	}
	else
	{
		return 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CObjects::NumDatabaseObjectsCached(int iSize)
{
	if (mbDatabase)
	{
		return mcDatabase.NumCached(iSize);
	}
	else
	{
		return 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CObjects::NumDatabaseObjectsCached(void)
{
	if (mbDatabase)
	{
		return mcDatabase.NumCached();
	}
	else
	{
		return 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
long long int CObjects::NumDatabaseNames(void)
{
	if (mbDatabase)
	{
		return mcDatabase.NumNames();
	}
	else
	{
		return 0;
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
BOOL CObjects::ValidateCanAllocate(char* szClassName)
{
	if ((szClassName == NULL) || (szClassName[0] == 0))
	{
		gcLogger.Error2(__METHOD__, " Cannot allocate an object of class with empty name.", NULL);
		return FALSE;
	}

	if (!mbInitialised)
	{
		gcLogger.Error2(__METHOD__, " Cannot allocate object of class [", szClassName, "].  CObjects has not been initialised.", NULL);
		return FALSE;
	}

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjects::ValidateCanAllocate(void)
{
	if (!mbInitialised)
	{
		gcLogger.Error2(__METHOD__, " Cannot allocate object.  CObjects has not been initialised.", NULL);
		return FALSE;
	}

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CObjects::Allocate(char* szClassName)
{
	CBaseObject*	pvObject;
	BOOL			bResult;

	bResult = ValidateCanAllocate(szClassName);
	if (!bResult)
	{
		return NULL;
	}

	pvObject = (CBaseObject*)mpcUnknownsAllocatingFrom->Add(szClassName);
	if (pvObject)
	{
		pvObject->CBaseObject::Allocate(this);
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
	pvObject->KillIdentifiers();
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
CNamedHollowObject* CObjects::AllocateNamedHollow(unsigned short iNumEmbedded)
{
	CNamedHollowObject*		pcHollow;
	int						iAdditionalBytes;
	void*					pvEmbedded;
	int						iHollowSize;

	if (iNumEmbedded == 0)
	{
		return NULL;
	}
	if (iNumEmbedded == 1)
	{
		pcHollow = Allocate<CNamedHollowObject>();
		pcHollow->Init(1);
		return pcHollow;
	}
	else
	{
		iAdditionalBytes = sizeof(CHollowEmbeddedObject) * (iNumEmbedded-1);
		pcHollow = Allocate<CNamedHollowObject>(iAdditionalBytes);
		pcHollow->Init(iNumEmbedded);

		iHollowSize = sizeof(CNamedHollowObject);
		pvEmbedded = RemapSinglePointer(pcHollow, iHollowSize);
		AppenedHollowEmbeddedObjects(pcHollow, iNumEmbedded, pvEmbedded);

		return pcHollow;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CHollowObject* CObjects::AllocateHollow(unsigned short iNumEmbedded)
{
	CHollowObject*		pcHollow;
	int					iAdditionalBytes;
	void*				pvEmbedded;

	if (iNumEmbedded == 0)
	{
		return NULL;
	}
	if (iNumEmbedded == 1)
	{
		pcHollow = Allocate<CHollowObject>();
		pcHollow->Init(1);
		return pcHollow;
	}
	else
	{
		iAdditionalBytes = sizeof(CHollowEmbeddedObject) * (iNumEmbedded-1);
		pcHollow = Allocate<CHollowObject>(iAdditionalBytes);
		pcHollow->Init(iNumEmbedded);

		pvEmbedded = RemapSinglePointer(pcHollow, sizeof(CHollowObject));
		AppenedHollowEmbeddedObjects(pcHollow, iNumEmbedded, pvEmbedded);

		return pcHollow;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjects::AppenedHollowEmbeddedObjects(CBaseObject* pcHollow, unsigned short iNumEmbedded, void* pvEmbedded) 
{
	CHollowEmbeddedObject	cEmbeddedObject;
	int						i;
	CHollowEmbeddedObject*	pcEmbeddedObject;

	cEmbeddedObject.SetEmbedded(pcHollow);

	for (i = 0; i < iNumEmbedded-1; i++)
	{
		pcEmbeddedObject = (CHollowEmbeddedObject*)pvEmbedded;
		memcpy(pcEmbeddedObject, &cEmbeddedObject, sizeof(CHollowEmbeddedObject));
		pvEmbedded = RemapSinglePointer(pvEmbedded, sizeof(CHollowEmbeddedObject));
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CStackPointers* CObjects::GetStackPointers(void)
{
	return mpcStackPointers;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void ObjectsInit(void)
{
	UnknownsInit();
	gcStackPointers.Init(2048);
	gcObjects.Init(&gcUnknowns, &gcStackPointers, (char*)NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void ObjectsInit(char* szWorkingDirectory)
{
	UnknownsInit();
	gcStackPointers.Init(2048);
	gcObjects.Init(&gcUnknowns, &gcStackPointers, szWorkingDirectory);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void ObjectsInit(CIndexedConfig* pcConfig)
{
	UnknownsInit();
	gcStackPointers.Init(2048);
	gcObjects.Init(&gcUnknowns, &gcStackPointers, pcConfig);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void ObjectsKill(void)
{
	gcObjects.Kill();
	gcStackPointers.ClearAllPointers();
	gcStackPointers.Kill();
	UnknownsKill();
}

