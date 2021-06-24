#include "BaseLib/Log.h"
#include "BaseLib/ErrorTypes.h"
#include "BaseLib/StackMemory.h"
#include "SerialisedObject.h"
#include "ObjectReader.h"
#include "Objects.h"
#include "HollowObject.h"
#include "InternalObjectDeserialiser.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInternalObjectDeserialiser::Init(CObjects* pcObjects, CDataConnection* pcDataConnection, CNamedIndexedObjects* pcMemory)
{
	CDependentReadObjects::Init();
	mpcObjects = pcObjects;
	mpcDataConnection = pcDataConnection;
	mpcMemory = pcMemory;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInternalObjectDeserialiser::Kill(void)
{
	mpcDataConnection = NULL;
	mpcMemory = NULL;
	mpcObjects = NULL;
	CDependentReadObjects::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CInternalObjectDeserialiser::Read(OIndex oi)
{
	CSerialisedObject*		pcSerialised;
	unsigned int			uiDataSize;
	CStackMemory<>			cTemp;
	BOOL					bExists;
	CBaseObject*			pcBaseObject;

	pcSerialised = (CSerialisedObject*)cTemp.Init();

	bExists = mpcDataConnection->Get(oi, &uiDataSize, pcSerialised, cTemp.GetStackSize());
	if (bExists)
	{
		if (uiDataSize <= (unsigned int)cTemp.GetStackSize())
		{
			return ReadSerialised(pcSerialised, uiDataSize);
		}
		else
		{
			pcSerialised = (CSerialisedObject*)cTemp.Init(uiDataSize);
			mpcDataConnection->Get(oi, pcSerialised);
			pcBaseObject = ReadSerialised(pcSerialised, uiDataSize);
			cTemp.Kill();
			return pcBaseObject;
		}
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
CBaseObject* CInternalObjectDeserialiser::Read(char* szObjectName)
{
	CSerialisedObject*	pcSerialised;
	unsigned int		uiDataSize;
	CStackMemory<>		cTemp;
	BOOL				bExists;
	CBaseObject*		pcBaseObject;

	pcSerialised = (CSerialisedObject*)cTemp.Init();

	bExists = mpcDataConnection->Get(szObjectName, &uiDataSize, pcSerialised, cTemp.GetStackSize());
	if (bExists)
	{
		if (uiDataSize <= (unsigned int)cTemp.GetStackSize())
		{
			return ReadSerialised(pcSerialised, uiDataSize);
		}
		else
		{
			pcSerialised = (CSerialisedObject*)cTemp.Init(uiDataSize);
			mpcDataConnection->Get(szObjectName, pcSerialised);
			pcBaseObject = ReadSerialised(pcSerialised, uiDataSize);
			cTemp.Kill();
			return pcBaseObject;
		}
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
CBaseObject* CInternalObjectDeserialiser::ReadSerialised(CSerialisedObject* pcSerialised, unsigned int iSize)
{
	CObjectReader	cReader;
	CBaseObject*	pvObject;
	BOOL			bResult;
	CMemoryFile		cMemoryFile;

	cMemoryFile.Init(pcSerialised, iSize);
	cMemoryFile.Open(EFM_Read);
	cReader.Init(&cMemoryFile, this);
	pvObject = cReader.Read();

	if (!pvObject)
	{
		cMemoryFile.Close();
		cReader.Kill();
		cMemoryFile.Kill();
		return NULL;
	}

	bResult = cReader.ReadHeapFroms(pvObject);

	cMemoryFile.Close();
	cReader.Kill();
	cMemoryFile.Kill();

	if (!bResult)
	{
		return NULL;
	}

	return pvObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CInternalObjectDeserialiser::AddHeapFromPointersAndCreateHollowObject(CDependentReadPointer* pcDependentReadPointer, CDependentReadObject* pcDependent)
{
	CEmbeddedObject*		pcEmbeddedObject;
	OIndex					oiNew;
	CBaseObject*			pcHollowObject;
	CBaseObject*			pcExistingObject;

	oiNew = pcDependentReadPointer->moiPointedTo;
	pcExistingObject = mpcMemory->Get(oiNew);
	if (pcExistingObject)
	{
		AddHeapFrom(pcExistingObject, pcDependentReadPointer->mppcPointedFrom, pcDependentReadPointer->mpcContaining);
		return TRUE;
	}
	else
	{
		if (pcDependent->mcType == OBJECT_POINTER_ID)
		{
			pcHollowObject = mpcObjects->GetObjectInMemoryOrAllocateHollowForceIndex(pcDependent->moi, pcDependentReadPointer->miNumEmbedded);
		}
		else if (pcDependent->mcType == OBJECT_POINTER_NAMED)
		{
			pcHollowObject = mpcObjects->GetNamedObjectInMemoryOrAllocateHollowForceIndex(pcDependent->mszObjectName.Text(), pcDependent->moi, pcDependentReadPointer->miNumEmbedded);
		}
		else
		{
			pcHollowObject = NULL;
		}

		pcEmbeddedObject = pcHollowObject->GetEmbeddedObject(pcDependentReadPointer->miEmbeddedIndex);
		if (pcEmbeddedObject)
		{
			AddHeapFrom(pcEmbeddedObject, pcDependentReadPointer->mppcPointedFrom, pcDependentReadPointer->mpcContaining);
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CInternalObjectDeserialiser::AllocateForDeserialisation(CObjectHeader* pcHeader)
{
	OIndex	oiExisting;
	char*	szName;

	if (pcHeader->mcType == OBJECT_POINTER_NULL)
	{
		return NULL;
	}
	else if (pcHeader->mcType == OBJECT_POINTER_ID)
	{
		return mpcObjects->GetObjectInMemoryAndReplaceOrAllocateUnitialised(pcHeader->mszClassName.Text(), pcHeader->moi);
	}
	else if (pcHeader->mcType == OBJECT_POINTER_NAMED)
	{
		szName = pcHeader->mszObjectName.Text();
		if (!StrEmpty(szName))
		{
			return mpcObjects->AllocateForExistingInDatabaseWithExplicitIdentifiers(pcHeader->mszClassName.Text(), szName, pcHeader->moi, &oiExisting);
		}
		else
		{
			return mpcObjects->GetObjectInMemoryAndReplaceOrAllocateUnitialised(pcHeader->mszClassName.Text(), pcHeader->moi);
		}
	}
	else
	{
		gcLogger.Error("Cant allocate object for unknown header type.");
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CInternalObjectDeserialiser::AddDependent(CObjectIdentifier* pcHeader, CEmbeddedObject** ppcPtrToBeUpdated, CBaseObject* pcObjectContainingPtrToBeUpdated, uint16 iNumEmbedded, uint16 iEmbeddedIndex)
{
	CDependentReadObject	cDependent;
	CDependentReadPointer	cPointer;

	if (!((pcHeader->mcType == OBJECT_POINTER_NAMED) || (pcHeader->mcType == OBJECT_POINTER_ID)))
	{
		return TRUE;
	}

	cDependent.Init(pcHeader);
	cPointer.Init(ppcPtrToBeUpdated, pcObjectContainingPtrToBeUpdated, pcHeader->moi, iNumEmbedded, iEmbeddedIndex);

	if (!AddHeapFromPointersAndCreateHollowObject(&cPointer, &cDependent))
	{
		cDependent.Kill();
		return FALSE;
	}
	cDependent.Kill();
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CInternalObjectDeserialiser::AddReverseDependent(CObjectIdentifier* pcHeader, CEmbeddedObject** ppcPtrToBeUpdated, CBaseObject* pcObjectContainingHeapFrom, uint16 iNumEmbedded, uint16 iEmbeddedIndex)
{
	CDependentReadObject	cDependent;
	CPointer				pExisitingInDatabase;

	if (!((pcHeader->mcType == OBJECT_POINTER_NAMED) || (pcHeader->mcType == OBJECT_POINTER_ID)))
	{
		return TRUE;
	}

	cDependent.Init(pcHeader);

	CDependentReadObject*	pcDependent;
	CBaseObject*			pcHollowObject;
	CEmbeddedObject*		pcEmbeddedObject;

	pcDependent = &cDependent;
	if (pcDependent->mcType == OBJECT_POINTER_ID)
	{
		pcHollowObject = mpcObjects->GetObjectInMemoryOrAllocateHollowForceIndex(pcDependent->moi, iNumEmbedded);
	}
	else if (pcDependent->mcType == OBJECT_POINTER_NAMED)
	{
		pcHollowObject = mpcObjects->GetNamedObjectInMemoryOrAllocateHollowForceIndex(pcDependent->mszObjectName.Text(), pcDependent->moi, iNumEmbedded);
	}
	else
	{
		pcHollowObject = NULL;
	}

	pcEmbeddedObject = pcHollowObject->GetEmbeddedObject(iEmbeddedIndex);
	//Something, something.  You must set the heap from on the (misnamed?) pcObjectContainingHeapFrom to be the pcEmbeddedObject hollow object.

	cDependent.Kill();

	return TRUE;
}

