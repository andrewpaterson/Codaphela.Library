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
#include "BaseLib/StackMemory.h"
#include "CoreLib/DataConnection.h"
#include "CoreLib/TransientSequence.h"
#include "BaseObject.h"
#include "InternalObjectSerialiser.h"
#include "InternalObjectDeserialiser.h"
#include "ObjectSource.h"
#include "ObjectRemapFrom.h"
#include "Objects.h"
#include "HollowObject.h"
#include "HollowEmbeddedObject.h"
#include "StackPointers.h"


CObjects	gcObjects;
BOOL		gbObjects = FALSE;


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void LogObjectAllocation(CBaseObject* pcObject, char* szMethod)
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
		gcLogger.Debug2(szMethod, " Allocate ", szClass, ": ", szIndex, " ", szName, " (", szAddress, ")]", NULL);
	}
	else
	{
		gcLogger.Debug2(szMethod, " Allocate NULL]", NULL);
	}
#endif // _DEBUG
#endif // DEBUG_OBJECT_ALLOCATION
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void LogObjectDestruction(CBaseObject* pcObject, char* szMethod)
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
		gcLogger.Debug2(szMethod, " Kill ", szClass, ": ", szIndex, " ", szName, " (", szAddress, ")]", NULL);
	}
	else
	{
		gcLogger.Debug2(szMethod, " Kill NULL]", NULL);
	}
#endif // _DEBUG
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
	mpcDataConnection = NULL;
	mpcSequenceConnection = NULL;
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

	mcMemory.Init();
	mcSource.Init();
	mcClasses.Init(this);

	mbInitialised = TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjects::Kill(void)
{
	mbInitialised = FALSE;

	mcMemory.ValidateNoDirty();

	mpcDataConnection = NULL;
	mpcSequenceConnection = NULL;

	mcClasses.Kill();
	mcSource.Kill();
	mcMemory.Kill();
	
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

	//mpcDataConnection->StartNameIteration()
	//	...

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
		SetFlag(&pcBaseObject->muiFlags, OBJECT_FLAGS_DUMPED, FALSE);

		if (pcBaseObject->IsObject())
		{
			pcObject = (CObject*)pcBaseObject;
			for (i = 0; i < pcObject->mapEmbedded.NumElements(); i++)
			{
				pcEmbeddedObject = *pcObject->mapEmbedded.Get(i);
				SetFlag(&pcEmbeddedObject->muiFlags, OBJECT_FLAGS_DUMPED, FALSE);
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
	if ((pcBaseObject->muiFlags & OBJECT_FLAGS_DUMPED) || (pcBaseObject->miDistToRoot < iLevel))
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

	pcBaseObject->SetFlag(OBJECT_FLAGS_DUMPED, TRUE);


	apcTos.Init();
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
	//If this method is called from an Object - rather than a test case - then it should be wrapped with a #ifdef _DEBUG
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
		if (!(pcBaseObject->muiFlags & OBJECT_FLAGS_TESTED_FOR_SANITY))
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
	CRoot* pcRoot;

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

		apcTos.Init();
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
BOOL CObjects::Flush(void)
{
	SIndexesIterator	sIter;
	OIndex				oi;
	BOOL				bResult;
	CBaseObject*		pcBaseObject;
	BOOL				bCanFindRoot;	
	BOOL				bDirty;

	if (mpcDataConnection)
	{
		bResult = TRUE;
		oi = StartMemoryIteration(&sIter);
		while (oi != INVALID_O_INDEX)
		{
			pcBaseObject = GetFromMemory(oi);
			bResult &= pcBaseObject->Flush();
			oi = IterateMemory(&sIter);
		}

		bResult &= mpcDataConnection->Flush(FALSE);
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
				pcBaseObject->SetDirty(FALSE);
			}
			oi = IterateMemory(&sIter);
		}

		return TRUE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjects::EvictInMemory(void)
{
	SIndexesIterator		sIter;
	OIndex					oi;
	CBaseObject*			pcBaseObject;
	CArrayBlockObjectPtr		apcBaseObjects;
	int						iCount;

	apcBaseObjects.Init();
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
		pcBaseObject->InternalFree();
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
			FreeObject(pcBaseObject);
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
BOOL CObjects::ForceSave(CBaseObject* pcObject)
{
	BOOL							bResult;
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
		pcObject->GetIdentifier(&sz);
		gcLogger.Error2(__METHOD__, " Cannot save object [", sz.Text(), "], Objects has data connection [NULL].", NULL);
		sz.Kill();
		return FALSE;
	}

	pcEmbeddingContainer = pcObject->GetEmbeddingContainer();

	cSerialiser.Init(mpcDataConnection);
	bResult = cSerialiser.Write(pcEmbeddingContainer);
	cSerialiser.Kill();

	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjects::AddObjectIntoMemoryWithIndex(CBaseObject* pvObject, OIndex oi)
{
	BOOL	bResult;

	bResult = mcMemory.AddWithID(pvObject, oi);
	if (bResult)
	{
		LOG_OBJECT_ALLOCATION(pvObject);
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjects::AddObjectIntoMemoryWithIndexAndName(CBaseObject* pvObject, char* szObjectName, OIndex oi)
{
	BOOL	bResult;

	bResult = mcMemory.AddWithIDAndName(pvObject, oi, szObjectName);
	if (bResult)
	{
		LOG_OBJECT_ALLOCATION(pvObject);
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjects::Replace(CBaseObject* pvNewObject, char* szExistingName, OIndex oiNew)
{
	if (!StrEmpty(szExistingName))
	{
		return mcMemory.ReplaceWithIDAndName(pvNewObject, szExistingName, oiNew);
	}
	else
	{
		CChars sz;
		sz.Init();
		pvNewObject->GetIdentifier(&sz);
		gcLogger.Error2(__METHOD__, " Cannot replace object [", sz.Text(), "] with an empty name.", NULL);
		sz.Kill();
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjects::Dename(CBaseObject* pvObject)
{
	char* szName;

	szName = pvObject->GetName();
	if (!StrEmpty(szName))
	{
		mcMemory.RemoveName(szName);
		pvObject->SetName("");
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjects::Deindex(CBaseObject* pvObject)
{
	return mcMemory.RemoveIndex(pvObject->GetIndex());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CRoot> CObjects::Root(void)
{
	Ptr<CRoot>	pRoot;

	pRoot = GetRoot();
	if (pRoot.IsNull())
	{
		pRoot = Malloc<CRoot>(ROOT_NAME)->Init();
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
		gcLogger.Error2(__METHOD__, " CObjects is not initialised.", NULL);
		return Null();
	}
	return Get(ROOT_NAME);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjects::HasRoot(void)
{
	Ptr<CRoot> pRoot = GetRoot();
	return pRoot.IsNotNull();
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
		pObject.Dereference();
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
	CInternalObjectDeserialiser		cDeserialiser;
	CBaseObject*					pvBaseObject;

	if (!mpcDataConnection)
	{
		return NULL;
	}

	if (!mpcDataConnection->Contains(oi))
	{
		return NULL;
	}

	cDeserialiser.Init(this, mpcDataConnection, &mcMemory);

	pvBaseObject = cDeserialiser.Read(oi);
	cDeserialiser.Kill();

	if (!pvBaseObject)
	{
		return NULL;
	}

	if (pvBaseObject->GetIndex() != oi)
	{
		gcLogger.Error2(__METHOD__, " Requested object with index [", IndexToString(oi), "] but object had index [", IndexToString(pvBaseObject->GetIndex()), "].", NULL);
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
	CInternalObjectDeserialiser		cDeserialiser;
	CBaseObject*					pvObject;

	if (!mpcDataConnection)
	{
		return NULL;
	}

	cDeserialiser.Init(this, mpcDataConnection, &mcMemory);
	pvObject = cDeserialiser.Read(szObjectName);
	cDeserialiser.Kill();
	return pvObject;
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
OIndex CObjects::GetNextIndex(void)
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
	CPointer		pObject;

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
		if (mpcDataConnection)
		{
			return mpcDataConnection->Contains(szObjectName);
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
BOOL CObjects::Contains(OIndex oi)
{
	CBaseObject* pvObject;

	//This does not check mcSources intentionally.

	pvObject = mcMemory.Get(oi);
	if (pvObject)
	{
		return TRUE;
	}
	else
	{
		if (mpcDataConnection)
		{
			return mpcDataConnection->Contains(oi);
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
			gcLogger.Error2(__METHOD__, " Object of class [", pcKilled->ClassName(), "] is marked for killing but is embedded in object with index [", IndexToString(pcContainer->GetIndex()),"] of class [", pcContainer->ClassName(), "].", NULL);
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
int64 CObjects::NumMemoryIndexes(void)
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
int64 CObjects::NumDatabaseIndexes(void)
{
	SIndexTreeFileIterator	sIter;
	OIndex					oi;
	int64					iCount;

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
int CObjects::NumDatabaseNames(void)
{
	SIndexTreeFileIterator	sIter;
	OIndex					oi;
	int						iCount;

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
CSequenceConnection* CObjects::GetIndexGenerator(void)
{
	return mpcSequenceConnection;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjects::ValidateCanAllocate(char* szClassName)
{
	if (StrEmpty(szClassName))
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
CBaseObject* CObjects::AllocateUninitialisedByClassName(char* szClassName)
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
		pvObject->Allocate(this);
	}
	return pvObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjects::FreeObject(CBaseObject* pvObject)
{
	Deindex(pvObject);
	Dename(pvObject);
	pvObject->FreeIdentifiers();
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
CHollowObject* CObjects::AllocateHollow(uint16 iNumEmbedded)
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
		pcHollow = AllocateUninitialisedByTemplate<CHollowObject>(0);
		pcHollow->Init(1);
		return pcHollow;
	}
	else
	{
		iAdditionalBytes = sizeof(CHollowEmbeddedObject) * (iNumEmbedded - 1);
		pcHollow = AllocateUninitialisedByTemplate<CHollowObject>(iAdditionalBytes);
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
void CObjects::AppenedHollowEmbeddedObjects(CBaseObject* pcHollow, uint16 iNumEmbedded, void* pvEmbedded) 
{
	int						i;
	CHollowEmbeddedObject*	pcEmbeddedObject;

	for (i = 0; i < iNumEmbedded-1; i++)
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
CStackPointers* CObjects::GetStackPointers(void)
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
CBaseObject* CObjects::AllocateNewUnitialised(char* szClassName)
{
	CBaseObject*	pvObject;
	OIndex			oi;

	pvObject = AllocateUninitialisedByClassName(szClassName);
	if (!pvObject)
	{
		return NULL;
	}

	oi = GetIndexGenerator()->GetNext();
	AddObjectIntoMemoryWithIndex(pvObject, oi);

	return pvObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CObjects::GetObjectInMemoryAndReplaceOrAllocateUnitialised(char* szClassName, OIndex oiForced)
{
	CBaseObject*	pvExisting;
	CBaseObject*	pvObject;
	BOOL			bResult;

	pvObject = AllocateUninitialisedByClassName(szClassName);
	if (!pvObject)
	{
		return NULL;
	}

	pvExisting = GetFromMemory(oiForced);
	if (pvExisting == NULL)
	{
		bResult = AddObjectIntoMemoryWithIndex(pvObject, oiForced);
		if (!bResult)
		{
			pvObject->Kill();
			return NULL;
		}

		return pvObject;
	}
	else
	{
		pvObject = ReplaceExisting(pvExisting, pvObject, oiForced);

		return pvObject;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CObjects::AllocateNewNamedUnitialised(char* szClassName, char* szObjectName)
{
	BOOL			bResult;
	CBaseObject*	pvObject;
	OIndex			oi;

	if (Contains(szObjectName))
	{
		gcLogger.Error2(__METHOD__, " Cannot allocate object named [", szObjectName, "] class [", szClassName, "].  It already exists.", NULL);
		return NULL;
	}

	pvObject = AllocateUninitialisedByClassName(szClassName);
	if (!pvObject)
	{
		gcLogger.Error2(__METHOD__, " Cannot allocate object named [", szObjectName, "] class [", szClassName, "].", NULL);
		return NULL;
	}

	oi = GetIndexGenerator()->GetNext();
	bResult = AddObjectIntoMemoryWithIndexAndName(pvObject, szObjectName, oi);
	if (!bResult)
	{
		mpcUnknownsAllocatingFrom->RemoveInKill(pvObject);
		return NULL;
	}

	return pvObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CObjects::GetNamedObjectInMemoryAndReplaceOrAllocateUnitialised(char* szClassName, char* szObjectName)
{
	CBaseObject*	pvOldObject;
	CBaseObject*	pvObject;
	CBaseObject*	pvReplacedObject;
	OIndex			oi;
	BOOL			bResult;

	//Old objects pointing to the oi are left still pointing to the oi?
	oi = INVALID_O_INDEX;
	pvOldObject = GetFromMemory(szObjectName);
	if (pvOldObject)
	{
		oi = pvOldObject->GetIndex();
	}
	else
	{
		pvOldObject = GetFromDatabase(szObjectName);
		if (pvOldObject)
		{
			oi = pvOldObject->GetIndex();
		}
		else
		{
			oi = GetNextIndex();
		}
	}

	pvObject = AllocateUninitialisedByClassName(szClassName);
	if (!pvObject)
	{
		gcLogger.Error2(__METHOD__, " Cannot allocate object named [", szObjectName, "] class [", szClassName, "].", NULL);
		return NULL;
	}

	if (pvOldObject)
	{
		pvReplacedObject = ReplaceExisting(pvOldObject, pvObject, oi);
		if (!pvReplacedObject)
		{
			mpcUnknownsAllocatingFrom->RemoveInKill(pvObject);
			return NULL;
		}
	}
	else
	{
		pvReplacedObject = pvObject;
	}

	bResult = AddObjectIntoMemoryWithIndexAndName(pvReplacedObject, szObjectName, oi);
	if (!bResult)
	{
		mpcUnknownsAllocatingFrom->RemoveInKill(pvReplacedObject);
		return NULL;
	}

	return pvReplacedObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CObjects::AllocateForExistingInDatabaseWithExplicitIdentifiers(char* szClassName, char* szObjectName, OIndex oi, OIndex* poiExisting)
{
	CBaseObject*	pvObject;
	BOOL			bResult;

	//Only called by the InternalObjectDeserialiser.

	pvObject = GetFromMemory(szObjectName);
	if (pvObject)
	{
		if (pvObject->IsHollow())
		{
			if (pvObject->GetIndex() != oi)
			{
				gcLogger.Error2(__METHOD__, " Cannot allocate object named [", szObjectName, "] class [", szClassName, "].  It already exists in memory with index [", LongLongToString(oi), "] but expected index [", LongLongToString(pvObject->GetIndex()), "].", NULL);
				return NULL;
			}
			else
			{
				return NULL;
			}
		}
		else
		{
			gcLogger.Error2(__METHOD__, " Cannot allocate object named [", szObjectName, "] class [", szClassName, "].  It already exists in memory.", NULL);
			return NULL;
		}
	}
	else
	{
		pvObject = GetFromMemory(oi);
		if (pvObject)
		{
			gcLogger.Error2(__METHOD__, " Cannot allocate object with index [", LongLongToString(oi), "] class [", szClassName, "].  It already exists in memory but was not named [", szObjectName, "].", NULL);
			return NULL;
		}

		pvObject = AllocateUninitialisedByClassName(szClassName);
		if (!pvObject)
		{
			return NULL;
		}

		bResult = AddObjectIntoMemoryWithIndexAndName(pvObject, szObjectName, oi);
		if (!bResult)
		{
			mpcUnknownsAllocatingFrom->RemoveInKill(pvObject);
			return NULL;
		}
		return pvObject;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CObjects::ReplaceExisting(CBaseObject* pvExisting, CBaseObject* pvObject, char* szObjectName, OIndex oiForced)
{
	BOOL				bResult;
	CObjectRemapFrom	cRemapper;

	if (pvExisting && pvObject)
	{
		Dename(pvExisting);
		Deindex(pvExisting);

		bResult = Replace(pvObject, szObjectName, oiForced);
		if (!bResult)
		{
			pvObject->Kill();
			return NULL;
		}

		cRemapper.Remap(pvExisting, pvObject, TRUE);

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
CBaseObject* CObjects::ReplaceExisting(CBaseObject* pvExisting, CBaseObject* pvObject, OIndex oiForced)
{
	BOOL				bResult;
	CObjectRemapFrom	cRemapper;

	if (pvExisting && pvObject)
	{
		Dename(pvExisting);
		Deindex(pvExisting);

		bResult = AddObjectIntoMemoryWithIndex(pvObject, oiForced);
		if (!bResult)
		{
			pvObject->Kill();
			return NULL;
		}

		cRemapper.Remap(pvExisting, pvObject, TRUE);

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
CBaseObject* CObjects::GetObjectInMemoryOrAllocateHollowForceIndex(OIndex oiForced, uint16 iNumEmbedded)
{
	CHollowObject*	pcHollow;
	BOOL			bResult;
	CBaseObject*	pcExisting;

	if (oiForced == INVALID_O_INDEX)
	{
		gcLogger.Error("CObjects::AddHollow Cannot allocate a hollow object with an invalid index.");
		return NULL;
	}

	pcExisting = GetFromMemory(oiForced);
	if (pcExisting)
	{
		LOG_OBJECT_ALLOCATION(pcExisting);
		return pcExisting;
	}

	pcHollow = AllocateHollow(iNumEmbedded);
	if (!pcHollow)
	{
		return NULL;
	}

	bResult = AddObjectIntoMemoryWithIndex(pcHollow, oiForced);
	if (bResult)
	{
		return pcHollow;
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
CBaseObject* CObjects::GetNamedObjectInMemoryOrAllocateHollowForceIndex(char* szObjectName, OIndex oiForced, uint16 iNumEmbedded)
{
	CHollowObject*	pcHollow;
	BOOL			bResult;
	CBaseObject*	pvExisting;

	if (oiForced == INVALID_O_INDEX)
	{
		gcLogger.Error2(__METHOD__, " CObjects::AddHollow Cannot allocate a hollow object with an invalid index.", NULL);
		return NULL;
	}
	
	if (StrEmpty(szObjectName))
	{
		return GetObjectInMemoryOrAllocateHollowForceIndex(oiForced, iNumEmbedded);
	}

	pvExisting = GetFromMemory(szObjectName);
	if (pvExisting)
	{
		return pvExisting;
	}

	pvExisting = GetFromMemory(oiForced);
	if (pvExisting)
	{
		gcLogger.Error2(__METHOD__, " Cannot add hollow object named [", szObjectName, "] another object with index [", IndexToString(oiForced), "] and name [", pvExisting->GetName(), "] already exists.", NULL);
		return NULL;
	}

	pcHollow = AllocateHollow(iNumEmbedded);
	if (!pcHollow)
	{
		return NULL;
	}

	bResult = AddObjectIntoMemoryWithIndexAndName(pcHollow, szObjectName, oiForced);
	if (!bResult)
	{
		mpcUnknownsAllocatingFrom->RemoveInKill(pcHollow);
		return NULL;
	}

	return pcHollow;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CObjects::GetNamedObjectInMemoryOrAllocateHollow(char* szObjectName, uint16 iNumEmbedded)
{
	CHollowObject*	pcHollow;
	BOOL			bResult;
	CBaseObject*	pvExisting;
	OIndex			oi;

	//Only called by the ExternalObjectDeserialiser.

	if (StrEmpty(szObjectName))
	{
		return NULL;
	}

	pvExisting = GetFromMemory(szObjectName);
	if (pvExisting)
	{
		return pvExisting;
	}

	pcHollow = AllocateHollow(iNumEmbedded);
	if (!pcHollow)
	{
		return NULL;
	}

	pcHollow->InitName(szObjectName);

	oi = GetIndexGenerator()->GetNext();
	bResult = AddObjectIntoMemoryWithIndexAndName(pcHollow, szObjectName, oi);
	if (!bResult)
	{
		mpcUnknownsAllocatingFrom->RemoveInKill(pcHollow);
		return NULL;
	}

	return pcHollow;
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
//
//////////////////////////////////////////////////////////////////////////
void ObjectsInit(CDataConnection* pcDataConnection, CSequenceConnection* pcSequenceConnection)
{
	if (gbObjects)
	{
		gcLogger.Error("Objects has already been initialised.");
	}

	UnknownsInit();
	TransientSequenceInit();
	gcStackPointers.Init(2048);
	gcObjects.Init(&gcUnknowns, &gcStackPointers, pcDataConnection, pcSequenceConnection);
	gbObjects = TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void ObjectsInit(CUnknowns* pcUnknowns, CStackPointers* pcStackPointers, CDataConnection* pcDataConnection, CSequenceConnection* pcSequenceConnection)
{
	if (gbObjects)
	{
		gcLogger.Error("Objects has already been initialised.");
	}

	UnknownsInit();
	TransientSequenceInit();
	gcStackPointers.Init(2048);
	gcObjects.Init(pcUnknowns, pcStackPointers, pcDataConnection, pcSequenceConnection);
	gbObjects = TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void ObjectsKill(void)
{
	if (!gbObjects)
	{
		gcLogger.Error("Global Memory has already been Killed.");
	}

	gcObjects.Kill();
	gcStackPointers.ClearAllPointers();
	gcStackPointers.Kill();
	TransientSequenceKill();
	UnknownsKill();

	gbObjects = FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL ObjectsValidate(void)
{
	if (!gbObjects)
	{
		gcLogger.Error("Objects have not been initialised.  Call ObjectsInit().");
		return FALSE;
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
BOOL ObjectsFlush(void)
{
	if (!ObjectsValidate())
	{
		return FALSE;
	}

	return gcObjects.Flush();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CRoot> ORoot(void)
{
	return gcObjects.Root();
}


