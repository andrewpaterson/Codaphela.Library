#include "BaseLib/Logger.h"
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
	size					uiDataSize;
	CStackMemory<>			cTemp;
	bool					bExists;
	CBaseObject*			pcBaseObject;

	pcSerialised = (CSerialisedObject*)cTemp.Init();

	bExists = mpcDataConnection->Get(oi, &uiDataSize, pcSerialised, cTemp.GetStackSize());
	if (bExists)
	{
		if (uiDataSize <= cTemp.GetStackSize())
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
	size				uiDataSize;
	CStackMemory<>		cTemp;
	bool				bExists;
	CBaseObject*		pcBaseObject;

	pcSerialised = (CSerialisedObject*)cTemp.Init();

	bExists = mpcDataConnection->Get(szObjectName, &uiDataSize, pcSerialised, cTemp.GetStackSize());
	if (bExists)
	{
		if (uiDataSize <= cTemp.GetStackSize())
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
CBaseObject* CInternalObjectDeserialiser::ReadSerialised(CSerialisedObject* pcSerialised, size iSize)
{
	CObjectReader	cReader;
	CBaseObject*	pcBaseObject;
	bool			bResult;
	CMemoryFile		cMemoryFile;

	cMemoryFile.Init(pcSerialised, iSize);
	cMemoryFile.Open(EFM_Read);
	cReader.Init(&cMemoryFile, this);
	pcBaseObject = cReader.Read();

	if (!pcBaseObject)
	{
		cMemoryFile.Close();
		cReader.Kill();
		cMemoryFile.Kill();
		return NULL;
	}

	bResult = cReader.ReadHeapFroms(pcBaseObject);

	cMemoryFile.Close();
	cReader.Kill();
	cMemoryFile.Kill();

	if (!bResult)
	{
		return NULL;
	}

	return pcBaseObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CInternalObjectDeserialiser::AllocateForDeserialisation(CObjectHeader* pcHeader)
{
	OIndex	oiExisting;
	char*	szName;
	char*	szClassName;

	if (pcHeader->meType == OBJECT_POINTER_NULL)
	{
		return NULL;
	}
	else
	{
		szClassName = pcHeader->mszClassName.Text();
		if (pcHeader->meType == OBJECT_POINTER_ID)
		{
			return mpcObjects->AllocateForInternalDeserialisationWithIndex(szClassName, pcHeader->moi);
		}
		else if (pcHeader->meType == OBJECT_POINTER_NAMED)
		{
			szName = pcHeader->mszObjectName.Text();
			if (!StrEmpty(szName))
			{
				return mpcObjects->AllocateForInternalDeserialisationWithNameAndIndex(szClassName, szName, pcHeader->moi, &oiExisting);
			}
			else
			{
				return mpcObjects->AllocateForInternalDeserialisationWithIndex(szClassName, pcHeader->moi);
			}
		}
	}
	gcLogger.Error("Cant allocate object for unknown header type.");
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CInternalObjectDeserialiser::GetFromMemory(CObjectIdentifier* pcIdentifier)
{
	OIndex	oiNew;

	if (pcIdentifier->meType == OBJECT_POINTER_ID)
	{
		oiNew = pcIdentifier->moi;
		return mpcMemory->Get(oiNew);
	}
	else if (pcIdentifier->meType == OBJECT_POINTER_NAMED)
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
bool CInternalObjectDeserialiser::AddDependent(CObjectIdentifier* pcObjectIdentifier, CEmbeddedObject** ppcPtrToBeUpdated, CBaseObject* pcObjectContainingPtrToBeUpdated, uint16 iNumEmbedded, uint16 iEmbeddedIndex)
{
	CEmbeddedObject*	pcEmbeddedObject;
	OIndex				oiNew;
	CBaseObject*		pcHollowObject;
	CBaseObject*		pcExistingObject;
	bool				bIsNamed;
	char*				szName;

	if (!((pcObjectIdentifier->meType == OBJECT_POINTER_NAMED) || (pcObjectIdentifier->meType == OBJECT_POINTER_ID)))
	{
		if (pcObjectIdentifier->meType == OBJECT_POINTER_NULL)
		{
			return true;
		}
		else
		{
			return gcLogger.Error2(__METHOD__, " Cannot add depenent with unknown type [", IntToString(pcObjectIdentifier->meType, 16), "].", NULL);
		}

	}

	pcExistingObject = GetFromMemory(pcObjectIdentifier);

	if (pcExistingObject)
	{
		*ppcPtrToBeUpdated = pcExistingObject;
		AddHeapFrom(pcExistingObject, pcObjectContainingPtrToBeUpdated);
		return true;
	}

	oiNew = pcObjectIdentifier->moi;
	bIsNamed = pcObjectIdentifier->IsNamed();

	if (bIsNamed)
	{
		szName = pcObjectIdentifier->GetName();
		pcHollowObject = mpcObjects->AllocateInternalHollowWithNameAndIndex(szName, oiNew, iNumEmbedded, pcObjectIdentifier->GetFatSize());
	}
	else
	{
		pcHollowObject = mpcObjects->AllocateInternalHollowWithIndex(oiNew, iNumEmbedded, pcObjectIdentifier->GetFatSize());
	}

	if (!pcHollowObject)
	{
		return false;
	}

	pcEmbeddedObject = pcHollowObject->GetEmbeddedObject(iEmbeddedIndex);
	if (pcEmbeddedObject)
	{
		*ppcPtrToBeUpdated = pcEmbeddedObject;
		AddHeapFrom(pcEmbeddedObject, pcObjectContainingPtrToBeUpdated);
		return true;
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
bool CInternalObjectDeserialiser::AddReverseDependent(CObjectIdentifier* pcObjectIdentifier, CEmbeddedObject** ppcPtrToBeUpdated, CBaseObject* pcObjectContainingHeapFrom, uint16 iNumEmbedded, uint16 iEmbeddedIndex, int iDistToRoot)
{
	CDependentReadObject	cDependent;
	CPointer				pExisitingInDatabase;

	if (!((pcObjectIdentifier->meType == OBJECT_POINTER_NAMED) || (pcObjectIdentifier->meType == OBJECT_POINTER_ID)))
	{
		return (pcObjectIdentifier->meType == OBJECT_POINTER_NULL);
	}

	cDependent.Init(pcObjectIdentifier);

	cDependent.Kill();

	return true;
}

