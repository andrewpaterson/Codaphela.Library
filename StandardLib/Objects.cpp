/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

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
#include "BaseLib/Logger.h"
#include "BaseLib/StackMemory.h"
#include "BaseLib/DataConnection.h"
#include "BaseLib/TransientSequence.h"
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
bool		gbObjects = false;


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
void ObjectCallbackNotSet(CBaseObject* pvObject)
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

	mAllocationCallback = &ObjectCallbackNotSet;
	mHollocationCallback = &ObjectCallbackNotSet;
	mDestructionCallback = &ObjectCallbackNotSet;

	mcMemory.Init();
	mcSource.Init();
	mcClasses.Init(this);

	mbInitialised = true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjects::Kill(void)
{
	mbInitialised = false;

	mcMemory.ValidateNoDirty();
	mcMemory.FreeObjects();

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
		PrintMemoryUseIteration(&sz);
		gcLogger.Error(sz.Text());
		sz.Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjects::PrintMemoryUseIteration(CChars* psz)
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
void CObjects::PrintMemoryUseRecursion(CChars* psz)
{
	mcMemory.GetObjects()->Print(psz, true, true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjects::PrintMemoryNames(CChars* psz)
{
	CIndexTreeMemory*			pcIndex;
	SIndexTreeMemoryIterator	sIter;
	bool						bExists;

	pcIndex = mcMemory.GetNames();

	bExists = pcIndex->StartIteration(&sIter, NULL, NULL, NULL, NULL, NULL, NULL);
	while (bExists)
	{
		psz->Append((char*)sIter.pvKey, sIter.iKeyLength);
		psz->AppendNewLine();

		bExists = pcIndex->Iterate(&sIter, NULL, NULL, NULL, NULL, NULL, NULL);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjects::DumpMemoryUseIteration(void)
{
	CChars	sz;

	sz.Init("------------------------- Indices -------------------------- \n");
	PrintMemoryUseIteration(&sz);
	sz.Append("------------------------------------------------------------ \n");
	sz.DumpKill();

	mcMemory.GetNames()->Dump();
	sz.Init("------------------------------------------------------------ \n");
	sz.DumpKill();

	mcMemory.GetObjects()->GetIndexes()->Dump();
	sz.Init("------------------------------------------------------------ \n");
	sz.DumpKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjects::DumpMemoryUseRecursion(void)
{
	CChars	sz;

	sz.Init("-------------------------- Indices --------------------------- \n");
	PrintMemoryUseRecursion(&sz);
	sz.Append("------------------------------------------------------------ \n");
	sz.DumpKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjects::DumpNames(void)
{
	CChars	sz;

	sz.Init("---------------------------- Names --------------------------- \n");
	sz.DumpKill();

	//mpcDataConnection->StartNameIteration()
	//	...

	sz.Init();
	sz.Append("------------------------------------------------------------ \n");
	sz.DumpKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjects::DumpMemoryNames(void)
{
	CChars						sz;
	CIndexTreeMemory*			pcIndex;
	SIndexTreeMemoryIterator	sIter;
	bool						bExists;

	sz.Init("------------------------- Memory Names ----------------------- \n");
	sz.DumpKill();

	pcIndex = mcMemory.GetNames();

	bExists = pcIndex->StartIteration(&sIter, NULL, NULL, NULL, NULL, NULL, NULL);
	while (bExists)
	{
		sz.Init((char*)sIter.pvKey, 0, sIter.iKeyLength);
		sz.AppendNewLine();
		sz.DumpKill();

		bExists = pcIndex->Iterate(&sIter, NULL, NULL, NULL, NULL, NULL, NULL);
	}

	sz.Init();
	sz.Append("------------------------------------------------------------ \n");
	sz.DumpKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjects::DumpGraph(void)
{
	CChars		sz;
	Ptr<CRoot>	pRoot;

	sz.Init("-------------------------- Graph --------------------------- \n");

	pRoot = Get(ROOT_NAME);
	if (pRoot.IsNotNull())
	{
		RecurseDumpGraph(&sz, pRoot.Object(), 0, false);
	}

	sz.Append("------------------------------------------------------------ \n");
	sz.DumpKill();

	SIndexesIterator	sIter;
	CBaseObject*		pcBaseObject;
	CBaseObject*		pcEmbeddedObject;
	CObject*			pcObject;
	size				i;
	size				uiNumEmbedded;

	pcBaseObject = mcMemory.StartIteration(&sIter);
	while (pcBaseObject)
	{
		SetFlagShort(&pcBaseObject->muiFlags, OBJECT_FLAGS_DUMPED, false);

		if (pcBaseObject->IsObject())
		{
			pcObject = (CObject*)pcBaseObject;
			uiNumEmbedded = pcObject->mapEmbedded.NumElements();
			for (i = 0; i < uiNumEmbedded; i++)
			{
				pcEmbeddedObject = *pcObject->mapEmbedded.Get(i);
				SetFlagShort(&pcEmbeddedObject->muiFlags, OBJECT_FLAGS_DUMPED, false);
			}
		}
		pcBaseObject = mcMemory.Iterate(&sIter);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjects::RecurseDumpGraph(CChars* psz, CEmbeddedObject* pcIncoming, int iLevel, bool bEmbedded)
{
	CObject*							pcObject;
	CArrayTemplateEmbeddedObjectPtr		apcTos;
	size								i;
	CEmbeddedObject*					pcToObject;
	CBaseObject*						pcEmbeddedObject;
	CBaseObject*						pcBaseObject;
	size								uiNumTos;
	size								uiNumEmbedded;

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

	pcBaseObject->SetFlag(OBJECT_FLAGS_DUMPED, true);


	apcTos.Init();
	pcBaseObject->BaseGetPointerTos(&apcTos);
	uiNumTos = apcTos.NumElements();
	for (i = 0; i < uiNumTos; i++)
	{
		pcToObject = *apcTos.Get(i);
		RecurseDumpGraph(psz, pcToObject, iLevel+1, false);
	}

	if (pcBaseObject->IsObject())
	{
		pcObject = (CObject*)pcBaseObject;
		uiNumEmbedded = pcObject->mapEmbedded.NumElements();
		for (i = 0; i < uiNumEmbedded; i++)
		{
			pcEmbeddedObject = *pcObject->mapEmbedded.Get(i);
			RecurseDumpGraph(psz, pcEmbeddedObject, iLevel, true);
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
			pcBaseObject->SetFlag(OBJECT_FLAGS_TESTED_FOR_SANITY, true);

			pcBaseObject->ValidateConsistency();
		}

		pcBaseObject = mcMemory.Iterate(&sIter);
	}

	pcBaseObject = mcMemory.StartIteration(&sIter);
	while (pcBaseObject)
	{
		pcBaseObject->SetFlag(OBJECT_FLAGS_TESTED_FOR_SANITY, false);
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
		pcBaseObject->SetFlag(OBJECT_FLAGS_TESTED_FOR_SANITY, false);
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
	size								i;
	CEmbeddedObject*					pcToObject;
	CBaseObject*						pcToContainerObject;
	size								uiNumTos;

	pcBaseObject->ValidateNotEmbedded(__METHOD__);

	if (!pcBaseObject->TestedForSanity())
	{
		pcBaseObject->SetFlag(OBJECT_FLAGS_TESTED_FOR_SANITY, true);

		pcBaseObject->ValidateConsistency();

		apcTos.Init();
		pcBaseObject->GetPointerTos(&apcTos);
		uiNumTos = apcTos.NumElements();
		for (i = 0; i < uiNumTos; i++)
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
void CObjects::SetAllocationCallback(AllocationCallback fAllocationCallback)
{
	mAllocationCallback = fAllocationCallback;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjects::SetHollocationCallback(HollocationCallback fHollocationCallback)
{
	mHollocationCallback = fHollocationCallback;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjects::SetDestructionCallback(DestructionCallback fDestructionCallback)
{
	mDestructionCallback = fDestructionCallback;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CObjects::Flush(void)
{
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
bool CObjects::EvictInMemory(void)
{
	SIndexesIterator		sIter;
	OIndex					oi;
	CBaseObject*			pcBaseObject;
	CArrayBlockObjectPtr	apcBaseObjects;
	size					iCount;

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
			KillObjects(&apcBaseObjects);

			iCount = 0;
			apcBaseObjects.ReInit();
		}

		oi = IterateMemory(&sIter);
	}

	KillDontFreeObjects(&apcBaseObjects);
	KillObjects(&apcBaseObjects);

	apcBaseObjects.Kill();

	mcMemory.ReInit();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjects::KillDontFreeObjects(CArrayBlockObjectPtr* papcObjectPts)
{
	size			i;
	CBaseObject*	pcBaseObject;
	size			uiNumElements;

	uiNumElements = papcObjectPts->NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		pcBaseObject = (CBaseObject*)(*papcObjectPts->Get(i));
		pcBaseObject->FreeInternal(true);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjects::KillObjects(CArrayBlockObjectPtr* papcObjectPts)
{
	size				i;
	CBaseObject*		pcBaseObject;
	size				uiNumElements;
	CArrayUnknownPtr	cArray;
	CUnknown**			pvData;

	if (papcObjectPts->IsNotEmpty())
	{
		uiNumElements = papcObjectPts->NumElements();
		for (i = 0; i < uiNumElements; i++)
		{
			pcBaseObject = (*papcObjectPts->Get(i));
			FreeObject(pcBaseObject);
		}

		pvData = (CUnknown**)papcObjectPts->GetData();
		cArray.Fake(pvData, uiNumElements);
		mpcUnknownsAllocatingFrom->RemoveInKill(&cArray);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CObjects::ForceSave(CBaseObject* pcObject)
{
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
		pcObject->GetIdentifier(&sz);
		gcLogger.Error2(__METHOD__, " Cannot save object [", sz.Text(), "], Objects has data connection [NULL].", NULL);
		sz.Kill();
		return false;
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
bool CObjects::AddIntoMemoryWithIndex(CBaseObject* pvObject)
{
	bool	bResult;

	bResult = mcMemory.AddIntoMemoryWithIndex(pvObject);
	if (bResult)
	{
		LOG_OBJECT_ALLOCATION(pvObject);
		mAllocationCallback(pvObject);
	}
	else
	{
		LOG_OBJECT_ALLOCATION_FAILURE(pvObject->ClassName(), pvObject->GetIndex(), "");
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CObjects::AddIntoMemoryWithNameAndIndex(CBaseObject* pvObject)
{
	bool	bResult;

	if (pvObject)
	{
		bResult = mcMemory.AddIntoMemoryWithNameAndIndex(pvObject);
		if (bResult)
		{
			LOG_OBJECT_ALLOCATION(pvObject);
			mAllocationCallback(pvObject);
		}
		else
		{
			LOG_OBJECT_ALLOCATION_FAILURE(pvObject->ClassName(), pvObject->GetIndex(), pvObject->GetName());
		}

		return bResult;
	}
	else
	{
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CObjects::RemoveMemoryIdentifiers(CBaseObject* pvObject)
{
	char*	szName;
	bool	bResult;

	bResult = mcMemory.RemoveIndex(pvObject->GetIndex());

	szName = pvObject->GetName();
	if (!StrEmpty(szName))
	{
		bResult &= mcMemory.RemoveName(szName);
	}
	return bResult;
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
void CObjects::LogicalDirty(CBaseObject* pcObject)
{
	if (mpcDataConnection && mpcDataConnection->IsWriteThrough())
	{
		pcObject->Flush();
	}
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
//
//////////////////////////////////////////////////////////////////////////
CPointer CObjects::Get(OIndex oi)
{
	CBaseObject*	pvObject;
	CPointer		pObject;

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
	return mcMemory.Get(oi);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CObjects::GetFromMemory(char* szObjectName)
{
	return mcMemory.Get(szObjectName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CObjects::GetFromMemory(char* szObjectName, OIndex oi)
{
	return mcMemory.Get(szObjectName, oi);
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
bool CObjects::Contains(char* szObjectName)
{
	CBaseObject*	pvObject;

	//This does not check mcSources intentionally.

	pvObject = mcMemory.Get(szObjectName);
	if (pvObject)
	{
		return true;
	}
	else
	{
		if (mpcDataConnection)
		{
			return mpcDataConnection->Contains(szObjectName);
		}
		else
		{
			return false;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CObjects::Contains(OIndex oi)
{
	CBaseObject* pvObject;

	//This does not check mcSources intentionally.

	pvObject = mcMemory.Get(oi);
	if (pvObject)
	{
		return true;
	}
	else
	{
		if (mpcDataConnection)
		{
			return mpcDataConnection->Contains(oi);
		}
		else
		{
			return false;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CObjects::Remove(CArrayBlockObjectPtr* papcKilled)
{
	size							i;
	size							uiNumElements;
	CBaseObject*					pcKilled;
	CBaseObject*					pcContainer;

	//No embedded objects should be in the list papcKilled.

	uiNumElements = papcKilled->NumElements();
	if (uiNumElements == 0)
	{
		return true;
	}

	for (i = 0; i < uiNumElements; i++)
	{
		pcKilled = *papcKilled->Get(i);
		if (pcKilled->IsEmbedded())
		{
			pcContainer = pcKilled->GetEmbeddingContainer();
			gcLogger.Error2(__METHOD__, " Object of class [", pcKilled->ClassName(), "] is marked for killing but is embedded in object with index [", IndexToString(pcContainer->GetIndex()),"] of class [", pcContainer->ClassName(), "].", NULL);
			return false;
		}
		else if (!pcKilled->IsAllocatedInObjects())
		{
			gcLogger.Error2(__METHOD__, " Object of class [", pcKilled->ClassName(), "] is marked for killing but is not allocated in Objects [0x", PointerToString(this),"].", NULL);
			return false;
		}
	}

	for (i = 0; i < uiNumElements; i++)
	{
		pcKilled = *papcKilled->Get(i);
		pcKilled->RemoveAllPointerTosDontKill();
	}

	KillDontFreeObjects(papcKilled);
	KillObjects(papcKilled);

	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPointer CObjects::Null(void)
{
	CPointer	pvObject;
	return pvObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CObjects::NumMemoryIndexes(void)
{
	return mcMemory.NumIndexed();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CObjects::NumMemoryNames(void)
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
int64 CObjects::NumDatabaseNames(void)
{
	SIndexTreeFileIterator	sIter;
	OIndex					oi;
	int64					iCount;

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
CBaseObject* CObjects::AllocateUninitialisedByClassName(const char* szClassName, OIndex oi)
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
		pcObject->InitIdentifiers("", oi);
	}
	return pcObject;
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
void CObjects::FreeObject(CBaseObject* pvObject)
{
	RemoveMemoryIdentifiers(pvObject);
	pvObject->FreeIdentifiers();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
OIndex CObjects::StartMemoryIteration(SIndexesIterator* psIter)
{
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
//
//////////////////////////////////////////////////////////////////////////
CHollowObject* CObjects::AllocateHollow(size uiNumEmbedded, OIndex oi)
{
	CHollowObject*	pcHollow;
	size			uiAdditionalBytes;
	void*			pvEmbedded;

	if (uiNumEmbedded == 0)
	{
		return NULL;
	}
	if (uiNumEmbedded == 1)
	{
		pcHollow = AllocateUninitialisedByTemplate<CHollowObject>(oi, 0);
		pcHollow->Init(1);
	}
	else
	{
		uiAdditionalBytes = sizeof(CHollowEmbeddedObject) * (uiNumEmbedded - 1);
		pcHollow = AllocateUninitialisedByTemplate<CHollowObject>(oi, uiAdditionalBytes);
		pcHollow->Init(uiNumEmbedded);

		pvEmbedded = RemapSinglePointer(pcHollow, sizeof(CHollowObject));
		AppenedHollowEmbeddedObjects(pcHollow, uiNumEmbedded, pvEmbedded);
	}
	return pcHollow;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CHollowObject* CObjects::AllocateHollow(size uiNumEmbedded, const char* szObjectName, OIndex oi)
{
	CHollowObject*	pcHollow;
	size			uiAdditionalBytes;
	void*			pvEmbedded;

	if (uiNumEmbedded == 0)
	{
		return NULL;
	}
	if (uiNumEmbedded == 1)
	{
		pcHollow = AllocateUninitialisedByTemplate<CHollowObject>(szObjectName, oi, 0);
		pcHollow->Init(1);
	}
	else
	{
		uiAdditionalBytes = sizeof(CHollowEmbeddedObject) * (uiNumEmbedded - 1);
		pcHollow = AllocateUninitialisedByTemplate<CHollowObject>(szObjectName, oi, uiAdditionalBytes);
		pcHollow->Init(uiNumEmbedded);

		pvEmbedded = RemapSinglePointer(pcHollow, sizeof(CHollowObject));
		AppenedHollowEmbeddedObjects(pcHollow, uiNumEmbedded, pvEmbedded);
	}
	return pcHollow;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjects::AppenedHollowEmbeddedObjects(CBaseObject* pcHollow, size uiNumEmbedded, void* pvEmbedded) 
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
CBaseObject* CObjects::AllocateUninitialisedByClassNameAndAddIntoMemory(char* szClassName)
{
	CBaseObject*	pvObject;
	OIndex			oi;
	bool			bResult;

	oi = GetNextIndex();
	pvObject = AllocateUninitialisedByClassName(szClassName, oi);
	if (!pvObject)
	{
		return NULL;
	}

	bResult = AddIntoMemoryWithIndex(pvObject);
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
CBaseObject* CObjects::AllocateNamedUninitialisedByClassNameAndAddIntoMemory(char* szClassName, char* szObjectName)
{
	bool			bResult;
	CBaseObject*	pvObject;
	OIndex			oi;

	if (Contains(szObjectName))
	{
		gcLogger.Error2(__METHOD__, " Cannot allocate object named [", szObjectName, "] class [", szClassName, "].  It already exists.", NULL);
		return NULL;
	}

	oi = GetNextIndex();
	pvObject = AllocateUninitialisedByClassName(szClassName, szObjectName, oi);
	if (!pvObject)
	{
		gcLogger.Error2(__METHOD__, " Cannot allocate object named [", szObjectName, "] class [", szClassName, "].", NULL);
		return NULL;
	}

	bResult = AddIntoMemoryWithNameAndIndex(pvObject);
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
CBaseObject* CObjects::GetNamedObjectInMemoryAndReplaceOrAllocateUninitialisedWithSameName(char* szClassName, char* szObjectName)
{
	CBaseObject*	pvOldObject;
	CBaseObject*	pvObject;
	OIndex			oi;
	bool			bResult;

	//Only called by the ExternalObjectDeserialiser.

	pvOldObject = GetFromMemory(szObjectName);
	if (!pvOldObject)
	{
		pvOldObject = GetFromDatabase(szObjectName);
	}
	oi = GetNextIndex();

	pvObject = AllocateUninitialisedByClassName(szClassName, szObjectName, oi);
	if (!pvObject)
	{
		gcLogger.Error2(__METHOD__, " Cannot allocate object named [", szObjectName, "] class [", szClassName, "].", NULL);
		return NULL;
	}

	if (pvOldObject)
	{
		bResult = ReplaceBaseObject(pvOldObject, pvObject);
		if (!bResult)
		{
			mpcUnknownsAllocatingFrom->RemoveInKill(pvObject);
			return NULL;
		}
	}
	else
	{
		bResult = AddIntoMemory(pvObject);
		if (!bResult)
		{
			mpcUnknownsAllocatingFrom->RemoveInKill(pvObject);
			return NULL;
		}
	}

	return pvObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CObjects::AllocateForInternalDeserialisationWithIndex(char* szClassName, OIndex oi)
{
	CBaseObject*	pvObject;
	bool			bResult;
	CHollowObject*	pcHollowObject;
	CBaseObject*	pcAllocatedObject;

	//Only called by the InternalObjectDeserialiser.

	pvObject = GetFromMemory(oi);
	if (pvObject)
	{
		if (pvObject->IsHollow())
		{
			pcHollowObject = (CHollowObject*)pvObject;
			pcAllocatedObject = AllocateUninitialisedByClassName(szClassName, oi);
			bResult = ReplaceBaseObject(pcHollowObject, pcAllocatedObject);
			if (!bResult)
			{
				return NULL;
			}
			else
			{
				return pcAllocatedObject;
			}
		}
		else
		{
			gcLogger.Error2(__METHOD__, " Cannot allocate object with index [", IndexToString(oi), "] class [", szClassName, "].  It already exists in memory (and is not hollow).", NULL);
			return NULL;
		}
	}
	else
	{
		pvObject = AllocateUninitialisedByClassName(szClassName, oi);
		if (!pvObject)
		{
			return NULL;
		}

		bResult = AddIntoMemoryWithIndex(pvObject);
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
CBaseObject* CObjects::AllocateForInternalDeserialisationWithNameAndIndex(char* szClassName, char* szObjectName, OIndex oi, OIndex* poiExisting)
{
	CBaseObject*	pvObject;
	bool			bResult;
	CHollowObject*	pcHollowObject;
	CBaseObject*	pcAllocatedObject;

	//Only called by the InternalObjectDeserialiser.

	pvObject = GetFromMemory(szObjectName, oi);
	if (pvObject)
	{
		if (pvObject->IsHollow())
		{
			pcHollowObject = (CHollowObject*)pvObject;
			if (pcHollowObject->GetIndex() != oi)
			{
				gcLogger.Error2(__METHOD__, " Cannot allocate object named [", szObjectName, "] class [", szClassName, "].  It already exists in memory with index [", LongToString(oi), "] but expected index [", LongToString(pcHollowObject->GetIndex()), "].", NULL);
				return NULL;
			}
			else
			{
				pcAllocatedObject = AllocateUninitialisedByClassName(szClassName, szObjectName, oi);
				bResult = ReplaceBaseObject(pcHollowObject, pcAllocatedObject);
				if (!bResult)
				{
					return NULL;
				}
				else
				{
					return pcAllocatedObject;
				}
			}
		}
		else
		{
			gcLogger.Error2(__METHOD__, " Cannot allocate object named [", szObjectName, "] class [", szClassName, "].  It already exists in memory (and is not hollow).", NULL);
			return NULL;
		}
	}
	else
	{
		pvObject = AllocateUninitialisedByClassName(szClassName, szObjectName, oi);
		if (!pvObject)
		{
			return NULL;
		}
	
		bResult = AddIntoMemoryWithNameAndIndex(pvObject);
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
bool CObjects::AddIntoMemory(CBaseObject* pvObject)
{
	if (!pvObject->IsNamed())
	{
		return AddIntoMemoryWithIndex(pvObject);
	}
	else
	{
		return AddIntoMemoryWithNameAndIndex(pvObject);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CObjects::ReplaceBaseObject(CBaseObject* pvExisting, CBaseObject* pvObject)
{
	CObjectRemapFrom	cRemapper;
	size				iCount;

	if (pvExisting && pvObject)
	{
		if (pvObject->HasHeapFroms())
		{
			return gcLogger.Error2(__METHOD__, " Cannot remap.  Object has head froms already.", NULL);
		}
		RemoveMemoryIdentifiers(pvExisting);

		iCount = cRemapper.Remap(pvExisting, pvObject);

		return AddIntoMemory(pvObject);
	}
	else
	{
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CHollowObject* CObjects::AllocateHollowWithIndex(OIndex oi, size uiNumEmbedded)
{
	CHollowObject*	pcHollow;
	bool			bResult;

	//Only called by the InternalObjectDeserialiser.

	if ((oi == INVALID_O_INDEX) || (oi == NULL_O_INDEX) || (oi == FIRST_O_INDEX))
	{
		gcLogger.Error2(__METHOD__, " Cannot allocate a hollow object with index [", IndexToString(oi), "].", NULL);
		return NULL;
	}

	pcHollow = AllocateHollow(uiNumEmbedded, oi);
	if (pcHollow)
	{
		LOG_OBJECT_ALLOCATION(pcHollow);
		mHollocationCallback(pcHollow);
	}
	else
	{
		LOG_OBJECT_ALLOCATION_FAILURE("CHollowObject", oi, "");
		return NULL;
	}

	bResult = AddIntoMemoryWithIndex(pcHollow);
	if (!bResult)
	{
		mpcUnknownsAllocatingFrom->RemoveInKill(pcHollow);
	}
	return pcHollow;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CHollowObject* CObjects::AllocateHollowWithNameAndIndex(char* szObjectName, OIndex oi, size uiNumEmbedded)
{
	CHollowObject*	pcHollow;
	bool			bResult;

	//Only called by the InternalObjectDeserialiser.

	if ((oi == INVALID_O_INDEX) || (oi == NULL_O_INDEX) || (oi == FIRST_O_INDEX))
	{
		gcLogger.Error2(__METHOD__, " Cannot allocate a hollow object with index [", IndexToString(oi), "].", NULL);
		return NULL;
	}

	if (StrEmpty(szObjectName))
	{
		gcLogger.Error2(__METHOD__, " Cannot allocate a named hollow object with no name.", NULL);
	}

	pcHollow = AllocateHollow(uiNumEmbedded, szObjectName, oi);
	if (pcHollow)
	{
		LOG_OBJECT_ALLOCATION(pcHollow);
	}
	else
	{
		LOG_OBJECT_ALLOCATION_FAILURE("CHollowObject", oi, "");
		return NULL;
	}

	bResult = AddIntoMemoryWithNameAndIndex(pcHollow);
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
CBaseObject* CObjects::GetNamedObjectInMemoryOrAllocateHollow(char* szObjectName, size uiNumEmbedded)
{
	CHollowObject*	pcHollow;
	bool			bResult;
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

	oi = GetNextIndex();
	pcHollow = AllocateHollow(uiNumEmbedded, szObjectName, oi);
	if (!pcHollow)
	{
		return NULL;
	}

	bResult = AddIntoMemoryWithNameAndIndex(pcHollow);
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
	gbObjects = true;
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
	gbObjects = true;
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

	gbObjects = false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool ObjectsValidate(void)
{
	if (!gbObjects)
	{
		gcLogger.Error("Objects have not been initialised.  Call ObjectsInit().");
		return false;
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
	return gcObjects.Root();
}

