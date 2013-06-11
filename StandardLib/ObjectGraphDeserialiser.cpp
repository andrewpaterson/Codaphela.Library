#include "BaseLib/Logger.h"
#include "BaseLib/ErrorTypes.h"
#include "Objects.h"
#include "ObjectDeserialiser.h"
#include "ObjectGraphDeserialiser.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObjectGraphDeserialiser::Init(CObjectReader* pcReader, BOOL bNamedHollows, CIndexGenerator* pcIndexGenerator, CObjectAllocator* pcAllocator, CDependentReadObjects* pcDependentReadObjects, CNamedIndexedObjects* pcMemory)
{
	CDependentObjectAdder::Init(pcDependentReadObjects);
	mpcReader = pcReader;
	mpcAllocator = pcAllocator;
	mpcMemory = pcMemory;
	mbNamedHollows = bNamedHollows;
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
	if (pObject.IsNull())
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
			if (mbNamedHollows)
			{
				if (pcDependent->IsIndexed())
				{
					bResult = ReadUnread(pcDependent);
					if (!bResult)
					{
						return FALSE;
					}
				}
				else
				{
					MarkRead(pcDependent->GetOldIndex());
				}
			}
			else
			{
				bResult = ReadUnread(pcDependent);
				if (!bResult)
				{
					return FALSE;
				}
			}
		}
		else
		{
			break;
		}
	}

	bResult = AddContainingPointersAndCreateHollowObjects();
	return bResult;
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
BOOL CObjectGraphDeserialiser::AddContainingPointersAndCreateHollowObjects(void)
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
BOOL CObjectGraphDeserialiser::AddContainingPointersAndCreateHollowObject(CDependentReadPointer* pcDependentReadPointer)
{
	OIndex					oiNew;
	CBaseObject*			pcBaseObject;
	CDependentReadObject*	pcDependentReadObject;
	CPointerObject			pObject;

	oiNew = GetNewIndexFromOld(pcDependentReadPointer->moiPointedTo);
	pcBaseObject = mpcMemory->Get(oiNew);

	if (!pcBaseObject)
	{
		pcDependentReadObject = mpcDependentObjects->GetObject(pcDependentReadPointer->moiPointedTo);
		if (pcDependentReadObject->mcType == OBJECT_POINTER_NAMED)
		{
			pObject = mpcAllocator->AddHollow(pcDependentReadObject->mszObjectName.Text());
			pcBaseObject = pObject.Object();
		}
		else if (pcDependentReadObject->mcType == OBJECT_POINTER_ID)
		{
			return FALSE;
		}
	}

	AddContainingPointer(pcBaseObject, pcDependentReadPointer->mppcPointedFrom, pcDependentReadPointer->mpcContaining);
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

