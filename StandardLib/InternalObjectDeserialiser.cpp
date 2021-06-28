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
		return mpcObjects->AllocateForInternalDeserialisationWithIndex(pcHeader->mszClassName.Text(), pcHeader->moi);
	}
	else if (pcHeader->mcType == OBJECT_POINTER_NAMED)
	{
		szName = pcHeader->mszObjectName.Text();
		if (!StrEmpty(szName))
		{
			return mpcObjects->AllocateForInternalDeserialisationWithNameAndIndex(pcHeader->mszClassName.Text(), szName, pcHeader->moi, &oiExisting);
		}
		else
		{
			return mpcObjects->AllocateForInternalDeserialisationWithIndex(pcHeader->mszClassName.Text(), pcHeader->moi);
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
CBaseObject* CInternalObjectDeserialiser::GetFromMemory(CObjectIdentifier* pcIdentifier)
{
	OIndex	oiNew;

	if (pcIdentifier->mcType == OBJECT_POINTER_ID)
	{
		oiNew = pcIdentifier->moi;
		return mpcMemory->Get(oiNew);
	}
	else if (pcIdentifier->mcType == OBJECT_POINTER_NAMED)
	{
		oiNew = pcIdentifier->moi;
		return mpcMemory->Get(pcIdentifier->GetName(), oiNew);
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
BOOL CInternalObjectDeserialiser::AddDependent(CObjectIdentifier* pcObjectPointerToIdentifier, CEmbeddedObject** ppcPtrToBeUpdated, CBaseObject* pcObjectContainingPtrToBeUpdated, uint16 iNumEmbedded, uint16 iEmbeddedIndex)
{
	CEmbeddedObject*		pcEmbeddedObject;
	OIndex					oiNew;
	CBaseObject*			pcHollowObject;
	CBaseObject*			pcExistingObject;
	BOOL					bIsNamed;

	if (!((pcObjectPointerToIdentifier->mcType == OBJECT_POINTER_NAMED) || (pcObjectPointerToIdentifier->mcType == OBJECT_POINTER_ID)))
	{
		return (pcObjectPointerToIdentifier->mcType == OBJECT_POINTER_NULL);
	}

	pcExistingObject = GetFromMemory(pcObjectPointerToIdentifier);

	if (pcExistingObject)
	{
		AddHeapFrom(pcExistingObject, pcObjectContainingPtrToBeUpdated);
		return TRUE;
	}


	oiNew = pcObjectPointerToIdentifier->moi;
	bIsNamed = pcObjectPointerToIdentifier->IsNamed();

	if (!bIsNamed)
	{
		pcHollowObject = mpcObjects->AllocateHollowWithIndex(oiNew, iNumEmbedded);
	}
	else
	{
		pcHollowObject = mpcObjects->AllocateHollowWithNameAndIndex(pcObjectPointerToIdentifier->GetName(), oiNew, iNumEmbedded);
	}

	if (!pcHollowObject)
	{
		return FALSE;
	}

	pcEmbeddedObject = pcHollowObject->GetEmbeddedObject(iEmbeddedIndex);
	if (pcEmbeddedObject)
	{
		*ppcPtrToBeUpdated = pcEmbeddedObject;
		AddHeapFrom(pcEmbeddedObject, pcObjectContainingPtrToBeUpdated);
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CInternalObjectDeserialiser::AddReverseDependent(CObjectIdentifier* pcHeader, CEmbeddedObject** ppcPtrToBeUpdated, CBaseObject* pcObjectContainingHeapFrom, uint16 iNumEmbedded, uint16 iEmbeddedIndex, int iDistToRoot)
{
	CDependentReadObject	cDependent;
	CPointer				pExisitingInDatabase;

	if (!((pcHeader->mcType == OBJECT_POINTER_NAMED) || (pcHeader->mcType == OBJECT_POINTER_ID)))
	{
		return (pcHeader->mcType == OBJECT_POINTER_NULL);
	}

	cDependent.Init(pcHeader);

	cDependent.Kill();

	return TRUE;
}

