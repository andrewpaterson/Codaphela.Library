#include "BaseLib/Logger.h"
#include "BaseLib/ErrorTypes.h"
#include "SerialisedObject.h"
#include "ObjectDeserialiser.h"
#include "Objects.h"
#include "HollowObject.h"
#include "IndexedDataObjectDeserialiser.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedDataObjectDeserialiser::Init(CObjectAllocator* pcAllocator, CNamedIndexedData* pcDatabase, CNamedIndexedObjects* pcMemory)
{
	CDependentObjectAdder::Init(&mcDependentObjects);
	mpcAllocator = pcAllocator;
	mpcDatabase = pcDatabase;
	mpcMemory = pcMemory;

	mcDependentObjects.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedDataObjectDeserialiser::Kill(void)
{
	mpcAllocator = NULL;
	mcDependentObjects.Kill();
	CDependentObjectAdder::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPointer CIndexedDataObjectDeserialiser::Read(OIndex oi)
{
	CSerialisedObject*		pcSerialised;

	pcSerialised = (CSerialisedObject*)mpcDatabase->Get(oi);
	return ReadSerialised(pcSerialised);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPointer CIndexedDataObjectDeserialiser::Read(char* szObjectName)
{
	CSerialisedObject*		pcSerialised;

	pcSerialised = (CSerialisedObject*)mpcDatabase->Get(szObjectName);
	if (pcSerialised)
	{
		return ReadSerialised(pcSerialised);
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
CPointer CIndexedDataObjectDeserialiser::ReadSerialised(CSerialisedObject* pcSerialised)
{
	CObjectDeserialiser		cDeserialiser;
	CPointer			pObject;

	cDeserialiser.Init(this);
	pObject = cDeserialiser.Load(pcSerialised);

	cDeserialiser.Kill();
	free(pcSerialised);

	if (pObject.IsNotNull())
	{
		AddContainingPointersAndCreateHollowObjects();
	}

	return pObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedDataObjectDeserialiser::AddContainingPointersAndCreateHollowObjects(void)
{
	CDependentReadPointer*	pcDependentReadPointer;
	int						i;
	int						iNum;

	iNum = mpcDependentObjects->NumPointers();
	for (i = 0; i < iNum; i++)
	{
		pcDependentReadPointer = mpcDependentObjects->GetPointer(i);
		if (!AddContainingPointersAndCreateHollowObject(pcDependentReadPointer))
		{
			return FALSE;
		}
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedDataObjectDeserialiser::AddContainingPointersAndCreateHollowObject(CDependentReadPointer* pcDependentReadPointer)
{
	CEmbeddedObject*		pcBaseObject;
	CDependentReadObject*	pcDependentReadObject;
	CPointer				pHollow;
	OIndex					oiNew;
	CHollowObject*			pcHollowObject;

	oiNew = pcDependentReadPointer->moiPointedTo;
	pcBaseObject = mpcMemory->Get(oiNew);
	if (pcBaseObject)
	{
		AddContainingPointer(pcBaseObject, pcDependentReadPointer->mppcPointedFrom, pcDependentReadPointer->mpcContaining);
		return TRUE;
	}
	else
	{
		pcDependentReadObject = mpcDependentObjects->GetObject(pcDependentReadPointer->moiPointedTo);
		if (pcDependentReadObject->mcType == OBJECT_POINTER_ID)
		{
			pHollow = mpcAllocator->AddHollow(pcDependentReadObject->moi, pcDependentReadPointer->miNumEmbedded);
			pcHollowObject = (CHollowObject*)pHollow.BaseObject();
		}
		else if (pcDependentReadObject->mcType == OBJECT_POINTER_NAMED)
		{
			pHollow = mpcAllocator->AddHollow(pcDependentReadObject->mszObjectName.Text(), pcDependentReadObject->moi, pcDependentReadPointer->miNumEmbedded);
			pcHollowObject = (CHollowObject*)pHollow.BaseObject();
		}

		pcBaseObject = pcHollowObject->GetEmbeddedObject(pcDependentReadPointer->miEmbeddedIndex);
		AddContainingPointer(pcBaseObject, pcDependentReadPointer->mppcPointedFrom, pcDependentReadPointer->mpcContaining);
		return TRUE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPointer CIndexedDataObjectDeserialiser::AllocateObject(CObjectHeader* pcHeader)
{
	if (pcHeader->mcType == OBJECT_POINTER_NULL)
	{
		return ONull;
	}
	else if (pcHeader->mcType == OBJECT_POINTER_ID)
	{
		return mpcAllocator->Add(pcHeader->mszClassName.Text(), pcHeader->moi);
	}
	else if (pcHeader->mcType == OBJECT_POINTER_NAMED)
	{
		return mpcAllocator->Add(pcHeader->mszClassName.Text(), pcHeader->mszObjectName.Text(), pcHeader->moi);
	}
	else
	{
		gcLogger.Error("Cant allocate object for unknown header type.");
		return ONull;
	}
}

