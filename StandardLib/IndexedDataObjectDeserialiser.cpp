#include "BaseLib/Log.h"
#include "BaseLib/ErrorTypes.h"
#include "BaseLib/StackMemory.h"
#include "SerialisedObject.h"
#include "ObjectDeserialiser.h"
#include "Objects.h"
#include "HollowObject.h"
#include "IndexedDataObjectDeserialiser.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedDataObjectDeserialiser::Init(CObjects* pcObjects, CDataConnection* pcDataConnection, CNamedIndexedObjects* pcMemory)
{
	CDependentObjectAdder::Init(&mcDependentObjects);
	mpcObjects = pcObjects;
	mpcDataConnection = pcDataConnection;
	mpcMemory = pcMemory;

	mcDependentObjects.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedDataObjectDeserialiser::Kill(void)
{
	mpcDataConnection = NULL;
	mpcMemory = NULL;
	mpcObjects = NULL;
	mcDependentObjects.Kill();
	CDependentObjectAdder::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CIndexedDataObjectDeserialiser::Read(OIndex oi)
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
			return ReadSerialised(pcSerialised);
		}
		else
		{
			pcSerialised = (CSerialisedObject*)cTemp.Init(uiDataSize);
			mpcDataConnection->Get(oi, pcSerialised);
			pcBaseObject = ReadSerialised(pcSerialised);
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
CBaseObject* CIndexedDataObjectDeserialiser::Read(char* szObjectName)
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
			return ReadSerialised(pcSerialised);
		}
		else
		{
			pcSerialised = (CSerialisedObject*)cTemp.Init(uiDataSize);
			mpcDataConnection->Get(szObjectName, pcSerialised);
			pcBaseObject = ReadSerialised(pcSerialised);
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
CBaseObject* CIndexedDataObjectDeserialiser::ReadSerialised(CSerialisedObject* pcSerialised)
{
	CObjectDeserialiser		cDeserialiser;
	CBaseObject*			pvObject;

	cDeserialiser.Init(this);
	pvObject = cDeserialiser.Load(pcSerialised);

	cDeserialiser.Kill();

	if (pvObject)
	{
		AddContainingPointersAndCreateHollowObjects();
	}

	return pvObject;
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
	OIndex					oiNew;
	CBaseObject*			pcHollowObject;

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
			pcHollowObject = mpcObjects->AllocateExistingHollow(pcDependentReadObject->moi, pcDependentReadPointer->miNumEmbedded);
		}
		else if (pcDependentReadObject->mcType == OBJECT_POINTER_NAMED)
		{
			pcHollowObject = mpcObjects->AllocateExistingHollowFromMemoryOrMaybeANewNamedHollow(pcDependentReadObject->mszObjectName.Text(), pcDependentReadObject->moi, pcDependentReadPointer->miNumEmbedded);
		}

		pcBaseObject = pcHollowObject->GetEmbeddedObject(pcDependentReadPointer->miEmbeddedIndex);
		if (pcBaseObject)
		{
			AddContainingPointer(pcBaseObject, pcDependentReadPointer->mppcPointedFrom, pcDependentReadPointer->mpcContaining);
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
CBaseObject* CIndexedDataObjectDeserialiser::AllocateObject(CObjectHeader* pcHeader)
{
	if (pcHeader->mcType == OBJECT_POINTER_NULL)
	{
		return NULL;
	}
	else if (pcHeader->mcType == OBJECT_POINTER_ID)
	{
		return mpcObjects->AllocateNewMaybeReplaceExisting(pcHeader->mszClassName.Text(), pcHeader->moi);
	}
	else if (pcHeader->mcType == OBJECT_POINTER_NAMED)
	{
		return mpcObjects->AllocateNewMaybeReplaceExisting(pcHeader->mszClassName.Text(), pcHeader->mszObjectName.Text(), pcHeader->moi);
	}
	else
	{
		gcLogger.Error("Cant allocate object for unknown header type.");
		return NULL;
	}
}

