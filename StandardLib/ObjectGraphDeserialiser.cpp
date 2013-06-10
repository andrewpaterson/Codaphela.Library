#include "BaseLib/Logger.h"
#include "BaseLib/ErrorTypes.h"
#include "Objects.h"
#include "ObjectDeserialiser.h"
#include "ObjectGraphDeserialiser.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectGraphDeserialiser::Init(CObjectReader* pcReader, CIndexGenerator* pcIndexGenerator, CObjectAllocator* pcAllocator, CDependentReadObjects* pcDependentReadObjects, CNamedIndexedObjects* pcMemory)
{
	CDependentObjectAdder::Init(pcDependentReadObjects);
	mpcReader = pcReader;
	mpcAllocator = pcAllocator;
	mpcMemory = pcMemory;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectGraphDeserialiser::Kill(void)
{
	mpcMemory = NULL;
	mpcAllocator = NULL;
	mpcDependentObjects = NULL;
	CDependentObjectAdder::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPointerObject CObjectGraphDeserialiser::Read(char* szObjectName)
{
	BOOL							bResult;
	CPointerObject					pObject;
	CSerialisedObject*			pcSerialised;
	
	bResult = mpcReader->Begin();
	if (!bResult)
	{
		return ONull;
	}

	pcSerialised = mpcReader->Read(szObjectName);
	pObject = ReadSerialsed(pcSerialised);
	if (!bResult)
	{
		return ONull;
	}

	bResult = ReadDependentObjects();
	if (!bResult)
	{
		return ONull;
	}

	bResult = mpcReader->End();
	if (!bResult)
	{
		return ONull;
	}

	return pObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjectGraphDeserialiser::ReadDependentObjects(void)
{
	BOOL							bResult;
	CPointerObject					pObject;
	CDependentReadObject*			pcDependent;

	for (;;)
	{
		pcDependent = mpcDependentObjects->GetUnread();
		if (pcDependent)
		{
			bResult = ReadUnread(pcDependent);
			if (!bResult)
			{
				return FALSE;
			}
		}
		else
		{
			break;
		}
	}

	bResult = UpdateDependentPointersAndCreateHollowObjects();
	if (!bResult)
	{
		return FALSE;
	}

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjectGraphDeserialiser::ReadUnread(CDependentReadObject* pcDependent)
{
	CSerialisedObject*			pcSerialised;
	char*						szObjectName;
	CObjectDeserialiser			cDeserialiser;
	CPointerObject				pObject;

	pcSerialised = NULL;
	if (pcDependent->IsNamed())
	{
		szObjectName = pcDependent->GetName();
		pcSerialised = mpcReader->Read(szObjectName);
		if (!pcSerialised)
		{
			return FALSE;
		}
	}
	else
	{
		pcSerialised = mpcReader->Read(pcDependent->GetOldIndex());
		if (!pcSerialised)
		{
			return FALSE;
		}
	}

	if (pcDependent->GetOldIndex() != pcSerialised->GetIndex())
	{
		gcLogger.Error2("pcDependent->GetOldIndex [", IndexToString(pcDependent->GetOldIndex()), "] != pcSerialised->GetIndex [", IndexToString(pcSerialised->GetIndex()), "]", NULL);
		return FALSE;
	}

	pObject = ReadSerialsed(pcSerialised);
	return pObject.IsNotNull();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPointerObject CObjectGraphDeserialiser::ReadSerialsed(CSerialisedObject* pcSerialised)
{
	CObjectDeserialiser			cDeserialiser;
	CPointerObject				pObject;
	OIndex						oiNew;
	OIndex						oiOld;

	oiOld = pcSerialised->GetIndex();
	cDeserialiser.Init(this);
	pObject = cDeserialiser.Load(pcSerialised);

	cDeserialiser.Kill();
	free(pcSerialised);

	if (pObject.IsNotNull())
	{
		MarkRead(oiOld);

		oiNew = pObject.GetIndex();
		mpcDependentObjects->AddIndexRemap(oiNew, oiOld);
	}

	return pObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectGraphDeserialiser::MarkRead(OIndex oi)
{
	mpcDependentObjects->Mark(oi);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObjectGraphDeserialiser::UpdateDependentPointersAndCreateHollowObjects(void)
{
	CDependentReadPointer*	pcReadPointer;
	int						i;
	int						iNum;
	CBaseObject*			pcBaseObject;
	OIndex					oiNew;

	iNum = mpcDependentObjects->NumPointers();
	for (i = 0; i < iNum; i++)
	{
		pcReadPointer = mpcDependentObjects->GetPointer(i);
		oiNew = GetNewIndexFromOld(pcReadPointer->moiPointedTo);

		pcBaseObject = mpcMemory->Get(oiNew);
		if (pcBaseObject)
		{
			AddContainingPointer(pcBaseObject, pcReadPointer->mppcPointedFrom, pcReadPointer->mpcContaining);
		}
		else
		{
			gcLogger.Error2("Could not get object with index [", IndexToString(oiNew), "] from memory trying to update containing pointer.", NULL);
			return FALSE;
		}
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
OIndex CObjectGraphDeserialiser::GetNewIndexFromOld(OIndex oiOld)
{
	return mpcDependentObjects->GetNewIndexFromOld(oiOld);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPointerObject CObjectGraphDeserialiser::AllocateObject(CObjectHeader* pcHeader)
{
	if (pcHeader->mcType == OBJECT_POINTER_NULL)
	{
		return ONull;
	}
	else if (pcHeader->mcType == OBJECT_POINTER_ID)
	{
		return mpcAllocator->Add(pcHeader->mszClassName.Text());
	}
	else if (pcHeader->mcType == OBJECT_POINTER_NAMED)
	{
		return mpcAllocator->Add(pcHeader->mszClassName.Text(), pcHeader->mszObjectName.Text());
	}
	else
	{
		gcLogger.Error("Cant allocate object for unknown header type.");
		return ONull;
	}
}

