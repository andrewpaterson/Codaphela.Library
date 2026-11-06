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
	CSerialisedObject*	pcSerialised;
	lsize				uiDataSize;
	CStackMemory<2 KB>	cTemp;
	CBaseObject*		pcBaseObject;

	pcSerialised = (CSerialisedObject*)cTemp.Init();
	if (!mpcDataConnection->Get(oi, &uiDataSize, pcSerialised, cTemp.GetStackSize()))
	{
		return NULL;
	}
	if (uiDataSize > cTemp.GetStackSize())
	{
		pcSerialised = (CSerialisedObject*)cTemp.Init(uiDataSize);
		mpcDataConnection->Get(oi, pcSerialised);
	}

	pcBaseObject = ReadSerialised(pcSerialised, uiDataSize);
	cTemp.Kill();
	return pcBaseObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CInternalObjectDeserialiser::Read(const char* szObjectName)
{
	OIndex	oi;

	oi = mpcDataConnection->GetIndex(szObjectName);
	if (oi == INVALID_O_INDEX)
	{
		return NULL;
	}
	return Read(oi);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CInternalObjectDeserialiser::ReadSerialised(CSerialisedObject* pcSerialised, lsize iSize)
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
		mpcObjects->ReplaceWithHollow(pcBaseObject);
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
	if (pcHeader->meType == OBJECT_POINTER_NULL)
	{
		return NULL;
	}
	return mpcObjects->AllocateForInternalDeserialisation(pcHeader->mszClassName.Text(), pcHeader->IsNamed() ? pcHeader->GetName() : "", pcHeader->moi);
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
		//The name is the identity.  If the named object has been replaced (and so has a new index) resolve to the replacement.
		return mpcMemory->Get(pcIdentifier->GetName());
	}
	else
	{
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The object already in memory (perhaps hollow), or else a hollow for it.
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CInternalObjectDeserialiser::GetFromMemoryOrAllocateHollow(CObjectIdentifier* pcIdentifier, uint16 iNumEmbedded)
{
	CBaseObject*	pcObject;

	pcObject = GetFromMemory(pcIdentifier);
	if (pcObject)
	{
		return pcObject;
	}
	return mpcObjects->AllocateInternalHollow(pcIdentifier->IsNamed() ? pcIdentifier->GetName() : "", pcIdentifier->moi, iNumEmbedded, pcIdentifier->GetFatSize());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CInternalObjectDeserialiser::AddDependent(CObjectIdentifier* pcObjectIdentifier, CEmbeddedObject** ppcPtrToBeUpdated, CBaseObject* pcObjectContainingPtrToBeUpdated, uint16 iNumEmbedded, uint16 iEmbeddedIndex)
{
	CBaseObject*	pcObject;

	pcObject = GetFromMemoryOrAllocateHollow(pcObjectIdentifier, iNumEmbedded);
	if (pcObject == NULL)
	{
		return false;
	}
	return PointTo(pcObject, iEmbeddedIndex, ppcPtrToBeUpdated, pcObjectContainingPtrToBeUpdated);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CInternalObjectDeserialiser::AddReverseDependent(CObjectIdentifier* pcObjectIdentifier, CEmbeddedObject** ppcPtrToBeUpdated, CBaseObject* pcObjectContainingHeapFrom, uint16 iNumEmbedded, uint16 iEmbeddedIndex)
{
	CBaseObject*	pcObject;

	pcObject = GetFromMemoryOrAllocateHollow(pcObjectIdentifier, iNumEmbedded);
	if (pcObject == NULL)
	{
		return false;
	}
	SafeAssign(ppcPtrToBeUpdated, pcObject);
	return true;
}

