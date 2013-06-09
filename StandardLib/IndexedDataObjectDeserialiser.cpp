#include "BaseLib/Logger.h"
#include "BaseLib/ErrorTypes.h"
#include "SerialisedObject.h"
#include "ObjectDeserialiser.h"
#include "Objects.h"
#include "IndexedDataObjectDeserialiser.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedDataObjectDeserialiser::Init(CObjectAllocator* pcAllocator, CNamedIndexedData* pcDatabase, CNamedIndexedObjects* pcMemory)
{
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
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPointerObject CIndexedDataObjectDeserialiser::Read(CSerialisedObject* pcSerialised)
{
	CObjectDeserialiser		cDeserialiser;
	CPointerObject			pObject;
	CBaseObject*			pcReadObject;

	cDeserialiser.Init(this);
	pObject = cDeserialiser.Load(pcSerialised);
	if (pObject.IsNull())
	{
		cDeserialiser.Kill();
		free(pcSerialised);
		return ONull;
	}

	cDeserialiser.Kill();
	free(pcSerialised);

	pcReadObject = pObject.Object();
	UpdateDependentPointersAndCreateHollowObjects(pcReadObject);

	return pObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPointerObject CIndexedDataObjectDeserialiser::Read(OIndex oi)
{
	CSerialisedObject*		pcSerialised;

	pcSerialised = (CSerialisedObject*)mpcDatabase->Get(oi);
	return Read(pcSerialised);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPointerObject CIndexedDataObjectDeserialiser::Read(char* szObjectName)
{
	CSerialisedObject*		pcSerialised;

	pcSerialised = (CSerialisedObject*)mpcDatabase->Get(szObjectName);
	return Read(pcSerialised);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedDataObjectDeserialiser::UpdateDependentPointersAndCreateHollowObjects(CBaseObject* pcReadObject)
{
	CDependentReadPointer*	pcDependentReadPointer;
	int						i;

	for (i = 0; i < mcDependentObjects.NumPointers(); i++)
	{
		pcDependentReadPointer = mcDependentObjects.GetPointer(i);
		FixPointerOrCreateHollowObject(pcReadObject, pcDependentReadPointer);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedDataObjectDeserialiser::FixPointerOrCreateHollowObject(CBaseObject* pcReadObject, CDependentReadPointer* pcDependentReadPointer)
{
	CBaseObject*			pcObject;
	CDependentReadObject*	pcDependentReadObject;
	CPointerObject			pObject;

	pcObject = mpcMemory->Get(pcDependentReadPointer->moiPointedTo);

	if (!pcObject)
	{
		pcDependentReadObject = mcDependentObjects.GetObject(pcDependentReadPointer->moiPointedTo);
		if (pcDependentReadObject->mcType == OBJECT_POINTER_ID)
		{
			pObject = mpcAllocator->AddHollow(pcDependentReadObject->moi);
			pcObject = pObject.Object();
		}
		else if (pcDependentReadObject->mcType == OBJECT_POINTER_NAMED)
		{
			pObject = mpcAllocator->AddHollow(pcDependentReadObject->mszObjectName.Text(), pcDependentReadObject->moi);
			pcObject = pObject.Object();
		}
	}
	FixPointer(pcObject, pcDependentReadPointer->mppcPointedFrom, pcDependentReadPointer->mpcContaining);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedDataObjectDeserialiser::AddDependent(CPointerHeader* pcHeader, CBaseObject** ppcPtrToBeUpdated, CBaseObject* pcObjectContainingPtrToBeUpdated)
{
	if ((pcHeader->mcType == OBJECT_POINTER_NAMED) || (pcHeader->mcType == OBJECT_POINTER_ID))
	{
		mcDependentObjects.Add(pcHeader, ppcPtrToBeUpdated, pcObjectContainingPtrToBeUpdated);
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPointerObject CIndexedDataObjectDeserialiser::AllocateObject(CObjectHeader* pcHeader)
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

